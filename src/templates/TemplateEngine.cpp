/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include "TemplateEngine.h"
#include "../helpers.h"

ZC_DEV_CONFIG

/**
 * TemplateEngine implementation
 */

TemplateEngine &TemplateEngine::get()
{
  static TemplateEngine instance;
  return instance;
}

/**
 * @return std::vector<std::filesystem::path>
 */
std::vector<std::filesystem::path> TemplateEngine::templates()
{
  return;
}

/**
 * @return std::vector<std::filesystem::path>
 */
std::vector<std::filesystem::path> TemplateEngine::p_templates()
{
  return;
}

/**
 * @param root
 * @param p_template
 * @return void
 */
void TemplateEngine::init_with_p_template(std::filesystem::path root, string p_template)
{
  return;
}

TemplateEngine::TemplateEngine()
{
}
