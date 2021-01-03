#include "read_grib2.hpp"
#include "grib2dec/grib2dec.h"
#include "utils.hpp"
#include "stream.hpp"
#include "struct.hpp"
#include "sections.hpp"

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace grib2dec;

namespace {

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

bool error(const char *msg)
{
    cerr << "read_grib2 error: " << msg << endl;
    return false;
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
