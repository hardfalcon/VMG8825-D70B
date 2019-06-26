/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_debug.c
   This module provide debug functionality.

   \remarks
      This file contains the automatically generated code, please be careful
      during editing.
*/

/* ========================================================================== */
/*                                 Includes                                   */
/* ========================================================================== */

#include "drv_tapi.h"
#include "drv_tapi_api.h"
#include "drv_tapi_debug.h"
#include "drv_tapi_errno.h"

#if !defined(__KERNEL__) && defined(LINUX)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#ifdef EVENT_LOGGER_DEBUG
   #ifdef LINUX
      #include <asm/ioctl.h>
   #endif
   #ifdef VXWORKS
      #include "ioctl.h"
   #endif
   #ifdef WIN32
      #include "../src/el_cfg.h"
   #endif
#endif /* EVENT_LOGGER_DEBUG */

#if (defined(IFXOS_USE_DEV_IO) && (IFXOS_USE_DEV_IO == 1))
   #include <ifxos_device_access.h>
   #ifdef LINUX
       #include <asm/ioctl.h> /** \todo Fix it */
   #endif
#endif
/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */

#define TAPI_EL_APPEND(fmt, ...) \
      n = snprintf(pOutput + len, (IFX_uint32_t)nOutLen - len, fmt, __VA_ARGS__), \
      len += (n < 0 || n >= (IFX_uint32_t)nOutLen - len) ? 0 : (IFX_uint32_t)n

#define TAPI_EL_ADD_PADDING(nCnt)  \
   for (k = 0; k < nCnt; k++)        \
   {                                 \
      n = snprintf(pOutput + len, (IFX_uint32_t)nOutLen - len, " "); \
      len += (n < 0 || n >= (IFX_uint32_t)nOutLen - len) ? 0 : (IFX_uint32_t)n; \
   }

/* ========================================================================== */
/*                             Type definitions                               */
/* ========================================================================== */

typedef struct _IoctlLkupTable
{
   IFX_uint32_t val;
   IFX_char_t name[40];
} IoctlLkupTable_t;

/* ========================================================================== */
/*                             Local variables                                */
/* ========================================================================== */

