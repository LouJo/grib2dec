#include "sections.hpp"
#include "data.hpp"

#include <string.h>
#include <vector>
#include <math.h>

namespace grib2dec {

/*
 * grib2 spec:
 * https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/
 *
 * NOAA winds download:
 * https://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_1p00.pl
 *
 * Usage help:
 * http://www.tecepe.com.br/wiki/index.php?title=NOAAWinds
*/

namespace {

const bool debug = false;

void readIdentificationSection(Stream& stream, Message& message)
{
    // generating center and subcenter
    stream.read(4);

    // table version
    if (stream.byte() != 2)
        throw not_implemented("master table version number is not 2");

    // version of local tables
    stream.read(1);

    // significance of reference time
    stream.read(1);

    // datetime
    stream.read(7);

    message.datetime.year = bigendian(*(uint16_t*)stream.data);
    message.datetime.month = stream.data[2];
    message.datetime.day = stream.data[3];
    message.datetime.hour = stream.data[4];
    message.datetime.minute = stream.data[5];
    message.datetime.second = stream.data[6];

    // production status
    stream.read(1);

    // type of processed data
    if (stream.byte() != 1)
        throw not_implemented("not forecast data");

    stream.sectionEnd();
}

void readLocalSection(Stream& stream)
{
    stream.sectionEnd();
}

void setSpatialFilter(Message& message)
{
    Grid& grid = message.grid;
    Filter& filter = message.filter;

    if (filter.spatialFilter.lonMin || filter.spatialFilter.lonMax) {
        if (grid.lon1 < filter.spatialFilter.lonMin)
            filter.i.front = ceil((filter.spatialFilter.lonMin - grid.lon1) / abs(grid.lonInc));
        else if (grid.lon1 > filter.spatialFilter.lonMax)
            filter.i.front = ceil((grid.lon1 - filter.spatialFilter.lonMax) / abs(grid.lonInc));

        if (filter.i.front)
            grid.lon1 += filter.i.front * grid.lonInc;

        assert(filter.i.front >= 0);

        if (grid.lon2 < filter.spatialFilter.lonMin)
            filter.i.back = ceil((filter.spatialFilter.lonMin - grid.lon2) / abs(grid.lonInc));
        else if (grid.lon2 > filter.spatialFilter.lonMax)
            filter.i.back = ceil((grid.lon2 - filter.spatialFilter.lonMax) / abs(grid.lonInc));

        if (filter.i.back)
            grid.lon2 -= filter.i.back * grid.lonInc;

        assert(filter.i.back >= 0);
    }

    if (filter.spatialFilter.latMin || filter.spatialFilter.latMax) {
        if (grid.lat1 < filter.spatialFilter.latMin)
            filter.j.front = ceil((filter.spatialFilter.latMin - grid.lat1) / abs(grid.latInc));
        else if (grid.lat1 > filter.spatialFilter.latMax)
            filter.j.front = ceil((grid.lat1 - filter.spatialFilter.latMax) / abs(grid.latInc));

        if (filter.j.front)
            grid.lat1 += filter.j.front * grid.latInc;

        assert(filter.j.front >= 0);

        if (grid.lat2 < filter.spatialFilter.latMin)
            filter.j.back = ceil((filter.spatialFilter.latMin - grid.lat2) / abs(grid.latInc));
        else if (grid.lat2 > filter.spatialFilter.latMax)
            filter.j.back = ceil((grid.lat2 - filter.spatialFilter.latMax) / abs(grid.latInc));

        if (filter.j.back)
            grid.lat2 -= filter.j.back * grid.latInc;

        assert(filter.j.back >= 0);
    }
}

void readGridTemplate0to3(Stream& stream, Message& message)
{
    Grid& grid = message.grid;

    // shape of earth
    switch (stream.byte()) {
    case 0:
        grid.earthRadius = 6367470.;
        break;
    case 1:
        grid.earthRadius = 0.;
        break;
    case 6:
        grid.earthRadius = 6371229.;
        break;
    default:
        throw not_implemented("earth shape not handled (only sphericals)");
    }

    // earth radius
    int earthRadiusFactor = stream.byte();
    int earthRadiusScaled = stream.len32();

    if (grid.earthRadius == 0.)
        grid.earthRadius = earthRadiusScaled / double(earthRadiusFactor);

    // scale factors
    stream.read(10);

    // Ni, Nj
    grid.ni = stream.len32();
    grid.nj = stream.len32();

    // basic angle and subdivisions
    int basicAngle = stream.len32();
    int subAngle = stream.len32();

    if (basicAngle == 0)
        basicAngle = 1;
    if (subAngle == -1)
        subAngle = 1000000.;

    double subA = subAngle;

    // first latitude and longitude
    grid.lat1 = stream.magSigned32() / subA;
    grid.lon1 = stream.magSigned32() / subA;

    // component flag
    stream.read(1); // should be 48 ?

    // last latitude and longitude
    grid.lat2 = stream.magSigned32() / subA;
    grid.lon2 = stream.magSigned32() / subA;

    /* directions increment.
     * manage signs with limits order.
     */
    int inci = abs(stream.magSigned32());
    int incj = abs(stream.magSigned32());

    if (grid.lon2 < grid.lon1)
        inci = -inci;
    if (grid.lat2 < grid.lat1)
        incj = -incj;

    grid.lonInc = inci / subA;
    grid.latInc = incj / subA;

    /* scanning mode.
     * 2 first bits are redondants with limits min and max
     * for increment sign.
     */
    if (stream.byte() & 0xfc)
        throw not_implemented("scanning mode: only raster is supported");

    // set filter
    setSpatialFilter(message);

    stream.sectionEnd();
}

void readGridDefinition(Stream& stream, Message& message)
{
    // source of grid definition
    if (stream.byte() != 0)
        throw not_implemented("source of grid definition other than 0 are not implemented");

    // number of data points
    stream.read(4);
    // int nbDataPts = stream.len32(); // already in data representation

    // octets for optional lists
    stream.read(1);

    // interpretation of number of points
    stream.read(1); // 2 ?

    // grid definitition number
    int gridDefinitionTpl = stream.len16();

    switch (gridDefinitionTpl) {
    case 0:
    case 1:
    case 2:
    case 3:
        return readGridTemplate0to3(stream, message);
    default:
        throw not_implemented("only grid definition 0 to 3 is supported (latitude and longitude");
    }
}

void readProductionDefinition(Stream& stream, Message& message)
{
    // number of coords
    stream.read(2);

    // product definition template
    stream.read(2);

    // parameter category
    message.category = static_cast<Category>(stream.byte() + message.discipline * 1000);

    // parameter
    message.parameter = static_cast<Parameter>(stream.byte() + message.category * 1000);

#if 0
    if (message.parameter != G2DEC_PARAMETER_WIND_U && message.parameter != G2DEC_PARAMETER_WIND_V)
        throw parsing_error("Unknown parameter");
#endif

    stream.sectionEnd();
}

void readDataRepresentationTemplate53(Stream& stream, Message& message)
{
    Packing& pack = message.packing;

    // templates 5.0, 5.2 and 5.3

    // R floating point value
    pack.R = stream.floatingPointNumber();

    // Binary scale factor E
    pack.E = stream.len16();

    // Decimal scale factor D
    pack.D = stream.len16();

    // Number of bits for each packed value
    pack.sampleBits = stream.byte();

    // Type (0: float)
    pack.valueType = stream.byte();

    if (stream.sectionRemain <= 0)
        return; // template 5.0

    // templates 5.2 and 5.3

    // group splitted method
    stream.read(1);
    // int groupSplit = stream.data[0];

    // missing value management
    if (stream.byte() != 0)
        throw not_implemented("no missing pt management handled");

    // missing values
    stream.read(8);

    // NG : number of group of values
    pack.NG = stream.len32();

    // reference for groups width
    pack.groupWidthRef = stream.byte();

    // number of bits for groups width
    pack.groupWidthBits = stream.byte();

    // reference for groups length
    pack.groupLengthRef = stream.len32();

    // length increment for group length
    pack.groupLengthInc = stream.byte();

    // true length of last group
    pack.lastGroupLength = stream.len32();

    // number of bits for scaled group lengths
    pack.scaledGroupLengthBits = stream.byte();

    if (stream.sectionRemain <= 0) {
        pack.spatialOrder = 0;
        return; // template 5.2
    }

    // templates 5.3

    // order of spatial difference
    pack.spatialOrder = stream.byte();
    if (pack.spatialOrder != 1 && pack.spatialOrder != 2)
        throw parsing_error("order of spatial differencing must be 1 or 2");

    // number of bytes for extra descriptors = 6-ww
    pack.extraBytes = stream.byte();

    // value formula doc pages 5, 41
    // page 39 for spatial filter

    stream.sectionEnd();
}

void readDataRepresentation(Stream& stream, Message& message)
{
    message.packing.nbValues = stream.len32();
    if (message.packing.nbValues != message.grid.ni * message.grid.nj)
        throw parsing_error("number of point is not ni x nj");

    message.packing.tpl = stream.len16();

    switch (message.packing.tpl) {
    case 0:
    case 2:
    case 3:
        return readDataRepresentationTemplate53(stream, message);
    default:
        throw not_implemented("data representation template not handled");
    }
}

} // local namespace

void readIndicatorSection(Stream& stream, Message& message)
{
    stream.sectionLen = stream.sectionRemain = 16;
    message.len = 0;

    stream.read(4);

    if (strncmp(stream.data, "GRIB", 4))
        throw parsing_error("not GRIB header");

    // reserved
    stream.read(2);

    // discipline
    message.discipline = static_cast<Discipline>(stream.byte());

    // edition number
    if (stream.byte() != 2)
        throw parsing_error("edition number is not 2");

    // msg len
    message.len = stream.len64();

    message.lenRead = stream.sectionLen;
}

void readSection(Stream& stream, Message& message, vector<double>& values)
{
    // section length
    stream.sectionBegin(message.len - message.lenRead);
    message.lenRead += stream.sectionLen;

    if (stream.sectionId == 8) {
        message.complete = true;
        return;
    }

    if (debug)
    cerr << " section " << stream.sectionId << " len " << stream.sectionLen
         << " remain " << (message.len - message.lenRead)
         << endl;

    switch (stream.sectionId) {
    case 1:
        readIdentificationSection(stream, message);
        break;
    case 2:
        readLocalSection(stream);
        break;
    case 3:
        readGridDefinition(stream, message);
        break;
    case 4:
        readProductionDefinition(stream, message);
        break;
    case 5:
        readDataRepresentation(stream, message);
        break;
    case 6:
        stream.sectionEnd();
        break;
    case 7:
        readData(stream, message, values);
        break;

    default:
        throw parsing_error("error : unknown section id");
    }
}

} // grib2dec
