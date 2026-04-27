#pragma once

#include <curl/curl.h>
#include <filesystem>
#include <string>

#include "helpers.hh"
#include "nlohmann/json_fwd.hpp"

class Network
{
public:
  static Network &getInstance();
  Network(const Network &) = delete;
  void operator=(const Network &) = delete;
  void download(const std::string &url, const std::filesystem::path &dest) const;
  std::string postAuth(const std::string &url, const std::string &payload, const std::string &token) const;
  std::string getAuth(const std::string &url, const std::string &token) const;
  std::string post(const std::string &url, const std::string &payload) const;
  std::string put(const std::string &url, const std::string &payload, const std::string &token) const;
  [[nodiscard]] nlohmann::json getIndex(const Logger &log) const;
  Network(const std::filesystem::path &tmp_dir) : tmp_dir_(tmp_dir), index_(tmp_dir_ / INDEX)
  {
    curl_global_init(CURL_GLOBAL_DEFAULT);
  }
  ~Network()
  {
    curl_global_cleanup();
  }

private:
  std::string request(
      const std::string &url, const std::string &method, const std::string &payload,
      const std::string &token = ""
  ) const;
  static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
  const std::filesystem::path tmp_dir_;
  std::filesystem::path index_;
};
