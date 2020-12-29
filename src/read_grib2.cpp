#include "read_grib2.hpp"
#include "regatta/regatta.h"
#include "utils.hpp"

#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;
using namespace regatta;

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

struct Datetime {
    int year, month, day;
    int hour, minute, second;
};

struct Message {
    int len;
    int nbLocal;
    Datetime datetime;
    bool complete;
    int lenRead;
};

bool error(const char *msg)
{
    cerr << "read_grib2 error: " << msg << endl;
    return false;
}

bool readFile(istream& fin, char *data, int len, const char *title = nullptr)
{
    fin.read(data, len);

    if (!fin) {
        cerr << "Cannot read " << len << " bytes";
        if (title)
            cerr << " for " << title;
        cerr << endl;
        return false;
    }

    return true;
}

#define READ(n) if (!readFile(fin, data, n)) return false

bool readEndOfSection(istream& fin, int len)
{
    fin.seekg(len, ios_base::cur);
    return !fin.eof();
}

bool readIndicatorSection(istream& fin, Message& message)
{
    char data[8];
    message.len = 0;

    READ(4);

    if (strncmp(data, "GRIB", 4))
        return error("Not GRIB header");

    // reserved
    READ(2);

    // discipline
    READ(1);

    if (data[0] != 0)
        return error("Discipline is not meteorological");


    // edition number
    READ(1);

    if (data[0] != 2)
        return error("Edition number is not 2");

    // msg len
    READ(8);

    message.len = len64(data);
    message.lenRead = 16;
    cerr << "msg len: " << message.len << endl;

    return true;
}

bool readIdentificationSection(istream& fin, Message& message, int sectionLen)
{
    char data[7];

    // generating center and subcenter
    READ(4);

    // table version
    READ(1);

    if (data[0] != 2)
        return error("Master table version number is not 2");

    // version of local tables
    READ(1);
    message.nbLocal = data[0];

    // significance of reference time
    READ(1);

    // datetime
    READ(7);

    message.datetime.year = bigendian(*(uint16_t*)data);
    message.datetime.month = data[2];
    message.datetime.day = data[3];
    message.datetime.hour = data[4];
    message.datetime.minute = data[5];
    message.datetime.second = data[6];

    // production status
    READ(1);

    // type of processed data
    READ(1);
    if (data[0] != 1)
        return error("Not forecast data");

    return readEndOfSection(fin, sectionLen - 16);
}

bool readLocalSection(istream& fin, int sectionLen)
{
    return readEndOfSection(fin, sectionLen);
}

bool readGridDefinition(istream& fin, int sectionLen)
{
    char data[4];

    // source of grid definition
    READ(1);
    if (data[0] != 0)
        return error("Grid definition other than 0 are not implemented");

    // number of data points
    READ(4);
    int nbDataPts = len32(data);

    // octets for optional lists
    READ(1);

    // interpretation of number of points
    READ(1); // 2 ?

    // grid definitition number
    READ(2);
    int gridDefinitionTpl = len16(data);
    cerr << "grid def: " << gridDefinitionTpl << endl;

    return readEndOfSection(fin, sectionLen - 9);
}

bool readSection(istream& fin, Message& message)
{
    char data[4];

    // section length
    READ(4);

    if (data[0] == '7' && data[1] == '7' && data[2] == '7' && data[3] == '7') {
        // 7777 : end section
        cerr << " end section" << endl;
        message.complete = true;
        return true;
    }

    int sectionLen = min<int>(len32(data), message.len - message.lenRead);

    // section number
    READ(1);
    int sectionId = int(data[0]);

    cerr << " section " << sectionId << " len " << sectionLen
         << " remain " << (message.len - message.lenRead)
         << endl;

    message.lenRead += sectionLen;
    sectionLen -= 5;

    switch (sectionId) {
    case 1:
        return readIdentificationSection(fin, message, sectionLen);
    case 2:
        return readLocalSection(fin, sectionLen);
    case 3:
        return readGridDefinition(fin, sectionLen);

    case 4:
    case 5:
    case 6:
    case 7:
        return readEndOfSection(fin, sectionLen);

    default:
        return error("error : unknown section id");
    }
}

bool readMessage(istream& fin, Message& message)
{
    message.complete = false;
    message.lenRead = 0;

    if (!readIndicatorSection(fin, message))
        return false;

    while (!message.complete && message.lenRead < message.len) {
        if (!readSection(fin, message))
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
