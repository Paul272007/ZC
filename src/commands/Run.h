#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "commands/BuildCommand.h"
#include "commands/Command.h"
#include "CompileMode.h"
#include "config/Dependency.h"
#include "Context.h"
#include "pkgs/Registry.h"
#include "ui/ShellCommand.h"

namespace zc
{

class Run : public Command, public BuildCommand
{
public:
  Run(
    const CommandContext &c_ctx, const BuildContext &b_ctx, const std::vector<std::string> &files,
    const std::vector<std::string> &args, bool preprocess, bool compile, bool assemble, bool plus,
    bool keep, bool add_std, bool static_link, bool no_flags, bool release
  );

  void operator()() override;

private:
  Registry &rg_ = Registry::get();

  const std::vector<std::filesystem::path> files_;
  const std::vector<std::string>           args_;

  const bool add_flags_;
  const bool add_std_;
  const bool plus_;
  const bool keep_;
  const bool static_;
  const bool release_;

  const CompileMode  mode_;
  const std::string  output_name_;
  const ShellCommand build_cmd_;

  void add_deps_to_cmd(ShellCommand &cmd) const;
  [[nodiscard]] bool has_cpp() const;
  [[nodiscard]] std::string get_output_name() const;
  [[nodiscard]] ShellCommand get_build_command() const;
  [[nodiscard]] std::vector<Dependency> get_dependencies() const;
};

} // namespace zc
