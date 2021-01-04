#ifndef __GRIB2DEC_HPP
#define __GRIB2DEC_HPP

#include "types.h"

#include <istream>

namespace grib2dec {

class Grib2Dec {
public:
    /**
     * Read next message.
     *
     * message structure will be fullfilled only if returned status is OK.
     * call this method until END status is returned.
     */
    virtual G2DEC_Status nextMessage(G2DEC_Message& message) = 0;

    virtual ~Grib2Dec() {}

    /**
     * Create a grib2 decoder with a filename.
     *
     * if file cannot be open, nullptr is returned.
     */
    static Grib2Dec *create(const char *filename);

    /**
     * Create a grib2 decoder with an input stream.
     */
    static Grib2Dec *create(std::istream& fin);
};

} // grib2dec

#endif
