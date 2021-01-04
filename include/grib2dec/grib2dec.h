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
