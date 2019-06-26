/******************************************************************************
                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************
   Module      : $RCSfile: drv_tapi_vxworks_bsp.c,v $
   Date        : $Date: 2003/12/06 19:07:47 $
   Description : TAPI Driver, VxWorks BSP part
******************************************************************************/

/* ============================= */
/* Group=Main                    */
/* ============================= */


#if defined(VXWORKS) && defined(INCLUDE_TAPI)
/*
  sorry for this style, but this is a easy way to include the driver code
  into the board support package of VxWorks
*/
   #include "drv_tapi_config.h"
   #include "drv_tapi_api.h"
   #include "drv_tapi_vxworks.c"
   #include "drv_tapi_common.c"
#endif
