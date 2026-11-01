#pragma once

#include "core/registry/target/RegistryTarget.hpp"
#include "core/util/directories.hpp"
#include "core/util/ZCError.hpp"
#include "framework/Exception.hpp"
#include "framework/fs.hpp"

namespace zc::core
{

struct RegistryBinTarget final : RegistryTarget {
  /// @brief Name of the binary executable
  std::string output;

  void writeToJson(json &j) const override
  {
    RegistryTarget::writeToJson(j);
    j["output"] = output;
  }

  void loadFromJson(const json &j) override
  {
    RegistryTarget::loadFromJson(j);
    GET_KEY(j, output);
  }

  [[nodiscard]] RegistryTargetType type() const override { return RegistryTargetType::Bin; }

  [[nodiscard]] std::expected<void, Exception>
  addSymlinks(const std::string_view /*pkg_name*/, const stdfs::path &snapshot_dir) const override
  {
    stdfs::create_directories(bin_links_dir());
    const stdfs::path bin_path = snapshot_dir / zc_bin_dir / output;
    if (!fs::file_exists(bin_path))
      return std::unexpected(Exception(FSError::FileNotFound, "expected binary {} not found", bin_path));

    const stdfs::path dest = bin_links_dir() / output;

    if (fs::symlink_exists(dest))
      return std::unexpected(Exception(ZCError::SymlinkAlreadyExists, "symlink {} already exists", dest));

    if (stdfs::exists(dest) || stdfs::is_symlink(dest))
      stdfs::remove(dest);

    stdfs::create_symlink(bin_path, dest);
    return {};
  }

  [[nodiscard]] std::expected<void, Exception>
  removeSymlinks(const std::string_view /*pkg_name*/) const override
  {
    const stdfs::path link_path = bin_links_dir() / output;
    if (stdfs::exists(link_path) || stdfs::is_symlink(link_path))
      stdfs::remove(link_path);
    return {};
  }

  [[nodiscard]] constexpr bool canBeADependency() const noexcept override { return false; }
};

} // namespace zc::core
