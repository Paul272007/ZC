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

std::string
Network::request(const std::string &url, const std::string &method, const std::string &payload, const std::string &token) const
{
  CURL *curl = curl_easy_init();
  if (!curl)
    throw ZCError(ZC_INTERNAL_ERROR, "Failed to initialize CURL");

  std::string readBuffer;
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
  headers = curl_slist_append(headers, "User-Agent: zc-cli");
  headers = curl_slist_append(headers, "X-GitHub-Api-Version: 2022-11-28");

  if (!token.empty())
    headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
  if (!payload.empty())
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(
      curl, CURLOPT_WRITEFUNCTION,
      +(size_t (*)(void *, size_t, size_t, void *))
          [](void *contents, size_t size, size_t nmemb, void *userp) -> size_t
      {
        ((std::string *)userp)->append((char *)contents, size * nmemb);
        return size * nmemb;
      }
  );
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

  CURLcode res = curl_easy_perform(curl);
  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK)
    throw ZCError(ZC_NETWORK_ERROR, "Network error during " + method + " request: " + curl_easy_strerror(res));

  if (http_code >= 400)
    throw ZCError(ZC_NETWORK_ERROR, "API Error (" + std::to_string(http_code) + "): " + readBuffer);

  return readBuffer;
}

std::string
Network::postAuth(const std::string &url, const std::string &payload, const std::string &token) const
{
  return request(url, "POST", payload, token);
}

std::string Network::getAuth(const std::string &url, const std::string &token) const
{
  return request(url, "GET", "", token);
}

std::string Network::post(const std::string &url, const std::string &payload) const
{
  return request(url, "POST", payload);
}

std::string Network::put(const std::string &url, const std::string &payload, const std::string &token) const
{
  return request(url, "PUT", payload, token);
}

json Network::getIndex(const Logger &log) const
{
  log(LogLevel::INFO, "Fetching registry index...");
  fs::create_directories(tmp_dir_);
  download(INDEX_URL, index_);
  return parseJsonFile(index_);
}
