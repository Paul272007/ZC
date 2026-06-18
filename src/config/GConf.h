/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <string>
#include <vector>

#include "Conf.h"

namespace zc
{

class GConf : public Conf
{
public:
  bool alway_keep = false;
  bool always_add_std = false;
  bool open_after_init = false;
  bool open_after_create = false;
  bool clear_before_run = false;
  bool move_bin_to_current_path = false;
  std::string token;
  std::string username;
  std::string c_compiler = "clang";
  std::string cxx_compiler = "clang++";
  std::string c_std = "c23";
  std::string cxx_std = "c++20";
  std::string editor = "vim";
  std::vector<std::string> flags = {"-Wall", "-Wextra"};

  static GConf &get();

  void login();

  void logout();

  ~GConf() override;

protected:
  void load() override;

  void write() override;

private:
  GConf();
};

} // namespace zc
