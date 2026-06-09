/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <filesystem>

#include "helpers.h"

ZC_DEV_CONFIG

const fs::path get_zc_root()
{
  return fs::path();
}

const std::filesystem::path get_project_root(const std::filesystem::path &base)
{
  return fs::path();
}
