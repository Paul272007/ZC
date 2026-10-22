#include "core/registry/target/RegistryTarget.hpp"

#include "core/registry/target/RegistryBinTarget.hpp"
#include "core/registry/target/RegistryHeaderTarget.hpp"
#include "core/registry/target/RegistryLibTarget.hpp"
#include "core/util/directories.hpp"
#include "core/util/ZCError.hpp"
#include "framework/Exception.hpp"
#include "framework/fs.hpp"

namespace zc::core
{

void RegistryTarget::writeToJson(json &j) const
{
  j["macros"]        = macros;
  j["internal_deps"] = internal_deps;
  j["external_deps"] = external_deps;
  j["type"]          = type();
}

void RegistryTarget::loadFromJson(const json &j)
{
  GET_OPT_KEY(j, macros);
  GET_OPT_KEY(j, internal_deps);
  GET_OPT_KEY(j, external_deps);
}

std::expected<void, Exception>
RegistryTarget::addHeaderSymlink(const std::string_view pkg_name, const stdfs::path &snapshot_dir) const
{
  const stdfs::path src = snapshot_dir / zc_include_dir / pkg_name / name;
  if (!fs::dir_exists(src))
    return std::unexpected(Exception(FSError::DirNotFound, "expected header directory {} not found", src));

  stdfs::create_directories(include_links_dir() / pkg_name);
  const stdfs::path dest = include_links_dir() / pkg_name / name;

  if (fs::symlink_exists(dest))
    return std::unexpected(Exception(ZCError::SymlinkAlreadyExists, "symlink {} already exists", dest));

  if (stdfs::exists(dest) || stdfs::is_symlink(dest))
    stdfs::remove(dest);

  stdfs::create_directory_symlink(src, dest);
  return {};
}

std::expected<void, Exception> RegistryTarget::removeHeaderSymlink(const std::string_view pkg_name) const
{
  const stdfs::path dest = include_links_dir() / pkg_name / name;
  if (stdfs::exists(dest) || stdfs::is_symlink(dest))
    stdfs::remove(dest);

  const stdfs::path pkg_inc_dir = include_links_dir() / pkg_name;
  std::error_code   ec;
  if (fs::dir_exists(pkg_inc_dir) && stdfs::is_empty(pkg_inc_dir, ec))
    stdfs::remove(pkg_inc_dir, ec);

  return {};
}

void to_json(json &j, const std::unique_ptr<RegistryTarget> &t)
{
  if (!t) {
    j = nullptr;
    return;
  }
  t->writeToJson(j);
}

void from_json(const json &j, std::unique_ptr<RegistryTarget> &t)
{
  auto type = RegistryTargetType::Undef;
  get_key(j, "type", type);

  switch (type) {
  case RegistryTargetType::Bin:
    t = std::make_unique<RegistryBinTarget>();
    break;
  case RegistryTargetType::Lib:
    t = std::make_unique<RegistryLibTarget>();
    break;
  case RegistryTargetType::Header:
    t = std::make_unique<RegistryHeaderTarget>();
    break;
  default:
    t = nullptr;
  }
  if (!t)
    return;

  t->loadFromJson(j);
}

} // namespace zc::core
