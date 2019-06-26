/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DSL Device Simulator
*/

#include "dsl_cpe_control.h"
#include "dsl_cpe_os.h"
#ifdef DSL_CPE_SIMULATOR_DRIVER
#include "drv_dsl_cpe_os_win32.h"
#endif /* DSL_CPE_SIMULATOR_DRIVER*/


#if defined(DSL_CPE_SIMULATOR_CONTROL) && !defined(DSL_CPE_SIMULATOR_DRIVER)
DSL_int_t DSL_CPE_SIM_Open(void *device, const char *appendix);
DSL_int_t DSL_SIM_Close( void *pprivate );
DSL_int_t DSL_SIM_Write( void *pprivate, const char *buf, const int len );
DSL_int_t DSL_SIM_Read( void *pprivate, char *buf, const int len );
DSL_int_t DSL_SIM_Ioctl( void *pprivate, unsigned int cmd, unsigned long arg );
DSL_int_t DSL_SIM_Poll( void *pprivate );
#endif /* defined(DSL_CPE_SIMULATOR_CONTROL) && !defined(DSL_CPE_SIMULATOR_DRIVER)*/

#if defined(DSL_CPE_SIMULATOR_CONTROL)
DSL_Error_t DSL_SimulatorInitialize(DSL_void_t);
#endif /* defined(DSL_CPE_SIMULATOR_CONTROL)*/
