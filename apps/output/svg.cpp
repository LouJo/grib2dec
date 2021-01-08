#include "svg.hpp"

#include <algorithm>
#include <iostream>

using namespace std;

namespace grib2dec_demo {

Svg::Svg(const string& filename) : Output(filename)
{
}

void Svg::setComponent(const G2DEC_Message& message)
{
    vector<double> *values;

    switch (message.parameter) {
    case G2DEC_PARAMETER_WIND_U:
        values = &ptsx;
        break;
    case G2DEC_PARAMETER_WIND_V:
        values = &ptsy;
        break;
    default:
        return;
    }

    values->resize(message.valuesLength);
    copy_n(message.values, message.valuesLength, values->data());

    grid = message.grid;
}

void Svg::end()
{
    if (ptsx.empty() || ptsy.empty() || ptsx.size() != ptsy.size() ||
        ptsx.size() != grid.ni * grid.nj) {
        cerr << "components error" << endl;
        return;
    }

    int scale = 20;
    int svgWidth = grid.ni * scale, svgHeight = grid.nj * scale;
    const double lineWidth = 1. / scale;

    out << "<svg version=\"1.1\" baseProfil=\"full\" xmlns=\"http://www.w3.org/2000/svg\"\n"
        << "     width=\"" << svgWidth << "\" height=\"" << svgHeight << "\""
        << " viewBox=\"0 0 " << grid.ni << " " << grid.nj << "\">\n";

    out << "<style>\n"
        << "  .wnd {\n"
        << "    stroke-width: " << lineWidth << ";\n"
        << "    stroke: #1766b5;\n"
        << "}\n"
        << "</style>\n";

    int i = 0;
    const double middleX = 0.5, middleY = 0.5;
    const double valueScale = 0.1;

    // background
    out << "<rect x=\"0\" y=\"0\" width=\"" << grid.ni << "\" height=\"" << grid.nj << "\" style=\"fill:white\"/>\n";

    // winds
    for (int y = 0; y < grid.nj; y++) {
        double posy1 = y + middleY;
        for (int x = 0; x < grid.ni; x++, i++) {
            double posx1 = x + middleX;
            double posx2 = posx1 + ptsx[i] * valueScale;
            double posy2 = posy1 - ptsy[i] * valueScale;
            out << "<line x1=\"" << posx1 << "\" y1=\"" << posy1 << "\""
                << " x2=\"" << posx2 << "\" y2=\"" << posy2 << "\""
                << " stroke=\"black\" class=\"wnd\""
                << " />\n";
        }
    }

    out << "</svg>" << endl;
}

}
