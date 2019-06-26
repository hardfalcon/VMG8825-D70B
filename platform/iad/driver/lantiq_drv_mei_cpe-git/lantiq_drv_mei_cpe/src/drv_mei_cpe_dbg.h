#ifndef _DRV_MEI_CPE_DBG_H
#define _DRV_MEI_CPE_DBG_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : This file contains OS specific defines for the VRX driver.

   Remarks     : Please use the compiler switches here if you have
                 more than one OS.
   ========================================================================== */


/* ============================================================================
   Global Includes
   ========================================================================= */
#include "ifx_types.h"
#include "drv_mei_cpe_config.h"
#include "drv_mei_cpe_os.h"

#if 0
#if !(MEI_DEBUG_PRINT == 1)
#warning "DEBUG PRINT - OFF"
#endif

#if !(MEI_ERROR_PRINT == 1)
#warning "ERROR PRINT - OFF"
#endif
#endif

/* ============================================================================
   VRX Driver Printout - Printout Levels
   ========================================================================= */

#define MEI_DRV_PRN_LEVEL_OFF          4
#define MEI_DRV_PRN_LEVEL_HIGH         3
#define MEI_DRV_PRN_LEVEL_NORMAL       2
#define MEI_DRV_PRN_LEVEL_LOW          1

#define MEI_DRV_PRN_LEVEL_ERR          MEI_DRV_PRN_LEVEL_HIGH
#define MEI_DRV_PRN_LEVEL_WRN          MEI_DRV_PRN_LEVEL_NORMAL
#define MEI_DRV_PRN_LEVEL_MSG          MEI_DRV_PRN_LEVEL_LOW

#define MEI_DRV_CRLF                   "\n\r"

#define MEI_DBG_LINE_PRINT_INDICATION  0x00000001


/* ============================================================================
   VRX Driver Printout - Macros for Printout Level Handling
   ========================================================================= */
#if ((MEI_DEBUG_PRINT == 1) || (MEI_ERROR_PRINT == 1))

/**
   DEBUG PRINTOUT - Declare a debug level variable (user print) for a given module.
*/
#define MEI_DRV_PRN_USR_MODULE_DECL(module_name)   extern IFX_uint32_t MEI_PrnUsrModule_##module_name

/**
   DEBUG PRINTOUT - Declare a debug level variable (interrupt print) for a given module.
*/
#define MEI_DRV_PRN_INT_MODULE_DECL(module_name)   extern IFX_uint32_t MEI_PrnIntModule_##module_name


/**
   DEBUG PRINTOUT - Create a debug level variable (user print) for a given module.
*/
#define MEI_DRV_PRN_USR_MODULE_CREATE(module_name, init_level) \
            IFX_uint32_t MEI_PrnUsrModule_##module_name = init_level

/**
   DEBUG PRINTOUT - Create a debug level variable (interrupt print) for a given module.
*/
#define MEI_DRV_PRN_INT_MODULE_CREATE(module_name, init_level) \
            IFX_uint32_t MEI_PrnIntModule_##module_name = init_level

/**
   DEBUG PRINTOUT - Create a local variable used only if debug/error print is enabled.
*/
#define MEI_DRV_PRN_LOCAL_VAR_CREATE(var_type, var_name, init_value)\
            var_type var_name = (init_value)
/**
   DEBUG PRINTOUT - Set the debug level (user print) for a given module.
*/
#define MEI_DRV_PRN_USR_LEVEL_SET(module_name, new_level) \
         { MEI_PrnUsrModule_##module_name = \
            (new_level>MEI_DRV_PRN_LEVEL_OFF) ?  MEI_DRV_PRN_LEVEL_OFF : \
            ((new_level<MEI_DRV_PRN_LEVEL_LOW) ? MEI_DRV_PRN_LEVEL_OFF :  new_level); }

