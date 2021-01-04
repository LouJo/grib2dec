#include "grib2dec/grib2dec.h"
#include "decoder.hpp"

using namespace grib2dec;


G2DEC_Handle G2DEC_Open(const char *filename)
{
    Grib2Dec *decoder = Grib2Dec::create(filename);
    return decoder;
}

G2DEC_Status G2DEC_setSpatialFilter(G2DEC_Handle handle,
                                    const G2DEC_SpatialFilter *filter)
{
    if (!handle || !filter)
        return G2DEC_STATUS_ERROR;

    return reinterpret_cast<Grib2Dec*>(handle)->setSpatialFilter(*filter);
}

G2DEC_Status G2DEC_nextMessage(G2DEC_Handle handle, G2DEC_Message *message)
{
    if (!handle || !message)
        return G2DEC_STATUS_ERROR;

    return reinterpret_cast<Grib2Dec*>(handle)->nextMessage(*message);
}

void G2DEC_Close(G2DEC_Handle handle)
{
    Grib2Dec *decoder = reinterpret_cast<Grib2Dec*>(handle);
    delete decoder;
}
