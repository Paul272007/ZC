/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _GCONF_H
#define _GCONF_H

#include <string>
#include <vector>

#include "Conf.h"

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

protected:
  void load();

  void write();

private:
  GConf();
};

#endif //_GCONF_H
