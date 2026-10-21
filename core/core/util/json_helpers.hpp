#pragma once

#include "framework/Exception.hpp"
#include "framework/util/types.hpp"
#include "ZCError.hpp"

#define GET_KEY(root, var)                             get_key(root, #var, var)
#define GET_OPT_KEY(root, var)                         get_optional_key(root, #var, var)
#define GET_KEY_WITH_DEFAULT(root, var, default_value) get_key_with_default(root, #var, var, default_value)

namespace zc::core
{

// template<typename T>
// concept HasEmpty = requires(const T &t) {
// { t.empty() } -> std::convertible_to<bool>;
//};

bool jsonContainsObject(const json &j, std::string_view key);
bool jsonContainsArray(const json &j, std::string_view key);

template<typename T>
void get_key(const json &json_conf, const std::string &key, T &variable)
{
  if (!json_conf.contains(key))
    throw_or_terminate(Exception(ZCError::MissingKey, "Expected key '{}' missing.", key));

  try {
    json_conf.at(key).get_to(variable);
  } catch (const json::type_error &) {
    throw_or_terminate(Exception(ZCError::TypeError, "Key '{}' has the wrong type", key));
  }
}

template<typename T>
void get_optional_key(const json &json_conf, std::string_view key, T &variable)
{
  if (!json_conf.contains(key))
    return;

  try {
    json_conf.at(key).get_to(variable);
  } catch (const json::type_error &) {
    throw_or_terminate(Exception(ZCError::TypeError, "Key '{}' has the wrong type", key));
  }
}

template<typename T>
void get_key_with_default(const json &json_conf, std::string_view key, T &variable, T default_value)
{
  if (!json_conf.contains(key)) {
    variable = default_value;
    return;
  }
  try {
    json_conf.at(key).get_to(variable);
  } catch (const json::type_error &) {
    throw_or_terminate(Exception(ZCError::TypeError, "Key '{}' has the wrong type", key));
  }
}

} // namespace zc::core
