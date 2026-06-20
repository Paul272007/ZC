/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace zc
{

class Version
{
public:
  Version(const std::string &text);

  Version(int major = 0, int minor = 0, int patch = 1); // Default : first non-empty version

  [[nodiscard]] std::string string() const;

  [[nodiscard]] auto operator<=>(const Version &) const = default;

  bool empty() const;

private:
  int major_ = 0; // Initialize at 0 => if string is empty, so is the version
  int minor_ = 0;
  int patch_ = 0;
};

inline void from_json(const nlohmann::json &j, Version &v)
{
  v = j.get<std::string>();
}

inline void to_json(nlohmann::json &j, const Version &v)
{
  j = v.string();
}

} // namespace zc
