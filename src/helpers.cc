#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <helpers.hh>
#include <sstream>
#include <vector>

#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

namespace
{
fs::path calculateProjectRoot(const fs::path &base)
{
  fs::path current = base;

  while (true)
  {
    if (fs::exists(current / ZC_FILE))
      return current;

    if (current == current.root_path() || current == current.parent_path())
      break;

    current = current.parent_path();
  }
  throw ZCError(ZC_NOT_A_ZC_PROJECT, "The given directory is not inside a ZC project");
}
}

const fs::path &getProjectRoot()
{
  static fs::path path = calculateProjectRoot(fs::current_path());
  return path;
}

const fs::path &getProjectRoot(const std::filesystem::path &base)
{
  // Function is always called for 1 path max.
  static fs::path path = calculateProjectRoot(base);
  return path;
}

const fs::path &getZCRootDir()
{
#if defined(_WIN32) || defined(_WIN64)
  static const char *home = getenv("USERPROFILE");
#else
  static const char *home = getenv("HOME");
#endif

  static fs::path zc_root = (home) ? fs::path(home) / ROOT_DIR : fs::current_path() / ROOT_DIR;

  return zc_root;
}

string escape_shell_arg(const string &arg)
{
  string escaped = "'";
  for (char c : arg)
  {
    if (c == '\'')
    {
      escaped += "'\\''";
    }
    else
    {
      escaped += c;
    }
  }
  escaped += "'";
  return escaped;
}

string join(const vector<string> &v, const string &separator)
{
  stringstream s;
  for (int i = 0; i < v.size(); i++)
  {
    if (i != 0)
      s << separator;
    s << v[i];
  }
  return s.str();
}

vector<string> split(const string &s, const char delimiter)
{
  vector<string> tokens;
  string token;
  istringstream tokenStream(s);
  while (getline(tokenStream, token, delimiter))
  {
    tokens.push_back(token);
  }
  return tokens;
}

std::string upper(const std::string &s)
{
  std::string output;
  output.reserve(s.size());
  for (const auto &c : s)
  {
    output += static_cast<char>(toupper(static_cast<unsigned char>(c)));
  }
  return output;
}
