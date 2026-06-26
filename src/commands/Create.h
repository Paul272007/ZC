#pragma once

#include "Command.h"

namespace zc
{

class Create : public Command
{
public:
  Create(
    bool force, bool edit, std::vector<std::string> &files, const std::vector<std::string> &input_files
  );

  void operator()() override;

private:
  const std::vector<std::filesystem::path> files_;
  const std::vector<std::filesystem::path> input_files_;

  const bool edit_;

  void write_declarations(const std::filesystem::path &f) const;
};

} // namespace zc
