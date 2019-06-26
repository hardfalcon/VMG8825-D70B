#ifndef _DRV_TAPI_DEBUG_H
#define _DRV_TAPI_DEBUG_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_debug.h
   This header provide interface to the debug functionality.
*/

/* ========================================================================== */
/*                                 Includes                                   */
/* ========================================================================== */
#ifdef EVENT_LOGGER_DEBUG
   #include "el_log_macros.h"
#if !defined(__KERNEL__) && defined(LINUX)
   #include "el_ioctl.h"
#endif
#endif /* EVENT_LOGGER_DEBUG */

/* ========================================================================== */
/*                       Global Macro Definitions                             */
/* ========================================================================== */

#ifdef EVENT_LOGGER_DEBUG
   #define TAPI_EL_IOCTL_MAX_LEN 150

#if !defined(__KERNEL__) && defined(LINUX)
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

   /* write ioctl logging macro */
   #define LOG_WR_IOCTL(dev_num, ch, ioctl, pdata, count, err)\
      /*lint ++flb */ \
      EL_USR_LOG_EVENT_IOCTL_WR(tapi_nEl_fd, TAPI_EL_USR_format, \
                                IFX_TAPI_DEV_TYPE_NONE, dev_num, ch, ioctl, \
                                pdata, count, err) \
      /*lint --flb */

   /* read ioctl logging macro */
   #define LOG_RD_IOCTL(dev_num, ch, ioctl, pdata, count, err)\
      /*lint ++flb */ \
      EL_USR_LOG_EVENT_IOCTL_RD(tapi_nEl_fd, TAPI_EL_USR_format, \
                                IFX_TAPI_DEV_TYPE_NONE, dev_num, ch, ioctl, \
                                pdata, count, err) \
      /*lint --flb */
#else /* !defined(__KERNEL__) && defined(LINUX) */
   /* write ioctl logging macro */
   #define LOG_WR_IOCTL(dev_num, ch, ioctl, pdata, count, err)\
      /*lint ++flb */ \
      EL_LOG_EVENT_IOCTL_WR(IFX_TAPI_DEV_TYPE_NONE, dev_num, ch, ioctl, pdata,\
         count, err) \
      /*lint --flb */

   /* read ioctl logging macro */
   #define LOG_RD_IOCTL(dev_num, ch, ioctl, pdata, count, err)\
      /*lint ++flb */ \
      EL_LOG_EVENT_IOCTL_RD(IFX_TAPI_DEV_TYPE_NONE, dev_num, ch, ioctl, pdata,\
         count, err) \
      /*lint --flb */
#endif /*!defined(__KERNEL__) && defined(LINUX)*/

#else /* EVENT_LOGGER_DEBUG */
   #define LOG_WR_IOCTL(dev_num, ch, ioctl, pdata, count, err)
   #define LOG_RD_IOCTL(dev_num, ch, ioctl, pdata, count, err)
#endif /* EVENT_LOGGER_DEBUG */

/* ========================================================================== */
/*                             Global variables                               */
/* ========================================================================== */
#ifdef EVENT_LOGGER_DEBUG
#if !defined(__KERNEL__) && defined(LINUX)
extern IFX_int32_t tapi_nEl_fd;
#endif /* !defined(__KERNEL__) && defined(LINUX) */
#endif /* EVENT_LOGGER_DEBUG */
/* ========================================================================== */
/*                             Type definitions                               */
/* ========================================================================== */

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */

extern IFX_int32_t TAPI_DebugReportSet (TAPI_DEV *pDev, IFX_uint32_t nLevel);
extern IFX_void_t TAPI_LogInit (TAPI_DEV *pDev);
extern IFX_void_t TAPI_LogClose (TAPI_DEV *pDev);
extern IFX_char_t const *TAPI_ioctlNameGet(IFX_uint32_t nIoctl);
#ifdef EVENT_LOGGER_DEBUG
#if !defined(__KERNEL__) && defined(LINUX)
IFX_int32_t TAPI_EL_USR_format( EL_IoctlAddLog_t *pLog, IFX_char_t **ppOutput);
#endif
#endif /* EVENT_LOGGER_DEBUG */

#ifdef DEBUG_SDI_ENABLED
#ifndef KERN_CRIT
#define KERN_CRIT
#endif

#ifndef TAPI_OS_PRINTK
#ifdef LINUX
#define TAPI_OS_PRINTK printk
#endif /* LINUX */
#ifdef VXWORKS
#define TAPI_OS_PRINTK printf
#endif /* VXWORKS */
#endif /* TAPI_OS_PRINTK */

#define SDI_GET_FILE(file_name) \
   ({ \
      char sFileName[] = file_name; \
      char *sResult = sFileName, *p = sFileName; \
      for (p = sFileName; *p; p++) if (*p == '/') sResult = p + 1; \
      sResult; \
   })

#define SDI() TAPI_OS_PRINTK (KERN_CRIT "!tapi! %s [%s,%d]" TAPI_CRLF, __FUNCTION__, SDI_GET_FILE(__FILE__), __LINE__)

#define SDI1(s) TAPI_OS_PRINTK (KERN_CRIT "!tapi! %s (%s) [%s,%d] " TAPI_CRLF, __FUNCTION__, s, SDI_GET_FILE(__FILE__), __LINE__)
#define SDI2(f,d) TAPI_OS_PRINTK (KERN_CRIT "!tapi! %s (%s = " f ") [%s,%d]" TAPI_CRLF, __FUNCTION__, #d, d, SDI_GET_FILE(__FILE__), __LINE__)

#define SDI_DUMP(mem, len) \
   do { \
      if (1) { \
         TAPI_OS_PRINTK (KERN_CRIT "!tapi! %s (%s, ", __FUNCTION__, #mem); \
         IFX_uint32_t i = 0; \
         IFX_uint8_t *p = (IFX_uint8_t*)(mem); \
         IFX_uint32_t max_i = (((len) + 1) & (~0x1)); \
         for (i=0; i<max_i; i++) \
            TAPI_OS_PRINTK ("%02X%s", p[i], (((i + 1) % 4) == 0) ? " "  : ""); \
         TAPI_OS_PRINTK (") [%s,%d]" TAPI_CRLF, SDI_GET_FILE(__FILE__), __LINE__); \
      } \
   } while(0)
#else
#define SDI()
#define SDI1(s)
#define SDI2(f,d)
#define SDI_DUMP(mem, len)
#define TAPI_OS_PRINTK(...)
#endif /* DEBUG_SDI_ENABLED */

#endif /* _DRV_TAPI_DEBUG_H */
