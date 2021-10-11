#ifndef _CRC16_H_
#define _CRC16_H_

#include <stdint.h>


uint16_t crc16(
        unsigned char const *buf,
        size_t size);

#endif
