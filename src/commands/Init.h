#pragma once

#include "../pkgs/PkgType.h"
#include "Command.h"

namespace zc
{

class Init : public Command
{
public:
  Init(
      bool force, const std::filesystem::path &p_root, bool git, bool edit, const std::string &author,
      const std::string &target, const std::string &p_template, const std::string &name, bool is_bin,
      bool is_lib, bool is_header, bool is_compose
  );

  void operator()() override;

private:
  const std::filesystem::path p_root_;
  const bool git_;
  const bool edit_;
  std::string name_;
  std::string target_;
  std::string author_;
  std::string p_template_;
  PkgType type_;
};

} // namespace zc
