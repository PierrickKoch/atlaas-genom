/**
 ** atlaasExecTaskCodels.cc
 **
 ** Codels called by execution task atlaasExecTask
 **
 ** Author: Pierrick Koch <pierrick.koch@laas.fr>
 ** Date:   2013-11-20
 **
 **/

#include <iostream>
#include <fstream>

#include <portLib.h>

#include "server/atlaasHeader.h"

#include <t3d/t3d.h>
#include <atlaas/atlaas.hpp>
#include <gdalwrap/gdal.hpp>

#define P3D_MIN_POINTS 10
#define P3D_SIGMA_VERTICAL 50

static atlaas::atlaas dtm;
static atlaas::points cloud;
static atlaas::matrix transform;

// for Fuse
static POSTER_ID velodyne_poster_id;
static velodyne3DImage* velodyne_ptr;
// for FillP3D
static double pom_x, pom_y;
static POSTER_ID pom_poster_id;
static DTM_P3D_POSTER* p3d_poster;

static std::ofstream tmplog("atlaas-genom.log");

/*------------------------------------------------------------------------
 *
 * atlaas_exec_task_init  --  Initialization codel (fIDS, ...)
 *
 * Description: 
 * 
 * Returns:    OK or ERROR
 */

STATUS
atlaas_exec_task_init(int *report)
{
  p3d_poster = (DTM_P3D_POSTER*) posterAddr(ATLAAS_P3DPOSTER_POSTER_ID);
  if (p3d_poster == NULL)
    return ERROR;

  tmplog << __func__ << std::endl;
  return OK;
}

/*------------------------------------------------------------------------
 *
 * atlaas_exec_task_end  --  Termination codel
 *
 * Description: 
 * 
 * Returns:    OK or ERROR
 */

STATUS
atlaas_exec_task_end(void)
{
  tmplog << __func__ << std::endl;
  tmplog.close();
  return OK;
}

/*------------------------------------------------------------------------
 * Init
 *
 * Description: 
 *
 * Reports:      OK
 *              S_atlaas_POSTER_NOT_FOUND
 */

/* atlaas_init_exec  -  codel EXEC of Init
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_init_exec(geodata *meta, int *report)
{
  /* try..catch ? */
  dtm.init(meta->width, meta->height, meta->scale,
           meta->custom_x, meta->custom_y, meta->custom_z,
           meta->utm_zone, meta->utm_north );

  /* Look up for POM Poster */
  if (posterFind(meta->pom_poster, &pom_poster_id) == ERROR) {
    std::cerr << "atlaas: cannot find pom poster" << std::endl;
    *report = S_atlaas_POSTER_NOT_FOUND;
    return ETHER;
  }

  /* Look up for Velodyne poster */
  if (posterFind(meta->velodyne_poster, &velodyne_poster_id) == ERROR) {
    std::cerr << "atlaas: cannot find velodyne poster" << std::endl;
    *report = S_atlaas_POSTER_NOT_FOUND;
    return ETHER;
  }

  /* Get Velodyne data address */
  velodyne_ptr = (velodyne3DImage*) posterAddr(velodyne_poster_id);
  if (velodyne_ptr == NULL) {
    *report = S_atlaas_POSTER_NOT_FOUND;
    return ETHER;
  }

  /* Increase the capacity of the point cloud */
  cloud.reserve(velodyne_ptr->height * velodyne_ptr->maxScanWidth);

  tmplog << __func__ << " cloud [" << velodyne_ptr->height << ", " <<
         velodyne_ptr->maxScanWidth << "]" << std::endl;

  return ETHER;
}

/** Update transformations
 *
 * sensor -> origin = sensor -> main -> origin
 */
void update_transform(/* velodyne3DImage* velodyne_ptr */) {
  T3D sensor_to_main, main_to_origin, sensor_to_origin;

  t3dInit(&sensor_to_origin,  T3D_BRYAN, T3D_ALLOW_CONVERSION);
  t3dInit(&sensor_to_main,    T3D_BRYAN, T3D_ALLOW_CONVERSION);
  t3dInit(&main_to_origin,    T3D_BRYAN, T3D_ALLOW_CONVERSION);
  /* Now get the relevant T3D from the PomSensorPos of the velodyne poster */
  memcpy(&sensor_to_main.euler.euler,
   &(velodyne_ptr->position.sensorToMain.euler), sizeof(POM_EULER));
  memcpy(&main_to_origin.euler.euler,
   &(velodyne_ptr->position.mainToOrigin.euler), sizeof(POM_EULER));
  /* Compose the T3Ds to obtain sensor to origin transformation */
  t3dCompIn(&sensor_to_origin, &sensor_to_main, &main_to_origin);
  t3dConvertTo(T3D_MATRIX, &sensor_to_origin);
  // use `reinterpret_cast` to pass c-array as an std::array
  // `const` make sure array::operator=() copy the content.
  transform = reinterpret_cast<const atlaas::matrix&>(sensor_to_origin.matrix.matrix);
}

