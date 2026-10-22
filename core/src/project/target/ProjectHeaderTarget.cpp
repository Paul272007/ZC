#include "core/project/target/ProjectHeaderTarget.hpp"

#include "core/project/Project.hpp"
#include "core/registry/target/RegistryHeaderTarget.hpp"

namespace zc::core
{

std::expected<void, Exception> ProjectHeaderTarget::install() const
{
  return installHeaders();
}

std::vector<std::string> ProjectHeaderTarget::generateBuildStatements() const
{
  return {};
}

std::vector<stdfs::path> ProjectHeaderTarget::outputFiles() const
{
  return {};
}

std::unique_ptr<RegistryTarget> ProjectHeaderTarget::whenInstalled() const
{
  auto target = std::make_unique<RegistryHeaderTarget>();
  fillRegistryTarget(*target);
  return target;
}

} // namespace zc::core
