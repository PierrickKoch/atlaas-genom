#ifndef ATLAASSTRUCT_H
#define ATLAASSTRUCT_H

#define POSTER_MAX_LEN 256

/* DTM_MAX_{LINES,COLUMNS} must be even,
   so that the structure is aligned with respect to double */
#define DTM_MAX_LINES   200
#define DTM_MAX_COLUMNS 200

#define ATLAAS_FILENAME "/tmp/atlaas.tif"


/** atlaas struct
 *
 * This file aims at defining a generic structur for GeoData.
 * To be used by Genom / pocolibs.
 * For file I/O, please use the gdal wrapper available online
 * https://github.com/pierriko/gdalwrap
 */

typedef struct geodata {
    /** width in meters */
    double width;

    /** height in meters */
    double height;

    /** scale of a pixel in meters */
    double scale;

    /** custom X origin in UTM in meters */
    double custom_x;

    /** custom Y origin in UTM in meters */
    double custom_y;

    /** UTM (X) origin in meters, top-left pixel position X (in UTM) */
    double utm_x;

    /** UTM (Y) origin in meters, top-left pixel position Y (in UTM) */
    double utm_y;

    /** UTM zone */
    int utm_zone;

    /** is UTM north? (boolean) */
    int utm_north;

    /** Velodyne poster name */
    char velodyne_poster[POSTER_MAX_LEN];
} geodata;

/** P3D_POSTER for local path planner
 * from dtm-genom/dtmStruct.h
 */
typedef enum demCellState {DTM_CELL_EMPTY, DTM_CELL_FILLED} demCellState;

typedef struct DTM_P3D_POSTER {
    int nbLines;
    int nbCols;

    /** Coordinates of the topleft corner
     * (0,0) pixel in the global frame
     */
    float xOrigin;
    float yOrigin;

    /** Size of a cell in meters */
    float xScale;
    float yScale;

    /** Elevation of the '0' value */
    float zOrigin;
    /** Scale to store the elevation */
    float zScale;

    /** To know if a cell is EMPTY or FILLED */
    demCellState state[DTM_MAX_LINES][DTM_MAX_COLUMNS];

    /** Altitude in float */
    float zfloat[DTM_MAX_LINES][DTM_MAX_COLUMNS];
} DTM_P3D_POSTER;


#endif /* ATLAASSTRUCT_H */
