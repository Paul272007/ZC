#pragma once

#include "objects/Registry.hh"

class GlobalRegistry : public Registry
{
public:
  explicit GlobalRegistry(const std::filesystem::path &file) : Registry(file)
  {
  }
};
