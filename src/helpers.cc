#include <algorithm>
#include <archive.h>
#include <archive_entry.h>
#include <fstream>
#include <openssl/evp.h>
#include <unordered_set>

#include "helpers.hh"
#include "objects/ZCError.hh"

using namespace std;
namespace fs = std::filesystem;

const fs::path getProjectRoot(const std::filesystem::path &base)
{
  if (!fs::exists(base))
    throw ZCError(ZC_NOT_FOUND, "The directory " + base.string() + " does not exist");

  fs::path current = base;

  while (true)
  {
    if (fs::exists(current / CONFIG))
      return current;

    if (current == current.root_path() || current == current.parent_path())
      break;

    current = current.parent_path();
  }
  throw ZCError(ZC_NOT_A_ZC_PROJECT, "The given directory is not inside a ZC project");
}

const fs::path getProjectRoot()
{
  static fs::path path = getProjectRoot(fs::current_path());
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
    if (pos != string::npos)
      throw ZCError(
          ZC_CONFIG_CONTENT_ERROR, &"The name of the package or target contains invalid character: "[c]
      );
  }
}

bool extract_archive(const std::string &filename, const std::string &dest)
{
  bool is_successful = true;
  struct archive *a;
  struct archive *ext;
  struct archive_entry *entry;
  int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
  int r;

  a = archive_read_new();
  archive_read_support_format_all(a);
  archive_read_support_filter_all(a);
  ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);

  if ((r = archive_read_open_filename(a, filename.c_str(), 10240)))
  {
    throw ZCError(ZC_TAR_ERROR, "Could not open archive: " + filename);
  }

  while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
  {
    std::string full_path = dest + "/" + archive_entry_pathname(entry);
    archive_entry_set_pathname(entry, full_path.c_str());

    r = archive_read_extract(a, entry, flags);
    if (r != ARCHIVE_OK)
    {
      is_successful = false;
    }
  }

  archive_read_close(a);
  archive_read_free(a);
  archive_write_close(ext);
  archive_write_free(ext);
  return is_successful;
}

std::string calculate_sha256(const fs::path &path)
{
  std::ifstream file(path, std::ios::binary);
  if (!file)
    throw ZCError(ZC_READING_ERROR, "Cannot open file " + path.string() + " for hashing");

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

  // Conversion en chaîne hexadécimale
  std::stringstream ss;
  for (unsigned int i = 0; i < hash_len; i++)
  {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }
  return ss.str();
}

void removeDuplicates(std::vector<std::string> &v)
{
  std::unordered_set<std::string> seen;
  auto new_end =
      std::remove_if(v.begin(), v.end(), [&](const std::string &s) { return !seen.insert(s).second; });

  v.erase(new_end, v.end());
}
