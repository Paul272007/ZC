#pragma once

#include <string>
#include <vector>

#include <helpers.hh>
#include <nlohmann/json.hpp>
#include <objects/Settings.hh>

class GlobalSettings : public Settings
{
public:
  /**
   * @brief Get an instance
   *
   * @return A GlobalSettings instance
   */
  static GlobalSettings &getInstance();

  void write() const override;

  bool clear_before_run_;
  bool auto_keep_;
  bool edit_on_init_;
  bool edit_on_create_;
  std::string editor_;
  std::vector<std::string> flags_;

protected:
  void load() override;

  GlobalSettings();
};
