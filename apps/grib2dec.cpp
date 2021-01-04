#include "output/output.hpp"

#include <grib2dec/grib2dec.hpp>
#include <grib2dec/grib2dec.h>

#include <iostream>
#include <assert.h>
#include <string.h>

using namespace std;
using namespace grib2dec_demo;

struct Parameters {
    string inputFile;
    string outputFile;
    string outputFormat;
    G2DEC_SpatialFilter filter;
};

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

int error(const char *msg, const char *msg2 = nullptr)
{
    cerr << "error: " << msg;
    if (msg2)
        cerr << msg2;
    cerr << endl;
    return -1;
}

bool parseArguments(int argc, char *argv[], Parameters& params)
{
    memset(&params.filter, 0, sizeof(params.filter));

    for (int i = 1; i < argc; i++) {
        string arg = string(argv[i]);
        if (i == argc - 1)
            continue; // all arguments have a parameter
        if (arg == "-i" || arg == "--input-file")
            params.inputFile = argv[++i];
        else if (arg == "-o" || arg == "--output-file")
            params.outputFile = argv[++i];
        else if (arg == "-f" || arg == "--format")
            params.outputFormat = argv[++i];
        else if (arg == "--lat-min")
            params.filter.latMin = atoi(argv[++i]);
        else if (arg == "--lat-max")
            params.filter.latMax = atoi(argv[++i]);
        else if (arg == "--lon-min")
            params.filter.lonMin = atoi(argv[++i]);
        else if (arg == "--lon-max")
            params.filter.lonMax = atoi(argv[++i]);
        else
            return error("unknown argument ", arg.c_str()), false;
    }

    if (params.inputFile.empty())
        return error("input file needed"), false;

    if (!params.outputFile.empty() && params.outputFormat.empty())
        params.outputFormat = "txt";

    return true;
}

int main(int argc, char *argv[])
{
    Parameters params;

    if (!parseArguments(argc, argv, params))
        return -1;

    grib2dec::Grib2Dec *decoder =
            grib2dec::Grib2Dec::create(params.inputFile.c_str());

    if (!decoder)
        return -1;

    decoder->setSpatialFilter(params.filter);

    Output *output = Output::create(params.outputFile, params.outputFormat);

    int nbMessages = 0;

    while (true) {
        G2DEC_Message message;
        auto status = decoder->nextMessage(message);
        if (status == G2DEC_STATUS_END)
            break;
        else if (status == G2DEC_STATUS_OK) {
            assert(message.valuesLength == message.grid.ni * message.grid.nj);
            output->setComponent(message);
            nbMessages++;
        }
    }

    delete output;
    delete decoder;

    return 0;
}
