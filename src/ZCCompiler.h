#pragma once

#include "ui/Interface.h"
#include <string>
#include <vector>

class ZCCompiler
{
public:
  ZCCompiler(
      const std::string &compiler, const std::vector<std::string> &flags,
      const std::vector<std::string> &sources
  );
  void operator()();

private:
  const std::string compiler_;
  const std::vector<std::string> flags_;
  const std::vector<std::string> sources_;
  const zc::Interface &if_ = zc::Interface::get();
};
