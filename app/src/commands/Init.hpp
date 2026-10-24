#pragma once

#include "Command.hpp"
#include "core/global-config/GlobalConfig.hpp"

namespace zc::app
{

class Init final : public Command
{
  const bool git_;
  const bool edit_;

  stdfs::path      root_dir_;
  std::string_view name_;
  std::string_view author_;
  std::string_view description_;
  std::string_view project_template_;

  std::vector<core::Language> languages_;

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

  // std::string chooseProjectTemplate()
  //{
  // // TODO: implement
  //}

public:
  explicit Init(
    const CommandCtx &cmd_ctx, const bool git, const bool edit, const std::string_view name,
    const std::string_view author, const std::string_view description,
    const std::string_view project_template, const std::vector<std::string_view> &languages
  )
    : Command(cmd_ctx, false),
      git_(git),
      edit_(edit),
      root_dir_(cmd_ctx.project_path),
      name_(name),
      author_(author),
      description_(description),
      project_template_(project_template)
  {
    for (const auto &str_l : languages)
      if (const auto l = core::language_from_str(str_l); l != core::Language::Unknown)
        languages_.push_back(l);
      else
        throw_or_terminate(Exception(core::ZCError::InvalidValue, "Unknown language '{}'", str_l));
  }

  void execute() override
  {
    const stdfs::path project_file = root_dir_ / core::zc_project_file;
    if (!force_ && fs::file_exists(project_file)) {
      const bool res =
        ui.ask("A project seems to already exist in this directory. Do you want to overwrite it ?");
      if (!res)
        throw_or_terminate(Exception(CliError::Interrupted, "Project creation aborted."));
    }

    json root;

    root["name"]        = name_.empty() ? ui.input(root_dir_.filename().string(), "Package name:") : name_;
    root["author"]      = author_.empty() ? core::gc().username() : author_;
    root["description"] = description_.empty() ? ui.input("", "Package description:") : description_;

    if (languages_.empty()) {
      root["languages"] = promptLanguages();
    } else {
      root["languages"] = json::object();
      for (const auto &l : languages_)
        root["languages"][to_str(l).value_or("C")] = core::gc().languages().at(l);
    }

    // TODO: verify that name is valid

    auto write_res = fs::write_json(project_file, root);
    if (!write_res)
      throw_or_terminate(write_res.error());

    // TODO: initialize with template
    // TODO: allow single target project creation using zc init

    if (core::gc().alwaysInitGit() || git_)
      sh::exec({ "git", "init" });

    if (core::gc().openAfterInit() || edit_)
      sh::exec({ core::gc().editor(), root_dir_.string() });
  }
};

} // namespace zc::app
