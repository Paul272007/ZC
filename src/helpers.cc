#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <helpers.hh>
#include <nlohmann/json.hpp>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

namespace
{
fs::path calculateProjectRoot(const fs::path &base)
{
  if (!fs::exists(base))
    throw ZCError(ZC_NOT_FOUND, "The directory " + base.string() + " does not exist");

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
} // namespace

const fs::path getProjectRoot()
{
  static fs::path path = calculateProjectRoot(fs::current_path());
  return path;
}

const fs::path getProjectRoot(const std::filesystem::path &base)
{
  return calculateProjectRoot(base);
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

std::string execAndGetOutput(const char *cmd)
{
  char buffer[128];
  string result = "";
  FILE *pipe = popen(cmd, "r");
  if (!pipe)
    throw std::runtime_error("popen() failed!");
  try
  {
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
      result += buffer;
    }
  }
  catch (...)
  {
    pclose(pipe);
    throw;
  }
  pclose(pipe);
  return result;
}

std::string urlEncode(const std::string &s)
{
  string result;
  for (char c : s)
  {
    if (c == '"')
      result += "%22";
    else if (c == ' ')
      result += "%20";
    else if (c == '\n')
      result += "%0A";
    else
      result += c;
  }
  return result;
}

void checkPackageName(const std::string &name)
{
  const char forbidden_chars[] = {'@', '#', ' ', '*',  '%', '!',  '?', '{', '}', '[', ']',
                                  '(', ')', '"', '\'', '/', '\\', '|', '~', '&', ';', ':'};

  if (name.at(0) == '-')
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "The name of the package begin with '-'");

  for (char c : forbidden_chars)
  {
    size_t pos = name.find(c);
    if (pos != string::npos) // TODO : change to target if target is being checked
      throw ZCError(ZC_CONFIG_CONTENT_ERROR, &"The name of the package contains invalid character: "[c]);
  }
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
