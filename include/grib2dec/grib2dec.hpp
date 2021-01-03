#ifndef __GRIB2DEC_HPP
#define __GRIB2DEC_HPP

#include "types.h"

#include <istream>

namespace grib2dec {

class Grib2Dec {
public:
    virtual G2DEC_Status nextMessage(G2DEC_Message& message) = 0;

    virtual ~Grib2Dec() {}

    static Grib2Dec *create(const char *filename);
    static Grib2Dec *create(std::istream& fin);
};

} // grib2dec

#endif
