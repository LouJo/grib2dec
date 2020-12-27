#include "read_grib2.hpp"
#include "regatta/regatta.h"

#include <stdio.h>
#include <iostream>

#include <grib2.h>

using namespace regatta;

namespace {

void tryme()
{
    g2int a, b, *c, *d;
    FILE *fd = fopen("/tmp/r", "rb");
    seekgb(fd, a, b, c, d);
}

}

void rgta_test()
{
    std::cout << "hello" << std::endl;
}
