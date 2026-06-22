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

vector<Dependency> get_file_includes(const fs::path &file, const vector<RegistryPkg> &pkgs)
{
  vector<string>     found_includes;
  vector<Dependency> required_libs;

  CXIndex index = clang_createIndex(0, 0);

  unsigned options = CXTranslationUnit_DetailedPreprocessingRecord; // To see #includes

  const char       *args[] = { "-x", "c++" };                       // Always compile as C++
  CXTranslationUnit unit = clang_parseTranslationUnit(index, file.c_str(), args, 2, nullptr, 0, options);

  if (unit)
  {
    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, visitor_find_includes, &found_includes); // Get all included file names

    for (const auto &inc : found_includes) // Compare with libraries map to extract flags
    {
      for (const auto &pkg : pkgs)
      {
        if (inc.find(pkg.name + "/") == 0 || inc == pkg.name + ".h" || inc == pkg.name + ".hh" ||
            inc == pkg.name + ".hpp")
        {
          required_libs.push_back(
            {
              .name    = pkg.name,
              .origin  = pkg.origin,
              .version = *std::max_element(pkg.versions.begin(), pkg.versions.end()),
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
