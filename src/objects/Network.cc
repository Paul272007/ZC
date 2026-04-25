#include "objects/Network.hh"
#include "files.hh"
#include "helpers.hh"
#include "nlohmann/json.hpp"
#include "objects/ZCError.hh"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

size_t Network::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}

void Network::download(const std::string &url, const std::filesystem::path &dest) const
{
  CURL *curl;
  FILE *fp;
  CURLcode res;

  curl = curl_easy_init();
  if (!curl)
  {
    throw ZCError(ZC_INTERNAL_ERROR, "Failed to initialize CURL");
  }

  fp = fopen(dest.string().c_str(), "wb");
  if (!fp)
  {
    curl_easy_cleanup(curl);
    throw ZCError(ZC_WRITING_ERROR, "Failed to open file for writing: " + dest.string());
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

  res = curl_easy_perform(curl);

  fclose(fp);

  if (res != CURLE_OK)
  {
    string err_msg = curl_easy_strerror(res);
    curl_easy_cleanup(curl);
    fs::remove(dest);
    throw ZCError(ZC_NETWORK_ERROR, "Network error: " + err_msg + " (" + url + ")");
  }

  curl_easy_cleanup(curl);
}

json Network::getIndex(const Logger &log) const
{
  log(LogLevel::INFO, "Fetching registry index...");
  fs::create_directories(tmp_dir_);
  download(INDEX_URL, index_);
  return parseJsonFile(index_);
}
