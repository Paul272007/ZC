#pragma once

#include <string>
#include <vector>

namespace zc
{

class MakeVariable
{
public:
  MakeVariable(const std::string &name);
  [[nodiscard]] std::string string() const;
  std::string make_declaration() const;
  void add(const std::string &value);
  void add_no_esc(const std::string &value);
  void add_make_var(const std::string &key);
  friend struct MakeVariableCmp;

private:
  std::string              name_;
  std::vector<std::string> elts_;
};

struct MakeVariableCmp
{
  using is_transparent = void; // activate heterogen search

  bool operator()(const MakeVariable &a, const MakeVariable &b) const { return a.name_ < b.name_; }

  bool operator()(const MakeVariable &a, const std::string &name) const { return a.name_ < name; }

  bool operator()(const std::string &name, const MakeVariable &b) const { return name < b.name_; }
};

} // namespace zc