/**
   DEBUG PRINTOUT - Set the debug level (interrupt print) for a given module.
*/
#define MEI_DRV_PRN_INT_LEVEL_SET(module_name, new_level) \
         { MEI_PrnIntModule_##module_name = \
            (new_level>MEI_DRV_PRN_LEVEL_OFF) ?  MEI_DRV_PRN_LEVEL_OFF : \
            ((new_level<MEI_DRV_PRN_LEVEL_LOW) ? MEI_DRV_PRN_LEVEL_OFF :  new_level); }



/**
   DEBUG PRINTOUT - Get the debug level (user print) for a given module.
*/
#define MEI_DRV_PRN_USR_LEVEL_GET(module_name)    MEI_PrnUsrModule_##module_name

/**
   DEBUG PRINTOUT - Get the debug level (interrupt print) for a given module.
*/
#define MEI_DRV_PRN_INT_LEVEL_GET(module_name)    MEI_PrnIntModule_##module_name

#else    /* #if (MEI_DEBUG_PRINT == 1) || (MEI_ERROR_PRINT == 1) */

#define MEI_DRV_PRN_USR_MODULE_DECL(module_name)
#define MEI_DRV_PRN_INT_MODULE_DECL(module_name)

#define MEI_DRV_PRN_USR_MODULE_CREATE(module_name, init_level)
#define MEI_DRV_PRN_INT_MODULE_CREATE(module_name, init_level)

#define MEI_DRV_PRN_LOCAL_VAR_CREATE(var_type, var_name, init_value)

#define MEI_DRV_PRN_USR_LEVEL_SET(module_name, new_level)
#define MEI_DRV_PRN_INT_LEVEL_SET(module_name, new_level)

#define MEI_DRV_PRN_USR_LEVEL_GET(module_name)    MEI_DRV_PRN_LEVEL_OFF
#define MEI_DRV_PRN_INT_LEVEL_GET(module_name)    MEI_DRV_PRN_LEVEL_OFF

#endif   /* #if (MEI_DEBUG_PRINT == 1) || (MEI_ERROR_PRINT == 1)  */



/* ============================================================================
   VRX Driver Printout - Debug Print, handle global switch
   ========================================================================= */
#if (MEI_DEBUG_PRINT == 1)

/**
   DEBUG PRINTOUT - Create a local variable used only if debug print is enabled.
*/
#define MEI_DRV_PRN_LOCAL_DBG_VAR_CREATE(var_type, var_name, init_value) \
            var_type var_name = (init_value)
/**
   DEBUG PRINTOUT - Create a global debug control varibale (once a time).
*/
#define MEI_DRV_PRN_DEBUG_CONTROL_CREATE(init_value) \
            IFX_uint32_t MEI_DebugCntrl = init_value; \
            IFX_uint32_t MEI_DebugCntrlLine[4] = {0, 0, 0, 0}
/**
   DEBUG PRINTOUT - Declare a global debug control varibale.
*/
#define MEI_DRV_PRN_DEBUG_CONTROL_DECL() \
            extern IFX_uint32_t MEI_DebugCntrl; \
            extern IFX_uint32_t MEI_DebugCntrlLine[4];


/**
   DEBUG PRINTOUT - get the global debug control varibale.
*/
#define MEI_DRV_PRN_DEBUG_CONTROL_GET()               MEI_DebugCntrl

/**
   DEBUG PRINTOUT - set the global debug control varibale.
*/
#define MEI_DRV_PRN_DEBUG_CONTROL_SET(newDbgCntrl) \
            { MEI_DebugCntrl = (IFX_uint32_t)newDbgCntrl; }

/**
   DEBUG PRINTOUT - get the global debug control line mask varibale.
*/
#define MEI_DRV_PRN_DEBUG_CONTROL_MASK_GET(maskNum)   MEI_DebugCntrlLine[maskNum]

