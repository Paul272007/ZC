/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <archive.h>
#include <archive_entry.h>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <openssl/evp.h>
#include <vector>

#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"

ZC_DEV_CONFIG

namespace zc
{

const fs::path &get_zc_root()
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
  fs::create_directories(get_zc_root());
}

void extract(const std::filesystem::path &archive, const std::filesystem::path &dest)
{
  archive_entry *entry;
  constexpr int flags =
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

  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
  const EVP_MD *md = EVP_sha256();
  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hash_len = 0;

  EVP_DigestInit_ex(mdctx, md, nullptr);

  char buffer[4096];
  while (file.read(buffer, sizeof(buffer)))
  {
    EVP_DigestUpdate(mdctx, buffer, file.gcount());
  }
  EVP_DigestUpdate(mdctx, buffer, file.gcount()); // Dernier bloc

  EVP_DigestFinal_ex(mdctx, hash, &hash_len);
  EVP_MD_CTX_free(mdctx);

  std::stringstream ss;
  for (unsigned int i = 0; i < hash_len; i++)
  {
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
  }
  return ss.str();
}

std::string base64_encode(const std::string &in)
{
  string out;
  int val = 0, valb = -6;
  for (const unsigned char c : in)
  {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0)
    {
      out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6)
    out.push_back(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]
    );
  while (out.size() % 4) out.push_back('=');
  return out;
}

std::string upper(const std::string &text)
{
  std::string output;
  output.reserve(text.size());
  for (const auto &c : text)
  {
    output += static_cast<char>(toupper(static_cast<unsigned char>(c)));
  }
  return output;
}

std::string lower(const std::string &text)
{
  std::string output;
  output.reserve(text.size());
  for (const auto &c : text)
  {
    output += static_cast<char>(tolower(static_cast<unsigned char>(c)));
  }
  return output;
}

std::string escape_shell_arg(const std::string &arg)
{
  std::string escaped;
  for (char c : arg)
  {
    if (c == '\'')
      escaped += "'\\''";
    else
      escaped += c;
  }
  return "'" + escaped + "'";
}

std::string exec_command(const std::string &cmd)
{
  std::string result;
  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    throw ZCException(ZCE_INTERNAL_ERROR, "popen() failed for command: " + cmd);
  char buffer[128];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) result += buffer;
  pclose(pipe);
  return result;
}

Targets parse_targets(const std::vector<std::string> &targets)
{
  Targets results;
  for (const auto &target : targets)
  {
    size_t at_pos = target.find('@');

    if (at_pos != string::npos)
      results.push_back({target.substr(0, at_pos), target.substr(at_pos + 1)});
    else
      results.push_back({target, {0, 0, 0}}); // 0.0.0 = empty version
  }
  return results;
}

void check_name(const std::string &name)
{
  if (name.at(0) == '-')
    throw ZCException(ZCE_CONTENT_ERROR, "The name of the package cannot begin with '-'");

  for (const auto &str : FORBIDDEN_NAMES)
    if (name == str)
      throw ZCException(ZCE_CONTENT_ERROR, "The name of the package or target is forbidden: " + string(str));

  for (char c : FORBIDDEN_CHARS)
  {
    size_t pos = name.find(c);
    if (pos != string::npos)
      throw ZCException(
          ZCE_CONTENT_ERROR,
          "The name of the package or target contains invalid character: " + std::string(1, c)
      );
  }
}

std::string read_file(const std::filesystem::path &file)
{
  string content;
  ifstream input(file);
  if (!input.is_open())
    throw ZCException(ZCE_READING_ERROR, "The file couldn't be read: " + file.string());
  input >> content;
  return content;
}

void write_file(const std::filesystem::path &file, const std::string &content)
{
  ofstream output(file);
  if (!output.is_open())
    throw ZCException(ZCE_WRITING_ERROR, "The file couldn't be written: " + file.string());
  output << content;
}

vector<fs::path> str_to_path(const vector<string> &vec)
{
  vector<fs::path> v;
  for (const auto &f : vec) v.emplace_back(f);
  return v;
}

} // namespace zc
