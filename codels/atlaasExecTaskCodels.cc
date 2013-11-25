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
#include <libimages3d.h>
#include <atlaas/atlaas.hpp>
#include <gdalwrap/gdal.hpp>

#define P3D_MIN_POINTS 10
#define P3D_SIGMA_VERTICAL 50

static atlaas::atlaas dtm;
static atlaas::points cloud;

static POSTER_ID velodyne_poster_id;
static velodyne3DImage* velodyne_ptr;

static int velodyne_width;
static int velodyne_height;

static DATA_IM3D im3d;
static int im3d_state;

static T3D sensor_to_origin;
static T3D main_to_origin;

static DTM_P3D_POSTER* p3d_poster;

static POSTER_ID pom_poster_id;


static std::ofstream tmplog("/tmp/atlaas.log");

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
  if (im3d_state != 0) {
    empty_data_im3d(&im3d);
    im3d_state = 0;
  }
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
 *              S_atlaas_IM3D_INIT
 */

/* atlaas_init_exec  -  codel EXEC of Init
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_init_exec(geodata *meta, int *report)
{
  /* try..catch ? */
  dtm.init(meta->width, meta->height, meta->scale,
           meta->custom_x, meta->custom_y,
           meta->utm_x, meta->utm_y,
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

  /* Initialize libimages3d : DATA_IM3D */
  if (im3d_state != 0) {
    empty_data_im3d(&im3d);
    im3d_state = 0;
  }
  if (init_data_im3d(&im3d, velodyne_ptr->height,
                     velodyne_ptr->maxScanWidth, 0, 0, 0) != ERR_IM3D_OK) {
    std::cerr << "atlaas: cannot initialize im3d" << std::endl;
    *report = S_atlaas_IM3D_INIT;
    return ETHER;
  }
  im3d_state = 1;

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
  T3D sensor_to_main;

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
}

/** Update libimages3d structure to change cloud's frame
 *
 * shamelessly stolen from dtm-genom/codels/conversion.c
 * TODO rewrite velodyne3DImage to be aligned (faster)
 */
void update_im3d(/* velodyne3DImage* velodyne_ptr */) {
  int i, j;
  DATA_PT3D* ip;
  velodyne3DPoint* vp;
  velodyne_width  = velodyne_ptr->width;
  velodyne_height = velodyne_ptr->height;

  /* Copy the poster into the im3d up to the number of lines that actually
     contains data */
  for (i = 0; i < velodyne_height; i++)
  for (j = 0; j < velodyne_width;  j++) {
    ip = data_im3d_pt(&im3d, i, j);
    vp = velodyne3DImageAt(velodyne_ptr, i, j);
    if (vp->status == VELODYNE_GOOD_3DPOINT) {
      ip->state = GOOD_PT3D;
      ip->coord_1 = vp->coordinates[0];
      ip->coord_2 = vp->coordinates[1];
      ip->coord_3 = vp->coordinates[2];
    } else {
      ip->state = BAD_PT3D;
    }
  }
  /* Flag the points not acquired as BAD_PT3D */
  for (i = 0; i < velodyne_height; i++)
  for (j = velodyne_width; j < velodyne_ptr->maxScanWidth; j++) {
    ip = data_im3d_pt(&im3d, i, j);
    ip->state = BAD_PT3D;
  }

  im3d.header.coordonnees = COORDONNEES_CARTESIENNES;
  im3d.header.capteur = CAPTEUR_LASER1;
  im3d.header.repere = REPERE_CAPTEUR;
  im3d.header.points = POINTS_CALCULES;
}

/** Update the point cloud
 *
 * Fill with all valid points from the libimages3d structure
 * in the custom global frame
 */
void update_cloud() {
  DATA_PT3D* ip;
  /* Resizes the cloud, make sure we will have enough space */
  cloud.resize(velodyne_height * velodyne_width);
  auto it = cloud.begin();

  /* Copy valid points in the point_cloud */
  for (int i = 0; i < velodyne_height; i++)
  for (int j = 0; j < velodyne_width;  j++) {
    ip = data_im3d_pt(&im3d, i, j);
    if (ip->state == GOOD_PT3D) {
      (*it)[0] = ip->coord_1;
      (*it)[1] = ip->coord_2;
      (*it)[2] = ip->coord_3;
      ++it;
    }
  }
  /* Removes the elements in the range [it,end] */
  cloud.erase(it, cloud.end());
}

