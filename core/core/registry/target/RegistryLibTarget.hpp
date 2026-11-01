#pragma once

#include "core/util/directories.hpp"
#include "core/util/ZCError.hpp"
#include "framework/Exception.hpp"
#include "framework/fs.hpp"
#include "RegistryTarget.hpp"

namespace zc::core
{

struct RegistryLibTarget final : RegistryTarget {
  std::string static_output;
  std::string shared_output;

  void writeToJson(json &j) const override
  {
    RegistryTarget::writeToJson(j);
    j["static_output"] = static_output;
    j["shared_output"] = shared_output;
  }

  void loadFromJson(const json &j) override
  {
    RegistryTarget::loadFromJson(j);
    GET_KEY(j, static_output);
    GET_KEY(j, shared_output);
  }

  [[nodiscard]] RegistryTargetType type() const override { return RegistryTargetType::Lib; }

  [[nodiscard]] std::expected<void, Exception>
  addSymlinks(const std::string_view pkg_name, const stdfs::path &snapshot_dir) const override
  {
    stdfs::create_directories(lib_links_dir());

    for (const auto &out : { static_output, shared_output }) {
      const stdfs::path lib_path = snapshot_dir / zc_lib_dir / out;
      if (!fs::file_exists(lib_path))
        return std::unexpected(Exception(FSError::FileNotFound, "expected library {} not found", lib_path));

      const stdfs::path dest = lib_links_dir() / out;
      if (fs::symlink_exists(dest))
        return std::unexpected(Exception(ZCError::SymlinkAlreadyExists, "symlink {} already exists", dest));

      if (stdfs::exists(dest) || stdfs::is_symlink(dest))
        stdfs::remove(dest);

      stdfs::create_symlink(lib_path, dest);
    }

    return addHeaderSymlink(pkg_name, snapshot_dir);
  }

  [[nodiscard]] std::expected<void, Exception>
  removeSymlinks(const std::string_view pkg_name) const override
  {
    for (const auto &out : { static_output, shared_output }) {
      const stdfs::path link_path = lib_links_dir() / out;
      if (stdfs::exists(link_path) || stdfs::is_symlink(link_path))
        stdfs::remove(link_path);
    }
    return removeHeaderSymlink(pkg_name);
  }

  [[nodiscard]] constexpr bool canBeADependency() const noexcept override { return true; }
};

} // namespace zc::core