/**
   DEBUG PRINTOUT - get the global debug control line mask varibale.
*/
#define MEI_DRV_PRN_DEBUG_CONTROL_MASK_SET(maskNum, newMaskVal) \
            { MEI_DebugCntrlLine[maskNum] = newMaskVal; }


/**
   Set the global printout level for the debug print (user).
*/
#define MEI_DRVOS_GDBG_USR_LEVEL_SET(new_level) \
         { MEI_PrnUsrModule_DBG_GLOBAL = \
            (new_level>MEI_DRV_PRN_LEVEL_OFF) ? MEI_DRV_PRN_LEVEL_OFF : \
            ((new_level<MEI_DRV_PRN_LEVEL_LOW) ? MEI_DRV_PRN_LEVEL_OFF :  ((IFX_uint32_t)new_level)); }

/**
   Set the global printout level for the debug print (interrupt).
*/
#define MEI_DRVOS_GDBG_INT_LEVEL_SET(new_level) \
         { MEI_PrnIntModule_DBG_GLOBAL = \
             (new_level>MEI_DRV_PRN_LEVEL_OFF) ? MEI_DRV_PRN_LEVEL_OFF : \
            ((new_level<MEI_DRV_PRN_LEVEL_LOW) ? MEI_DRV_PRN_LEVEL_OFF :  ((IFX_uint32_t)new_level)); }

/**
   Get the global printout level for the debug print (user).
*/
#define MEI_DRVOS_GDBG_USR_LEVEL_GET(new_level)  MEI_PrnUsrModule_DBG_GLOBAL

/**
   Get the global printout level for the debug print (interrupt).
*/
#define MEI_DRVOS_GDBG_INT_LEVEL_GET(new_level)  MEI_PrnIntModule_DBG_GLOBAL


#else    /* #if (MEI_DEBUG_PRINT == 1) */

#define MEI_DRV_PRN_LOCAL_DBG_VAR_CREATE(var_type, var_name, init_value)

#define MEI_DRV_PRN_DEBUG_CONTROL_CREATE(init_value)
#define MEI_DRV_PRN_DEBUG_CONTROL_DECL()

#define MEI_DRV_PRN_DEBUG_CONTROL_GET()
#define MEI_DRV_PRN_DEBUG_CONTROL_SET(newDbgCntrl)

#define MEI_DRV_PRN_DEBUG_CONTROL_MASK_GET(maskNum)
#define MEI_DRV_PRN_DEBUG_CONTROL_MASK_SET(maskNum, newMaskVal)

#define MEI_DRVOS_GDBG_USR_LEVEL_SET(new_level)
#define MEI_DRVOS_GDBG_INT_LEVEL_SET(new_level)

#define MEI_DRVOS_GDBG_USR_LEVEL_GET(new_level)  MEI_DRV_PRN_LEVEL_OFF
#define MEI_DRVOS_GDBG_INT_LEVEL_GET(new_level)  MEI_DRV_PRN_LEVEL_OFF

#endif   /* #if (MEI_DEBUG_PRINT == 1) */


#if (MEI_DEBUG_PRINT == 1)

/* ============================================================================
   VRX Driver Printout - Macros for Debug Printouts
   ========================================================================= */

/*
#if !defined(MEI_PRINT_USR) || !defined(MEI_PRINT_INT)
#error "please define your OS specific debug print macro
#endif
*/

