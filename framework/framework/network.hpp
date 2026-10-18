#pragma once

#include <expected>

#include "framework/Exception.hpp"
#include "framework/util/types.hpp"

namespace zc::net
{

enum class Method : u8 {
  Get,
  Post,
  Put,
  Delete,
  Patch,
  Head,
};

struct Header {
  std::string name;
  std::string value;
};

struct Request {
  std::string url;
  std::string body;

  Method method{ Method::Get };
  bool   follow_redirects{ true };

  std::vector<Header>       headers;
  std::chrono::milliseconds timeout{ 10000 };

  Request &withHeader(std::string name, std::string value)
  {
    headers.push_back({ .name = std::move(name), .value = std::move(value) });
    return *this;
  }
};

struct Response {
  u64         status_code{ 0 };
  std::string body;

  [[nodiscard]] constexpr bool ok() const noexcept { return status_code >= 200 && status_code < 300; }
};

std::expected<Response, Exception> send(const Request &request);

std::expected<Response, Exception> get(const std::string &url, const std::vector<Header> &headers = {});

std::expected<Response, Exception>
post(const std::string &url, const std::string &payload, const std::vector<Header> &headers = {});

std::expected<Response, Exception>
put(const std::string &url, const std::string &payload, const std::vector<Header> &headers = {});

std::expected<void, Exception>
download(const std::string &url, const stdfs::path &dest, const std::vector<Header> &headers = {});

} // namespace zc::net
