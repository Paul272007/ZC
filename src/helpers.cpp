#include "helpers.h"

#include <archive.h>
#include <archive_entry.h>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <openssl/evp.h>
#include <thread>
#include <unordered_set>
#include <vector>

#include "config/GConf.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "pkgs/Network.h"
#include "pkgs/Registry.h"
#include "templates/TemplateEngine.h"
#include "ui/Interface.h"
#include "ui/ShellCommand.h"

ZC_DEV_CONFIG

namespace zc
{

zc::Interface &ui()
{
  return zc::Interface::get();
}

zc::Network &net()
{
  return zc::Network::get();
}

zc::TemplateEngine &te()
{
  return zc::TemplateEngine::get();
}

zc::GConf &gc()
{
  return zc::GConf::get();
}

zc::Registry &rg()
{
  return zc::Registry::get();
}

const fs::path &zc_root()
{
  static const fs::path zc_root = []
  {
    const char *home = getenv(USER_HOME_ENV);

    fs::path root = (home) ? fs::path(home) / ZC_DIR : fs::current_path() / ZC_DIR;

    if (!fs::exists(root))
      fs::create_directories(root);

    return root;
  }();

  return zc_root;
}

std::filesystem::path get_project_root(const std::filesystem::path &base)
{
  if (base.empty())
    return get_project_root(fs::current_path());

  if (!fs::exists(base))
    throw ZCException(ZCE_NOT_FOUND, "The directory " + base.string() + " does not exist");

  fs::path current = base;

  while (true)
  {
    if (fs::exists(current / ZC_FILE))
      return current;

    if (current == current.root_path() || current == current.parent_path())
      break;

    current = current.parent_path();
  }
  throw ZCException(ZCE_NOT_A_ZC_PROJECT, "The given directory is not inside a ZC project");
}

void create_zc_root()
{
  fs::create_directories(zc_root());
}

void extract(const std::filesystem::path &archive, const std::filesystem::path &dest)
{
  archive_entry *entry;
  constexpr int  flags =
    ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;

  struct archive *a = archive_read_new();
  archive_read_support_format_all(a);
  archive_read_support_filter_all(a);
  struct archive *ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);

  if (archive_read_open_filename(a, archive.c_str(), 10240))
    throw ZCException(ZCE_READING_ERROR, "Could not open archive: " + archive.string());

  while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
  {
    fs::path full_path = dest / archive_entry_pathname(entry);
    archive_entry_set_pathname(entry, full_path.c_str());

    if (archive_read_extract(a, entry, flags) != ARCHIVE_OK)
      throw ZCException(ZCE_ARCHIVE_ERROR, "Could not extract archive: " + archive.string());
  }

  archive_read_close(a);
  archive_read_free(a);
  archive_write_close(ext);
  archive_write_free(ext);
}

std::string sha256(const std::filesystem::path &path)
{
  std::ifstream file(path, std::ios::binary);
  if (!file)
    throw ZCException(ZCE_READING_ERROR, "Cannot open file " + path.string() + " for hashing");

  EVP_MD_CTX   *mdctx = EVP_MD_CTX_new();
  const EVP_MD *md    = EVP_sha256();
  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int  hash_len = 0;

  EVP_DigestInit_ex(mdctx, md, nullptr);

  char buffer[4096];
  while (file.read(buffer, sizeof(buffer)))
    EVP_DigestUpdate(mdctx, buffer, static_cast<size_t>(file.gcount()));
  EVP_DigestUpdate(mdctx, buffer, static_cast<size_t>(file.gcount()));

  EVP_DigestFinal_ex(mdctx, hash, &hash_len);
  EVP_MD_CTX_free(mdctx);

  std::stringstream ss;
  for (unsigned int i = 0; i < hash_len; i++)
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
  return ss.str();
}

std::string base64_encode(const std::string &in)
{
  string out;
  int    val = 0, valb = -6;
  for (const char signed_c : in)
  {
    const auto c = static_cast<unsigned char>(signed_c);

    val   = (val << 8) + c;
    valb += 8;
    while (valb >= 0)
    {
      out.push_back(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]
      );
      valb -= 6;
    }
  }
  if (valb > -6)
    out.push_back(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]
    );
  while (out.size() % 4)
    out.push_back('=');
  return out;
}

size_t get_jobs_count(const int input_jobs)
{
  if (input_jobs > 0)
    return static_cast<size_t>(input_jobs);

  size_t jobs = std::thread::hardware_concurrency();
  if (jobs == 0)
    return 1;
  return jobs;
}

std::vector<Target> parse_targets(const std::vector<std::string> &targets)
{
  std::vector<Target> pairs;
  for (const auto &target : targets)
    if (const size_t at_pos = target.find('@'); at_pos != std::string::npos)
      pairs.emplace_back(target.substr(0, at_pos), target.substr(at_pos + 1));
    else
      pairs.emplace_back(target, Version::empty());
  return pairs;
}

