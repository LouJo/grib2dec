#include "txt.hpp"

using namespace std;

namespace grib2dec_demo {

Txt::Txt(const string& filename) : Output(filename)
{
}

void Txt::setComponent(const G2DEC_Message& message)
{
    const G2DEC_Grid& grid = message.grid;

    out << "Parameter id: " << message.parameter << endl;
    out << "Latitude: [" << grid.lat1 << ", " << grid.lat2 << ", " << grid.latInc << "]"
        << ", Longitude: [" << grid.lon1 << ", " << grid.lon2 << ", " << grid.lonInc << "]"
        << endl;

    int n = 0;
    for (int j = 0; j < grid.nj; j++) {
        for (int i = 0; i < grid.ni; i++)
            out << message.values[n++] << " ";
        out << endl;
    }
}

void Txt::end()
{
}

}
