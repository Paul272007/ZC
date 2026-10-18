#pragma once

#include <expected>
#include <optional>
#include <string>

#include "core/util/ZCError.hpp"
#include "directories.hpp"
#include "framework/Exception.hpp"
#include "framework/network.hpp"
#include "framework/util/types.hpp"

namespace zc::core
{

class GithubClient
{
  std::string token_;

public:
  explicit GithubClient(std::string token = "") : token_(std::move(token)) {}

  [[nodiscard]] std::expected<json, Exception> get(const std::string &url) const
  {
    net::Request req{ .url = url, .body = "", .method = net::Method::Get, .headers = {} };
    req.withHeader("Accept", "application/vnd.github+json")
      .withHeader("User-Agent", "zc-cli")
      .withHeader("X-GitHub-Api-Version", "2022-11-28");

    if (!token_.empty())
      req.withHeader("Authorization", "Bearer " + token_);

    auto res = net::send(req);
    if (!res)
      return std::unexpected(res.error());

    if (!res->ok()) {
      return std::unexpected(
        Exception(ZCError::Unknown, "GitHub API error ({}): {}", res->status_code, res->body)
      );
    }

    return json::parse(res->body);
  }

  [[nodiscard]] static std::expected<json, Exception> getIndex()
  {
    static std::optional<json> cached_index;

    if (cached_index)
      return *cached_index;

    const stdfs::path index_path = tmp_dir() / zc_index_file;
    stdfs::create_directories(tmp_dir());

    const auto dl_res = net::download(std::string(zc_index_url), index_path);
    if (!dl_res)
      return std::unexpected(dl_res.error());

    auto json_res = fs::read_json(index_path);
    if (!json_res)
      return std::unexpected(json_res.error());

    cached_index = std::move(*json_res);
    return *cached_index;
  }
};

} // namespace zc::core
