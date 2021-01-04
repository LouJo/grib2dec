#include "output.hpp"
#include "txt.hpp"

#include <iostream>

using namespace std;

namespace grib2dec_demo {

namespace {

class NullOutput : public Output {
public:
    NullOutput() {}
    void setComponent(const G2DEC_Message& message) {}
    void end() {}
};

} // local namespace

Output::Output()
    : out(cout)
{
}

Output::Output(const string& filename)
    : out(filename.empty() || filename == "-" ? cout : fileOut)
{
    if (!filename.empty())
        fileOut.open(filename);
}

Output *Output::create(const std::string& filename, const std::string& format)
{
    if (filename.empty() && format.empty())
        return new NullOutput();
    else
        // txt by default
        return new Txt(filename);
}

}
