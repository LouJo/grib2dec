#ifndef __OUTPUT_SVG_HPP
#define __OUTPUT_SVG_HPP

#include "output.hpp"

#include <vector>

namespace grib2dec_demo {

class Svg : public Output {
public:
    Svg(const std::string& filename);

    void setComponent(const G2DEC_Message& message);
    void end();

private:
    std::vector<double> ptsx, ptsy;
    G2DEC_Grid grid;
};

} // gribdec-demo

#endif
