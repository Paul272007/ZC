/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _NETWORK_H
#define _NETWORK_H

#include <string>

class Network
{
public:
  static Network &get();

  /**
   * @param url
   * @param payload
   */
  void post(const std::string &url, const std::string &payload);

  /**
   * @param url
   * @param payload
   */
  void put(const std::string &url, const std::string &payload);

  ~Network();

private:
  Network();

  /**
   * @param url
   * @param method
   * @param payload
   */
  void request(const std::string &url, const std::string &method, const std::string &payload);
};

#endif //_NETWORK_H
