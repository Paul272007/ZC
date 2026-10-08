#pragma once

#include <filesystem>
#include <string>

#include "commands/Components/Components.h"
#include "Context.h"
#include "pkgs/PkgType.h"

namespace zc
{

class ComponentsAdd : public Components
{
public:
  ComponentsAdd(
    const ComponentsContext &ctx, bool edit, std::string name, std::string target, bool is_bin, bool is_lib,
    bool is_header
    // std::string p_template, // add component templates
  );

  void operator()() override;

private:
  const std::filesystem::path p_root_;

  const bool  edit_;
  PkgType     type_;
  std::string name_;
  std::string target_;
  std::string author_;
  std::string p_template_;
};

} // namespace zc
