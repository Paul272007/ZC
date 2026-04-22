#pragma once

#include "objects/Registry.hh"

class LocalRegistry : public Registry
{
public:
  explicit LocalRegistry(const std::filesystem::path &file);
};
