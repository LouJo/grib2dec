#ifndef __UTILS_HPP
#define __UTILS_HPP

#include <stdint.h>

namespace regatta {

inline uint16_t bigendian(uint16_t v)
{
    if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        return __builtin_bswap16(v);
    else
        return v;
}

inline uint32_t bigendian(uint32_t v)
{
    if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        return __builtin_bswap32(v);
    else
        return v;
}

inline uint64_t bigendian(uint64_t v)
{
    if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        return __builtin_bswap64(v);
    else
        return v;
}

}

#endif