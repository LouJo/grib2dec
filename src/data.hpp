#ifndef __DATA_HPP
#define __DATA_HPP

#include "stream.hpp"
#include "struct.hpp"

#include <vector>

namespace grib2dec {

void readData(Stream& stream, Message& message, vector<double>& values);

} // grib2dec

#endif
