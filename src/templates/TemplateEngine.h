#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../Language.h"
#include "../ui/Interface.h"
#include "../ui/Table.h"

namespace zc
{

class TemplateEngine
{
public:
  static TemplateEngine &get();
  TemplateEngine(TemplateEngine &&)                 = delete;
  TemplateEngine &operator=(TemplateEngine &&)      = delete;
  TemplateEngine(const TemplateEngine &)            = delete;
  TemplateEngine &operator=(const TemplateEngine &) = delete;
  ~TemplateEngine()                                 = default;

  [[nodiscard]] const std::vector<std::string> &templates() const;
  [[nodiscard]] const std::vector<std::string> &p_templates() const;

  [[nodiscard]] Table templates_table() const;
  [[nodiscard]] Table p_templates_table() const;

  [[nodiscard]] bool init_with_template(const std::filesystem::path &file, Language l) const;

  void
  init_with_p_template(const std::filesystem::path &root, const std::string &p_template, bool force) const;

private:
  Interface &if_ = Interface::get();

  const std::filesystem::path templates_dir_;
  const std::filesystem::path p_templates_dir_;

  explicit TemplateEngine(const std::filesystem::path &root);
};

} // namespace zc
