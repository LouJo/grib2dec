#include <grib2dec/grib2dec.hpp>

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
        return -1;

    grib2dec::Grib2Dec *decoder = grib2dec::Grib2Dec::create(argv[1]);
    int nbMessages = 0;

    while (true) {
        G2DEC_Message message;
        auto status = decoder->nextMessage(message);
        if (status != G2DEC_STATUS_OK)
            break;
        nbMessages++;
    }

    cout << nbMessages << " messages read" << endl;

    delete decoder;

    return 0;
}
