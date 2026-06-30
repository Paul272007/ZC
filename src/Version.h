#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace zc
{

class Version
{
public:
  Version(const std::string &text);
  Version(int major = 0, int minor = 0, int patch = 0);

  [[nodiscard]] auto operator<=>(const Version &) const = default;

  [[nodiscard]] std::string string() const;

  [[nodiscard]] bool empty() const;

  [[nodiscard]] int major() const { return major_; }

  [[nodiscard]] int minor() const { return minor_; }

  [[nodiscard]] int patch() const { return patch_; }

private:
  int major_ = 0;
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
