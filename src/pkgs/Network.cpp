/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include "Network.h"
#include "../helpers.h"

ZC_DEV_CONFIG

/**
 * Network implementation
 */

/**
 * @return Network
 */
Network &Network::get()
{
  static Network instance;
  return instance;
}

/**
 * @param url
 * @param payload
 * @return void
 */
void Network::post(const std::string &url, const std::string &payload)
{
  return;
}

/**
 * @param url
 * @param payload
 * @return void
 */
void Network::put(const std::string &url, const std::string &payload)
{
  return;
}

Network::~Network()
{
}

Network::Network()
{
}

/**
 * @param url
 * @param method
 * @param payload
 */
void Network::request(const std::string &url, const std::string &method, const std::string &payload)
{
}
