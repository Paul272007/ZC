#include "clang_utils.h"

#include <algorithm>
#include <clang-c/Index.h>

#include "excepts/ZCException.h"
#include "helpers.h"

ZC_DEV_CONFIG

namespace
{

struct VisitorContext
{
  zc::Declarations *decls;
  const string     *content;

  vector<pair<unsigned, unsigned>> typedef_ranges;
};

void rtrim(string &s)
{
  while (!s.empty() && isspace(s.back()))
    s.pop_back();
}

pair<unsigned, unsigned> get_cursor_offsets(const CXCursor &cursor)
{
  const CXSourceRange    range = clang_getCursorExtent(cursor);
  const CXSourceLocation start = clang_getRangeStart(range);
  const CXSourceLocation end   = clang_getRangeEnd(range);

  unsigned start_offset, end_offset;
  clang_getInstantiationLocation(start, nullptr, nullptr, nullptr, &start_offset);
  clang_getInstantiationLocation(end, nullptr, nullptr, nullptr, &end_offset);

  return { start_offset, end_offset };
}

string get_cursor_text(const CXCursor &cursor, const string &content)
{
  auto [start, end] = get_cursor_offsets(cursor);
  if (end <= start || end > content.length())
    return "";
  return content.substr(start, end - start);
}

bool is_inside_typedef(const CXCursor &cursor, const vector<pair<unsigned, unsigned>> &ranges)
{
  auto [start, end] = get_cursor_offsets(cursor);
  for (const auto &[fst, snd] : ranges)
  {
    if (start >= fst && end <= snd)
    {
      if (start == fst && end == snd)
        continue;
      return true;
    }
  }
  return false;
}

// --- Visitors
CXChildVisitResult visitor_find_includes(CXCursor cursor, CXCursor /*parent*/, CXClientData client_data)
{
  auto *includes_vec = static_cast<vector<string> *>(client_data);

  if (clang_getCursorKind(cursor) == CXCursor_InclusionDirective) // Target only #include directives
  {
    const CXString spelling = clang_getCursorSpelling(cursor);
    const string   name     = clang_getCString(spelling);

    includes_vec->push_back(name); // Add inclusion to our list

    clang_disposeString(spelling);
  }

  return CXChildVisit_Continue;
}

CXChildVisitResult visitor_find_typedefs(CXCursor cursor, CXCursor /*parent*/, CXClientData client_data)
{
  auto              *ctx  = static_cast<VisitorContext *>(client_data);
  const CXCursorKind kind = clang_getCursorKind(cursor);

  if (const CXSourceLocation loc = clang_getCursorLocation(cursor); !clang_Location_isFromMainFile(loc))
    return CXChildVisit_Continue;

  if (kind == CXCursor_TypedefDecl)
    ctx->typedef_ranges.push_back(get_cursor_offsets(cursor));

  return CXChildVisit_Continue;
}

CXChildVisitResult visitor_extract(CXCursor cursor, CXCursor /*parent*/, CXClientData client_data)
{
  const auto        *ctx  = static_cast<VisitorContext *>(client_data);
  const CXCursorKind kind = clang_getCursorKind(cursor);

  if (clang_getCursorLinkage(cursor) == CXLinkage_Internal)
    return CXChildVisit_Continue;

  if (const CXSourceLocation loc = clang_getCursorLocation(cursor); !clang_Location_isFromMainFile(loc))
    return CXChildVisit_Continue;

  if (kind == CXCursor_EnumDecl || kind == CXCursor_StructDecl || kind == CXCursor_UnionDecl)
  {
    if (is_inside_typedef(cursor, ctx->typedef_ranges))
      return CXChildVisit_Continue;
  }

  // Extract text
  string text = get_cursor_text(cursor, *ctx->content);
  if (text.empty())
    return CXChildVisit_Continue;

  switch (kind)
  {
  case CXCursor_InclusionDirective:
    ctx->decls->includes.push_back(text + "\n");
    break;

  case CXCursor_MacroDefinition:
    if (!clang_Cursor_isMacroBuiltin(cursor))
      ctx->decls->macros.push_back(text);
    break;

  case CXCursor_TypedefDecl:
    rtrim(text);
    if (!text.empty() && text.back() == ';')
      text.pop_back();
    ctx->decls->typedefs.push_back(text);
    break;

  case CXCursor_EnumDecl:
    if (clang_isCursorDefinition(cursor))
      ctx->decls->enums.push_back(text);
    break;

  case CXCursor_StructDecl:
    if (clang_isCursorDefinition(cursor))
      ctx->decls->structs.push_back(text);
    break;

  case CXCursor_UnionDecl:
    if (clang_isCursorDefinition(cursor))
      ctx->decls->unions.push_back(text);
    break;

  case CXCursor_VarDecl:
  {
    if (const size_t equal_pos = text.find('='); equal_pos != string::npos)
      text = text.substr(0, equal_pos);

    rtrim(text);
    if (!text.empty() && text.back() == ';')
      text.pop_back();

    if (text.find("extern") == string::npos)
      text = "extern " + text;
    ctx->decls->globals.push_back(text);
    break;
  }

  case CXCursor_FunctionDecl:
  {
    const CXString name_str = clang_getCursorSpelling(cursor);
    const string   name     = clang_getCString(name_str);
    clang_disposeString(name_str);

    if (name != "main")
    {
      size_t brace_pos = text.find('{');
      if (brace_pos != string::npos)
        text = text.substr(0, brace_pos);

      rtrim(text);
      if (!text.empty() && text.back() == ';')
        text.pop_back();
      ctx->decls->functions.push_back(text);
    }
    break;
  }

  default:
    break;
  }
  return CXChildVisit_Continue;
}

} // namespace

