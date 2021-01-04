#ifndef __GRIB2DEC_H
#define __GRIB2DEC_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Library handle
 */
typedef void* G2DEC_Handle;

/**
 * Open library with a file path.
 * If file cannot be open, NULL is returned.
 */
G2DEC_Handle G2DEC_Open(const char *filename);

/**
 * Set spatial filtering for data points.
 *
 * When filter is set, lon1, lon2, lat1 and lat2 in message.grid are set
 * accordingly to filter
 */
G2DEC_Status G2DEC_setSpatialFilter(G2DEC_Handle handle,
                                    const G2DEC_SpatialFilter *filter);

/**
 * Read next message.
 *
 * message structure will be fullfilled only if returned status is OK.
 * call this method until END status is returned.
 */
G2DEC_Status G2DEC_nextMessage(G2DEC_Handle handle, G2DEC_Message *message);

/**
 * Close library
 */
void G2DEC_Close(G2DEC_Handle handle);

#ifdef __cplusplus
}
#endif

#endif
