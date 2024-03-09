#pragma once
#include "NetPkgGps.hpp"
namespace avlComms
{
    typedef struct STRUCT_ALIGNED NetPkgHeader
    {
        uint16_t termId;
        uint32_t paqSeq;
        uint32_t paqSize;
        uint16_t msgType;
        NetPkgGps gpsData;
        uint32_t msgFlags;
    } NetPkgHeader;

    typedef struct STRUCT_ALIGNED NetPkgSend
    {
        NetPkgHeader header;
        uint8_t *data;
        uint16_t Crc16;

    } NetPkgSend;

} // namespace avlComms