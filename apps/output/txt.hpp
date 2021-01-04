#ifndef __OUTPUT_TXT_HPP
#define __OUTPUT_TXT_HPP

#include "output.hpp"

namespace grib2dec_demo {

class Txt : public Output {
public:
    Txt(const std::string& filename);

    void setComponent(const G2DEC_Message& message);
    void end();
};

} // gribdec-demo

#endif
