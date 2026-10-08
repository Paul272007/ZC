#include "ComponentsAdd.h"

#include <filesystem>
#include <utility>

#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

ComponentsAdd::ComponentsAdd(
  const ComponentsContext &ctx, bool edit, std::string name, std::string target, bool is_bin, bool is_lib,
  bool is_header
)
  : Components(ctx),
    edit_(edit),
    type_(
      parse_mode<PkgType>(
        {
          { PkgType::BIN, is_bin },
          { PkgType::LIB, is_lib },
          { PkgType::HEADER, is_header },
        },
        PkgType::UNDEF, "Project cannot have multiple types"
      )
    ),
    name_(std::move(name)),
    target_(std::move(target))
{
}

void ComponentsAdd::operator()()
{
  if (const auto comp_dir = p().root_dir / name_; fs::exists(comp_dir))
  {
    string msg;
    if (fs::is_directory(comp_dir))
      msg = "This component seems to already exist. Do you want to overwrite it ?";
    else
      msg = "The file " + comp_dir.string() + " already exists. Do you want to overwrite it ?";

    if (!force_ && !if_.ask(msg))
      throw ZCException(ZCE_ABORTED, "Project creation aborted.");

    zc::remove(p_root_ / ZC_FILE);
  }
}

} // namespace zc
