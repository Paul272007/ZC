/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _VERSION_H
#define _VERSION_H

#include <nlohmann/json.hpp>
#include <string>

class Version
{
public:
  /**
   * @param text
   */
  Version(const std::string &text);

  /**
   * @param major
   * @param minor
   * @param patch
   */
  Version(int major = 0, int minor = 0, int patch = 0);

  [[nodiscard]] std::string string() const;

  [[nodiscard]] auto operator<=>(const Version &) const = default;

private:
  int major_;
  int minor_;
  int patch_;
};

inline void from_json(const nlohmann::json &j, Version &v)
{
  v = j.get<std::string>();
}

inline void to_json(nlohmann::json &j, const Version &v)
{
  j = v.string();
}

#endif //_VERSION_H
