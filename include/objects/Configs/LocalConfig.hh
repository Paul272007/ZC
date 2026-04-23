#pragma once

#include <filesystem>
#include <optional>

#include "Config.hh"
#include "objects/Version.hh"

enum class Type
{
  BIN,
  LIB,
  UNDEF
};

class LocalConfig : public Config
{
public:
  explicit LocalConfig(const std::filesystem::path &file);
  void write() const override;

  bool static_compile_;
  Type type_;
  std::string name_;
  std::string author_;
  std::string target_;
  std::optional<Version> version_;
  std::filesystem::path src_folder_;
  std::filesystem::path include_folder_;

protected:
  void load() override;
};
