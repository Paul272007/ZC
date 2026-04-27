#pragma once

#include <filesystem>
#include <vector>

#include "Controller.hh"
#include "objects/Configs/GlobalConfig.hh"
#include "objects/Table.hh"

class GlobalController : public Controller
{
public:
  GlobalController(Logger log, bool force);
  ~GlobalController() = default;

  void initializeWithTemplate(const std::filesystem::path &root, const std::string &template_to_use) const;
  void login();
  void logout();
  [[nodiscard]] std::vector<std::filesystem::path> getTemplates() const;
  [[nodiscard]] std::vector<std::filesystem::path> getProjectTemplates() const;
  [[nodiscard]] Table templatesTable() const;
  [[nodiscard]] Table projectTemplatesTable() const;

  GlobalConfig *gc_;

private:
  std::filesystem::path templates_dir_ = root_dir_ / TEMPLATES;
  std::filesystem::path p_templates_dir_ = root_dir_ / PROJECT_TEMPLATES;
};
