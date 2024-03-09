#pragma once
#include "System.hpp"

namespace avlComms
{
    typedef struct STRUCT_ALIGNED NetPkgGps
    {
        uint32_t timestamp;
        uint32_t latitude;
        uint32_t longitude;
        uint16_t speed : 9;
        uint16_t hdop : 5;
        uint16_t gpsValid : 1;
        uint16_t isHistoric : 1;
    } NetPkgGps;
} // namespace avlComms