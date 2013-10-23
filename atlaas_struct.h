#ifndef ATLAASSTRUCT_H
#define ATLAASSTRUCT_H

#define POSTER_MAX_LEN 256

/** atlaas struct
 *
 * This file aims at defining a generic structur for GeoData.
 * To be used by Genom / pocolibs.
 * For file I/O, please use the gdal wrapper available online
 * https://github.com/pierriko/gdalwrap
 */

typedef struct {
    unsigned long size;
    float data[512*512*3];
} raster;


typedef struct {
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
} geodata;

typedef struct {
    int pom;
    char pom_poster[POSTER_MAX_LEN];
    int velodyne;
    char velodyne_poster[POSTER_MAX_LEN];
} connect;

typedef struct {
    /** params */
    geodata meta;
    connect conn;
    /** actual data */
    raster band;
} atlaas_t;

#endif /* ATLAASSTRUCT_H */