#define IOCTL_LKUP_TBL_ADD(ioctl) \
   { ioctl, #ioctl }

/**
   Lookup table TAPI ioctl command

   \remarks
      !!!IMPORTANT!!!
      Never touch that data between 'IOCTL_LOOKUP_START'
         and 'IOCTL_LOOKUP_END' and 'IOCTL_QOS_LOOKUP_START'
         and 'IOCTL_QOS_LOOKUP_END' directly.
      Use 'drv_tapi_io.xml' and 'drv_tapi_qos_io.xml'
         with 'make ioctl' instead!
*/
static IoctlLkupTable_t const tapi_ioctl_table[] = {
/* IOCTL_LOOKUP_START */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CH_INIT),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DEV_START),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DEV_STOP),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_FEED_SET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_FEED_GET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_TYPE_SET),
#if defined(TAPI_FEAT_ALM_LEC)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_WLEC_PHONE_CFG_SET),
#endif /* defined(TAPI_FEAT_ALM_LEC) */
#if defined(TAPI_FEAT_ALM_LEC)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_WLEC_PHONE_CFG_GET),
#endif /* defined(TAPI_FEAT_ALM_LEC) */
#if defined(TAPI_FEAT_PCM_LEC)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_WLEC_PCM_CFG_SET),
#endif /* defined(TAPI_FEAT_PCM_LEC) */
#if defined(TAPI_FEAT_PCM_LEC)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_WLEC_PCM_CFG_GET),
#endif /* defined(TAPI_FEAT_PCM_LEC) */
#if defined(TAPI_FEAT_DIAL)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_HOOK_VT_SET),
#endif /* defined(TAPI_FEAT_DIAL) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PHONE_VOLUME_SET),
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PCM_VOLUME_SET),
#endif /* defined(TAPI_FEAT_PCM) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PHONE_RELATIVE_LEVEL_SET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PHONE_RELATIVE_LEVEL_GET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_LEVEL_SET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_HOOK_STATUS_GET),
#if defined(TAPI_FEAT_METERING)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_METER_CFG_SET),
#endif /* defined(TAPI_FEAT_METERING) */
#if defined(TAPI_FEAT_METERING)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_METER_START),
#endif /* defined(TAPI_FEAT_METERING) */
#if defined(TAPI_FEAT_METERING)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_METER_STOP),
#endif /* defined(TAPI_FEAT_METERING) */
#if defined(TAPI_FEAT_METERING)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_METER_BURST),
#endif /* defined(TAPI_FEAT_METERING) */
#if defined(TAPI_FEAT_METERING)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_METER_STATISTICS_GET),
#endif /* defined(TAPI_FEAT_METERING) */
#if defined(TAPI_FEAT_TONEGEN)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_LEVEL_SET),
#endif /* defined(TAPI_FEAT_TONEGEN) */
#if defined(TAPI_FEAT_TONETABLE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_TABLE_CFG_SET),
#endif /* defined(TAPI_FEAT_TONETABLE) */
#if defined(TAPI_FEAT_TONEGEN)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_PLAY),
#endif /* defined(TAPI_FEAT_TONEGEN) */
#if defined(TAPI_FEAT_TONEGEN)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_LOCAL_PLAY),
#endif /* defined(TAPI_FEAT_TONEGEN) */
#if defined(TAPI_FEAT_TONEGEN)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_NET_PLAY),
#endif /* defined(TAPI_FEAT_TONEGEN) */
#if defined(TAPI_FEAT_TONEGEN)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_BUSY_PLAY),
#endif /* defined(TAPI_FEAT_TONEGEN) */
#if defined(TAPI_FEAT_TONEGEN)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_RINGBACK_PLAY),
#endif /* defined(TAPI_FEAT_TONEGEN) */
#if defined(TAPI_FEAT_TONEGEN)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_DIALTONE_PLAY),
#endif /* defined(TAPI_FEAT_TONEGEN) */
#if defined(TAPI_FEAT_TONEGEN)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_LOCAL_STOP),
#endif /* defined(TAPI_FEAT_TONEGEN) */
#if defined(TAPI_FEAT_TONEGEN)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_NET_STOP),
#endif /* defined(TAPI_FEAT_TONEGEN) */
#if defined(TAPI_FEAT_TONEGEN)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_STOP),
#endif /* defined(TAPI_FEAT_TONEGEN) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_SIG_DETECT_ENABLE),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_SIG_DETECT_DISABLE),
#if defined(TAPI_FEAT_DTMF)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DTMF_RX_CFG_SET),
#endif /* defined(TAPI_FEAT_DTMF) */
#if defined(TAPI_FEAT_DTMF)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DTMF_RX_CFG_GET),
#endif /* defined(TAPI_FEAT_DTMF) */
#if defined(TAPI_FEAT_CPTD)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_CPTD_START),
#endif /* defined(TAPI_FEAT_CPTD) */
#if defined(TAPI_FEAT_CPTD)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_CPTD_STOP),
#endif /* defined(TAPI_FEAT_CPTD) */
#if defined(TAPI_FEAT_CID)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CID_RX_START),
#endif /* defined(TAPI_FEAT_CID) */
#if defined(TAPI_FEAT_CID)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CID_RX_STOP),
#endif /* defined(TAPI_FEAT_CID) */
#if defined(TAPI_FEAT_CID)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CID_RX_STATUS_GET),
#endif /* defined(TAPI_FEAT_CID) */
#if defined(TAPI_FEAT_CID)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CID_RX_DATA_GET),
#endif /* defined(TAPI_FEAT_CID) */
#if defined(TAPI_FEAT_CID)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CID_TX_INFO_START),
#endif /* defined(TAPI_FEAT_CID) */
#if defined(TAPI_FEAT_CID)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CID_TX_INFO_STOP),
#endif /* defined(TAPI_FEAT_CID) */
#if defined(TAPI_FEAT_CID)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CID_TX_SEQ_START),
#endif /* defined(TAPI_FEAT_CID) */
#if defined(TAPI_FEAT_CID)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CID_CFG_SET),
#endif /* defined(TAPI_FEAT_CID) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_RTP_CFG_SET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_RTP_OOB)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_EV_OOB_DTMF_SET),
#endif /* defined(TAPI_FEAT_RTP_OOB) */
#if defined(TAPI_FEAT_RTP_OOB)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_EV_OOB_MFTD_SET),
#endif /* defined(TAPI_FEAT_RTP_OOB) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_RTP_PT_CFG_SET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_NET_CFG_SET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_RTCP_STATISTICS_GET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_RTCP_STATISTICS_RESET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_RTCP_XR_BLOCK_GET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_RTP_OOB)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_EV_GENERATE),
#endif /* defined(TAPI_FEAT_RTP_OOB) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_EV_GENERATE_ABCD_CFG),
#if defined(TAPI_FEAT_PACKET)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_FLUSH),
#endif /* defined(TAPI_FEAT_PACKET) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_JB_CFG_SET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_JB_STATISTICS_GET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_JB_STATISTICS_RESET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MAP_PHONE_ADD),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MAP_PHONE_REMOVE),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MAP_DATA_ADD),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MAP_DATA_REMOVE),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MAP_PCM_ADD),
#endif /* defined(TAPI_FEAT_PCM) */
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MAP_PCM_REMOVE),
#endif /* defined(TAPI_FEAT_PCM) */
#if defined(TAPI_FEAT_DECT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MAP_DECT_ADD),
#endif /* defined(TAPI_FEAT_DECT) */
#if defined(TAPI_FEAT_DECT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MAP_DECT_REMOVE),
#endif /* defined(TAPI_FEAT_DECT) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MAP_MON_ADD),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MAP_MON_REMOVE),
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_ENC_CFG_SET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DEC_CFG_SET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_ENC_START),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_ENC_STOP),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_ENC_HOLD),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DEC_START),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DEC_STOP),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_COD_DEC_HP_SET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_COD_VOLUME_SET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_ENC_AGC_CFG),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_ENC_AGC_ENABLE),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_ENC_VAD_CFG_SET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_ANNOUNCEMENTS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_COD_ANNOUNCE_CFG_SET),
#endif /* defined(TAPI_FEAT_ANNOUNCEMENTS) */
#if defined(TAPI_FEAT_ANNOUNCEMENTS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_COD_ANNOUNCE_BUFFER_FREE),
#endif /* defined(TAPI_FEAT_ANNOUNCEMENTS) */
#if defined(TAPI_FEAT_ANNOUNCEMENTS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_COD_ANNOUNCE_START),
#endif /* defined(TAPI_FEAT_ANNOUNCEMENTS) */
#if defined(TAPI_FEAT_ANNOUNCEMENTS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_COD_ANNOUNCE_STOP),
#endif /* defined(TAPI_FEAT_ANNOUNCEMENTS) */
#if defined(TAPI_FEAT_BABYPHONE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_ENC_ROOM_NOISE_DETECT_START),
#endif /* defined(TAPI_FEAT_BABYPHONE) */
#if defined(TAPI_FEAT_BABYPHONE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_ENC_ROOM_NOISE_DETECT_STOP),
#endif /* defined(TAPI_FEAT_BABYPHONE) */
#if defined(TAPI_FEAT_KPI)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_KPI_CH_CFG_SET),
#endif /* defined(TAPI_FEAT_KPI) */
#if defined(TAPI_FEAT_KPI)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_KPI_GRP_CFG_SET),
#endif /* defined(TAPI_FEAT_KPI) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_VERSION_GET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DEBUG_REPORT_SET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_VERSION_CHECK),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CAP_NR),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CAP_LIST),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CAP_NLIST),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CAP_CHECK),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LASTERR),
#if defined(TAPI_FEAT_RINGENGINE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_RING_CFG_SET),
#endif /* defined(TAPI_FEAT_RINGENGINE) */
#if defined(TAPI_FEAT_RINGENGINE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_RING_CADENCE_HR_SET),
#endif /* defined(TAPI_FEAT_RINGENGINE) */
#if defined(TAPI_FEAT_RINGENGINE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_RING_CFG_GET),
#endif /* defined(TAPI_FEAT_RINGENGINE) */
#if defined(TAPI_FEAT_RINGENGINE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_RING),
#endif /* defined(TAPI_FEAT_RINGENGINE) */
#if defined(TAPI_FEAT_RINGENGINE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_RING_MAX_SET),
#endif /* defined(TAPI_FEAT_RINGENGINE) */
#if defined(TAPI_FEAT_RINGENGINE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_RING_CADENCE_SET),
#endif /* defined(TAPI_FEAT_RINGENGINE) */
#if defined(TAPI_FEAT_RINGENGINE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_RING_START),
#endif /* defined(TAPI_FEAT_RINGENGINE) */
#if defined(TAPI_FEAT_RINGENGINE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_RING_STOP),
#endif /* defined(TAPI_FEAT_RINGENGINE) */
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PCM_IF_CFG_SET),
#endif /* defined(TAPI_FEAT_PCM) */
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PCM_CFG_SET),
#endif /* defined(TAPI_FEAT_PCM) */
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PCM_CFG_GET),
#endif /* defined(TAPI_FEAT_PCM) */
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PCM_ACTIVATION_SET),
#endif /* defined(TAPI_FEAT_PCM) */
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PCM_ACTIVATION_GET),
#endif /* defined(TAPI_FEAT_PCM) */
#if defined(TAPI_FEAT_HDLC)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PCM_HDLC_CFG_SET),
#endif /* defined(TAPI_FEAT_HDLC) */
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PCM_LOOP_CFG_SET),
#endif /* defined(TAPI_FEAT_PCM) */
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PCM_DEC_HP_SET),
#endif /* defined(TAPI_FEAT_PCM) */
#if defined(TAPI_FEAT_FAX_STACK)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_SESS_START),
#endif /* defined(TAPI_FEAT_FAX_STACK) */
#if defined(TAPI_FEAT_FAX_STACK)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_SESS_STOP),
#endif /* defined(TAPI_FEAT_FAX_STACK) */
#if defined(TAPI_FEAT_FAX_STACK)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_SESS_STATISTICS_GET),
#endif /* defined(TAPI_FEAT_FAX_STACK) */
#if defined(TAPI_FEAT_FAX_STACK)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_CAP_GET),
#endif /* defined(TAPI_FEAT_FAX_STACK) */
#if defined(TAPI_FEAT_FAX_STACK)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_CFG_SET),
#endif /* defined(TAPI_FEAT_FAX_STACK) */
#if defined(TAPI_FEAT_FAX_STACK)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_CFG_GET),
#endif /* defined(TAPI_FEAT_FAX_STACK) */
#if defined(TAPI_FEAT_FAX_STACK)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_FDP_CFG_SET),
#endif /* defined(TAPI_FEAT_FAX_STACK) */
#if defined(TAPI_FEAT_FAX_STACK)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_FDP_CFG_GET),
#endif /* defined(TAPI_FEAT_FAX_STACK) */
#if defined(TAPI_FEAT_FAX_STACK)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_TRACE_SET),
#endif /* defined(TAPI_FEAT_FAX_STACK) */
#if defined(TAPI_FEAT_FAX_T38)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_MOD_START),
#endif /* defined(TAPI_FEAT_FAX_T38) */
#if defined(TAPI_FEAT_FAX_T38)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_DEMOD_START),
#endif /* defined(TAPI_FEAT_FAX_T38) */
#if defined(TAPI_FEAT_FAX_T38)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_STOP),
#endif /* defined(TAPI_FEAT_FAX_T38) */
#if defined(TAPI_FEAT_FAX_T38)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_T38_STATUS_GET),
#endif /* defined(TAPI_FEAT_FAX_T38) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TEST_HOOKGEN),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TEST_LOOP),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_EVENT_GET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_EVENT_ENABLE),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_EVENT_DISABLE),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_EVENT_MULTI_ENABLE),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_EVENT_MULTI_DISABLE),
#if defined(TAPI_FEAT_POLL)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_POLL_CONFIG),
#endif /* defined(TAPI_FEAT_POLL) */
#if defined(TAPI_FEAT_POLL)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_POLL_DEV_ADD),
#endif /* defined(TAPI_FEAT_POLL) */
#if defined(TAPI_FEAT_POLL)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_POLL_DEV_REM),
#endif /* defined(TAPI_FEAT_POLL) */
#if defined(TAPI_FEAT_POLL)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_POLL_TEST),
#endif /* defined(TAPI_FEAT_POLL) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_DIAL_CFG_SET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_FLASH_CFG_SET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_OSI_CFG_SET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_DIAL_START),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_DIAL_STOP),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_HOOK_SET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_FLASH_SET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_BAT_GET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_HOOK_GET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_APOH_GET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_RING_GET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_POLARITY_GET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_FXO)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_FXO_LINE_MODE_SET),
#endif /* defined(TAPI_FEAT_FXO) */
#if defined(TAPI_FEAT_DECT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DECT_ACTIVATION_SET),
#endif /* defined(TAPI_FEAT_DECT) */
#if defined(TAPI_FEAT_DECT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DECT_CFG_SET),
#endif /* defined(TAPI_FEAT_DECT) */
#if defined(TAPI_FEAT_DECT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DECT_ENC_CFG_SET),
#endif /* defined(TAPI_FEAT_DECT) */
#if defined(TAPI_FEAT_DECT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DECT_EC_CFG_SET),
#endif /* defined(TAPI_FEAT_DECT) */
#if defined(TAPI_FEAT_DECT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DECT_VOLUME_SET),
#endif /* defined(TAPI_FEAT_DECT) */
#if defined(TAPI_FEAT_DECT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_DECT_STATISTICS_GET),
#endif /* defined(TAPI_FEAT_DECT) */
#if defined(TAPI_FEAT_DECT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_DECT_PLAY),
#endif /* defined(TAPI_FEAT_DECT) */
#if defined(TAPI_FEAT_DECT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_DECT_STOP),
#endif /* defined(TAPI_FEAT_DECT) */
#if defined(TAPI_FEAT_MWL)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MWL_ACTIVATION_SET),
#endif /* defined(TAPI_FEAT_MWL) */
#if defined(TAPI_FEAT_MWL)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_MWL_ACTIVATION_GET),
#endif /* defined(TAPI_FEAT_MWL) */
#if defined(TAPI_FEAT_PEAKD)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PEAK_DETECT_START),
#endif /* defined(TAPI_FEAT_PEAKD) */
#if defined(TAPI_FEAT_PEAKD)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PEAK_DETECT_STOP),
#endif /* defined(TAPI_FEAT_PEAKD) */
#if defined(TAPI_FEAT_PEAKD)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PEAK_DETECT_RESULT_GET),
#endif /* defined(TAPI_FEAT_PEAKD) */
#if defined(TAPI_FEAT_MF_R2)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_MF_R2_START),
#endif /* defined(TAPI_FEAT_MF_R2) */
#if defined(TAPI_FEAT_MF_R2)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_TONE_MF_R2_STOP),
#endif /* defined(TAPI_FEAT_MF_R2) */
#if defined(TAPI_FEAT_CALIBRATION)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CALIBRATION_START),
#endif /* defined(TAPI_FEAT_CALIBRATION) */
#if defined(TAPI_FEAT_CALIBRATION)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CALIBRATION_STOP),
#endif /* defined(TAPI_FEAT_CALIBRATION) */
#if defined(TAPI_FEAT_CALIBRATION)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CALIBRATION_RESULTS_GET),
#endif /* defined(TAPI_FEAT_CALIBRATION) */
#if defined(TAPI_FEAT_CALIBRATION)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CALIBRATION_CFG_SET),
#endif /* defined(TAPI_FEAT_CALIBRATION) */
#if defined(TAPI_FEAT_CALIBRATION)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CALIBRATION_CFG_GET),
#endif /* defined(TAPI_FEAT_CALIBRATION) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_COMTEL_START),
#if defined(TAPI_FEAT_CONT_MEAS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CONTMEASUREMENT_REQ),
#endif /* defined(TAPI_FEAT_CONT_MEAS) */
#if defined(TAPI_FEAT_CONT_MEAS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CONTMEASUREMENT_GET),
#endif /* defined(TAPI_FEAT_CONT_MEAS) */
#if defined(TAPI_FEAT_CONT_MEAS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_CONTMEASUREMENT_RESET),
#endif /* defined(TAPI_FEAT_CONT_MEAS) */
#if defined(TAPI_FEAT_CAP_MEAS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_MEASURE_CAPACITANCE_START),
#endif /* defined(TAPI_FEAT_CAP_MEAS) */
#if defined(TAPI_FEAT_CAP_MEAS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_MEASURE_CAPACITANCE_STOP),
#endif /* defined(TAPI_FEAT_CAP_MEAS) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_BATTERY_VOLTAGE_GET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_BATTERY_VOLTAGE_SET),
#if defined(TAPI_FEAT_NLT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_NLT_TEST_START),
#endif /* defined(TAPI_FEAT_NLT) */
#if defined(TAPI_FEAT_NLT)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_NLT_RESULT_GET),
#endif /* defined(TAPI_FEAT_NLT) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_NLT_CONFIGURATION_OL_SET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_NLT_CONFIGURATION_OL_GET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_NLT_CONFIGURATION_RMES_SET),
#if defined(TAPI_FEAT_CAP_MEAS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_NLT_CAPACITANCE_RESULT_GET),
#endif /* defined(TAPI_FEAT_CAP_MEAS) */
#if defined(TAPI_FEAT_CAP_MEAS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_NLT_CAPACITANCE_START),
#endif /* defined(TAPI_FEAT_CAP_MEAS) */
#if defined(TAPI_FEAT_CAP_MEAS)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_NLT_CAPACITANCE_STOP),
#endif /* defined(TAPI_FEAT_CAP_MEAS) */
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_VERSION_DEV_ENTRY_GET),
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_VERSION_CH_ENTRY_GET),
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_RTP_EXT_STATISTICS_GET),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_VOICE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_RTP_EXT_STATISTICS_CFG),
#endif /* defined(TAPI_FEAT_VOICE) */
#if defined(TAPI_FEAT_GR909)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_GR909_START),
#endif /* defined(TAPI_FEAT_GR909) */
#if defined(TAPI_FEAT_GR909)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_GR909_STOP),
#endif /* defined(TAPI_FEAT_GR909) */
#if defined(TAPI_FEAT_GR909)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_GR909_RESULT),
#endif /* defined(TAPI_FEAT_GR909) */
#if defined(TAPI_FEAT_MOS_LQE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_COD_MOS_CFG_SET),
#endif /* defined(TAPI_FEAT_MOS_LQE) */
#if defined(TAPI_FEAT_MOS_LQE)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_COD_MOS_RESULT_GET),
#endif /* defined(TAPI_FEAT_MOS_LQE) */
#if defined(TAPI_FEAT_PHONE_DETECTION)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_PHONE_DETECT_CFG_SET),
#endif /* defined(TAPI_FEAT_PHONE_DETECTION) */
#if defined(TAPI_FEAT_PHONE_DETECTION)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_LINE_PHONE_DETECT_CFG_GET),
#endif /* defined(TAPI_FEAT_PHONE_DETECTION) */
#if defined(TAPI_FEAT_SRTP)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_SRTP_CFG_SET),
#endif /* defined(TAPI_FEAT_SRTP) */
#if defined(TAPI_FEAT_SRTP)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_SRTP_MKI_SET),
#endif /* defined(TAPI_FEAT_SRTP) */
#if defined(TAPI_FEAT_SRTP)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_SRTP_MKI_CFG_SET),
#endif /* defined(TAPI_FEAT_SRTP) */
#if defined(TAPI_FEAT_SRTP)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_SRTP_CAP_GET),
#endif /* defined(TAPI_FEAT_SRTP) */
#if defined(TAPI_FEAT_SRTP)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_SRTP_STATISTICS_GET),
#endif /* defined(TAPI_FEAT_SRTP) */
#if defined(TAPI_FEAT_SRTP)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_SRTP_STATISTICS_RESET),
#endif /* defined(TAPI_FEAT_SRTP) */
#if defined(TAPI_FEAT_SRTP)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_SRTP_PACKET_INDEX_SET),
#endif /* defined(TAPI_FEAT_SRTP) */
#if defined(TAPI_FEAT_SRTP)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PKT_SRTP_PACKET_INDEX_GET),
#endif /* defined(TAPI_FEAT_SRTP) */
#if defined(TAPI_FEAT_PCM)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_PCM_MUTE_CFG_SET),
#endif /* defined(TAPI_FEAT_PCM) */
#if defined(TAPI_FEAT_AMR)
   IOCTL_LKUP_TBL_ADD (IFX_TAPI_COD_AMR_GET),
