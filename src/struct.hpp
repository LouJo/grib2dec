#ifndef __STRUCT_HPP
#define __STRUCT_HPP

#include "grib2dec/types.h"

namespace grib2dec {

typedef G2DEC_Discipline Discipline;
typedef G2DEC_Category Category;
typedef G2DEC_Parameter Parameter;

struct Datetime {
    int year, month, day;
    int hour, minute, second;
};

struct Grid {
    double earthRadius;
    int ni, nj;
    double la1, lo1;
    double la2, lo2;
};

struct Packing {
    int tpl;  // 0, 2 or 3
    int nbValues;
    float R;
    int E;
    int D;
    int sampleBits;
    int valueType;
    int NG;  // number of groups
    int groupWidthRef;
    int groupWidthBits;
    int groupLengthRef;
    int groupLengthInc;
    int lastGroupLength;
    int scaledGroupLengthBits;
    int spatialOrder;
    int extraBytes;
};

struct Message {
    int len;
    int nbLocal;
    Datetime datetime;
    bool complete;
    int lenRead;
    Grid grid;
    Discipline discipline;
    Category category;
    Parameter parameter;
    Packing packing;
};

} // grib2dec

#endif
