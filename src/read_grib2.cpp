#include "read_grib2.hpp"
#include "regatta/regatta.h"

#include <stdio.h>
#include <iostream>
#include <vector>

extern "C" {
#include <grib2.h>
}

using namespace regatta;

namespace {

void tryme(const char *filename)
{
    g2int iseek = 0, mseek = 32000, lskip = 0, lgrib = 0;
    FILE *fd = fopen(filename, "rb");
    if (!fd) {
        std::cerr << "file not found" << std::endl;
        return;
    }

    std::vector<uint8_t> data;
    g2int  listsec0[3], listsec1[13], numlocal, numfields;

    while (true) {
        seekgb(fd, iseek, mseek, &lskip, &lgrib);
        if (!lgrib)
            break;
        fseek(fd, lskip, SEEK_SET);
        data.resize(lgrib);
        int len = fread(data.data(), 1, lgrib, fd);
        std::fill_n(listsec0, 3, 0);
        std::fill_n(listsec1, 13, 0);
        int ierr=g2_info(data.data(), listsec0, listsec1, &numfields, &numlocal);

        std::cerr << "section len " << lgrib << " err " << ierr
                  << " date " << listsec1[5] << "/" << listsec1[6] << "/" << listsec1[7]
                  << " " << listsec1[8] << ":" << listsec1[9] << ":" << listsec1[10]
                  << " fields " << numfields
                  << std::endl;

        if(0) 
        for (int n=0; n < numfields; n++) {
            gribfield  *gfld;
            ierr=g2_getfld(data.data(), n + 1, 1, 1, &gfld);
            g2_free(gfld);
        }

        iseek = lskip + lgrib;
    }
}

}

void rgta_test(const char *grib2_filename)
{
    std::cerr << "open grib2 file " << grib2_filename << std::endl;
    tryme(grib2_filename);
}
