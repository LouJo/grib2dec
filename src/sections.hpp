#ifndef __SECTIONS_HPP
#define __SECTIONS_HPP

#include "struct.hpp"
#include "stream.hpp"

#include <vector>

namespace grib2dec {

void readIndicatorSection(Stream& stream, Message& message);
void readSection(Stream& stream, Message& message, vector<double>& values);

} // grib2dec

#endif
