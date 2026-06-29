#include "MakeVariable.h"

#include <string>

#include "helpers.h"

ZC_DEV_CONFIG

namespace zc
{

MakeVariable::MakeVariable(std::string name) : name_(std::move(name)) {}

std::string MakeVariable::string() const
{
  return join(elts_, " ");
}

std::string MakeVariable::make_declaration() const
{
  return name_ + " = " + string() + "\n";
}

void MakeVariable::add(const std::string &value)
{
  elts_.push_back(esc(value));
}

void MakeVariable::add_no_esc(const std::string &value)
{
  elts_.push_back(value);
}

void MakeVariable::add_make_var(const std::string &key)
{
  elts_.push_back("$(" + key + ")");
}

} // namespace zc
