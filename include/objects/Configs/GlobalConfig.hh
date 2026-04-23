#pragma once

#include <filesystem>
#include <vector>

#include "Config.hh"

class GlobalConfig : public Config
{
public:
  explicit GlobalConfig(const std::filesystem::path &file);
  void write() const override;

  bool move_binary_to_current_path_;
  bool clear_before_run_;
  bool auto_keep_;
  bool edit_on_init_;
  bool edit_on_create_;
  std::string editor_;
  std::vector<std::string> flags_;

protected:
  void load() override;
};
