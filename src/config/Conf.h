#pragma once

#include <filesystem>
#include <utility>

namespace zc
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
  const std::filesystem::path file_;

  bool modified_ = false;

  explicit Conf(std::filesystem::path file) : file_(std::move(file)) {}

  virtual void load()  = 0;
  virtual void write() = 0;
};

} // namespace zc
