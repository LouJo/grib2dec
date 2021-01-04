#include "txt.hpp"

using namespace std;

namespace grib2dec_demo {

Txt::Txt(const string& filename) : Output(filename)
{
}

void Txt::setComponent(const G2DEC_Message& message)
{
    out << "Parameter id: " << message.parameter << endl;
}

void Txt::end()
{
}

}
