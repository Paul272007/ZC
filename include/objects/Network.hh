#pragma once

#include <curl/curl.h>
#include <filesystem>
#include <string>

class Network
{
public:
  static Network &getInstance();
  Network(const Network &) = delete;
  void operator=(const Network &) = delete;
  void download(const std::string &url, const std::filesystem::path &dest);
  Network()
  {
    curl_global_init(CURL_GLOBAL_DEFAULT);
  }
  ~Network()
  {
    curl_global_cleanup();
  }

private:
  static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
};
