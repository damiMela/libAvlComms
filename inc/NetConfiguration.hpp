#pragma once

#include <string>
#include <inttypes.h>
#define NET_DEFAULT_PKG_SIZE 512
#define NET_DEFAULT_TIMEOUT 1000
#define NET_DEFAULT_MAX_RETRIES 3
namespace avlComms
{

  typedef struct NetConfiguration
  {
    uint32_t termId;
    std::string ip;
    uint16_t port;
    uint16_t timeout;
    uint8_t maxRetries;
    uint16_t paqMaxSize;

    NetConfiguration(const uint32_t &termId, const std::string &ip, const uint16_t &port,
                     const uint16_t &timeout = NET_DEFAULT_TIMEOUT,
                     const uint8_t &maxRetries = NET_DEFAULT_MAX_RETRIES,
                     uint16_t PaqMaxSize = NET_DEFAULT_PKG_SIZE)

        : termId(termId), ip(ip), port(port), timeout(timeout),
          maxRetries(maxRetries), paqMaxSize(paqMaxSize)
    {
    }
  } NetConfiguration;
}
