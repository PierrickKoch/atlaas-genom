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
    /** width in pixel */
    unsigned long width;

    /** height in pixel */
    unsigned long height;

    /** transform, from gdal.GeoTransform:
     * [0] : top-left pixel position X (in UTM)
     * [1] : w-e pixel resolution
     * [2] : rotation, 0 if image is "north up"
     * [3] : top-left pixel position Y (in UTM)
     * [4] : rotation, 0 if image is "north up"
     * [5] : n-s pixel resolution
     */
    double transform[6];

    /** custom origin
     * [0] : custom X origin (in UTM)
     * [1] : custom Y origin (in UTM)
     */
    double custom[2];

    /** utm
     * [0] : zone
     * [1] : is north ? (boolean)
     */
    int utm[2];

    /** Velodyne poster name */
    char velodyne_poster[POSTER_MAX_LEN];
} geodata;


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
