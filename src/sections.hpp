#ifndef __SECTIONS_HPP
#define __SECTIONS_HPP

#include "struct.hpp"
#include "stream.hpp"

namespace grib2dec {

void readIndicatorSection(Stream& stream, Message& message);
void readSection(Stream& stream, Message& message);

} // grib2dec

#endif
