#include <grib2dec/grib2dec.hpp>
#include <grib2dec/grib2dec.h>

#include <iostream>
#include <assert.h>

using namespace std;

int usage()
{
    cerr << "grib2dec : demo program for grib2dec library" << endl;
    cerr << "usage:" << endl;
    cerr << " -i | --input-file : input file in grib2 format" << endl;
    cerr << " -o | --output-file : output file for parsed data (- for stdout)" << endl;
    cerr << " -f | --format  txt|svg : output format" << endl;
    cerr << " --lat-min : minimum latitude in degree" << endl;
    cerr << " --lat-max : maximum latitude in degree" << endl;
    cerr << " --lon-min : minimum longitude in degree" << endl;
    cerr << " --lon-max : maximum longitude in degree" << endl;

    return -1;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return usage();

    grib2dec::Grib2Dec *decoder = grib2dec::Grib2Dec::create(argv[1]);
    int nbMessages = 0;

    while (true) {
        G2DEC_Message message;
        auto status = decoder->nextMessage(message);
        if (status == G2DEC_STATUS_END)
            break;
        else if (status == G2DEC_STATUS_OK) {
            nbMessages++;
            cerr << message.valuesLength << " pts read" << endl;
            assert(message.valuesLength == message.grid.ni * message.grid.nj);
        }
    }

    cout << nbMessages << " messages read" << endl;

    delete decoder;

    return 0;
}
