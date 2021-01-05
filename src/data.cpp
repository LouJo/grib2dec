#include "data.hpp"

#include <cmath>

using namespace std;

namespace grib2dec {
namespace {

class SpatialFilterOp {
public:
    SpatialFilterOp(const Message& message) {
        const Filter& filter = message.filter;

        skipBegin = filter.i.front + filter.j.front * message.grid.ni;
        skipEnd = filter.i.back + filter.j.back * message.grid.ni;
        skipI = filter.i.front + filter.i.back;
        nbI = message.grid.ni - skipI;
        nbJ = message.grid.nj - filter.j.front - filter.j.back;
        skip = skipBegin;
        nb = nbI;
    }

    bool addValue() {
        if (skip > 0) {
            skip--;
            return false;
        } else if (nb > 0) {
            nb--;
            if (nb == 0) {
                nbJ--;
                if (nbJ > 0) {
                    skip = skipI;
                    nb = nbI;
                } else {
                    skip = skipEnd;
                }
            }
            return true;
        }
        else {
            return false;
        }
    }

private:
    int skipBegin, skipEnd, skipI;
    int nbI, nbJ;
    int skip, nb;
};

void readDataBits(Stream& stream, int nbBits, vector<int>& data)
{
    for (auto& v : data)
        v = stream.bits(nbBits);

    stream.bitsEnd();
}

void getScaleParameters(const Packing& pack, double& ref, double& scale)
{
    double dscale = pow(10., -pack.D);
    ref = pack.R * dscale;
    scale = pow(2., pack.E) * dscale;
}

template <int spatialOrder>
void readComplexPackingValues(Stream& stream, const Message& message, int h1,
                              int h2, int hmin, vector<double>& values)
{
    static_assert(spatialOrder >= 0 && spatialOrder <= 2);
    const Packing& pack = message.packing;

    // group references
    vector<int> refs(pack.NG);
    readDataBits(stream, pack.sampleBits, refs);

    // group widths
    vector<int> widths(pack.NG);
    readDataBits(stream, pack.groupWidthBits, widths);

    // group lengths
    vector<int> lengths(pack.NG);

    {
        readDataBits(stream, pack.scaledGroupLengthBits, lengths);
        const int inc = pack.groupLengthInc, ref = pack.groupLengthRef;
        for (int& v : lengths)
            v = v * inc + ref;
    }

    // scale parameters
    double ref, scale;
    getScaleParameters(pack, ref, scale);

    // packed values
    int groupId = 0;
    int groupLength = lengths[groupId];
    int nbBits = widths[groupId];
    int sampleId = 0;
    int groupRef = refs[groupId];
    int valueId = 0;

    const Filter& filter = message.filter;
    int nbValues = (message.grid.ni - filter.i.front - filter.i.back) *
                   (message.grid.nj - filter.j.front - filter.j.back);

    // spatial filter
    SpatialFilterOp filterOp(message);

    values.resize(nbValues);

    for (int i = 0; i < spatialOrder; i++) {
        // read first values for nothing
        stream.bits(nbBits);
        int x;
        if (i == 0)
            x = h1;
        else
            x = h2;

        // spatial filter
        if (filterOp.addValue())
            values[valueId++] = ref + scale * x;

        // group management
        sampleId++;
        if (sampleId == groupLength) {
            groupId++;
            if (groupId == pack.NG)
                break;
            sampleId = 0;
            groupLength = lengths[groupId];
            nbBits = widths[groupId];
            groupRef = refs[groupId];
        }
    }

    while (true) {
        int x = stream.bits(nbBits) + groupRef;
        // optimise at compile time for order
        if (spatialOrder == 1) {
            x += hmin + h1;
            h1 = x;
        } else if (spatialOrder == 2) {
            x += hmin - h1 + 2 * h2;
            h1 = h2;
            h2 = x;
        }

        // spatial filter
        if (filterOp.addValue()) {
            assert(valueId < int(values.size()));
            values[valueId++] = ref + scale * x;
        }

        // group management
        sampleId++;
        if (sampleId == groupLength) {
            groupId++;
            if (groupId == pack.NG)
                break;
            sampleId = 0;
            groupLength = lengths[groupId];
            nbBits = widths[groupId];
            groupRef = refs[groupId];
        }
    }
}

template <int tpl>
void readDataTemplate(Stream& stream, const Message& message, vector<double>& values)
{
    static_assert(tpl == 2 || tpl == 3);
    const Packing& pack = message.packing;

    // extra values for spatial filter

    int h1 = -1;
    int h2 = -1;
    int hmin = -1;

    if (tpl == 3) {
        // template 5.3
        assert(pack.spatialOrder == 1 || pack.spatialOrder == 2);

        h1 = stream.bytes(pack.extraBytes);

        if (pack.spatialOrder == 2)
            h2 = stream.bytes(pack.extraBytes);

        hmin = stream.bytes(pack.extraBytes);
    } else {
        assert(pack.spatialOrder == 0);
    }

    // read values with complex packing

    switch (pack.spatialOrder) {
    case 0:
        // template 5.2
        readComplexPackingValues<0>(stream, message, h1, h2, hmin, values);
        break;
    case 1:
        // template 5.3
        readComplexPackingValues<1>(stream, message, h1, h2, hmin, values);
        break;
    case 2:
        // template 5.3
        readComplexPackingValues<2>(stream, message, h1, h2, hmin, values);
        break;
    }

    stream.sectionEnd();
}

} // local namespace

void readData(Stream& stream, Message& message, vector<double>& values)
{
    values.clear();

    switch (message.packing.tpl) {
    case 2:
        return readDataTemplate<2>(stream, message, values);
    case 3:
        return readDataTemplate<3>(stream, message, values);
    default:
        throw not_implemented("data template not handled");
    }
}

} // grib2dec