/** Update the point cloud in sensor frame (need transform)
 *
 * Fill with all valid points from the velodyne3DImage structure
 * TODO rewrite velodyne3DImage to be aligned (faster)
 */
void update_cloud(/* velodyne3DImage* velodyne_ptr */) {
  velodyne3DPoint* vp;

  /* Resizes the cloud, make sure we will have enough space */
  cloud.resize(velodyne_ptr->height * velodyne_ptr->width);
  auto it = cloud.begin();

  /* Copy the poster into the point cloud
     up to the number of lines that actually contains data */
  for (int i = 0; i < velodyne_ptr->height; i++)
  for (int j = 0; j < velodyne_ptr->width;  j++) {
    vp = velodyne3DImageAt(velodyne_ptr, i, j);
    if (vp->status == VELODYNE_GOOD_3DPOINT) {
      (*it)[0] = vp->coordinates[0];
      (*it)[1] = vp->coordinates[1];
      (*it)[2] = vp->coordinates[2];
      ++it;
    }
  }
  /* Removes the elements in the range [it,end] */
  cloud.erase(it, cloud.end());
}

STATUS update_pos() {
  POM_POS pos;
  /* read current robot position main_to_origin from POM */
  if (atlaasPOM_POSPosterRead(pom_poster_id, &(pos)) == ERROR)
    return ERROR;

  pom_x = pos.mainToOrigin.euler.x;
  pom_y = pos.mainToOrigin.euler.y;
  return OK;
}

/** Convert from dtm to p3d_poster
 * see: dtm-genom/codels/califeStructToPoster.c : dtm_to_p3d_poster
 *
 * use pom_{x,y} to define the window in the dtm that will be
 * copied in the p3d_poster DTM_MAX_{LINES,COLUMNS}
*/
void update_p3d_poster() {
  int delta, x_min, x_max, y_min, y_max;
  // we use internal data, faster to convert to P3D structure
  const atlaas::cells_info_t& data = dtm.get_internal();
  // we use the map only for meta-data, so no need to update it
  const gdalwrap::gdal& map = dtm.get_unsynced_map();
  /* Reset all fields of DTM_P3D_POSTER */
  memset((DTM_P3D_POSTER*) p3d_poster, 0, sizeof (DTM_P3D_POSTER));

  /* robot pose */
  const gdalwrap::point_xy_t& ppx_robot = map.point_custom2pix(pom_x, pom_y);

  /* header */
  p3d_poster->nbLines = DTM_MAX_LINES; // "x"
  p3d_poster->nbCols  = DTM_MAX_COLUMNS; // "y"
  p3d_poster->zOrigin = 0; /* TODO */
  p3d_poster->xScale  = map.get_scale_x();
  p3d_poster->yScale  = map.get_scale_y();
  p3d_poster->zScale  = 1.0;

  x_min = ppx_robot[0] - p3d_poster->nbLines / 2;
  x_max = ppx_robot[0] + p3d_poster->nbLines / 2;
  if (x_min < 0) {
    tmplog << __func__ << " shrink [-x] " << x_min << std::endl;
    p3d_poster->nbLines += x_min;
    x_min = 0;
  } else {
    delta = map.get_height() - x_max;
    if (delta < 0) {
      tmplog << __func__ << " shrink [+x] " << delta << std::endl;
      p3d_poster->nbLines += delta;
      x_max = map.get_height();
    }
  }

  y_min = ppx_robot[1] - p3d_poster->nbCols / 2;
  y_max = ppx_robot[1] + p3d_poster->nbCols / 2;
  if (y_min < 0) {
    tmplog << __func__ << " shrink [-y] " << y_min << std::endl;
    p3d_poster->nbCols += y_min;
    y_min = 0;
  } else {
    delta = map.get_width() - y_max;
    if (delta < 0) {
      tmplog << __func__ << " shrink [+y] " << delta << std::endl;
      p3d_poster->nbCols += delta;
      y_max = map.get_width();
    }
  }
  const gdalwrap::point_xy_t& custom_origin = map.point_pix2custom(x_min, y_min);
  p3d_poster->xOrigin = custom_origin[0];
  p3d_poster->yOrigin = custom_origin[1];

  for (int pi = 0, di = x_min; di < x_max; pi++, di++)
  for (int pj = 0, dj = y_min; dj < y_max; pj++, dj++) {
    const auto& cell = data[ map.index_pix(di, dj) ];
    if (cell[atlaas::N_POINTS] < P3D_MIN_POINTS) {
      p3d_poster->state[pi][pj]  = DTM_CELL_EMPTY;
      p3d_poster->zfloat[pi][pj] = 0.0;
    } else {
      if (cell[atlaas::VARIANCE] > P3D_SIGMA_VERTICAL) {
        p3d_poster->zfloat[pi][pj] = cell[atlaas::Z_MAX];
      } else {
        p3d_poster->zfloat[pi][pj] = cell[atlaas::Z_MEAN];
      }
      p3d_poster->state[pi][pj] = DTM_CELL_FILLED;
    }
  }
}

