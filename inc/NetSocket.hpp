#pragma once

#include <inttypes.h>
#include <netinet/in.h>
#include <vector>
#include <string>

#include "NetTypes.hpp"

namespace avlComms
{
    class NetSocket
    {
    private:
        // socket
        int sockfd;
        sockaddr_in serverAddr;
        std::string ip;
        uint16_t port;

    public:
        NetSocket(std::string &ip, uint16_t port);
        ~NetSocket();
        net_errno_t Init();
        net_errno_t Connect();
        net_errno_t Disconnect();
        net_errno_t Read(std::vector<uint8_t> &buffer, uint16_t &msgLen, uint16_t timeout = 0, bool peek = false);
        net_errno_t Send(void *buffer, const uint16_t msgLen);
        net_errno_t ReadDataAvailable(bool &dataAvailable, uint16_t timeout = 0);
    };

} // namespace avlComms
