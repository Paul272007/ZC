/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <string>
#include <vector>

#include "../CompileMode.h"
#include "../config/Dependency.h"
#include "../config/GConf.h"
#include "../pkgs/Registry.h"
#include "Command.h"

namespace zc
{

class Run : public Command
{
public:
  Run(bool force, const std::vector<std::string> &files, const std::vector<std::string> &args,
      bool preprocess, bool compile, bool assemble, bool plus, bool keep, bool add_std, bool static_link,
      bool no_flags);

  void operator()() override;

private:
  GConf &gc_ = GConf::get();
  Registry &rg_ = Registry::get();
  CompileMode mode_;
  const std::vector<std::filesystem::path> files_;
  const bool plus_ = false;
  const bool add_std_ = false;
  const bool keep_ = false;
  const bool static_ = false;
  const bool add_flags_ = true;
  const std::vector<std::string> args_;
  std::string output_name_;
  std::string build_cmd_;

  bool has_cpp();
  std::string get_build_command();
  std::string get_output_name();
  std::vector<Dependency> get_dependencies();
};

} // namespace zc