#endif /* defined(TAPI_FEAT_AMR) */
/* IOCTL_LOOKUP_END */

#ifdef TAPI_FEAT_QOS
/* IOCTL_QOS_LOOKUP_START */
   IOCTL_LKUP_TBL_ADD (FIO_QOS_START),
   IOCTL_LKUP_TBL_ADD (FIO_QOS_ON_SOCKET_START),
   IOCTL_LKUP_TBL_ADD (FIO_QOS_STOP),
   IOCTL_LKUP_TBL_ADD (FIO_QOS_CLEAN),
/* IOCTL_QOS_LOOKUP_END */
#endif /* TAPI_FEAT_QOS */
   { 0, "" }
};

#ifdef EVENT_LOGGER_DEBUG
#if !defined(__KERNEL__) && defined(LINUX)
static IFX_char_t *tapi_pUsrLogBuffer = IFX_NULL;
IFX_int32_t tapi_nEl_fd = -1;
/** Keeps information about number of TAPI devices which are registered in
    Event Logger. When last device is unregistered then memory
    (tapi_pUsrLogBuffer) should be free and file descriptor (tapi_nEl_fd)
    should be closed. */
static IFX_int32_t tapi_nEl_devCounter = 0;
#endif /* !defined(__KERNEL__) && defined(LINUX) */
#endif /* EVENT_LOGGER_DEBUG */

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */
#ifdef EVENT_LOGGER_DEBUG
#if !defined(__KERNEL__) && defined(LINUX)
static IFX_void_t TAPI_EL_Register (TAPI_DEV *pDev);
static IFX_void_t TAPI_EL_Unregister (TAPI_DEV *pDev);
#else
static IFX_int32_t TAPI_EL_format(
      IFX_int32_t nDevType,
      IFX_int32_t nDevNum,
      IFX_int32_t nChNum,
      IFX_char_t* sTime,
      IFX_int32_t nLogType,
      IFX_int32_t nDataSize,
      IFX_void_t* pData,
      IFX_char_t* pOutput,
      IFX_int32_t nOutLen
   );
