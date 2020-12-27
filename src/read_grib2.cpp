#include "read_grib2.hpp"
#include "regatta/regatta.h"
#include "utils.hpp"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include <fstream>
#include <string.h>

using namespace std;
using namespace regatta;

namespace {

/*
    https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/
*/

bool error(const char *msg)
{
    cerr << "read_grib2 error: " << msg << endl;
    return false;
}

bool readFile(ifstream& fin, char *data, int len, const char *title = nullptr)
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

bool readIndicatorSection(ifstream& fin, int& msgLen)
{
    char data[8];
    msgLen = 0;

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

    msgLen = bigendian(*((uint64_t*)data));
    cerr << "msg len: " << msgLen << endl;

    return true;
}

bool readIdentificationSection(ifstream& fin)
{
    char data[11];

    // section len
    READ(4);
    int sectionLen = bigendian(*((uint32_t*)data));

    // number, generating center and subcenter
    READ(5);

    // table version
    READ(1);

    if (data[0] != 2)
        return error("Master table version number is not 2");

    // version of local tables
    READ(1);
    int localTablesNb = data[0];
    cerr << "local tables: " << localTablesNb << endl;

    // significance of reference time
    READ(1);

    // datetime
    READ(7);

    int year = bigendian(*(uint16_t*)data);
    int month = data[2];
    int day = data[3];
    int hour = data[4];
    int minute = data[5];
    int second = data[6];

    cerr << "date: " << year << "-" << month << "-" << day << " "
         << hour << ":" << minute << ":" << second
         << endl;

    // production status
    READ(1);

    // type of processed data
    READ(1);
    if (data[0] != 1)
        return error("Not forecast data");

    sectionLen -= 21;
    while (sectionLen > 0) {
        int len = min(sectionLen, 8);
        READ(len);
        sectionLen -= len;
    }

    return true;
}

bool readGrib2(const char *filename)
{
    ifstream fin(filename);

    if (!fin.is_open())
        return error("Cannot open filename");

    int msgLen = 0;
    int skip = 0;

    for(;; skip += msgLen) {
        fin.seekg(skip, ios_base::beg);

        char c = fin.get();
        if (fin.eof())
            break;
        fin.putback(c);

        if (!readIndicatorSection(fin, msgLen))
            continue;
        if (!readIdentificationSection(fin))
            continue;
    }

    return true;
}

}

void rgta_test(const char *grib2_filename)
{
    std::cerr << "open grib2 file " << grib2_filename << std::endl;
    readGrib2(grib2_filename);
}
