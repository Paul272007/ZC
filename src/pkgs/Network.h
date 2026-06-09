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
  void post(std::string url, std::string payload);

  /**
   * @param url
   * @param payload
   */
  void put(std::string url, std::string payload);

  ~Network();

private:
  Network();

  /**
   * @param url
   * @param method
   * @param payload
   */
  void request(std::string url, std::string method, std::string payload);
};

#endif //_NETWORK_H
