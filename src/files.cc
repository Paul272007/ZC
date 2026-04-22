#include <chrono>
#include <clang-c/Index.h>
#include <fstream>

#include "files.hh"
#include "nlohmann/json.hpp"
#include "objects/Registry.hh"
#include "objects/ZCError.hh"

using namespace std;

namespace
{

struct VisitorContext
{
  Declarations *decls;
  const string *content;
  vector<pair<unsigned, unsigned>> typedef_ranges;
};

/**
 * @brief Helper to trim strings
 */
void rtrim(string &s)
{
  while (!s.empty() && isspace(s.back()))
  {
    s.pop_back();
  }
}

/**
 * @brief Helper to get the (start, end) offsets of a cursor
 */
pair<unsigned, unsigned> get_cursor_offsets(CXCursor cursor)
{
  CXSourceRange range = clang_getCursorExtent(cursor);
  CXSourceLocation start = clang_getRangeStart(range);
  CXSourceLocation end = clang_getRangeEnd(range);

  unsigned start_offset, end_offset;
  clang_getInstantiationLocation(start, nullptr, nullptr, nullptr, &start_offset);
  clang_getInstantiationLocation(end, nullptr, nullptr, nullptr, &end_offset);

  return {start_offset, end_offset};
}

string get_cursor_text(CXCursor cursor, const string &content)
{
  auto [start, end] = get_cursor_offsets(cursor);

  if (end <= start || end > content.length())
  {
    return "";
  }
  return content.substr(start, end - start);
}

// Checks if a cursor is included in an already seen typedef
bool is_inside_typedef(CXCursor cursor, const vector<pair<unsigned, unsigned>> &ranges)
{
  auto [start, end] = get_cursor_offsets(cursor);
  for (const auto &range : ranges)
  {
    if (start >= range.first && end <= range.second)
    {
      if (start == range.first && end == range.second)
        continue;
      return true;
    }
  }
  return false;
}

// --- Visitors ---

// 1 : Find typedefs
CXChildVisitResult visitor_find_typedefs(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
  auto *ctx = static_cast<VisitorContext *>(client_data);
  CXCursorKind kind = clang_getCursorKind(cursor);

  CXSourceLocation loc = clang_getCursorLocation(cursor);
  if (!clang_Location_isFromMainFile(loc))
    return CXChildVisit_Continue;

  if (kind == CXCursor_TypedefDecl)
  {
    ctx->typedef_ranges.push_back(get_cursor_offsets(cursor));
  }

  return CXChildVisit_Continue;
}

// 2 : Extraction
CXChildVisitResult visitor_extract(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
  auto *ctx = static_cast<VisitorContext *>(client_data);
  CXCursorKind kind = clang_getCursorKind(cursor);

  if (clang_getCursorLinkage(cursor) == CXLinkage_Internal)
    return CXChildVisit_Continue;

  CXSourceLocation loc = clang_getCursorLocation(cursor);
  if (!clang_Location_isFromMainFile(loc))
    return CXChildVisit_Continue;

  if (kind == CXCursor_EnumDecl || kind == CXCursor_StructDecl || kind == CXCursor_UnionDecl)
  {
    if (is_inside_typedef(cursor, ctx->typedef_ranges))
    {
      return CXChildVisit_Continue;
    }
  }

  // Extract text
  string text = get_cursor_text(cursor, *ctx->content);
  if (text.empty())
    return CXChildVisit_Continue;

  if (kind == CXCursor_InclusionDirective)
  {
    (*ctx->decls)["includes"].push_back(text + "\n");
  }
  else if (kind == CXCursor_MacroDefinition)
  {
    if (!clang_Cursor_isMacroBuiltin(cursor))
    {
      (*ctx->decls)["macros"].push_back(text);
    }
  }
  else if (kind == CXCursor_TypedefDecl)
  {
    rtrim(text);
    if (!text.empty() && text.back() == ';')
      text.pop_back();
    (*ctx->decls)["typedefs"].push_back(text);
  }
  else if (kind == CXCursor_EnumDecl)
  {
    if (clang_isCursorDefinition(cursor))
    {
      (*ctx->decls)["enums"].push_back(text);
    }
  }
  else if (kind == CXCursor_StructDecl)
  {
    if (clang_isCursorDefinition(cursor))
    {
      (*ctx->decls)["structs"].push_back(text);
    }
  }
  else if (kind == CXCursor_UnionDecl)
  {
    if (clang_isCursorDefinition(cursor))
    {
      (*ctx->decls)["unions"].push_back(text);
    }
  }
  else if (kind == CXCursor_VarDecl)
  {
    size_t equal_pos = text.find('=');
    if (equal_pos != string::npos)
      text = text.substr(0, equal_pos);

    rtrim(text);
    if (!text.empty() && text.back() == ';')
      text.pop_back();

    if (text.find("extern") == string::npos)
      text = "extern " + text;
    (*ctx->decls)["globals"].push_back(text);
  }
  else if (kind == CXCursor_FunctionDecl)
  {
    CXString name_str = clang_getCursorSpelling(cursor);
    string name = clang_getCString(name_str);
    clang_disposeString(name_str);

    if (name != "main")
    {
      size_t brace_pos = text.find('{');
      if (brace_pos != string::npos)
        text = text.substr(0, brace_pos);

      rtrim(text);
      if (!text.empty() && text.back() == ';')
        text.pop_back();
      (*ctx->decls)["functions"].push_back(text);
    }
  }

  return CXChildVisit_Continue;
}

CXChildVisitResult visitor_find_includes(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
  auto *includes_vec = static_cast<vector<string> *>(client_data);

  // Target only #include directives
  if (clang_getCursorKind(cursor) == CXCursor_InclusionDirective)
  {
    const CXString spelling = clang_getCursorSpelling(cursor);
    const string name = clang_getCString(spelling);

    // Add inclusion to our list
    includes_vec->push_back(name);

    clang_disposeString(spelling);
  }

  return CXChildVisit_Continue;
}
} // namespace

