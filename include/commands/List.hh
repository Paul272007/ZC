#pragma once

#include <filesystem>

#include <commands/Command.hh>
#include <objects/File.hh>
#include <objects/Registry.hh>
#include <objects/Table.hh>

class List : public Command
{
public:
  List(bool force, bool quiet, bool global, bool templates, bool p_templates);

  std::vector<File> getTemplates() const;
  Table templatesTable() const;

  std::vector<std::filesystem::path> getProjectTemplates() const;
  Table projectTemplatesTable() const;

  int operator()() override;

private:
  const Registry registry_;
  const bool templates_;
  const bool p_templates_;
};
