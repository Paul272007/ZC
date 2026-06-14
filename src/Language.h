/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _LANGUAGE_H
#define _LANGUAGE_H

#include <nlohmann/json.hpp>
#include <string>

#include "helpers.h"

enum Language
{
  C,
  CXX,
  ASM_NASM,
  UNKNOWN_LANGUAGE
};

inline bool is_cxx(const std::string &text)
{
  if (const auto u_text = upper(text); u_text == "CXX" || u_text == "C++" || u_text == "CPP" || u_text == "CC")
    return true;
  return false;
}

inline void from_json(const nlohmann::json &j, Language &l)
{
  if (const std::string lang_upper = upper(j.get<std::string>()); is_cxx(lang_upper))
    l = CXX;
  else if (lang_upper == "C")
    l = C;
  else if (lang_upper == "ASM_NASM")
    l = ASM_NASM;
  else
    l = UNKNOWN_LANGUAGE;
}

inline void to_json(nlohmann::json &j, const Language &l)
{
  switch (l)
  {
  case C:
    j = "C";
    break;
  case CXX:
    j = "CXX";
    break;
  case ASM_NASM:
    j = "ASM_NASM";
    break;
  default:
    j = "UNKNOWN_LANGUAGE";
    break;
  }
}

#endif //_LANGUAGE_H
