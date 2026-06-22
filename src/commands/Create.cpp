#include "Create.h"

#include "commands/Command.h"
#include "helpers.h"

ZC_DEV_CONFIG

namespace zc
{

Create::Create(
  const bool force, const bool edit, std::vector<std::string> &files,
  const std::vector<std::string> &input_files
)
  : Command(force), edit_(edit), files_(str_to_path(files)), input_files_(str_to_path(input_files))
{
}

void Create::operator()() {}

} // namespace zc
