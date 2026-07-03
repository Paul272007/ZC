#pragma once

#include <filesystem>

#include "commands/ProjectCommand.h"
#include "pkgs/Registry.h"
#include "templates/TemplateEngine.h"

namespace zc
{

enum class ListType : uint8_t
{
  SHOW_PKGS,
  SHOW_DEPS,
  SHOW_REMOTE,
  SHOW_TEMPLATES,
  SHOW_P_TEMPLATES,
};

class List : public ProjectCommand
{
public:
  List(
    bool force, const std::filesystem::path &p_root, bool deps, bool templates, bool p_templates,
    bool remote, bool simple
  );

  void operator()() override;

private:
  Registry       &reg_ = Registry::get();
  TemplateEngine &te_  = TemplateEngine::get();

  ListType   type_;
  const bool simple_;
};

} // namespace zc
