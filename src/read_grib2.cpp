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
    https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/
*/

struct Datetime {
    int year, month, day;
    int hour, minute, second;
};

struct Section {
    int len;
    int nbLocal;
    Datetime datetime;
};

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

bool readIndicatorSection(ifstream& fin, Section& section)
{
    char data[8];
    section.len = 0;

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

    section.len = bigendian(*((uint64_t*)data));
    cerr << "msg len: " << section.len << endl;

    return true;
}

bool readIdentificationSection(ifstream& fin, Section& section)
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
    section.nbLocal = data[0];
    cerr << "local tables: " << section.nbLocal << endl;

    // significance of reference time
    READ(1);

    // datetime
    READ(7);

    section.datetime.year = bigendian(*(uint16_t*)data);
    section.datetime.month = data[2];
    section.datetime.day = data[3];
    section.datetime.hour = data[4];
    section.datetime.minute = data[5];
    section.datetime.second = data[6];

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

bool readLocalSection(ifstream& fin)
{
    char data[4];

    // local section len
    READ(4);
    int len = bigendian(*((uint32_t*)data));

    // Section number
    READ(1);

    // all len
    len -= 5;
    while (len > 0) {
        int l = min(len, 4);
        READ(l);
        len -= l;
    }

    return true;
}

bool readGridDefinition(ifstream& fin)
{
    char data[4];

    // len
    READ(4);
    int len = bigendian(*((uint32_t*)data));

    // number of the section
    READ(1);

    // source of grid definition
    READ(1);
    if (data[0] != 0)
        return error("Grid definition other than 0 are not implemented");

    READ(4);
    int nbDataPts = bigendian(*((uint32_t*)data));
    cerr << nbDataPts << " data pts" << endl;
    return true;
}

bool readSection(ifstream& fin, Section& section)
{
    if (!readIndicatorSection(fin, section))
        return false;
    if (!readIdentificationSection(fin, section))
        return false;

    for (int i = 0; i < section.nbLocal; i++)
        if (!readLocalSection(fin))
            return false;

    if (!readGridDefinition(fin))
        return false;

    return true;
}

bool readGrib2(const char *filename)
{
    ifstream fin(filename);

    if (!fin.is_open())
        return error("Cannot open filename");

    Section section;
    section.len = 0;

    int skip = 0;

    for(;; skip += section.len) {
        fin.seekg(skip, ios_base::beg);

        char c = fin.get();
        if (fin.eof())
            break;
        fin.putback(c);

        if (!readSection(fin, section))
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