/**
   Debug printout - triggered from user call
*/
#define PRN_DBG_USR_NL(module_name, dbg_level, print_message) \
            do { \
               if ( (dbg_level >= MEI_PrnUsrModule_##module_name) && (dbg_level >= MEI_PrnUsrModule_DBG_GLOBAL) ) \
                  { MEI_PRINT_USR print_message ; }\
            } while(0)

/**
   Debug printout - triggered from user call
*/
#define PRN_DBG_USR_RAW(module_name, dbg_level, print_message) \
            do { \
               if ( (dbg_level >= MEI_PrnUsrModule_##module_name) && (dbg_level >= MEI_PrnUsrModule_DBG_GLOBAL) ) \
                  { MEI_PRINT_INT_RAW print_message ; }\
            } while(0)

/**
   Debug printout - triggered from interrupt call
*/
#define PRN_DBG_INT_NL(module_name, dbg_level, print_message) \
            do { \
               if ( (dbg_level >= MEI_PrnIntModule_##module_name) && (dbg_level >= MEI_PrnIntModule_DBG_GLOBAL) ) \
                  { MEI_PRINT_INT print_message ; }\
            } while(0)

/**
   Line Debug printout - triggered from user call
*/
#define PRN_DBG_USR(module_name, dbg_level, line_num, print_message) \
            do { \
               if ( MEI_DbgCheckForLine((IFX_uint8_t)line_num) == IFX_TRUE) \
               {  if ( (dbg_level >= MEI_PrnUsrModule_##module_name) && (dbg_level >= MEI_PrnUsrModule_DBG_GLOBAL) ) \
                     { MEI_PRINT_USR print_message ; } } \
            } while(0)


/**   Line Debug printout - triggered from interrupt call
*/
#define PRN_DBG_INT(module_name, dbg_level, line_num, print_message) \
            do { \
               if ( MEI_DbgCheckForLine((IFX_uint8_t)line_num) == IFX_TRUE) \
               {  if ( (dbg_level >= MEI_PrnIntModule_##module_name) && (dbg_level >= MEI_PrnIntModule_DBG_GLOBAL) ) \
                     { MEI_PRINT_INT print_message ; } } \
            } while(0)
#else

#define PRN_DBG_USR_NL(module_name, dbg_level, print_message)
#define PRN_DBG_USR_RAW(module_name, dbg_level, print_message)
#define PRN_DBG_INT_NL(module_name, dbg_level, print_message)
#define PRN_DBG_USR(module_name, dbg_level, line_num, print_message)
#define PRN_DBG_INT(module_name, dbg_level, line_num, print_message)

#endif      /* #if (MEI_DEBUG_PRINT == 1) */


#if (MEI_ERROR_PRINT == 1)

/* ============================================================================
   VRX Driver Printout - Macros for Error Printouts
   ========================================================================= */
/*
#if !defined(MEI_PRINT_INT) || !defined(MEI_PRINT_USR)
#error "please define your OS specific error print macro
#endif
*/
/**
   ERROR printout - triggered from user call
*/
#define PRN_ERR_USR_NL(module_name, dbg_level, print_message) \
            do { \
               if(dbg_level >= MEI_PrnUsrModule_##module_name) {\
                   MEI_PRINT_USR print_message ; }\
            } while(0)

/**
   ERROR printout - triggered from interrupt call
*/
#define PRN_ERR_INT_NL(module_name, dbg_level, print_message) \
            do { \
               if(dbg_level >= MEI_PrnIntModule_##module_name) {\
                   MEI_PRINT_INT print_message ; }\
            } while(0)

#else

#define PRN_ERR_USR_NL(module_name, dbg_level, print_message)
#define PRN_ERR_INT_NL(module_name, dbg_level, print_message)
#endif      /* #if (MEI_ERROR_PRINT == 1) */




/* ============================================================================
   Export Debug Infos
   ========================================================================= */

/* VRX-Driver: Global level debug control (seperate error / debug print) */
MEI_DRV_PRN_USR_MODULE_DECL(DBG_GLOBAL);
MEI_DRV_PRN_INT_MODULE_DECL(DBG_GLOBAL);

/* VRX-Driver: Declare global debug control variable */
MEI_DRV_PRN_DEBUG_CONTROL_DECL();


#if (MEI_DEBUG_PRINT == 1)
extern IFX_boolean_t MEI_DbgCheckForLine(
                              IFX_uint8_t  lineNum);
#endif

#endif      /* #ifndef _DRV_MEI_CPE_DBG_H */

