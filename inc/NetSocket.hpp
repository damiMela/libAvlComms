#pragma once

#include <inttypes.h>
#include <netinet/in.h>
#include <vector>
#include <string>

#include "NetTypes.hpp"
#include "CResult.hpp"

namespace avlComms
{
    template <typename T>
    class NetSocketResult : public cutils::Result<T, net_errno_t>
    {
    public:
        NetSocketResult(const T &value) : cutils::Result<T, net_errno_t>(value) {}
        NetSocketResult(net_errno_t errorCode) : cutils::Result<T, net_errno_t>(errorCode) {}
    };

    class NetSocket
    {

    private:
        // socket
        int sockfd;
        sockaddr_in serverAddr;
        std::string ip;
        uint16_t port;

    public:
        NetSocket(int sockfd);
        NetSocket(std::string &ip, uint16_t port);
        ~NetSocket();
        net_errno_t Init();
        net_errno_t Connect();
        net_errno_t Disconnect();
        NetSocketResult<uint16_t> Read(std::vector<uint8_t> &buffer, uint16_t timeout = 0, bool peek = false);
        net_errno_t Send(void *buffer, const uint16_t msgLen);
        NetSocketResult<bool> ReadDataAvailable(uint16_t timeout = 0);

        int getSockFd() { return sockfd; }
    };

} // namespace avlComms
