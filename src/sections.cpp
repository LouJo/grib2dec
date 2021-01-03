#include "sections.hpp"
#include "data.hpp"

#include <string.h>
#include <vector>

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
        throw parsing_error("master table version number is not 2");

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
        throw parsing_error("not forecast data");

    stream.sectionEnd();
}

void readLocalSection(Stream& stream)
{
    stream.sectionEnd();
}

void readGridTemplate30(Stream& stream, Message& message)
{
    Grid& grid = message.grid;

    // shape of earth
    switch (stream.byte()) {
    case 0:
        grid.earthRadius = 6367470.;
        break;
    case 6:
        grid.earthRadius = 6371229.;
        break;
    default:
        throw parsing_error("earth shape not handled (only sphericals)");
    }

    // scale factors
    stream.read(15);

    // Ni, Nj
    grid.ni = stream.len32();
    grid.nj = stream.len32();

    // basic angle and subdivisions
    stream.read(8);

    // first latitude and longitude
    grid.la1 = stream.len32() / 1000000.;
    grid.lo1 = stream.len32() / 1000000.;

    // component flag
    if (stream.byte() != 48)
        throw parsing_error("only component flag 48 is supported (inc x and y)");

    // last latitude and longitude - false values ?
    grid.la2 = stream.len32() / 1000000.;
    grid.lo2 = stream.len32() / 1000000.;

    // directions increment
    stream.read(8);

    // scanning mode
    if (stream.byte() != 0)
        throw parsing_error("scanning mode 0 only is supported");

    stream.sectionEnd();
}

void readGridDefinition(Stream& stream, Message& message)
{
    // source of grid definition
    if (stream.byte() != 0)
        throw parsing_error("grid definition other than 0 are not implemented");

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
        return readGridTemplate30(stream, message);
    default:
        throw parsing_error("only grid definition 0 is supported (equidistant cylindrical)");
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
        throw parsing_error("no missing pt management handled");

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
        throw parsing_error("data representation template not handled");
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
