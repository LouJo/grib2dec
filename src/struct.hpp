#ifndef __STRUCT_HPP
#define __STRUCT_HPP

#include "grib2dec/types.h"

namespace grib2dec {

typedef G2DEC_Discipline Discipline;
typedef G2DEC_Category Category;
typedef G2DEC_Parameter Parameter;
typedef G2DEC_Datetime Datetime;

struct Grid {
    double earthRadius;
    int ni = 0;
    int nj = 0;
    double la1, lo1;
    double la2, lo2;
};

struct Packing {
    int tpl = -1;  // 0, 2 or 3
    int nbValues = 0;
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
    int len = 0;
    Datetime datetime;
    bool complete = false;
    int lenRead = 0;
    Grid grid;
    Discipline discipline = G2DEC_DISCIPLINE_UNKNOWN;
    Category category = G2DEC_CATEGORY_UNKNOWN;
    Parameter parameter = G2DEC_PARAMETER_UNKNOWN;
    Packing packing;
};

} // grib2dec

#endif
