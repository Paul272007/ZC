#pragma once

#include "commands/Command.hh"
#include "objects/Controllers/GlobalController.hh"

class List : public Command
{
public:
  List(
      bool force, bool quiet, bool global, bool templates, bool p_templates, bool simple, bool remote,
      const std::string &path
  );

  int operator()() override;

private:
  const bool global_;
  const bool templates_;
  const bool p_templates_;
  const bool simple_;
  const bool remote_;
  const std::string path_;
  GlobalController g_;
};