unique_ptr<Declarations> parse(const std::filesystem::path &f)
{
  unique_ptr<Declarations> decls = make_unique<Declarations>();

  // 1. Read file and extract content
  string content = read(f);
  if (content.empty())
    return decls;

  // 2. Initialize libclang index
  CXIndex index = clang_createIndex(0, 0);

  // 3. Compilation arguments
  const char *args[] = {"-x", "c", "-I.", "-Iinclude"};

  // 4. Parse file
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, f.c_str(), args, size(args), nullptr, 0,
      CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_KeepGoing
  );

  if (unit == nullptr)
  {
    clang_disposeIndex(index);
    throw ZCError(ZC_PARSING_ERROR, "Unable to parse translation unit: " + f.string());
  }

  // 5. Launch visitor
  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  VisitorContext ctx = {decls.get(), &content};
  // 1
  clang_visitChildren(cursor, visitor_find_typedefs, &ctx);
  // 2
  clang_visitChildren(cursor, visitor_extract, &ctx);

  // 6. Cleaning
  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  return decls;
}

void writeDeclarations(const Declarations &decls, const std::filesystem::path &file)
{
  stringstream content;

  // Custom header
  content << "/*\n\tThis file was automatically generated by ZC\n";
  const auto now = chrono::system_clock::now();
  auto now_sec = chrono::floor<chrono::seconds>(now);
  const string s = format("{:%F %T}", now_sec);
  content << "\tDate of creation: " << s << " (UTC)\n";
  content << "\tEditing this file manually could break it.\n*/\n\n";

  // Header guards
  content << "#pragma once\n\n";

  auto it = decls.find("includes");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Includes */\n";
    for (const auto &inc : it->second)
    {
      content << inc;
    }
    content << '\n';
  }

  it = decls.find("macros");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Macros */\n";
    for (const auto &macro : it->second)
    {
      content << "#define " << macro << '\n';
    }
    content << '\n';
  }

  content << '\n'
          << "#ifdef __cplusplus\n"
          << "extern \"C\" {\n"
          << "#endif\n\n";

  it = decls.find("enums");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Enums */\n";
    for (const auto &en : it->second)
    {
      content << en << ";\n";
    }
    content << '\n';
  }

  it = decls.find("unions");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Unions */\n";
    for (const auto &un : it->second)
    {
      content << un << ";\n";
    }
    content << '\n';
  }

  it = decls.find("structs");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Structures */\n";
    for (const auto &struc : it->second)
    {
      content << struc << ";\n";
    }
    content << '\n';
  }

  it = decls.find("typedefs");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Typedefs */\n";
    for (const auto &td : it->second)
    {
      content << td << ";\n";
    }
    content << '\n';
  }

  it = decls.find("globals");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Global variables */\n";
    for (const auto &glob : it->second)
    {
      content << glob << ";\n";
    }
    content << '\n';
  }

  it = decls.find("functions");
  if (it != decls.end() && !it->second.empty())
  {
    content << "/* Functions */\n";
    for (const auto &func : it->second)
    {
      content << func << ";\n";
    }
    content << '\n';
  }

  content << "#ifdef __cplusplus\n"
          << "}\n"
          << "#endif\n";

  write(file, content.str());
}

