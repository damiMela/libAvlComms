#include "NetSocket.hpp"

#include <fcntl.h> // For non-blocking socket
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>

using namespace avlComms;

NetSocket::NetSocket(int sockfd) : sockfd(sockfd)
{
}

NetSocket::NetSocket(std::string &ip, uint16_t port) : ip(ip), port(port)
{
}

NetSocket::~NetSocket()
{
}

net_errno_t NetSocket::Init()
{
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0)
        return SOCK_INET_ERROR;

    return SOCK_NO_ERROR;
}

net_errno_t NetSocket::Connect()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        return SOCK_CREATE_ERROR;
    }

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
    {
        close(sockfd);
        return SOCK_CONFIG_ERROR;
    }

    int result = connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (result < 0 && errno != EINPROGRESS)
    {
        close(sockfd);
        return SOCK_CONNECT_ERROR;
    }

    return SOCK_NO_ERROR;
}

net_errno_t NetSocket::Disconnect()
{
    close(sockfd);
    sockfd = 0;
    return SOCK_NO_ERROR;
}
#include <iostream>
#include <string.h>
NetSocketResult<uint16_t> NetSocket::Read(std::vector<uint8_t> &buffer, uint16_t timeout, bool peek)
{
    uint8_t flags = peek ? MSG_PEEK : 0;
    ssize_t bytes = recv(sockfd, &buffer[0], buffer.capacity(), flags);
    if (bytes < 0)
    {
        return NetSocketResult<uint16_t>(SOCK_RECV_ERROR);
    }

    if (bytes == 0)
        return NetSocketResult<uint16_t>(SOCK_CLOSED_BY_HOST);

    return NetSocketResult<uint16_t>(bytes);
}

net_errno_t NetSocket::Send(void *buffer, const uint16_t msgLen)
{
    if (send(sockfd, buffer, msgLen, 0) < 0)
        return SOCK_SEND_ERROR;

    return SOCK_NO_ERROR;
}

NetSocketResult<bool> NetSocket::ReadDataAvailable(uint16_t timeout)
{
    struct pollfd readfds;
    readfds.fd = sockfd;
    readfds.events = POLLIN;

    int retval = poll(&readfds, 1, timeout);
    if (retval == -1)
        return NetSocketResult<bool>(SOCK_POLL_ERROR);

    if (retval == 0)
        return NetSocketResult<bool>(false);

    // Data available to read
    char buf;
    int bytesRead = recv(sockfd, &buf, 1, MSG_PEEK);
    if (bytesRead == -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return NetSocketResult<bool>(false);
        else
            return NetSocketResult<bool>(SOCK_RECV_ERROR);
    }
    else if (bytesRead == 0)
        return NetSocketResult<bool>(SOCK_CLOSED_BY_HOST);
    else
        return NetSocketResult<bool>(true);
}
