#pragma once

#include <filesystem>
#include <optional>
#include <string>

class Config
{
public:
  virtual ~Config() = default;
  virtual void write() const = 0;

  std::optional<bool> add_std_;
  std::string c_compiler_;
  std::string cpp_compiler_;
  std::optional<std::string> c_std_;
  std::optional<std::string> cpp_std_;

protected:
  explicit Config(const std::filesystem::path &file) : file_(file)
  {
  }
  virtual void load() = 0;

  const std::filesystem::path file_;
};
