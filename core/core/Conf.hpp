#pragma once

#include "framework/util/types.hpp"

namespace zc::core
{

class Conf
{
public:
  Conf(const Conf &)            = delete;
  Conf(Conf &&)                 = default;
  Conf &operator=(const Conf &) = delete;
  Conf &operator=(Conf &&)      = delete;
  virtual ~Conf()               = default;

protected:
  /// @brief Path to config file
  const stdfs::path file_;
  /// @brief Indicates whether the configuration was modified and config file needs to be written to save changes
  bool modified_ = false;

  explicit Conf(stdfs::path file) : file_(std::move(file)) {}

  virtual void load()  = 0;
  virtual void write() = 0;
};

} // namespace zc::core
