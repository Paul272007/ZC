/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _CONF_H
#define _CONF_H

#include <filesystem>

class Conf
{
public:
  ~Conf();

protected:
  std::filesystem::path file_;
  bool modified_;

  Conf();

  /**
   * @param file
   */
  Conf(const std::filesystem::path &file);

  virtual void load() = 0;

  virtual void write() = 0;
};

#endif //_CONF_H
