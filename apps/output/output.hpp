#ifndef __OUTPUT_HPP
#define __OUTPUT_HPP

#include "grib2dec/types.h"

#include <fstream>
#include <ostream>

namespace grib2dec_demo {

class Output {
public:
    Output();
    Output(const std::string& filename);

    virtual void setComponent(const G2DEC_Message& message) = 0;
    virtual void end() = 0;

    virtual ~Output() {}

    static Output *create(const std::string& filename,
                          const std::string& format);

protected:
    std::ostream& out;

private:
    std::ofstream fileOut;
};

} // gribdec-demo

#endif