void update_pos(const POM_POS& pos) {
  t3dInit(&main_to_origin, T3D_BRYAN, T3D_ALLOW_CONVERSION);
  main_to_origin.euler.euler[0] = pos.mainToOrigin.euler.yaw;
  main_to_origin.euler.euler[1] = pos.mainToOrigin.euler.pitch;
  main_to_origin.euler.euler[2] = pos.mainToOrigin.euler.roll;
  main_to_origin.euler.euler[3] = pos.mainToOrigin.euler.x;
  main_to_origin.euler.euler[4] = pos.mainToOrigin.euler.y;
  main_to_origin.euler.euler[5] = pos.mainToOrigin.euler.z;
}

/** Convert from dtm to p3d_poster
 * see: dtm-genom/codels/califeStructToPoster.c : dtm_to_p3d_poster
 *
 * TODO use main_to_origin to define the window in the dtm that will be
 * copied in the p3d_poster
*/
void update_p3d_poster() {
  const atlaas::points_info_t& data = dtm.get_internal();
  const gdalwrap::gdal& map = dtm.get_unsynced_map();
  const auto& origin = map.point_pix2custom(0, 0);
  /* Reset all fields of DTM_P3D_POSTER */
  memset((DTM_P3D_POSTER*) p3d_poster, 0, sizeof (DTM_P3D_POSTER));

  /* header */
  p3d_poster->nbLines = map.get_height();
  p3d_poster->nbCols = map.get_width();
  p3d_poster->zOrigin = 0; /* TODO */
  p3d_poster->xScale = map.get_scale_x();
  p3d_poster->yScale = map.get_scale_y();
  p3d_poster->zScale = 1.0;
  p3d_poster->xOrigin = origin[0];
  p3d_poster->yOrigin = origin[1];

  // TODO use map.index_custom()
  size_t idx = 0;
  for (int i = 0; i < map.get_height(); i++)
  for (int j = 0; j < map.get_width();  j++) {
    const auto& cell = data[idx++];
    if (cell[atlaas::N_POINTS] < P3D_MIN_POINTS) {
      p3d_poster->state[i][j]  = DTM_CELL_EMPTY;
      p3d_poster->zfloat[i][j] = 0.0;
    } else {
      if (cell[atlaas::SIGMA_Z] > P3D_SIGMA_VERTICAL) {
        p3d_poster->zfloat[i][j] = cell[atlaas::Z_MAX];
      } else {
        p3d_poster->zfloat[i][j] = cell[atlaas::Z_MEAN];
      }
      p3d_poster->state[i][j] = DTM_CELL_FILLED;
    }
  }
}

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
  posterTake(velodyne_poster_id, POSTER_READ);
  update_transform();
  update_im3d();
  posterGive(velodyne_poster_id);

  /* Change the cloud's frame, from libimages3d/src/imutil.c */
  if (change_repere_data_im3d (&im3d, &sensor_to_origin) != ERR_IM3D_OK) {
    std::cerr << "atlaas: error in changing the 3D image frame" << std::endl;
    *report = S_atlaas_TRANSFORMATION_ERROR;
    return ETHER;
  }
  update_cloud();
  tmplog << __func__ << " merge cloud of " << cloud.size() << " points" << std::endl;
  dtm.merge(cloud);

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
    dtm.get().save(ATLAAS_FILENAME);
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
    dtm.export8u(ATLAAS_ZMEAN_FILENAME);
  } catch ( std::exception& e ) {
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
  POM_POS pos;

  /* read current robot position main_to_origin from POM */
  if (atlaasPOM_POSPosterRead(pom_poster_id, &(pos)) == ERROR) {
    std::cerr << "atlaas: unable to read POM poster" << std::endl;
    *report = S_atlaas_POM_READ_ERROR;
    return ETHER;
  }
  update_pos(pos);

  posterTake(ATLAAS_P3DPOSTER_POSTER_ID, POSTER_WRITE);
  update_p3d_poster();
  posterGive(ATLAAS_P3DPOSTER_POSTER_ID);

  return ETHER;
}