#endif /* !defined(__KERNEL__) && defined(LINUX) */
#endif /* EVENT_LOGGER_DEBUG */

/* ========================================================================== */
/*                         Function implementation                            */
/* ========================================================================== */

/**
   Set the trace level

   \param pDev    reference to device context
   \param nLevel  Minimal level for traced messages

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
IFX_int32_t TAPI_DebugReportSet (TAPI_DEV *pDev, IFX_uint32_t nLevel)
{
   IFX_UNUSED (pDev);

   SetTraceLevel (TAPI_DRV, nLevel);

   return TAPI_statusOk;
}

#ifdef EVENT_LOGGER_DEBUG
/**
   Register device in Event Logger driver

   \param pDev  - reference to device context

   \return none
*/
static IFX_void_t TAPI_EL_Register (TAPI_DEV *pDev)
{
#if !defined(__KERNEL__) && defined(LINUX)
   IFX_int32_t nRet;
   EL_IoctlRegister_t stReg;

   strcpy(stReg.sName, DRV_TAPI_NAME);
   stReg.nType = IFX_TAPI_DEV_TYPE_NONE;
   stReg.nDevNum = pDev->nDevID;

   nRet = ioctl(tapi_nEl_fd, EL_REGISTER, (IFX_int32_t)&stReg);
   if (nRet != IFX_SUCCESS)
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
             ("DRV_ERROR: Registration in event logger failed, dev ID:%d "
              "(File: %s, line: %d)\n",
              pDev->nDevID, __FILE__, __LINE__));
   }
