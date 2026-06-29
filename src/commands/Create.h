#pragma once

#include <map>
#include <vector>

#include "../clang_utils.h"
#include "../Language.h"
#include "../templates/TemplateEngine.h"
#include "Command.h"

namespace zc
{

class Create : public Command
{
public:
  Create(
    bool force, bool edit, const std::vector<std::string> &files,
    const std::vector<std::string> &input_files
  );

  void operator()() override;

private:
  TemplateEngine &te_ = TemplateEngine::get();

  std::map<Language, Declarations> declarations_;

  std::map<Language, std::vector<std::filesystem::path>> files_;
  std::map<Language, std::vector<std::filesystem::path>> input_files_;

  std::vector<std::string> files_to_edit_;

  const bool edit_;

  void get_declarations(Language l);
  void merge_declarations(const Declarations &src, Language dest);
  void write_declarations(const std::filesystem::path &f, Language l) const;
};

} // namespace zc
