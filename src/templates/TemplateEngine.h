/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _TEMPLATEENGINE_H
#define _TEMPLATEENGINE_H

#include <filesystem>
#include <vector>

class TemplateEngine
{
public:
  static TemplateEngine &get();

  std::vector<std::filesystem::path> templates();

  std::vector<std::filesystem::path> p_templates();

  /**
   * @param root
   * @param p_template
   */
  void init_with_p_template(const std::filesystem::path &root, const std::string &p_template);

private:
  const std::filesystem::path templates_dir_;
  const std::filesystem::path p_templates_dir;

  TemplateEngine();
};

#endif //_TEMPLATEENGINE_H
