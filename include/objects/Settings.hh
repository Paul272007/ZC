#pragma once

#include <filesystem>
#include <string>

#include <helpers.hh>

class Settings
{
public:
  virtual ~Settings() = default;

  /**
   * @brief Write the current configuration into the configuration file
   */
  virtual void write() const = 0;

  const std::filesystem::path root_;
  const std::filesystem::path config_path_;
  std::string c_compiler_;
  std::string cpp_compiler_;
  std::string c_std_;
  std::string cpp_std_;
  bool add_std_;

protected:
  Settings(const std::filesystem::path &root) : root_(root), config_path_(root / ZC_FILE)
  {
  }

  /**
   * @brief Load the configuration file
   */
  virtual void load() = 0;
};
