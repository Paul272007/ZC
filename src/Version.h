/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _VERSION_H
#define _VERSION_H

#include <string>

class Version
{
public:
  /**
   * @param text
   */
  Version(std::string text);

  /**
   * @param major
   * @param minor
   * @param patch
   */
  Version(int major, int minor, int patch);

  std::string string();

private:
  int major_;
  int minor_;
  int patch_;
};

#endif //_VERSION_H
