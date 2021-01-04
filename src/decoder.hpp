#ifndef __DECODER_HPP
#define __DECODER_HPP

#include "grib2dec/grib2dec.hpp"
#include "stream.hpp"

#include <fstream>
#include <vector>

namespace grib2dec {

class Decoder : public Grib2Dec {
public:
    Decoder(std::istream&);
    Decoder(const char *filename);

    virtual G2DEC_Status setSpatialFilter(const G2DEC_SpatialFilter& filter);
    virtual G2DEC_Status nextMessage(G2DEC_Message& message);

private:
    istream& fin;
    ifstream fileStream;
    size_t nextMessagePos = 0;
    bool ended = false;
    G2DEC_SpatialFilter spatialFilter;

    std::vector<double> values;
};

} // grib2dec

#endif
