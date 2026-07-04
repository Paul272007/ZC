#pragma once

#include <filesystem>
#include <vector>

#include "commands/Command.h"
#include "Context.h"
#include "pkgs/PkgType.h"
#include "templates/TemplateEngine.h"

namespace zc
{

class Init : public Command
{
public:
  Init(
    const CommandContext &ctx, std::filesystem::path p_root, bool git, bool edit, std::string author,
    std::string target, std::string p_template, std::string name, bool is_bin, bool is_lib, bool is_header,
    bool is_compose, const std::vector<std::string> &languages
  );

  void operator()() override;

private:
  const TemplateEngine &te_ = TemplateEngine::get();

  const std::filesystem::path p_root_;
  std::vector<Language>       languages_;

  const bool  git_;
  const bool  edit_;
  PkgType     type_;
  std::string name_;
  std::string target_;
  std::string author_;
  std::string p_template_;
};

} // namespace zc
