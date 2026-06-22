#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <utility>

#include "../ui/Interface.h"

namespace zc
{

class Conf
{
public:
  virtual ~Conf() = default;

protected:
  const Interface &if_ = Interface::get();

  const std::filesystem::path file_;

  bool modified_ = false;

  explicit Conf(std::filesystem::path file) : file_(std::move(file)) {}

  virtual void load() = 0;

  virtual void write() = 0;
};

} // namespace zc
