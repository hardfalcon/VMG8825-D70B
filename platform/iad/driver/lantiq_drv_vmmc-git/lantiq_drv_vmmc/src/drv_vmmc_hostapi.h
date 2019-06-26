#ifndef _DRV_VMMC_HOSTAPI_H
#define _DRV_VMMC_HOSTAPI_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_hostapi.h
   This file contains the defines, the structures declarations and the global
   function declarations for the VMMC host.
*/

/* ================================ */
/* Includes                         */
/* ================================ */
#ifdef EVENT_LOGGER_DEBUG
#include "el_log_macros.h"
#endif /* EVENT_LOGGER_DEBUG */

/* ================================ */
/* Defines                          */
/* ================================ */

#ifdef EVENT_LOGGER_DEBUG

enum { DEV_TYPE_VOICE_MACRO = IFX_TAPI_DEV_TYPE_VOICE_MACRO };

/* Event Logger (debugging) macros, defining more
   meaningful names to Event Logger macros */
/* register read logging macro */
#define LOG_RD_REG(dev_num, ch, reg_offset, reg_data, count)\
   EL_LOG_EVENT_REG_RD(DEV_TYPE_VOICE_MACRO, dev_num, ch, reg_offset,\
                       reg_data, count)

/* single register write logging macro */
#define LOG_WR_REG(dev_num, ch, reg_offset, val)\
   do {\
      IFX_uint16_t tmp;\
      tmp = (val);\
      EL_LOG_EVENT_REG_WR(DEV_TYPE_VOICE_MACRO, dev_num, ch, reg_offset, &tmp, 1);\
   } while(0)

/* multiple register write logging macro */
#define LOG_WR_REG_MULTI(dev_num, ch, reg_offset, reg_data, count)\
   EL_LOG_EVENT_REG_WR(DEV_TYPE_VOICE_MACRO, dev_num, ch, reg_offset,\
                       reg_data, count)

/* command read logging macro */
#define LOG_RD_CMD(dev_num, ch, pcmd, pdata, count, err)\
   EL_LOG_EVENT_CMD_RD(DEV_TYPE_VOICE_MACRO, dev_num, ch, pcmd, pdata,\
                       count, err)

/* command write logging macro */
#define LOG_WR_CMD(dev_num, ch, pdata, count, err)\
   EL_LOG_EVENT_CMD_WR(DEV_TYPE_VOICE_MACRO, dev_num, ch, pdata, count, err)

/* voice inbox read logging macro */
#define LOG_RD_PKT(dev_num, ch, pdata, count, err)\
   EL_LOG_EVENT_PKT_RD(DEV_TYPE_VOICE_MACRO, dev_num, ch, pdata, (count) >> 1, err)

/* voice outbox write logging macro */
#define LOG_WR_PKT(dev_num, ch, pdata, count, err)\
   EL_LOG_EVENT_PKT_WR(DEV_TYPE_VOICE_MACRO, dev_num, ch, pdata, (count) >> 1, err)

/* event mailbox reading logging macro */
#define LOG_RD_EVENT_MBX(dev_num, ch, pdata, count)\
   EL_LOG_EVENT_EVT_MBX_RD(DEV_TYPE_VOICE_MACRO, dev_num, ch, pdata, count)

#else /* EVENT_LOGGER_DEBUG */

#define LOG_RD_REG(dev_num, ch, reg_offset, reg_data, count)
#define LOG_WR_REG(dev_num, ch, reg_offset, val)
#define LOG_WR_REG_MULTI(dev_num, ch, reg_offset, reg_data, count)
#define LOG_RD_CMD(dev_num, ch, pcmd, pdata, count, err)
#define LOG_WR_CMD(dev_num, ch, pdata, count, err)
#define LOG_RD_PKT(dev_num, ch, pdata, count, err)
#define LOG_WR_PKT(dev_num, ch, pdata, count, err)
#define LOG_EVENT_IRQ(dev_type, dev_num, ch, irq_name, irq_details)
#define LOG_RD_EVENT_MBX(dev_num, ch, pdata, count)

#endif /* EVENT_LOGGER_DEBUG */

#endif /* _DRV_VMMC_HOSTAPI_H */