void write(const std::filesystem::path &file, const std::string &content)
{
  ofstream stream(file);

  if (!stream.is_open())
    throw ZCError(ZC_WRITING_ERROR, "Could not write " + file.string());

  stream << content;
  stream.close();
}

bool isCpp(const std::filesystem::path &file)
{
  const char *exts[] = {".cc", ".cpp", ".cxx"};
  for (const char *ext : exts)
    if (file.extension().string() == ext)
      return true;
  return false;
}

vector<string> getFileInclusions(const std::filesystem::path &file, const std::vector<Package> &pkgs)
{
  vector<string> found_includes;
  vector<string> required_libs;

  CXIndex index = clang_createIndex(0, 0);

  // To see #includes
  unsigned options = CXTranslationUnit_DetailedPreprocessingRecord;

  const char *args[] = {"-x", "c++"}; // Always compile as C++
  CXTranslationUnit unit = clang_parseTranslationUnit(index, file.c_str(), args, 2, nullptr, 0, options);

  if (unit)
  {
    CXCursor cursor = clang_getTranslationUnitCursor(unit);

    // Get all included file names
    clang_visitChildren(cursor, visitor_find_includes, &found_includes);

    // Compare with libraries map to extract flags
    for (const auto &inc : found_includes)
    {
      for (const auto &package : pkgs)
      {
        if (inc.find(package.name + "/") == 0 || inc == package.name + ".h" || inc == package.name + ".hh" ||
            inc == package.name + ".hpp")
        {
          bool already_present = std::any_of(
              required_libs.begin(), required_libs.end(), [&](const auto &p) { return p == package.name; }
          );

          if (!already_present)
            required_libs.push_back(package.binary);
        }
      }
    }

    clang_disposeTranslationUnit(unit);
  }

  clang_disposeIndex(index);
  return required_libs;
}

nlohmann::json parseJsonFile(const std::filesystem::path &file_path)
{
  nlohmann::json parsed_json;

  if (!std::filesystem::exists(file_path))
    throw ZCError(ZC_CONFIG_NOT_FOUND, "The JSON file was not found: " + file_path.string());

  std::ifstream input(file_path);
  if (!input.is_open())
    throw ZCError(ZC_CONFIG_READING_ERROR, "The JSON file couldn't be read: " + file_path.string());

  try
  {
    input >> parsed_json;
  }
  catch (const nlohmann::json::parse_error &e)
  {
    throw ZCError(
        ZC_CONFIG_PARSING_ERROR, "The JSON file couldn't be parsed: " + file_path.string() + ": " + e.what()
    );
  }

  return parsed_json;
}

void writeJsonFile(const nlohmann::json &json, const std::filesystem::path &file_path)
{
  ofstream output(file_path);
  if (!output.is_open())
    throw ZCError(ZC_CONFIG_WRITING_ERROR, "The JSON file couldn't be written: " + file_path.string());

  output << json.dump(2);
  output.close();
}

string read(const std::filesystem::path &file)
{
  ifstream stream(file);

  if (!stream.is_open())
    return "";
  stringstream buffer;
  buffer << stream.rdbuf();
  return buffer.str();
}