#else
   EL_REG_Register (DRV_TAPI_NAME, IFX_TAPI_DEV_TYPE_NONE,
                    pDev->nDevID /* dev num */,
                    TAPI_EL_format /* cb func */);
#ifdef WIN32
   EL_CFG_SetupLogging(EL_LOG_TYPE_ALL, DRV_TAPI_NAME);
#endif
#if 0
   {
      IFX_char_t buf[20];

      /* Workaround. For Danube "ifx_tapi_open" isn't called on startup. Thence
       * when "echo "*"  > /proc/driver/el/logs/all" is called in, for example,
       * custom-user-startup script, driver hasn't been registered yet and after
       * registration here log for TAPI is disabled. It should be enabled.
       * It is also impossible to register TAPI driver for event logger on startup,
       * because number of devices is unknown. */
      sprintf (buf, "%s:%d", DRV_TAPI_NAME, pDev->nDevID);
      EL_CFG_SetupLogging(EL_LOG_TYPE_ALL, buf);
   }
#endif /* 0 */
#endif
}

/**
   Unregister device in Event Logger driver

   \param pDev  - reference to device context

   \return none
*/
static IFX_void_t TAPI_EL_Unregister (TAPI_DEV *pDev)
{
#if !defined(__KERNEL__) && defined(LINUX)
   IFX_int32_t nRet;
   EL_IoctlRegister_t stReg;

   strcpy(stReg.sName, DRV_TAPI_NAME);
   stReg.nType = IFX_TAPI_DEV_TYPE_NONE;
   stReg.nDevNum = pDev->nDevID;

   nRet = ioctl(tapi_nEl_fd, EL_UNREGISTER, (IFX_int32_t)&stReg);
   if (nRet != IFX_SUCCESS)
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
             ("DRV_ERROR: Unregistration in event logger failed, dev ID:%d "
              "(File: %s, line: %d)\n",
              pDev->nDevID, __FILE__, __LINE__));
   }
