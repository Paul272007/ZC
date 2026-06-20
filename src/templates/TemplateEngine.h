/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <filesystem>
#include <vector>

#include "ui/Table.h"

namespace zc
{

class TemplateEngine
{
public:
  static TemplateEngine &get();
  TemplateEngine(const TemplateEngine &) = delete;
  void operator=(const TemplateEngine &) = delete;

  std::vector<std::filesystem::path> templates() const;

  std::vector<std::filesystem::path> p_templates() const;

  Table templates_table() const;

  Table p_templates_table() const;

  /**
   * @param root
   * @param p_template
   */
  void
  init_with_p_template(const std::filesystem::path &root, const std::string &p_template, bool force) const;

private:
  const std::filesystem::path templates_dir_;
  const std::filesystem::path p_templates_dir_;

  explicit TemplateEngine(const std::filesystem::path &root);
};

} // namespace zc
