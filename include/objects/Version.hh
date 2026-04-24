#pragma once

#include <string>
#include <tuple>

class Version
{
public:
  // TODO : add semver flags such as -alpha etc
  Version(int major, int minor, int patch);
  Version(const std::string &v);

  std::tuple<int, int, int> to_tuple() const;
  std::string string() const;

  bool operator==(const Version &other) const;
  bool operator!=(const Version &other) const;
  bool operator<(const Version &other) const;
  bool operator>(const Version &other) const;
  bool operator<=(const Version &other) const;
  bool operator>=(const Version &other) const;

  friend std::ostream &operator<<(std::ostream &os, const Version &v);

private:
  int major_ = 0;
  int minor_ = 0;
  int patch_ = 0;
};
