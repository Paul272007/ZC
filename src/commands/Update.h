#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../helpers.h"
#include "../pkgs/Registry.h"
#include "Command.h"

namespace zc
{

class Update : public Command
{
public:
  Update(bool force, const std::string &path, std::vector<std::string> &targets);

  void operator()() override;

private:
  const std::filesystem::path path_;
  const Targets targets_;
  Registry &reg_ = Registry::get();
};

} // namespace zc
