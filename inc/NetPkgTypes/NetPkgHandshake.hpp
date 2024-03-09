#pragma once
#include "System.hpp"

namespace avlComms
{
    typedef struct STRUCT_ALIGNED NetHandshake
    {
        uint16_t termId;
        uint16_t version;
        uint32_t serialNumber;
        uint16_t Crc16;
    } NetHandshake;

    typedef struct STRUCT_ALIGNED NetHandshakeAck
    {
        uint32_t paqSeq;
        uint16_t Crc16;
    } NetHandshakeAck;

} // namespace avlComms