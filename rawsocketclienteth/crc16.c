#include <stdio.h>
#include <stdint.h>
#include "crc16.h"


uint16_t crc16(
        unsigned char const *const buf,
        size_t const size)
{
    uint32_t crc = 0;
    uint16_t *buf16 = NULL;
    size_t i = 0;

    if (NULL != buf && 0 != size)
    {
        buf16 = (uint16_t *)buf;

        for (i = size; i > 1; i -= 2)
        {
            crc += *buf16;
            buf16++;
        }

        if (i > 0)
        {
            crc += (uint8_t)*buf16;
        }
        
        while ((crc >> 16) > 0)
        {
            crc = (crc >> 16) + (crc & 0xFFFF);
        }
    }
    else
    {
        return 0;
    }

    return ~((uint16_t)crc);
}
