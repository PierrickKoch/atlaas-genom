#ifndef ATLAASSTRUCT_H
#define ATLAASSTRUCT_H

#define POSTER_MAX_LEN 256

/* DTM_MAX_{LINES,COLUMNS} must be even,
   so that the structure is aligned with respect to double */
#define DTM_MAX_LINES   200
#define DTM_MAX_COLUMNS 200

#define ATLAAS_HEIGHTMAP "atlaas.jpg"

/** Default init args */
#define ATLAAS_DEFAULT_WIDTH     50.0
#define ATLAAS_DEFAULT_HEIGHT    50.0
#define ATLAAS_DEFAULT_SCALE     0.1
#define ATLAAS_DEFAULT_CUSTOM_X  0.0
#define ATLAAS_DEFAULT_CUSTOM_Y  0.0
#define ATLAAS_DEFAULT_CUSTOM_Z  0.0
#define ATLAAS_DEFAULT_UTM_ZONE  31
#define ATLAAS_DEFAULT_UTM_NORTH 1
#define ATLAAS_DEFAULT_VELODYNE_POSTER "velodyneThreeDImage"
#define ATLAAS_DEFAULT_POM_POSTER "pomPos"

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

    /** custom Z origin in UTM in meters */
    double custom_z;

    /** UTM zone */
    int utm_zone;

    /** is UTM north? (boolean) */
    int utm_north;

    /** Velodyne poster name */
    char velodyne_poster[POSTER_MAX_LEN];

    /** POM poster name */
    char pom_poster[POSTER_MAX_LEN];
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
