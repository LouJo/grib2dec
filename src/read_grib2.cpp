#include "read_grib2.hpp"
#include "grib2dec/grib2dec.h"
#include "utils.hpp"
#include "stream.hpp"

#include <iostream>
#include <fstream>
#include <string.h>
#include <assert.h>

using namespace std;
using namespace grib2dec;

namespace {

/*
 * grib2 spec:
 * https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/
 *
 * NOAA winds download:
 * https://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_1p00.pl?dir=%2Fgfs.20201226%2F06
 *
 * Usage help:
 * http://www.tecepe.com.br/wiki/index.php?title=NOAAWinds
*/

const bool debug = false;

enum Component { U, V, NbComponent };

struct Datetime {
    int year, month, day;
    int hour, minute, second;
};

struct Grid {
    double earthRadius;
    int ni, nj;
    double la1, lo1;
    double la2, lo2;
};

struct Packing {
    int tpl;  // 0, 2 or 3
    float R;
    int E;
    int D;
    int sampleBits;
    int valueType;
    int NG;  // number of groups
    int groupWidthRef;
    int groupWidthBits;
    int groupLengthRef;
    int groupLengthInc;
    int lastGroupLength;
    int scaledGroupLengthBits;
    int spatialOrder;
    int extraBytes;
};

struct Message {
    int len;
    int nbLocal;
    Datetime datetime;
    bool complete;
    int lenRead;
    Grid grid;
    Component comp;
    Packing packing;
};

bool error(const char *msg)
{
    cerr << "read_grib2 error: " << msg << endl;
    return false;
}

#define STREAM(n) if (!stream.read(n)) return false

bool readIndicatorSection(Stream& stream, Message& message)
{
    stream.sectionLen = stream.sectionRemain = 16;
    message.len = 0;

    STREAM(4);

    if (strncmp(stream.data, "GRIB", 4))
        return error("Not GRIB header");

    // reserved
    STREAM(2);

    // discipline
    STREAM(1);

    if (stream.data[0] != 0)
        return error("Discipline is not meteorological");

    // edition number
    STREAM(1);

    if (stream.data[0] != 2)
        return error("Edition number is not 2");

    // msg len
    STREAM(8);

    message.len = stream.len64();
    message.lenRead = stream.sectionLen;

    return true;
}

bool readIdentificationSection(Stream& stream, Message& message)
{
    // generating center and subcenter
    STREAM(4);

    // table version
    STREAM(1);

    if (stream.data[0] != 2)
        return error("Master table version number is not 2");

    // version of local tables
    STREAM(1);
    message.nbLocal = stream.data[0];

    // significance of reference time
    STREAM(1);

    // datetime
    STREAM(7);

    message.datetime.year = bigendian(*(uint16_t*)stream.data);
    message.datetime.month = stream.data[2];
    message.datetime.day = stream.data[3];
    message.datetime.hour = stream.data[4];
    message.datetime.minute = stream.data[5];
    message.datetime.second = stream.data[6];

    // production status
    STREAM(1);

    // type of processed data
    STREAM(1);
    if (stream.data[0] != 1)
        return error("Not forecast data");

    return stream.sectionEnd();
}

bool readLocalSection(Stream& stream)
{
    return stream.sectionEnd();
}

bool readGridTemplate30(Stream& stream, Message& message)
{
    Grid& grid = message.grid;

    // shape of earth
    STREAM(1);
    switch (stream.data[0]) {
    case 0:
        grid.earthRadius = 6367470.;
        break;
    case 6:
        grid.earthRadius = 6371229.;
        break;
    default:
        return error("Earth shape not handled (only sphericals)");
    }

    // scale factors
    STREAM(15);

    // Ni, Nj
    STREAM(4);
    grid.ni = stream.len32();
    STREAM(4);
    grid.nj = stream.len32();

    // basic angle and subdivisions
    STREAM(8);

    // first latitude and longitude
    STREAM(4);
    grid.la1 = stream.len32() / 1000000.;
    STREAM(4);
    grid.lo1 = stream.len32() / 1000000.;

    // component flag
    STREAM(1);
    if (stream.data[0] != 48)
        return error("Only component flag 48 is supported (inc x and y)");

    // last latitude and longitude - false values ?
    STREAM(4);
    grid.la2 = stream.len32() / 1000000.;
    STREAM(4);
    grid.lo2 = stream.len32() / 1000000.;

    // directions increment
    STREAM(8);

    // scanning mode
    STREAM(1);
    if (stream.data[0] != 0)
        return error("Scanning mode 0 only is supported");

    return stream.sectionEnd();
}

bool readGridDefinition(Stream& stream, Message& message)
{
    // source of grid definition
    STREAM(1);
    if (stream.data[0] != 0)
        return error("Grid definition other than 0 are not implemented");

    // number of data points
    STREAM(4);
    // int nbDataPts = stream.len32(); // already in data representation

    // octets for optional lists
    STREAM(1);

    // interpretation of number of points
    STREAM(1); // 2 ?

    // grid definitition number
    STREAM(2);

    int gridDefinitionTpl = stream.len16();

    switch (gridDefinitionTpl) {
    case 0:
        return readGridTemplate30(stream, message);
    default:
        return error("Only grid definition 0 is supported (equidistant cylindrical)");
    }
}

bool readProductionDefinition(Stream& stream, Message& message)
{
    // number of coords
    STREAM(2);

    // product definition template
    STREAM(2);

    // category and number
    STREAM(2);

    message.comp = NbComponent;

    if (stream.data[0] != 2)
        return error("Product discipline is not momentum");

    switch (stream.data[1]) {
    case 2:
        message.comp = U;
        break;
    case 3:
        message.comp = V;
        break;
    default:
        return error("Component is not U or V speed of wind");
    }

    return stream.sectionEnd();
}

bool readDataRepresentationTemplate53(Stream& stream, Message& message)
{
    Packing& pack = message.packing;

    // templates 5.0, 5.2 and 5.3

    // R floating point value
    STREAM(4);
    pack.R = *((float*)stream.data);

    // Binary scale factor E
    STREAM(2);
    pack.E = stream.len16();

    // Decimal scale factor D
    STREAM(2);
    pack.D = stream.len16();

    // Number of bits for each packed value
    STREAM(1);
    pack.sampleBits = stream.data[0];

    // Type (0: float)
    STREAM(1);
    pack.valueType = stream.data[0];

    if (stream.sectionRemain <= 0)
        return true; // template 5.0

    // templates 5.2 and 5.3

    // group splitted method
    STREAM(1);
    // int groupSplit = stream.data[0];

    // missing value management
    STREAM(1);
    if (stream.data[0] != 0)
        return error("No missing pt management handled");

    // missing values
    STREAM(8);

    // NG : number of group of values
    STREAM(4);
    pack.NG = stream.len32();

    // reference for groups width
    STREAM(1);
    pack.groupWidthRef = stream.data[0];

    // number of bits for groups width
    STREAM(1);
    pack.groupWidthBits = stream.data[0];

    // reference for groups length
    STREAM(4);
    pack.groupLengthRef = stream.len32();

    // length increment for group length
    STREAM(1);
    pack.groupLengthInc = stream.data[0];

    // true length of last group
    STREAM(4);
    pack.lastGroupLength = stream.len32();

    // number of bits for scaled group lengths
    STREAM(1);
    pack.scaledGroupLengthBits = stream.data[0];

    if (stream.sectionRemain <= 0)
        return true; // template 5.2

    // templates 5.3

    // order of spatial difference
    STREAM(1);
    pack.spatialOrder = stream.data[0];
    if (pack.spatialOrder != 1 && pack.spatialOrder != 2)
        return error("Order of spatial differencing must be 1 or 2");

    // number of bytes for extra descriptors = 6-ww
    STREAM(1);
    pack.extraBytes = stream.data[0];

    // value formula doc pages 5, 41
    // page 39 for spatial filter

    return stream.sectionEnd();
}

bool readDataRepresentation(Stream& stream, Message& message)
{
    STREAM(4);
    int nb = stream.len32();
    if (nb != message.grid.ni * message.grid.nj)
        return error("Number of point is not ni x nj");

    STREAM(2);
    message.packing.tpl = stream.len16();


    switch (message.packing.tpl) {
    case 0:
    case 2:
    case 3:
        return readDataRepresentationTemplate53(stream, message);
    default:
        return error("Data representation template not handled");
    }
}

bool readSection(Stream& stream, Message& message)
{
    // section length
    stream.sectionBegin(message.len - message.lenRead);
    message.lenRead += stream.sectionLen;

    if (stream.sectionId == 7) {
        message.complete = true;
        return true;
    }

    if (debug)
    cerr << " section " << stream.sectionId << " len " << stream.sectionLen
         << " remain " << (message.len - message.lenRead)
         << endl;

    switch (stream.sectionId) {
    case 1:
        return readIdentificationSection(stream, message);
    case 2:
        return readLocalSection(stream);
    case 3:
        return readGridDefinition(stream, message);
    case 4:
        return readProductionDefinition(stream, message);
    case 5:
        return readDataRepresentation(stream, message);

    case 6:
    case 7:
        return stream.sectionEnd();

    default:
        return error("error : unknown section id");
    }
}

bool readMessage(istream& fin, Message& message)
{
    Stream stream(fin);
    message.len = 0;
    message.complete = false;
    message.lenRead = 0;

    if (!readIndicatorSection(stream, message))
        return false;

    while (!message.complete && message.lenRead < message.len) {
        if (!readSection(stream, message))
            return false;
    }

    return true;
}

bool readGrib2(const char *filename)
{
    ifstream fin(filename);

    if (!fin.is_open())
        return error("Cannot open filename");

    Message message;
    message.len = 0;

    int skip = 0;

    for(;; skip += message.len) {
        fin.seekg(skip, ios_base::beg);

        char c = fin.get();
        if (fin.eof())
            break;
        fin.putback(c);

        if (!readMessage(fin, message)) {
            if (message.len == 0)
                break;
            else
                continue;
        }
    }

    return true;
}

}

void rgta_test(const char *grib2_filename)
{
    std::cerr << "open grib2 file " << grib2_filename << std::endl;
    readGrib2(grib2_filename);
}
