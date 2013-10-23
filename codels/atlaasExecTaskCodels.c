/**
 ** atlaasExecTaskCodels.c
 **
 ** Codels called by execution task atlaasExecTask
 **
 ** Author: 
 ** Date: 
 **
 **/

#include <portLib.h>

#include "server/atlaasHeader.h"


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
 */

/* atlaas_init_exec  -  codel EXEC of Init
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_init_exec(geodata *meta, int *report)
{
  /* ... add your code here ... */
  return ETHER;
}

/*------------------------------------------------------------------------
 * Connect
 *
 * Description: 
 *
 * Reports:      OK
 *              S_atlaas_POSTER_NOT_FOUND
 *              S_atlaas_POSTERS_NOT_COMPATIBLE
 */

/* atlaas_connect_exec  -  codel EXEC of Connect
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_connect_exec(connect *conn, int *report)
{
  /* ... add your code here ... */
  return ETHER;
}

/*------------------------------------------------------------------------
 * Fuse
 *
 * Description: 
 *
 * Reports:      OK
 *              S_atlaas_NOT_CONNECTED
 */

/* atlaas_fuse_exec  -  codel EXEC of Fuse
   Returns:  EXEC END ETHER FAIL ZOMBIE */
ACTIVITY_EVENT
atlaas_fuse_exec(int *report)
{
  /* ... add your code here ... */
  return ETHER;
}


