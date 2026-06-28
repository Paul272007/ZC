#pragma once

#include <string>
#include <vector>

#include "../CompileMode.h"
#include "../config/Dependency.h"
#include "../pkgs/Registry.h"
#include "Command.h"

namespace zc
{

class Run : public Command
{
public:
  Run(
    bool force, const std::vector<std::string> &files, const std::vector<std::string> &args,
    bool preprocess, bool compile, bool assemble, bool plus, bool keep, bool add_std, bool static_link,
    bool no_flags
  );

  void operator()() override;

private:
  Registry &rg_ = Registry::get();

  const std::vector<std::filesystem::path> files_;
  const std::vector<std::string>           args_;

  CompileMode mode_;
  const bool  add_flags_ = true;
  const bool  plus_      = false;
  const bool  add_std_   = false;
  const bool  keep_      = false;
  const bool  static_    = false;
  std::string output_name_;
  std::string build_cmd_;

  bool has_cpp() const;
  std::string get_build_command() const;
  std::string get_output_name() const;
  std::vector<Dependency> get_dependencies() const;
};

} // namespace zc
