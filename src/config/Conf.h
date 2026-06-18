/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

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
  const std::filesystem::path file_;
  const Interface &if_ = Interface::get();
  bool modified_ = false;

  /**
   * @param file
   */
  explicit Conf(std::filesystem::path file) : file_(std::move(file))
  {
  }

  virtual void load() = 0;

  virtual void write() = 0;
};

} // namespace zc