#else
   EL_REG_Unregister (DRV_TAPI_NAME, IFX_TAPI_DEV_TYPE_NONE,
                      pDev->nDevID /* dev num */);
#endif
}
#endif /* EVENT_LOGGER_DEBUG */

/**
   TAPI syslog initialization

   \param pDev  - reference to device context

   \return none
*/
/*lint -save -esym(715, pDev) */
IFX_void_t TAPI_LogInit (TAPI_DEV *pDev)
{
#ifdef EVENT_LOGGER_DEBUG
#if !defined(__KERNEL__) && defined(LINUX)
   /* allocate memory for needed by functions which parse logs sent to
      event logger. */
   if (tapi_pUsrLogBuffer == IFX_NULL)
   {
      tapi_pUsrLogBuffer = TAPI_OS_Malloc(EL_MAX_LONGSTRING_LEN_IN_LOG);
      if (tapi_pUsrLogBuffer == IFX_NULL)
      {
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
                ("DRV_ERROR: memory allocation failed (File: %s, line: %d)\n",
                 __FILE__, __LINE__));
      }
   }

   if (tapi_nEl_fd < 0)
   {
      tapi_nEl_fd = open(EL_DEVICE_NAME, O_RDWR, 0644);
      if (tapi_nEl_fd < 0)
      {
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
                ("DRV_ERROR: open of event logger device failed "
                 "(File: %s, line: %d)\n",
                 __FILE__, __LINE__));
      }
   }
   tapi_nEl_devCounter++;
#endif /* !defined(__KERNEL__) && defined(LINUX) */
   TAPI_EL_Register(pDev);
#else /* EVENT_LOGGER_DEBUG */
   IFX_UNUSED(pDev);
#endif /* EVENT_LOGGER_DEBUG */
}

/**
   TAPI syslog uninitialization

   \param pDev  - reference to device context

   \return none
*/
IFX_void_t TAPI_LogClose (TAPI_DEV *pDev)
{
#ifdef EVENT_LOGGER_DEBUG
   TAPI_EL_Unregister(pDev);
#if !defined(__KERNEL__) && defined(LINUX)
   tapi_nEl_devCounter--;
   if (tapi_nEl_devCounter <= 0)
   {
      if (tapi_pUsrLogBuffer != IFX_NULL)
         TAPI_OS_Free(tapi_pUsrLogBuffer);
      if (tapi_nEl_fd >= 0)
      {
         IFXOS_DeviceClose(tapi_nEl_fd);
         tapi_nEl_fd = -1;
      }
   }
#endif /* !defined(__KERNEL__) && defined(LINUX) */
#else /* EVENT_LOGGER_DEBUG */
   IFX_UNUSED(pDev);
#endif /* EVENT_LOGGER_DEBUG */
}
/*lint -restore */

/*
   Retrieve named IOCTL representatuion

   \param nIoctl  TAPI IOCTL number

   \return
      Handle to the zero terminated string

   \remarks
      Do not require memory freeing.
      Do not thread-safe realization for unknown ioctls.
*/
IFX_char_t const *TAPI_ioctlNameGet(IFX_uint32_t nIoctl)
{
   static IFX_char_t pBuf[0x11];
   IoctlLkupTable_t const *pIoctl = tapi_ioctl_table;

   while (pIoctl->val != 0 && pIoctl->val != nIoctl)
      pIoctl++;

   if (pIoctl->val != 0)
      return pIoctl->name;

   /* \note: Used local static buffer to skip memory management code.
      That is not thread-safe code, but used because of it is executed in
      rarely cases, like for unsupported or low-level services.
      Feel free to rewrite that implementation if it is raise some problems.
    */
   snprintf (pBuf, sizeof (pBuf), "0x%08X", nIoctl);

   return pBuf;
}

