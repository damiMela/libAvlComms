#pragma once

#include "System.hpp"
#include <string.h>
#include <Crc16.hpp>

namespace avlComms
{
    typedef struct STRUCT_ALIGNED NetPkgReceive
    {
        uint32_t paqSize;
        uint16_t msgType;
        uint8_t *data;
        uint16_t Crc16;

        static bool Parse(std::vector<uint8_t> &buffer, NetPkgReceive &pkg)
        {
            const uint8_t dataSize = sizeof(NetPkgReceive) - sizeof(uint8_t *);

            if (buffer.size() < dataSize)
                return false;

            pkg.paqSize = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
            pkg.msgType = buffer[4] | (buffer[5] << 8);
            pkg.Crc16 = buffer[buffer.size() - 2] | (buffer[buffer.size() - 1] << 8);

            uint16_t calcCrc = CRC16::calculateCRC(buffer.data(), buffer.size() - 2);
            if (calcCrc != pkg.Crc16)
                return false;

            if (pkg.data != NULL)
                delete[] pkg.data;

            if (pkg.paqSize - dataSize == 0)
                return true;

            pkg.data = new uint8_t[pkg.paqSize - dataSize];
            memcpy(pkg.data, &buffer[6], pkg.paqSize);
            return true;
        }
    } NetPkgReceive;

} // namespace avlComms