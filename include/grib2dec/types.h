#ifndef __G2DEC_TYPES_H
#define __G2DEC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Status
 */
typedef enum {
    G2DEC_STATUS_OK = 0,

    G2DEC_STATUS_END,
    G2DEC_STATUS_PARSE_ERROR,
    G2DEC_STATUS_END_OF_STREAM_ERROR,
    G2DEC_STATUS_NOT_IMPLEMENTED,
} G2DEC_Status;

/**
 * Product discipline
 */
typedef enum {
    G2DEC_DISCIPLINE_UNKNOWN = -1,

    G2DEC_DISCPLINE_METEOROLOGICAL = 0,
    G2DEC_DISCPLINE_HYDROLOGIC = 1,
    G2DEC_DISCPLINE_LAND_SURFACE = 2,
    G2DEC_DISCPLINE_SPACE = 3,
    G2DEC_DISCPLINE_OCEANOGRAPHIC = 10

} G2DEC_Discipline;

/**
 * Parameter category, depending of discipline.
 * Value = 1000 * discipline + parameter
 */
typedef enum {
    G2DEC_CATEGORY_UNKNOWN = -1,

    // meteorological
    G2DEC_CATEGORY_TEMPERATURE = 0,
    G2DEC_CATEGORY_MOISTURE = 1,
    G2DEC_CATEGORY_MOMENTUM = 2,
    G2DEC_CATEGORY_MASS = 3,
    G2DEC_CATEGORY_SHORT_WAVE_RADIATION = 4,
    G2DEC_CATEGORY_LONG_WAVE_RADIATION = 5,
    G2DEC_CATEGORY_CLOUD = 6,
    G2DEC_CATEGORY_THERMODYNAMIC_STABILITY_INDICES = 7,
    G2DEC_CATEGORY_AEROSOLS = 13,
    G2DEC_CATEGORY_TRACE_GASES = 14,
    G2DEC_CATEGORY_RADAR = 15,
    G2DEC_CATEGORY_NUCLEAR = 18,
    G2DEC_CATEGORY_PHYSICAL_ATMOSPHERIC = 19,
    G2DEC_CATEGORY_ASCII_STRING = 253,

    // hydrologic
    G2DEC_CATEGORY_HYDROLOGY_BASIC_PRODUCTS = 1000,
    G2DEC_CATEGORY_HYDROLOGY_PROBABILITIES = 1001,

    // land surface
    G2DEC_CATEGORY_VEGETATION = 2000,
    G2DEC_CATEGORY_SOIL_PRODUCTS = 2003,

    // space products
    G2DEC_CATEGORY_IMAGE_FORMAT = 3000,
    G2DEC_CATEGORY_QUANTITATIVE = 3001,

    // oceanographic
    G2DEC_CATEGORY_WAVES = 10000,
    G2DEC_CATEGORY_CURRENTS = 10001,
    G2DEC_CATEGORY_ICE = 10002,
    G2DEC_CATEGORY_SURFACE_PROPERTIES = 10003,
    G2DEC_CATEGORY_SUB_SURFACE_PROPERTIES = 10004,
} G2DEC_Category;

/**
 * Parameter, depending of discipline and category.
 * Value = 1000 * category + parameter
 *
 * see specs in doc/ folder, Table 4.2
 * feel free to add more :)
 */
typedef enum {
    G2DEC_PARAMETER_UNKNOWN = -1,

    G2DEC_PARAMETER_WIND_U = 2002,
    G2DEC_PARAMETER_WIND_V = 2003,
} G2DEC_Parameter;

/**
 * Date structure
 */
typedef struct G2DEC_Datetime {
    int year, month, day;
    int hour, minute, second;
} G2DEC_Datetime;

/**
 * Message structure
 */
typedef struct G2DEC_Message {
    G2DEC_Discipline discipline;
    G2DEC_Category category;
    G2DEC_Parameter parameter;
    G2DEC_Datetime datetime;

    double *values;
    int valuesLength;
} G2DEC_Message;

#ifdef __cplusplus
}
#endif

#endif
