#include "AvlNetClient.hpp"

#include <unistd.h>
#include <cerrno>

#include "Crc16.hpp"
#include "Logger/ClassLogger.hpp"

#define BUFFER_SIZE 512

using namespace cutils;
using namespace avlComms;

const char *stateNames[] = {
    "CLIENT_DISCONNECTED",
    "CLIENT_CONNECTED",
    "CLIENT_SEND",
    "CLIENT_WAIT_ACK", // wait ack from server
    "CLIENT_RCV_ACK",  // process ack from server
    "CLIENT_RCV_PKG",  // process new msg from server
    "CLIENT_SEND_ERROR",
    "CLIENT_CONNECTION_ERROR",
    "CLIENT_CLEAR_CONNECTION"};

AvlNetClient::AvlNetClient(const NetConfiguration &config, ILogger *logger) : _config(config),
                                                                              socket(_config.ip, _config.port),
                                                                              cancelationFlag(false),
                                                                              connected(false),
                                                                              connectionRequested(false),
                                                                              disconnectionRequested(false),
                                                                              rcvBuffer(BUFFER_SIZE),
                                                                              logger(logger, "netClient"),
                                                                              error(SOCK_NO_ERROR)

{
}

AvlNetClient::~AvlNetClient()
{
}

bool AvlNetClient::Init()
{
    net_errno_t ret = socket.Init();
    if (ret != SOCK_NO_ERROR)
    {
        SetError(ret);
        return false;
    }
    this->Start();
    return true;
}

void AvlNetClient::_SetState(NetClientState newState)
{
    if (newState == state)
        return;

    state = newState;
    logger.TRACE("state: %s", stateNames[state]);
};

bool AvlNetClient::_IsQueueEmpty()
{
    CScopeLock<CThreadLock> lock(queue_mtx);
    return messageQueue.empty();
}

net_send_result_t AvlNetClient::_SendFront()
{
    std::vector<uint8_t> message;
    CScopeLock<CThreadLock> lock(queue_mtx);

    if (messageQueue.empty())
        return SEND_NO_DATA;

    // message = messageQueue.front();
    net_errno_t res = socket.Send(message.data(), message.size());
    if (res != SOCK_NO_ERROR)
    {
        SetError(res);
        return SEND_FAILED;
    }
    return SEND_OK;
}

net_send_result_t AvlNetClient::_SendAck(NetPkgReceive &pkg, bool isNack)
{
    NetPkgAck ack;
    ack.Crc16 = 0x7070; // pkg.Crc16;
    ack.success = isNack;

    net_errno_t res = socket.Send(&ack, sizeof(NetPkgAck));
    if (res != SOCK_NO_ERROR)
    {
        SetError(res);
        return SEND_FAILED;
    }
    return SEND_OK;
}

net_recv_result_t AvlNetClient::_ReadAck()
{
    rcvBufferLen = 0;
    NetSocketResult<uint16_t> result = socket.Read(rcvBuffer, _config.timeout);
    if (result.hasValue())
    {
        rcvBufferLen = result.Value();
        return RECV_OK;
    }

    net_errno_t error = result.Error();
    if (error == SOCK_RECV_ERROR)
        return RECV_FAILED;

    if (error == SOCK_CLOSED_BY_HOST)
        return RECV_CLOSED_BY_HOST;

    if (false) // check crc
        return RECV_MSG_ERROR;

    return RECV_OK;
}

net_recv_result_t AvlNetClient::_ReadPacket()
{
    rcvBufferLen = 0;
    NetSocketResult<uint16_t> result = socket.Read(rcvBuffer, _config.timeout);

    if (result.hasValue())
    {
        rcvBufferLen = result.Value();
        return RECV_OK;
    }

    net_errno_t error = result.Error();
    if (error == SOCK_RECV_ERROR)
        return RECV_FAILED;

    if (error == SOCK_CLOSED_BY_HOST)
        return RECV_CLOSED_BY_HOST;

    if (false) // chaeck whatevs
        return RECV_MSG_ERROR;

    return RECV_OK;
}

void AvlNetClient::EnqueueMessage(void *message, uint16_t len)
{
    NetPkgSend pkg;
    pkg.header.termId = _config.termId;
    pkg.header.paqSize = len;

    pkg.data = new uint8_t[len];
    memset(pkg.data, 0, len);
    memcpy(pkg.data, message, len);
    pkg.Crc16 = CRC16::calculateCRC(&pkg, sizeof(NetPkgHeader) + len);

    CScopeLock<CThreadLock> lock(queue_mtx);
    messageQueue.push(pkg);
}

