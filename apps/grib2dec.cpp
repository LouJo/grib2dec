#include <grib2dec/grib2dec.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
        return -1;

    rgta_test(argv[1]);
    return 0;
}
