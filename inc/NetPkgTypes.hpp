#pragma once

#include "NetPkgTypes/NetPkgHandshake.hpp"
#include "NetPkgTypes/NetPkgSend.hpp"
#include "NetPkgTypes/NetPkgReceive.hpp"

namespace avlComms
{
    typedef struct STRUCT_ALIGNED NetPkgAck
    {
        uint8_t success;
        uint16_t Crc16;
    } NetPkgAck;

}
