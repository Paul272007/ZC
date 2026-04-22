#pragma once

#include <filesystem>
#include <map>
#include <vector>

#include "nlohmann/json_fwd.hpp"
#include "objects/Registry.hh"

using Declarations = std::map<std::string, std::vector<std::string>>;

void write(const std::filesystem::path &file, const std::string &content);

std::string read(const std::filesystem::path &file);

void writeDeclarations(const Declarations &decls, const std::filesystem::path &file);

std::unique_ptr<Declarations> parse(const std::filesystem::path &f);

bool isCpp(const std::filesystem::path &file);

std::vector<std::string>
getFileInclusions(const std::filesystem::path &file, const std::vector<Package> &pkgs);

[[nodiscard]] nlohmann::json parseJsonFile(const std::filesystem::path &file_path);

void writeJsonFile(const nlohmann::json &json, const std::filesystem::path &file_path);