std::string pretty_path(const std::filesystem::path &path)
{
  std::string p{ path.string() };
  if (const char *home = getenv(USER_HOME_ENV))
  {
    std::string home_str{ home };
    if (p.starts_with(home_str))
      p.replace(0, home_str.length(), "~");
  }
  if (fs::exists(path) && fs::is_directory(path))
    return p + "/";
  return p;
}

std::string join(const std::vector<std::string> &v, const std::string &separator)
{
  stringstream s;
  for (size_t i = 0; i < v.size(); i++)
  {
    if (i != 0)
      s << separator;
    s << v[i];
  }
  return s.str();
}

std::string upper(const std::string &text)
{
  std::string output;
  output.reserve(text.size());
  for (const auto &c : text)
    output += static_cast<char>(toupper(static_cast<unsigned char>(c)));
  return output;
}

std::string lower(const std::string &text)
{
  std::string output;
  output.reserve(text.size());
  for (const auto &c : text)
    output += static_cast<char>(tolower(static_cast<unsigned char>(c)));
  return output;
}

std::string esc(const std::string &arg)
{
  std::string escaped;
  for (const char c : arg)
    if (c == '\'')
      escaped += "'\\''";
    else
      escaped += c;
  return "'" + escaped + "'";
}

void check_name(const std::string &name)
{
  if (name.at(0) == '-')
    throw ZCException(ZCE_CONTENT_ERROR, "The name of the package cannot begin with '-'");

  for (const auto &str : FORBIDDEN_NAMES)
    if (name == str)
      throw ZCException(
        ZCE_CONTENT_ERROR, "The name of the package or target is forbidden: " + string(str)
      );

  for (const char c : FORBIDDEN_CHARS)
    if (const size_t pos = name.find(c); pos != string::npos)
      throw ZCException(
        ZCE_CONTENT_ERROR,
        "The name of the package or target contains invalid character: " + std::string(1, c)
      );
}

std::string read_file(const std::filesystem::path &file)
{
  std::ifstream input(file);
  if (!input.is_open())
    throw ZCException(ZCE_READING_ERROR, "The file couldn't be read: " + file.string());
  std::stringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

void write_file(const std::filesystem::path &file, const std::string &content)
{
  ofstream output(file);
  if (!output.is_open())
    throw ZCException(ZCE_WRITING_ERROR, "The file couldn't be written: " + file.string());
  output << content;
}

nlohmann::json read_json(const std::filesystem::path &file_path)
{
  nlohmann::json parsed_json;

  if (!std::filesystem::exists(file_path))
    throw ZCException(ZCE_NOT_FOUND, "The JSON file was not found: " + file_path.string());

  std::ifstream input(file_path);
  if (!input.is_open())
    throw ZCException(ZCE_READING_ERROR, "The JSON file couldn't be read: " + file_path.string());

  try
  {
    input >> parsed_json;
  }
  catch (const nlohmann::json::parse_error &e)
  {
    throw ZCException(
      ZCE_PARSING_ERROR, "The JSON file couldn't be parsed: " + file_path.string() + ": " + e.what()
    );
  }
  return parsed_json;
}

void write_json(const nlohmann::json &json, const std::filesystem::path &file_path)
{
  ofstream output(file_path);
  if (!output.is_open())
    throw ZCException(ZCE_WRITING_ERROR, "The JSON file couldn't be written: " + file_path.string());
  output << json.dump(2);
  output.close();
}

vector<fs::path> str_to_path(const vector<string> &vec)
{
  vector<fs::path> v;
  v.reserve(vec.size());
  for (const auto &f : vec)
    v.emplace_back(f);
  return v;
}

std::vector<std::string> split(const std::string &str, char delimiter)
{
  std::vector<std::string> tokens;

  size_t start = 0;
  size_t end   = str.find(delimiter);

  while (end != std::string::npos)
  {
    if (end != start)
      tokens.push_back(str.substr(start, end - start));
    start = end + 1;
    end   = str.find(delimiter, start);
  }

  if (start < str.length())
    tokens.push_back(str.substr(start));

  return tokens;
}

bool has_pkg_config()
{
  return ShellCommand::exec({ "pkg-config", "--version" }, output::hide_all) == 0;
}

std::string get_pkg_config_flags(const std::string &pkg_name, const bool cflags)
{
  ShellCommand cmd{ vector<string>{ "pkg-config", "--libs", pkg_name } };
  if (cflags)
    cmd << "--cflags";
  std::string result;

  cmd.output_actions(128, [&](const string &l) { result += l; }, output::hide_err);

  if (!result.empty() && result.back() == '\n')
    result.pop_back();

  return result;
}

void merge(const std::vector<std::string> &src, std::vector<std::string> &dest)
{
  unordered_set existing(dest.begin(), dest.end());

  for (const auto &item : src)
  {
    if (!existing.contains(item))
    {
      dest.push_back(item);
      existing.insert(item);
    }
  }
}

} // namespace zc
