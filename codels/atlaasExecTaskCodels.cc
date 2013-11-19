/**
 ** atlaasExecTaskCodels.cc
 **
 ** Codels called by execution task atlaasExecTask
 **
 ** Author: 
 ** Date: 
 **
 **/

#include <portLib.h>

#include "server/atlaasHeader.h"

#include <t3d/t3d.h>
#include <atlaas/atlaas.hpp>

static atlaas::atlaas dtm;

static POSTER_ID velodyne_poster_id;
static velodyne3DImage* velodyne_ptr;

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
  /* ... add your code here ... */
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
  return OK;
}

/*------------------------------------------------------------------------
 * Init
 *
 * Description: 
 *
 * Reports:      OK
 *              S_atlaas_BAD_INIT_PARAMS
 *              S_atlaas_INTERNAL_ATLAAS_CREATION_FAILED
 *              S_atlaas_INTERNAL_ATLAAS_INIT_FAILED
 *              S_atlaas_POSTER_NOT_FOUND
 *              S_atlaas_POSTERS_NOT_COMPATIBLE
 */

/* atlaas_init_exec  -  codel EXEC of Init
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_init_exec(geodata *meta, int *report)
{
  /* try..catch ? */
  dtm.init(meta->width, meta->height,
           meta->transform[1], /* scale (W-E) */
           meta->custom[0], meta->custom[1],
           meta->transform[0], meta->transform[3],
           meta->utm[0], meta->utm[1] );

  /* Look up for Velodyne poster */
  if (posterFind(meta->velodyne_poster, &velodyne_poster_id) == ERROR) {
    fprintf(stderr, "atlaas: cannot find velodyne poster\n");
    *report = S_atlaas_POSTER_NOT_FOUND;
    return ETHER;
  }

  /* Get Velodyne data address */
  velodyne_ptr = (velodyne3DImage*) posterAddr(velodyne_poster_id);
  if (velodyne_ptr == NULL) {
    *report = S_atlaas_POSTER_NOT_FOUND;
    return ETHER;
  }

  return ETHER;
}


atlaas::point6d to_origin(/* velodyne3DImage* velodyne_ptr */) {
  T3D t3d_sensor_to_origin;
  T3D t3d_sensor_to_main;
  T3D t3d_main_to_origin;
  atlaas::point6d sensor_to_origin;

  t3dInit(&t3d_sensor_to_origin, T3D_BRYAN, T3D_ALLOW_CONVERSION);
  t3dInit(&t3d_sensor_to_main,   T3D_BRYAN, T3D_ALLOW_CONVERSION);
  t3dInit(&t3d_main_to_origin,   T3D_BRYAN, T3D_ALLOW_CONVERSION);
  /* Now get the relevant T3D from the PomSensorPos of the velodyne poster */
  memcpy(&t3d_sensor_to_main.euler.euler,
   &(velodyne_ptr->position.sensorToMain.euler), sizeof(POM_EULER));
  memcpy(&t3d_main_to_origin.euler.euler,
   &(velodyne_ptr->position.mainToOrigin.euler), sizeof(POM_EULER));
  /* Compose the T3Ds to obtain sensor to origin transformation */
  t3dCompIn(&t3d_sensor_to_origin, &t3d_sensor_to_main, &t3d_main_to_origin);

  /* TODO fill sensor_to_origin from covariance matrix in t3d */
  return sensor_to_origin;
}

atlaas::points cloud(/* velodyne3DImage* velodyne_ptr */) {
  atlaas::points point_cloud;
  int height = velodyne_ptr->height;
  int width  = velodyne_ptr->width;

  /* Copy valid points in the point_cloud
    see: velodyne-genom/velodyneClient.h : velodyne3DImage
    and  dtm-genom/codels/conversion.c : dtm_fillIm3dFromVyn3DImage */
  for (int i = 0; i < height; i++)
  for (int j = 0; j < width;  j++) {
    const velodyne3DPoint &vp = velodyne_ptr->points[i * width + j];
    if (vp.status == VELODYNE_GOOD_3DPOINT) {
      atlaas::point_xyz_t point = {
        vp.coordinates[0],
        vp.coordinates[1],
        vp.coordinates[2] };
      point_cloud.push_back( point );
    }
  }

  return point_cloud;
}


/*------------------------------------------------------------------------
 * Fuse
 *
 * Description: 
 *
 * Reports:      OK
 *              S_atlaas_POSTER_NOT_FOUND
 */

/* atlaas_fuse_exec  -  codel EXEC of Fuse
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_fuse_exec(int *report)
{
  /* Get the velodyne poster, fills the internal DATA_IM3D and TR3D with it  */
  if (atlaasvelodyne3DImagePosterRead (velodyne_poster_id, velodyne_ptr) != OK) {
    fprintf (stderr, "atlaas: can not read Velodyne poster\n");
    *report = S_atlaas_POSTER_NOT_FOUND;
    return ETHER;
  }
  dtm.merge(cloud(), to_origin());

  return ETHER;
}
