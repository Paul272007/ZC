#pragma once

#include <string>

#include "core/Conf.hpp"
#include "core/util/directories.hpp"

namespace zc::core
{

class GlobalConfig final : public Conf
{
  u32 jobs_count_ = 1;

  std::string c_compiler_;
  std::string cxx_compiler_;

  explicit GlobalConfig() : Conf(zc_dir() / zc_config_file) {}

protected:
  void load() override;
  void write() override;

public:
  [[nodiscard]] static GlobalConfig &get();

  [[nodiscard]] u32 jobs_count() const noexcept { return jobs_count_; }

  [[nodiscard]] std::string_view c_compiler() const noexcept { return c_compiler_; }

  [[nodiscard]] std::string_view cxx_compiler() const noexcept { return cxx_compiler_; }
};

} // namespace zc::core
