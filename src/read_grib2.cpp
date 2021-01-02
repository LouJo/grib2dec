#include "read_grib2.hpp"
#include "grib2dec/grib2dec.h"
#include "utils.hpp"
#include "stream.hpp"

#include <iostream>
#include <fstream>
#include <string.h>
#include <assert.h>
#include <vector>

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
    if (stream.byte() != 0)
        throw parsing_error("discipline is not meteorological");

    // edition number
    if (stream.byte() != 2)
        throw parsing_error("edition number is not 2");

    // msg len
    message.len = stream.len64();

    message.lenRead = stream.sectionLen;
}

void readIdentificationSection(Stream& stream, Message& message)
{
    // generating center and subcenter
    stream.read(4);

    // table version
    if (stream.byte() != 2)
        throw parsing_error("master table version number is not 2");

    // version of local tables
    message.nbLocal = stream.byte();

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

    // category and number
    stream.read(2);

    message.comp = NbComponent;

    if (stream.data[0] != 2)
        throw parsing_error("product discipline is not momentum");

    switch (stream.data[1]) {
    case 2:
        message.comp = U;
        break;
    case 3:
        message.comp = V;
        break;
    default:
        throw parsing_error("component is not U or V speed of wind");
    }

    stream.sectionEnd();
}

void readDataRepresentationTemplate53(Stream& stream, Message& message)
{
    Packing& pack = message.packing;

    // templates 5.0, 5.2 and 5.3

    // R floating point value
    stream.read(4);
    pack.R = *((float*)stream.data);

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

    if (stream.sectionRemain <= 0)
        return; // template 5.2

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
    int nb = stream.len32();
    if (nb != message.grid.ni * message.grid.nj)
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

void readDataTemplate53(Stream& stream, const Message& message, vector<double>& values)
{
    const Packing& pack = message.packing;
    int v2 = -1;

    int v1 = stream.bytes(pack.extraBytes);

    if (pack.spatialOrder >= 2)
        v2 = stream.bytes(pack.extraBytes);

    int minValue = stream.bytes(pack.extraBytes);

    cerr << "diff values: " << minValue << " " << v1 << " " << v2 << endl;

    stream.sectionEnd();
}

void readData(Stream& stream, Message& message, vector<double>& values)
{
    values.reserve(message.grid.ni * message.grid.nj);

    switch (message.packing.tpl) {
    case 3:
        return readDataTemplate53(stream, message, values);
    default:
        throw parsing_error("data template not handled");
    }
}

void readSection(Stream& stream, Message& message)
{
    // section length
    stream.sectionBegin(message.len - message.lenRead);
    message.lenRead += stream.sectionLen;
    vector<double> values;

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

void readMessage(istream& fin, Message& message)
{
    Stream stream(fin);
    message.len = 0;
    message.complete = false;
    message.lenRead = 0;

    readIndicatorSection(stream, message);

    while (!message.complete && message.lenRead < message.len)
        readSection(stream, message);
}

bool readGrib2(const char *filename)
{
    ifstream fin(filename);

    if (!fin.is_open())
        return error("Cannot open filename");

    Message message;
    message.len = 0;

    int skip = 0;
    int nbRead = 0;

    for(;; skip += message.len) {
        fin.seekg(skip, ios_base::beg);

        char c = fin.get();
        if (fin.eof())
            break;
        fin.putback(c);

        try {
            readMessage(fin, message);
        } catch (const parsing_error& e) {
            cerr << e.what() << endl;

            if (message.len == 0)
                break;
            else
                continue;
        }
        nbRead++;
    }
    cerr << nbRead << " messages read" << endl;

    return true;
}

}

void rgta_test(const char *grib2_filename)
{
    std::cerr << "open grib2 file " << grib2_filename << std::endl;
    readGrib2(grib2_filename);
}
