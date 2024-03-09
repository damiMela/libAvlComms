#include "NetSocket.hpp"

#include <fcntl.h> // For non-blocking socket
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>

using namespace avlComms;

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
net_errno_t NetSocket::Read(std::vector<uint8_t> &buffer, uint16_t &msgLen, uint16_t timeout, bool peek)
{
    msgLen = 0;
    uint8_t flags = peek ? MSG_PEEK : 0;
    std::vector<uint8_t> prevBuffer = buffer;
    ssize_t bytes = recv(sockfd, &buffer[0], buffer.size(), flags);
    if (peek)
        buffer = prevBuffer;
    if (bytes < 0)
    {
        return SOCK_RECV_ERROR;
    }

    if (bytes == 0)
        return SOCK_CLOSED_BY_HOST;

    return SOCK_NO_ERROR;
}

net_errno_t NetSocket::Send(void *buffer, const uint16_t msgLen)
{
    if (send(sockfd, buffer, msgLen, 0) < 0)
        return SOCK_SEND_ERROR;

    return SOCK_NO_ERROR;
}

net_errno_t NetSocket::ReadDataAvailable(bool &dataAvailable, uint16_t timeout)
{
    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    dataAvailable = false;

    int ret = poll(fds, 1, timeout); // timeout is in milliseconds
    if (ret < 0)
        return SOCK_POLL_ERROR;

    if (ret > 0 && fds[0].revents & POLLIN)
        dataAvailable = true;

    return SOCK_NO_ERROR;
}
