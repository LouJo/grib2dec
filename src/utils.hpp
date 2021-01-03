#ifndef __UTILS_HPP
#define __UTILS_HPP

#include "grib2dec/types.h"

#include <stdint.h>
#include <exception>
#include <string>

namespace grib2dec {

class file_open_error : public std::exception {
};

class parsing_error : public std::exception {
public:
    parsing_error(const std::string& msg,
                  G2DEC_Status status = G2DEC_STATUS_PARSE_ERROR,
                  const char *prefix = "parsing error") {
        errorMsg = std::string(prefix) + ": " + msg;
        errorStatus = status;
    }

    const char *what() const throw() {
        return errorMsg.c_str();
    }

    G2DEC_Status status() const {
        return errorStatus;
    }

private:
    std::string errorMsg;
    G2DEC_Status errorStatus;
};

class not_implemented : public parsing_error {
public:
    not_implemented(const std::string& msg)
        : parsing_error(msg, G2DEC_STATUS_NOT_IMPLEMENTED, "not implemented")
    {}
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