// ------------------------------ STATE MACHINE ------------------------------

void AvlNetClient::_Run()
{
    _SetState(CLIENT_DISCONNECTED);
    while (!cancelationFlag)
    {

        switch (_GetState())
        {
        case CLIENT_DISCONNECTED:
        {
            if (!connectionRequested)
                break;

            logger.DEBUG("connection Requested");
            net_errno_t ret = socket.Connect();
            if (ret == SOCK_NO_ERROR)
            {
                logger.DEBUG("Connected to server");
                connected = true;
                _SetState(CLIENT_CONNECTED);
            }
            else
            {
                logger.ERROR("internalError: %d. errno: %s", error, strerror(errno));
                SetError(ret);
            }

            connectionRequested = false;
            break;
        }

        case CLIENT_CONNECTED:
        {
            if (disconnectionRequested)
            {
                _SetState(CLIENT_CLEAR_CONNECTION);
                break;
            }

            // reading should be first. if not, the client could be always sending
            NetSocketResult<bool> result = socket.ReadDataAvailable();
            if (!result.hasValue())
            {
                SetError(result.Error());
                _SetState(CLIENT_CLEAR_CONNECTION);
            }
            else if (result.Value() == true)
                _SetState(CLIENT_RCV_PKG);

            else if (!_IsQueueEmpty())
                _SetState(CLIENT_SEND);

            break;
        }

        case CLIENT_SEND:
        {
            net_send_result_t res = _SendFront();
            if (res == SEND_OK)
                _SetState(CLIENT_RCV_ACK);
            else if (res == SEND_FAILED)
                _SetState(CLIENT_SEND_ERROR);
            else
                _SetState(CLIENT_CONNECTED);

            break;
        }

        case CLIENT_WAIT_ACK:
        {
            NetSocketResult<bool> result = socket.ReadDataAvailable();
            if (!result.hasValue() || result.Value() == false)
            {
                txRetries++;
                if (txRetries >= _config.maxRetries)
                    _SetState(CLIENT_SEND_ERROR);
                else
                    _SetState(CLIENT_SEND);
                break;
                _SetState(CLIENT_RCV_ACK);
            }
            break;
        }

        case CLIENT_RCV_ACK:
        {
            net_recv_result_t res = _ReadAck();
            if (res == RECV_OK)
            {
                txRetries = 0;
                CScopeLock<CThreadLock> lock(queue_mtx);
                messageQueue.pop();
                _SetState(CLIENT_CONNECTED);
            }

            else if (res == RECV_FAILED || res == RECV_MSG_ERROR)
                _SetState(CLIENT_SEND);

            else if (res == RECV_CLOSED_BY_HOST)
                _SetState(CLIENT_CONNECTION_ERROR);

            break;
        }

        case CLIENT_RCV_PKG:
        {
            net_recv_result_t res = _ReadPacket(); // load rcvBuffer with incoming data
            if (res == RECV_OK)
            {
                logger.DEBUG("Received data: %d", rcvBufferLen);
                NetPkgReceive receivePkg;
                bool pkgOk = NetPkgReceive::Parse(rcvBuffer, receivePkg);
                // TODO: handle message
                _ResetBuffer();
                _SendAck(receivePkg, !pkgOk);
                _SetState(CLIENT_CONNECTED);
            }

            else if (res == RECV_FAILED || res == RECV_MSG_ERROR)
                _SetState(CLIENT_SEND);

            else if (res == RECV_CLOSED_BY_HOST)
                _SetState(CLIENT_CONNECTION_ERROR);

            break;
        }

        case CLIENT_SEND_ERROR:
            // TODO: handle error
            _SetState(CLIENT_DISCONNECTED);
            break;

        case CLIENT_CONNECTION_ERROR:
            SetError(SOCK_CLOSED_BY_HOST);
            logger.DEBUG("Connection closed by host");
            _SetState(CLIENT_CLEAR_CONNECTION);
            break;

        case CLIENT_CLEAR_CONNECTION:
            socket.Disconnect();
            txRetries = 0;
            disconnectionRequested = false;
            connected = false;
            _SetState(CLIENT_DISCONNECTED);
            break;
        }
        usleep(100); // prevent thread from using all cpu resources
    }
}