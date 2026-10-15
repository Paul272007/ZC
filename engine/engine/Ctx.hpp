#pragma once

#include <unordered_map>
#include <variant>
#include <vector>

#include "framework/util/concepts.hpp"
#include "framework/util/helpers.hpp"

namespace zc::engine
{

class Ctx
{
  using Value = std::variant<std::string, std::vector<std::string>>;

  std::unordered_map<std::string, Value> vars_;

public:
  Ctx() = default;

  Ctx(std::initializer_list<std::pair<std::string, std::string>> vars)
  {
    for (const auto &[k, v] : vars)
      vars_.emplace(k, v);
  }

  template<Formattable T>
  void set(std::string key, T value)
  {
    if constexpr (std::is_same_v<T, std::string>)
      vars_[std::move(key)] = std::move(value);
    else if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, const char *>)
      vars_[std::move(key)] = std::string(value);
    else
      vars_[std::move(key)] = std::format("{}", value);
  }

  template<std::ranges::input_range Range>
  requires Formattable<std::ranges::range_value_t<Range>>
  void setList(std::string key, const Range &range)
  {
    std::vector<std::string> list;
    for (const auto &item : range)
      if constexpr (std::is_same_v<std::ranges::range_value_t<Range>, std::string>)
        list.push_back(item);
      else
        list.push_back(std::format("{}", item));
    vars_[std::move(key)] = std::move(list);
  }

  [[nodiscard]] std::string_view get(const std::string_view key) const
  {
    if (const auto it = vars_.find(std::string(key));
        it != vars_.end() && std::holds_alternative<std::string>(it->second))
      return std::get<std::string>(it->second);
    return "";
  }

  [[nodiscard]] const std::vector<std::string> *getList(const std::string_view key) const
  {
    if (const auto it = vars_.find(std::string(key));
        it != vars_.end() && std::holds_alternative<std::vector<std::string>>(it->second))
      return &std::get<std::vector<std::string>>(it->second);
    return nullptr;
  }

  [[nodiscard]] bool is_truthy(const std::string_view key) const
  {
    const auto it = vars_.find(std::string(key));
    if (it == vars_.end())
      return false;
    if (std::holds_alternative<std::string>(it->second)) {
      const auto &s = std::get<std::string>(it->second);
      return !s.empty() && !equals_any(s, "0", "false", "no", "off");
    }
    return !std::get<std::vector<std::string>>(it->second).empty();
  }

  // TODO: add useDate(format) to automatically add "date" to the keys with given format
};

} // namespace zc::engine
