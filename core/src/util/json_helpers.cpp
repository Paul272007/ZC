#include "core/util/json_helpers.hpp"

namespace zc::core
{

bool jsonContainsObject(const json &j, std::string_view key)
{
  return j.contains(key) && j.at(key).is_object();
}

} // namespace zc::core
