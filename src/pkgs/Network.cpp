/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "../helpers.h"
#include "../excepts/ZCException.h"
#include "Network.h"

#include "../ui/Interface.h"

ZC_DEV_CONFIG_JSON

/**
 * Network implementation
 */

/**
 * @return Network
 */
Network &Network::get()
{
  static Network instance;
  return instance;
}

void Network::download(const std::string &url, const std::filesystem::path &dest) const
{
  CURL *curl = curl_easy_init();

  if (!curl)
    throw ZCException(ZCE_INTERNAL_ERROR, "Failed to initialize CURL");

  FILE *fp = fopen(dest.string().c_str(), "wb");

  if (!fp)
  {
    curl_easy_cleanup(curl);
    throw ZCException(ZCE_WRITING_ERROR, "Failed to open file for writing: " + dest.string());
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

  const CURLcode res = curl_easy_perform(curl);

  fclose(fp);

  if (res != CURLE_OK)
  {
    const string err_msg = curl_easy_strerror(res);
    curl_easy_cleanup(curl);
    fs::remove(dest);
    throw ZCException(ZCE_NETWORK_ERROR, "Network error: " + err_msg + " (" + url + ")");
  }

  curl_easy_cleanup(curl);
}

/**
 * @param url
 * @param payload
 * @return void
 */
std::string Network::post(const std::string &url, const std::string &payload, const std::string &token)
{
  return request(url, "POST", payload, token);
}

/**
 * @param url
 * @param payload
 * @return void
 */
std::string Network::put(const std::string &url, const std::string &payload, const std::string &token)
{
  return request(url, "PUT", payload, token);
}

std::string Network::get(const std::string &url, const std::string &payload, const std::string &token)
{
  return request(url, "GET", payload, token);
}

const nlohmann::json &Network::get_index() const
{
  static const json json_index = [&] {
    const auto &if_ = Interface::get();
    if_.info("Fetching registry index...");
    const fs::path tmp_dir = get_zc_root() / TMP_DIR;
    const fs::path index = tmp_dir / INDEX_FILE;
    fs::create_directories(tmp_dir);
    download(INDEX_URL, index);
    return if_.read_json(index);
  }();
  
  return json_index;
}

Network::~Network()
{
  curl_global_cleanup();
}

Network::Network()
{
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

string Network::request(
    const std::string &url, const std::string &method, const std::string &payload, const std::string &token
)
{
  {
    CURL *curl = curl_easy_init();
    if (!curl)
      throw ZCException(ZCE_INTERNAL_ERROR, "Failed to initialize CURL");

    std::string readBuffer;
    curl_slist *headers = nullptr;
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
        +static_cast<size_t (*)(void *, size_t, size_t, void *)>(
            [](void *contents, const size_t size, const size_t nmemb, void *userp) -> size_t
            {
              static_cast<std::string *>(userp)->append(static_cast<char *>(contents), size * nmemb);
              return size * nmemb;
            }
        )
    );
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    const CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
      throw ZCException(ZCE_NETWORK_ERROR, "Network error during " + method + " request: " + curl_easy_strerror(res));

    if (http_code >= 400)
      throw ZCException(ZCE_NETWORK_ERROR, "API Error (" + std::to_string(http_code) + "): " + readBuffer);

    return readBuffer;
  }
}
