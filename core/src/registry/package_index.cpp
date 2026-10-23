#include "core/registry/package_index.hpp"

#include "core/util/archive.hpp"
#include "core/util/crypto.hpp"
#include "core/util/GithubClient.hpp"
#include "core/util/json_helpers.hpp"

namespace zc::core::index
{

std::expected<RemotePkg, Exception> resolve(const std::string_view name, const Version version)
{
  auto index_res = GithubClient::getIndex();
  if (!index_res)
    return std::unexpected(index_res.error());

  const auto &root = *index_res;
  if (!jsonContainsObject(root, "packages"))
    return std::unexpected(Exception(ZCError::InvalidIndex, "Invalid package index format"));

  const auto &pkgs = root["packages"];
  if (!pkgs.contains(name))
    return std::unexpected(Exception(ZCError::PkgNotFound, "Package '{}' not found in remote index", name));

  const auto &pkg_json = pkgs[name];

  Version resolved_version;
  if (version.isEmpty() || version.isLatest()) {
    if (!pkg_json.contains("latest")) {
      return std::unexpected(
        Exception(ZCError::VersionNotFound, "No latest version specified for '{}'", name)
      );
    }
    resolved_version = pkg_json["latest"].get<std::string>();
  } else {
    resolved_version = version;
  }

  const std::string ver_str = resolved_version.string();
  if (!jsonContainsObject(pkg_json, "versions")) {
    return std::unexpected(
      Exception(ZCError::InvalidIndex, "Missing \"versions\" key for package '{}' in index", name)
    );
  }
  if (!pkg_json["versions"].contains(ver_str)) {
    return std::unexpected(
      Exception(ZCError::VersionNotFound, "Version '{}' not found for package '{}'", ver_str, name)
    );
  }

  const json       &ver_data = pkg_json["versions"][ver_str];
  const std::string url      = ver_data.value("url", "");
  const std::string sha256   = ver_data.value("sha256", "SKIP");

  if (url.empty())
    return std::unexpected(Exception(ZCError::MissingUrl, "Missing URL for {}@{}", name, ver_str));

  return RemotePkg{
    .name    = std::string(name),
    .url     = url,
    .sha256  = sha256,
    .version = resolved_version,
  };
}

std::expected<std::map<std::string, Version, std::less<>>, Exception> availablePkgs()
{
  auto index_res = GithubClient::getIndex();
  if (!index_res)
    return std::unexpected(index_res.error());

  const json &root = *index_res;
  if (!jsonContainsObject(root, "packages"))
    return std::unexpected(Exception(ZCError::InvalidIndex, "Invalid package index format"));

  std::map<std::string, Version, std::less<>> pkgs;

  for (const auto &[k, v] : root["packages"].items()) {
    if (!v.is_object()) {
      return std::unexpected(
        Exception(ZCError::VersionNotFound, "No latest version specified for '{}'", k)
      );
    }
    std::string latest;
    GET_KEY(v, latest);
    pkgs.insert_or_assign(k, latest);
  }
  return pkgs;
}

std::expected<stdfs::path, Exception> fetchAndExtract(const RemotePkg &pkg)
{
  const stdfs::path archive_path = tmp_dir() / std::format("{}-{}.tar.gz", pkg.name, pkg.version);
  stdfs::path       extract_dir  = tmp_dir() / std::format("{}-{}", pkg.name, pkg.version);

  const auto dl_res = net::download(pkg.url, archive_path);
  if (!dl_res)
    return std::unexpected(dl_res.error());

  if (pkg.sha256 != "SKIP") {
    auto hash_res = crypto::sha256(archive_path);
    if (!hash_res)
      return std::unexpected(hash_res.error());

    if (pkg.sha256 != *hash_res)
      return std::unexpected(
        Exception(ZCError::HashMismatch, "/!\\ Hash mismatch for {}@{}", pkg.name, pkg.version)
      );
  }

  stdfs::create_directories(extract_dir);
  const auto ext_res = archive::extract(archive_path, extract_dir);
  if (!ext_res)
    return std::unexpected(ext_res.error());

  for (const auto &entry : stdfs::directory_iterator(extract_dir))
    if (entry.is_directory())
      return entry.path();

  return extract_dir;
}

} // namespace zc::core::index
