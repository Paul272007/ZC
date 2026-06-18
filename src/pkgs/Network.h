/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <filesystem>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace zc
{

class Network
{
public:
  [[nodiscard]] static Network &get();
  Network(const Network &) = delete;
  void operator=(const Network &) = delete;

  void download(const std::string &url, const std::filesystem::path &dest) const;

  /**
   * @param url
   * @param payload
   * @param token
   */
  std::string post(const std::string &url, const std::string &payload, const std::string &token = "");

  /**
   * @param url
   * @param payload
   * @param token
   */
  std::string put(const std::string &url, const std::string &payload, const std::string &token = "");

  /**
   * @param url
   * @param payload
   * @param token
   */
  std::string get(const std::string &url, const std::string &payload, const std::string &token = "");

  [[nodiscard]] const nlohmann::json &get_index() const;

  ~Network();

private:
  Network();

  /**
   * @param url
   * @param method
   * @param payload
   * @param token
   */
  std::string request(
      const std::string &url, const std::string &method, const std::string &payload,
      const std::string &token = ""
  );
};

} // namespace zc
