#pragma once

namespace avlComms
{
    typedef enum __net_errno_t__
    {
        SOCK_NO_ERROR,
        SOCK_INET_ERROR,
        SOCK_CREATE_ERROR,
        SOCK_CONFIG_ERROR,
        SOCK_CONNECT_ERROR,
        SOCK_POLL_ERROR,
        SOCK_SEND_ERROR,
        SOCK_RECV_ERROR,
        SOCK_CLOSED_BY_HOST

    } net_errno_t;

    typedef enum __net_send_result_t__
    {
        SEND_OK,
        SEND_NO_DATA,
        SEND_FAILED
    } net_send_result_t;

    typedef enum __net_recv_result_t__
    {
        RECV_OK,
        RECV_FAILED,
        RECV_CLOSED_BY_HOST,
        RECV_MSG_ERROR
    } net_recv_result_t;

} // namespace avlComms