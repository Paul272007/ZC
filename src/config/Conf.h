/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <utility>

#include "../excepts/ZCException.h"
#include "../ui/Interface.h"

namespace zc
{

class Conf
{
public:
  virtual ~Conf() = default;

  template <typename T>
  static void get_key(const nlohmann::json &json_conf, const std::string &key, T &variable)
  {
    if (!json_conf.contains(key))
      throw ZCException(ZCE_MISSING_PROPERTY, "Expected property '" + key + "' missing.");

    try
    {
      json_conf.at(key).get_to(variable);
    }
    catch ([[maybe_unused]] const nlohmann::json::type_error &_)
    {
      throw ZCException(ZCE_TYPE_ERROR, "Configuration error: key '" + key + "' has the wrong type");
    }
  }

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