#ifdef EVENT_LOGGER_DEBUG
#if !defined(__KERNEL__) && defined(LINUX)
/**
   Provide ioctl data formatting

   \param pLog    - pointer to EL_IoctlAddLog_t structure
   \param pOutput - formatted log in text format, used for the output
   \param nOutLen - available length of the buffer

   \return Callback status, look at enum EL_CB_Status_t and
           pOutput variable can contain valid log as the null terminated string
           for the output
*/
static IFX_int32_t TAPI_EL_IOCTL_USR_format(EL_IoctlAddLog_t *pLog,
                                            IFX_char_t *pOutput,
                                            IFX_int32_t nOutLen)
{
   IFX_uint32_t nIoctl = 0, nErr = 0;
   IFX_uint32_t i = 0, len = 0;
   IFX_int32_t n = 0;
   IFX_uint16_t val_len = 0;
   IFX_boolean_t bContinued = IFX_FALSE;
   IFX_char_t* pData = IFX_NULL;

   if ((pOutput == IFX_NULL) || (nOutLen == 0))
   {
      return EL_CB_StatusErr;
   }

   switch (pLog->nLogType)
   {
      case EL_LOG_TYPE_IOCTL_WR:
         val_len = pLog->uLogDetails.stIoctl_Wr.nDataLength;
         nErr = pLog->uLogDetails.stIoctl_Wr.nStatus;
         nIoctl = pLog->uLogDetails.stIoctl_Wr.nCmd;
         pData = pLog->uLogDetails.stIoctl_Wr.pData;
         break;
      case EL_LOG_TYPE_IOCTL_RD:
         val_len = pLog->uLogDetails.stIoctl_Rd.nDataLength;
         nErr = pLog->uLogDetails.stIoctl_Rd.nStatus;
         nIoctl = pLog->uLogDetails.stIoctl_Rd.nCmd;
         pData = pLog->uLogDetails.stIoctl_Rd.pData;
         if (nErr != 0)
            TAPI_EL_APPEND (" < ERROR [%08X]", nErr);
         else if (val_len == 0)
            TAPI_EL_APPEND ("%s", " >");
         else
            TAPI_EL_APPEND ("%s", " <");
         break;
      default:
         return EL_CB_StatusFormat;
   }

   /* Get length of the ioctl data */
   if (pData == IFX_NULL)
      val_len = 0;

   TAPI_EL_APPEND (" (%s)", TAPI_ioctlNameGet (nIoctl));

   TAPI_EL_APPEND (" %08X", nIoctl);

   if ((val_len > TAPI_EL_IOCTL_MAX_LEN) || (val_len >= (nOutLen - len)))
   {
      val_len = min ((IFX_uint32_t)TAPI_EL_IOCTL_MAX_LEN, nOutLen - (len + 10U));/*lint !e155 */
      bContinued = IFX_TRUE;
   }

   for (i = 0; i < val_len; i++)
   {
      TAPI_EL_APPEND ("%s%02X",
         ((i & 0x3) == 0x0) ? " " : "",
         ((IFX_uint8_t*)pData)[i]);
   }

   if (bContinued == IFX_TRUE)
   {
      TAPI_EL_APPEND ("%s", " ...");
   }

   return EL_CB_StatusOutput;
}
#else /* !defined(__KERNEL__) && defined(LINUX) */
/**
   Provide ioctl data formatting

   \param nLogType - type of the log
   \param nDataSize - size of the data passed to this function
   \param pData - pointer to the data block
   \param pOutput - formatted log in text format, used for the output
   \param nOutLen - available length of the buffer

   \return Callback status, look at enum EL_CB_Status_t and
           pOutput variable can contain valid log as the null terminated string
           for the output
   \remarks
      pData format:
         [data1 size][data1][data2 size][data2]...[dataN size][dataN]
*/
static IFX_int32_t TAPI_EL_IOCTL_format(
      IFX_int32_t nLogType,
      IFX_uint32_t nDataSize,
      IFX_void_t* pData,
      IFX_char_t* pOutput,
      IFX_uint32_t nOutLen
   )
{
   IFX_uint32_t nIoctl = 0, nErr = 0;
   IFX_uint32_t i = 0, len = 0;
   IFX_int32_t n = 0;
   IFX_uint16_t val_len = 0;
   IFX_boolean_t bContinued = IFX_FALSE;

   if ((pOutput == IFX_NULL) || (nOutLen == 0))
   {
      return EL_CB_StatusErr;
   }

   /* Get length of the ioctl data */
   if (nDataSize > 6U)
      val_len = ((IFX_uint16_t*)pData)[3];
   else
      val_len = 0;

   /* last 4 bytes contain return value (error/success) */
   for (i = nDataSize - 4U; i < nDataSize; i++)
      nErr = (nErr << 8) | ((IFX_uint8_t*)pData)[i];

   if (nLogType == EL_LOG_TYPE_IOCTL_RD)
   {
      if (nErr != 0)
         TAPI_EL_APPEND (" < ERROR [%08X]", nErr);
      else if (val_len == 0)
         TAPI_EL_APPEND ("%s", " >");
      else
         TAPI_EL_APPEND ("%s", " <");
   }

   memcpy (&nIoctl, ((IFX_uint8_t*)pData) + 2, sizeof (IFX_uint32_t));

   TAPI_EL_APPEND (" (%s)", TAPI_ioctlNameGet (nIoctl));

   TAPI_EL_APPEND (" %08X", nIoctl);

   if ((val_len > TAPI_EL_IOCTL_MAX_LEN) || (val_len >= (nOutLen - len)))
   {
      val_len = min ((IFX_uint32_t)TAPI_EL_IOCTL_MAX_LEN, nOutLen - (len + 10U));/*lint !e155 */
      bContinued = IFX_TRUE;
   }

   for (i = 0; i < val_len; i++)
   {
      TAPI_EL_APPEND ("%s%02X",
         ((i & 0x3) == 0x0) ? " " : "",
         ((IFX_uint8_t*)pData)[8 + i]);
   }

   if (bContinued == IFX_TRUE)
   {
      TAPI_EL_APPEND ("%s", " ...");
   }

   return EL_CB_StatusOutput;
}
#endif /* !defined(__KERNEL__) && defined(LINUX) */

