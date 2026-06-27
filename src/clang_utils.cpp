#include "clang_utils.h"

#include <algorithm>
#include <clang-c/Index.h>

#include "helpers.h"

ZC_DEV_CONFIG

namespace
{

CXChildVisitResult visitor_find_includes(CXCursor cursor, CXCursor parent, CXClientData client_data)
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

} // namespace

namespace zc
{

vector<Dependency> get_file_includes(const fs::path &file, const map<string, Pkg> &pkgs)
{
  vector<string>     found_includes;
  vector<Dependency> required_libs;

  CXIndex index = clang_createIndex(0, 0);

  unsigned options = CXTranslationUnit_DetailedPreprocessingRecord; // To see #includes

  const char       *args[] = { "-x", "c++" };                       // Always compile as C++
  CXTranslationUnit unit   = clang_parseTranslationUnit(index, file.c_str(), args, 2, nullptr, 0, options);

  if (unit)
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
              .version = *std::max_element(pair.second.versions.begin(), pair.second.versions.end()),
              // FIX : the used library in the include_links_dir is not always the latest one
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

} // namespace zc
