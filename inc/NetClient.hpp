#pragma once
#include <queue>
#include <vector>

#include "CThread.hpp"
#include "CLock.hpp"

#include "NetConfiguration.hpp"
#include "NetTypes.hpp"
#include "NetPkgTypes.hpp"
#include "NetSocket.hpp"

#include "Logger/ClassLogger.hpp"

namespace avlComms
{
    class NetClient : protected cutils::CThread
    {
    private:
        typedef enum NetClientState
        {
            CLIENT_DISCONNECTED,
            CLIENT_CONNECTED,
            CLIENT_SEND,
            CLIENT_WAIT_ACK, // wait ack from server
            CLIENT_RCV_ACK,  // process ack from server
            CLIENT_RCV_PKG,  // process new msg from server
            CLIENT_SEND_ERROR,
            CLIENT_CONNECTION_ERROR,
            CLIENT_CLEAR_CONNECTION
        } NetClientState;

    private:
        // configuration
        NetConfiguration _config;

        NetSocket socket;
        // queue
        std::queue<NetPkgSend> messageQueue;
        cutils::CThreadLock queue_mtx;

        // state
        NetClientState state, prevState;
        bool cancelationFlag;
        bool connected;
        bool connectionRequested;
        bool disconnectionRequested;
        uint8_t txRetries;

        // buffer
        std::vector<uint8_t> rcvBuffer; // Adjust buffer size as needed
        uint16_t rcvBufferLen;
        inline void _ResetBuffer()
        {
            rcvBuffer.clear();
            rcvBufferLen = 0;
        }

        inline NetClientState _GetState() { return state; }
        inline NetClientState _GetPrevState() { return prevState; }
        inline void _SetState(NetClientState newState);

        // logger
        cutils::ClassLogger logger;

        // error
        void SetError(net_errno_t newError) { error = (error != SOCK_NO_ERROR) ? error : newError; };
        void ResetError() { error = SOCK_NO_ERROR; };

    protected:
        void _Run();

        inline bool _IsQueueEmpty();

        // reading methods
        net_recv_result_t _ReadAck();
        net_recv_result_t _ReadPacket();
        bool _ReadDataAvailable(uint16_t timeout = 5);
        bool _IsConnectionAlive(uint16_t timeout = 5);

        // writing methods
        net_send_result_t
        _SendFront();
        net_send_result_t _SendAck(NetPkgReceive &pkg, bool isNack = false);

    public:
        NetClient(const NetConfiguration &config, cutils::ILogger *logger);
        ~NetClient();
        bool Init();
        void RequestConnection() { connectionRequested = true; };
        void RequestDisconnection() { disconnectionRequested = true; };
        void EnqueueMessage(const uint8_t *message);
        bool IsConnected() { return connected; };

        net_errno_t error;
    };
};