#if !defined(__KERNEL__) && defined(LINUX)
/**
   Function used to format logs for Event Logger driver.

   \param pLog    - pointer to EL_IoctlAddLog_t structure
   \param pOutput - formatted log in text format, used for the output

   \return Callback status, look at enum EL_CB_Status_t and
           pOutput variable can contain valid log as the null terminated string
           for the output
*/
IFX_int32_t TAPI_EL_USR_format( EL_IoctlAddLog_t *pLog, IFX_char_t **ppOutput)
{
   IFX_uint32_t len = 0;
   IFX_int32_t n = 0;
   IFX_int32_t nOutLen = EL_MAX_LONGSTRING_LEN_IN_LOG;
   IFX_char_t *pOutput = tapi_pUsrLogBuffer;
   IFX_int32_t nRet;

   if (pLog == IFX_NULL)
   {
      return EL_CB_StatusErr;
   }
   memset(pOutput, 0, EL_MAX_LONGSTRING_LEN_IN_LOG);

   TAPI_EL_APPEND ("%s ", DRV_TAPI_NAME);

   switch (pLog->nLogType)
   {
      case EL_LOG_TYPE_IOCTL_WR:
         TAPI_EL_APPEND ("%s", "[iw]");
         break;
      case EL_LOG_TYPE_IOCTL_RD:
         TAPI_EL_APPEND ("%s", "[ir]");
         break;
      default:
         return EL_CB_StatusFormat;
   }

   TAPI_EL_APPEND (" %d %d", pLog->nDevNum, pLog->nChNum);

   nRet = TAPI_EL_IOCTL_USR_format(pLog, pOutput + len, nOutLen - len);
   *ppOutput = pOutput;
   return nRet;
}
#else /* !defined(__KERNEL__) && defined(LINUX) */
/**
   Test callback driver function

   \param nDevType - type of the device
   \param nDevNum - device number
   \param nChNum - channel number
   \param sTime - time in text format
   \param nLogType - type of the log
   \param nDataSize - size of the data passed to this function
   \param pData - pointer to the data block
   \param pOutput - formatted log in text format, used for the output
   \param nOutLen - available length of the buffer

   \return Callback status, look at enum EL_CB_Status_t and
           pOutput variable can contain valid log as the null terminated string
           for the output
*/
#ifdef TAPI_VERSION3
static IFX_int32_t TAPI_EL_format(
      IFX_int32_t nDevType,
      IFX_int32_t nDevNum,
      IFX_int32_t nChNum,
      IFX_char_t* sTime,
      IFX_int32_t nLogType,
      IFX_int32_t nDataSize,
      IFX_void_t* pData,
      IFX_char_t* pOutput,
      IFX_int32_t nOutLen
   )
{
   IFX_uint32_t len = 0;
   IFX_int32_t n = 0;
   IFX_int32_t k;
   TAPI_DEV *pTapiDev = IFX_NULL;

   TAPI_ASSERT (sTime != IFX_NULL);
   TAPI_ASSERT (pData != IFX_NULL);
   TAPI_ASSERT (pOutput != IFX_NULL);
   TAPI_ASSERT (nOutLen > 0);

   IFX_UNUSED (nDevType);

   if ((pOutput == IFX_NULL) || (nOutLen <= 0))
   {
      return EL_CB_StatusErr;
   }

   /* Low Level drivers used in CPE do not have callback functions for log
      formating. Spaces are added to have similar log formating like in
      low level drivers. */
   switch (nLogType)
   {
      case EL_LOG_TYPE_IOCTL_WR:
         TAPI_EL_APPEND ("%s", "[iw]");
         TAPI_EL_ADD_PADDING(2);
         break;
      case EL_LOG_TYPE_IOCTL_RD:
         TAPI_EL_APPEND ("%s", "[ir]");
         TAPI_EL_ADD_PADDING(2);
         break;
      default:
         return EL_CB_StatusFormat;
   }
   TAPI_EL_APPEND ("%s", sTime );

   pTapiDev = TAPI_DeviceGetByID(nDevNum);
   if (IFX_NULL == pTapiDev)
   {
      /* transparently handle if device not found with such ID */
      if (strlen(DRV_TAPI_NAME) < 10)
          TAPI_EL_ADD_PADDING((10 - strlen(DRV_TAPI_NAME)));
      TAPI_EL_APPEND ("%s", DRV_TAPI_NAME);
   }
   else
   {
      /* append low level information */
      if ((strlen(DRV_TAPI_NAME) + strlen(pTapiDev->pDevDrvCtx->drvName)) < 10)
          TAPI_EL_ADD_PADDING((10 - strlen(DRV_TAPI_NAME) - strlen(pTapiDev->pDevDrvCtx->drvName)));
      TAPI_EL_APPEND ("%s-%s", DRV_TAPI_NAME, pTapiDev->pDevDrvCtx->drvName);
      /* replace global TAPI ID in to the low-level specified device number */
      nDevNum = pTapiDev->nDev;
   }
   TAPI_EL_ADD_PADDING(7);
   TAPI_EL_APPEND ("%02d", nDevNum);
   TAPI_EL_ADD_PADDING(2);
   TAPI_EL_APPEND ("%02d", nChNum);

   return TAPI_EL_IOCTL_format (nLogType, (IFX_uint32_t)nDataSize, pData,
      pOutput + len, (IFX_uint32_t)nOutLen - len);
}
#else /* TAPI_VERSION3 */
static IFX_int32_t TAPI_EL_format(
      IFX_int32_t nDevType,
      IFX_int32_t nDevNum,
      IFX_int32_t nChNum,
      IFX_char_t* sTime,
      IFX_int32_t nLogType,
      IFX_int32_t nDataSize,
      IFX_void_t* pData,
      IFX_char_t* pOutput,
      IFX_int32_t nOutLen
   )
{
   IFX_uint32_t len = 0;
   IFX_int32_t n = 0;
   TAPI_DEV *pTapiDev = IFX_NULL;

   TAPI_ASSERT (sTime != IFX_NULL);
   TAPI_ASSERT (pData != IFX_NULL);
   TAPI_ASSERT (pOutput != IFX_NULL);
   TAPI_ASSERT (nOutLen > 0);

   IFX_UNUSED (nDevType);

   if ((pOutput == IFX_NULL) || (nOutLen <= 0))
   {
      return EL_CB_StatusErr;
   }

   pTapiDev = TAPI_DeviceGetByID(nDevNum);
   if (IFX_NULL == pTapiDev)
   {
      /* transparently handle if device not found with such ID */
      TAPI_EL_APPEND ("%s %s ", sTime, DRV_TAPI_NAME);
   }
   else
   {
      /* append low level information */
      TAPI_EL_APPEND ("%s %s-%s ", sTime, DRV_TAPI_NAME, pTapiDev->pDevDrvCtx->drvName);
      /* replace global TAPI ID in to the low-level specified device number */
      nDevNum = pTapiDev->nDev;
   }

   switch (nLogType)
   {
      case EL_LOG_TYPE_IOCTL_WR:
         TAPI_EL_APPEND ("%s", "[iw]");
         break;
      case EL_LOG_TYPE_IOCTL_RD:
         TAPI_EL_APPEND ("%s", "[ir]");
         break;
      default:
         return EL_CB_StatusFormat;
   }

   TAPI_EL_APPEND (" %d %d", nDevNum, nChNum);

   return TAPI_EL_IOCTL_format (nLogType, (IFX_uint32_t)nDataSize, pData,
      pOutput + len, (IFX_uint32_t)nOutLen - len);
}
#endif /* TAPI_VERSION3 */
#endif /* !defined(__KERNEL__) && defined(LINUX) */
#endif /* EVENT_LOGGER_DEBUG */

/* ========================================================================== */
/*                         Function pointer exports                           */
/* ========================================================================== */
