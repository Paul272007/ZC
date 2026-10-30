#pragma once

#include <string>
#include <string_view>

#include "commands/AppCommand.hpp"
#include "core/global-config/GlobalConfig.hpp"
#include "framework/cli/Args.hpp"

namespace zc::app
{

class Init final : public AppCommand<"init", "Create and initialize new project">
{
  ZC_FLAG(git, 'g', "Initialize git repository");
  ZC_FLAG(edit, 'E', "Open project in editor once initialized");

  ZC_OPTION(name, std::string_view, 'n', "New project", "");
  ZC_OPTION(author, std::string_view, 'a', "Set project author", "");
  ZC_OPTION(description, std::string_view, 'd', "Set project description", "");
  ZC_OPTION(template, std::string_view, 't', "Project template to use", "");
  ZC_LIST_OPTION(languages, std::string_view, 'l', "Set project languages");

  static std::map<std::string, core::LanguageConf> promptLanguages()
  {
    std::map<std::string, core::LanguageConf> selected_languages;

    std::vector<core::Language> available_languages;
    std::vector<bool>           selected(false, core::gc().languages().size());

    for (const auto &l : core::gc().languages() | std::views::keys)
      available_languages.push_back(l);

    ui.checkboxes(available_languages, selected, "Select languages to use:");

    for (u32 i = 0; i < selected.size(); i++)
      if (selected[i])
        selected_languages.insert_or_assign(
          std::string(to_str(available_languages[i]).value_or("Unknown")),
          core::gc().languages().at(available_languages.at(i))
        );

    return selected_languages;
  }

public:
  Init() = default;

  auto options() { return std::tie(git_, edit_, name_, author_, description_, template_, languages_); }

  void run() override
  {
    std::vector<core::Language> languages;
    for (const auto &str_l : *languages_)
      if (const auto l = core::language_from_str(str_l); l != core::Language::Unknown)
        languages.push_back(l);
      else
        throw_or_terminate(Exception(core::ZCError::InvalidValue, "Unknown language '{}'", str_l));

    const stdfs::path root_dir     = *project_path_;
    const stdfs::path project_file = root_dir / core::zc_project_file;
    if (!force_ && fs::file_exists(project_file)) {
      const bool res =
        ui.ask("A project seems to already exist in this directory. Do you want to overwrite it ?");
      if (!res)
        throw_or_terminate(Exception(CliError::Interrupted, "Project creation aborted."));
    }

    json root;

    root["name"]        = name_->empty() ? ui.input(root_dir.filename().string(), "Package name:") : *name_;
    root["author"]      = author_->empty() ? core::gc().username() : *author_;
    root["description"] = description_->empty() ? ui.input("", "Package description:") : *description_;

    if (languages.empty()) {
      root["languages"] = promptLanguages();
    } else {
      root["languages"] = json::object();
      for (const auto &l : languages)
        root["languages"][to_str(l).value_or("C")] = core::gc().languages().at(l);
    }

    auto write_res = fs::write_json(project_file, root);
    if (!write_res)
      throw_or_terminate(write_res.error());

    if (core::gc().alwaysInitGit() || git_)
      sh::exec({ "git", "init" });

    if (core::gc().openAfterInit() || edit_)
      sh::exec({ core::gc().editor(), root_dir.string() });
  }
};

} // namespace zc::app