// RAII patern
class poster_locker {
  const POSTER_ID& poster_id;
public:
  poster_locker(const POSTER_ID& id, POSTER_OP op) : poster_id(id) {
    posterTake(poster_id, op);
  }
  ~poster_locker() {
    posterGive(poster_id);
  }
};

/*------------------------------------------------------------------------
 * Fuse
 *
 * Description: 
 *
 * Reports:      OK
 *              S_atlaas_POSTER_NOT_FOUND
 *              S_atlaas_TRANSFORMATION_ERROR
 */

/* atlaas_fuse_exec  -  codel EXEC of Fuse
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_fuse_exec(int *report)
{
  tmplog << __func__ << std::endl;
  try {
    {
      poster_locker lock( velodyne_poster_id, POSTER_READ );
      update_transform();
      update_cloud();
    }
    tmplog << __func__ << " merge cloud of " << cloud.size() << " points" << std::endl;
    dtm.merge(cloud, transform); // fuse/merge is done here
    tmplog << __func__ << " merged " << std::endl;
  } catch ( std::exception& e ) {
    tmplog << __func__ << " error '" << e.what() << "'" << std::endl;
    std::cerr << __func__ << " error '" << e.what() << "'" << std::endl;
    *report = S_atlaas_TRANSFORMATION_ERROR;
  }
  return ETHER;
}

/*------------------------------------------------------------------------
 * Save
 *
 * Description:
 *
 * Reports:      OK
 *              S_atlaas_WRITE_ERROR
 */

/* atlaas_save_exec  -  codel EXEC of Save
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_save_exec(int *report)
{
  tmplog << __func__ << std::endl;
  try {
    dtm.save_currents();
  } catch ( std::exception& e ) {
    std::cerr << "atlaas::save failed, with message '" << e.what() << "'" << std::endl;
    *report = S_atlaas_WRITE_ERROR;
  }
  return ETHER;
}

/*------------------------------------------------------------------------
 * Export8u
 *
 * Description:
 *
 * Reports:      OK
 *              S_atlaas_WRITE_ERROR
 */

/* atlaas_export8u_exec  -  codel EXEC of Export8u
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_export8u_exec(int *report)
{
  tmplog << __func__ << std::endl;
  try {
    dtm.export8u(ATLAAS_HEIGHTMAP);
  } catch ( std::exception& e ) {
    tmplog << __func__ << "[ERROR] '" << e.what() << "'" << std::endl;
    std::cerr << "atlaas::export8u failed, with message '" << e.what() << "'" << std::endl;
    *report = S_atlaas_WRITE_ERROR;
  }
  return ETHER;
}

/*------------------------------------------------------------------------
 * FillP3D
 *
 * Description:
 *
 * Reports:      OK
 *              S_atlaas_POM_READ_ERROR
 */

/* atlaas_fill_p3d  -  codel EXEC of FillP3D
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_fill_p3d(int *report)
{
  tmplog << __func__ << std::endl;
  if (update_pos() == ERROR) {
    std::cerr << "atlaas: unable to read POM poster" << std::endl;
    *report = S_atlaas_POM_READ_ERROR;
    return ETHER;
  }
  poster_locker lock( ATLAAS_P3DPOSTER_POSTER_ID, POSTER_WRITE );
  update_p3d_poster();

  return ETHER;
}

