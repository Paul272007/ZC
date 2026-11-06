#include "framework/network.hpp"

#include <curl/curl.h>
#include <fstream>
#include <memory>

namespace zc::net
{

namespace
{

struct CurlGlobalInit {
  CurlGlobalInit(const CurlGlobalInit &)            = delete;
  CurlGlobalInit(CurlGlobalInit &&)                 = delete;
  CurlGlobalInit &operator=(const CurlGlobalInit &) = delete;
  CurlGlobalInit &operator=(CurlGlobalInit &&)      = delete;

  CurlGlobalInit() { curl_global_init(CURL_GLOBAL_DEFAULT); }

  ~CurlGlobalInit() { curl_global_cleanup(); }
};

void ensure_curl_init()
{
  static const CurlGlobalInit init;
}

struct CurlCleanup {
  void operator()(CURL *c) const
  {
    if (c)
      curl_easy_cleanup(c);
  }
};

struct SlistCleanup {
  void operator()(curl_slist *l) const
  {
    if (l)
      curl_slist_free_all(l);
  }
};

using UniqueCurl  = std::unique_ptr<CURL, CurlCleanup>;
using UniqueSlist = std::unique_ptr<curl_slist, SlistCleanup>;

const char *to_string(const Method m) noexcept
{
  switch (m) {
  case Method::Get:
    return "GET";
  case Method::Post:
    return "POST";
  case Method::Put:
    return "PUT";
  case Method::Delete:
    return "DELETE";
  case Method::Patch:
    return "PATCH";
  case Method::Head:
    return "HEAD";
  default:
    return "UNKNOWN";
  }
}

} // namespace

std::expected<Response, Exception> send(const Request &request)
{
  ensure_curl_init();

  const UniqueCurl curl(curl_easy_init());
  if (!curl)
    return std::unexpected(Exception(OSError::CurlError, "Failed to initialize CURL"));

  curl_slist *raw_headers = nullptr;
  for (const auto &[name, val] : request.headers)
    raw_headers = curl_slist_append(raw_headers, std::format("{}: {}", name, val).c_str());
  UniqueSlist headers(raw_headers);

  Response response;
  curl_easy_setopt(curl.get(), CURLOPT_URL, request.url.c_str());
  curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, to_string(request.method));

  if (headers)
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());

  if (!request.body.empty())
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, request.body.c_str());

  if (request.follow_redirects)
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);

  curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, static_cast<long>(request.timeout.count()));

  // Memory reading callback
  curl_easy_setopt(
    curl.get(), CURLOPT_WRITEFUNCTION,
    +[](void *contents, const size_t size, const size_t nmemb, void *userp) -> size_t {
      auto *buf = static_cast<std::string *>(userp);
      buf->append(static_cast<char *>(contents), size * nmemb);
      return size * nmemb;
    }
  );
  curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response.body);

  if (const CURLcode res = curl_easy_perform(curl.get()); res != CURLE_OK) {
    return std::unexpected(
      Exception(OSError::NetworkError, "Network error on {}: {}", request.url, curl_easy_strerror(res))
    );
  }

  curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response.status_code);
  return response;
}

std::expected<Response, Exception> get(const std::string &url, const std::vector<Header> &headers)
{
  return send(Request{ .url = url, .body = "", .method = Method::Get, .headers = headers });
}

std::expected<Response, Exception>
post(const std::string &url, const std::string &payload, const std::vector<Header> &headers)
{
  return send(Request{ .url = url, .body = payload, .method = Method::Post, .headers = headers });
}

std::expected<Response, Exception>
put(const std::string &url, const std::string &payload, const std::vector<Header> &headers)
{
  return send(Request{ .url = url, .body = payload, .method = Method::Put, .headers = headers });
}

std::expected<void, Exception>
download(const std::string &url, const stdfs::path &dest, const std::vector<Header> &headers)
{
  ensure_curl_init();
  const UniqueCurl curl(curl_easy_init());
  if (!curl)
    return std::unexpected(Exception(OSError::CurlError, "Failed to initialize CURL"));

  std::ofstream out(dest, std::ios::binary);
  if (!out)
    return std::unexpected(Exception(FSError::WritingError, "Failed to open destination file"));

  curl_slist *raw_headers = nullptr;
  for (const auto &[name, val] : headers)
    raw_headers = curl_slist_append(raw_headers, std::format("{}: {}", name, val).c_str());

  const UniqueSlist slist(raw_headers);

  curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
  if (slist)
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, slist.get());

  curl_easy_setopt(
    curl.get(), CURLOPT_WRITEFUNCTION,
    +[](void *ptr, const size_t size, const size_t nmemb, void *stream) -> size_t {
      auto *f = static_cast<std::ofstream *>(stream);
      f->write(static_cast<char *>(ptr), static_cast<std::streamsize>(size * nmemb));
      return size * nmemb;
    }
  );
  curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &out);

  if (const CURLcode res = curl_easy_perform(curl.get()); res != CURLE_OK)
    return std::unexpected(Exception(OSError::PipeError, "Download failed: {}", curl_easy_strerror(res)));

  return {};
}

} // namespace zc::net