namespace zc
{

vector<Dependency> get_file_includes(const fs::path &file, const map<string, Pkg> &pkgs)
{
  vector<string>     found_includes;
  vector<Dependency> required_libs;

  const CXIndex index = clang_createIndex(0, 0);

  constexpr unsigned options = CXTranslationUnit_DetailedPreprocessingRecord; // To see #includes

  const char *args[] = { "-x", "c++" };                                       // Always compile as C++

  if (const CXTranslationUnit unit =
        clang_parseTranslationUnit(index, file.c_str(), args, 2, nullptr, 0, options))
  {
    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, visitor_find_includes, &found_includes); // Get all included file names

    for (const auto &inc : found_includes) // Compare with libraries map to extract flags
    {
      for (const auto &pair : pkgs)
      {
        if (inc.find(pair.first + "/") == 0 || inc == pair.first + ".h" || inc == pair.first + ".hh" ||
            inc == pair.first + ".hpp")
        {
          required_libs.push_back(
            {
              .name    = pair.first,
              .origin  = pair.second.origin,
              .version = pair.second.versions.rbegin()->first,
              // FIX: the used library in the include_links_dir is not always the latest one
            }
          );
        }
      }
    }
    clang_disposeTranslationUnit(unit);
  }
  clang_disposeIndex(index);
  return required_libs;
}

Declarations parse_declarations(const std::filesystem::path &file)
{
  Declarations decls;

  string content = read_file(file);
  if (content.empty())
    return decls;

  CXIndex index = clang_createIndex(0, 0);

  const char *args[] = { "-x", "c" };

  CXTranslationUnit unit = clang_parseTranslationUnit(
    index, file.c_str(), args, size(args), nullptr, 0,
    CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_KeepGoing
  );

  if (unit == nullptr)
  {
    clang_disposeIndex(index);
    throw ZCException(ZCE_PARSING_ERROR, "Unable to parse translation unit: " + file.string());
  }

  CXCursor       cursor = clang_getTranslationUnitCursor(unit);
  VisitorContext ctx    = { &decls, &content, {} };
  clang_visitChildren(cursor, visitor_find_typedefs, &ctx);
  clang_visitChildren(cursor, visitor_extract, &ctx);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  return decls;
}

} // namespace zc
