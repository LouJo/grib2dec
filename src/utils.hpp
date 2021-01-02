#ifndef __UTILS_HPP
#define __UTILS_HPP

#include <stdint.h>
#include <exception>
#include <string>

namespace grib2dec {

class parsing_error : public std::exception {
public:
    parsing_error(const std::string& msg) {
        errorMsg = std::string("parsing error: ") + msg;
    }

    const char *what() const throw() {
        return errorMsg.c_str();
    }

private:
    std::string errorMsg;
};

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

inline uint16_t len16(const char *data)
{
    return bigendian(*((uint16_t*)data));
}

inline uint32_t len32(const char *data)
{
    return bigendian(*((uint32_t*)data));
}

inline uint64_t len64(const char *data)
{
    return bigendian(*((uint64_t*)data));
}


}

#endif
