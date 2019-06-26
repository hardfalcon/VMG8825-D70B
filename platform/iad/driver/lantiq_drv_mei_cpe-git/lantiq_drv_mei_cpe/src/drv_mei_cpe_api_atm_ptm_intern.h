/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _DRV_MEI_CPE_API_ATM_PTM_INTERN_H
#define _DRV_MEI_CPE_API_ATM_PTM_INTERN_H

/* ============================================================================
   Description : MEI CPE Driver internal API for ATM/PTM kernel drivers
   ========================================================================= */

/* ==========================================================================
   includes
   ========================================================================== */
#include "ifx_types.h"

#include "drv_mei_cpe_config.h"
#include "drv_mei_cpe_interface.h"

#ifdef PPA_SUPPORTS_CALLBACKS
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
#include <net/dsl_tc.h>
#else
#include <net/ppa_stack_al.h>
#endif
#endif /* #ifdef PPA_SUPPORTS_CALLBACKS */

/** Maximum number of ERB retrieval attempts */
#define MEI_DEV_ERB_ATTEMPTS  5
/** Timeout setting for ERB address update in mili seconds */
#define MEI_DEV_ERB_TIMEOUT   500

#if (MEI_EXPORT_INTERNAL_API == 1) && (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1)

typedef struct {
   unsigned int rate_fast;
   unsigned int rate_intl;
} MEI_XTM_ShowtimeEnter_t;

typedef struct {
   unsigned int dummy;
} MEI_XTM_ShowtimeExit_t;

typedef struct {
   unsigned int request_type;
   unsigned int is_bonding;
} MEI_TC_Request_t;

typedef struct {
   unsigned int reset_type;
} MEI_TC_Reset_t;

extern IFX_int32_t MEI_InternalXtmSwhowtimeEntrySignal(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              MEI_XTM_ShowtimeEnter_t *pArgXtm);

extern IFX_int32_t MEI_InternalXtmSwhowtimeExitSignal(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              MEI_XTM_ShowtimeExit_t *pArgXtm);

extern IFX_int32_t MEI_InternalTcRequest(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              MEI_TC_Request_t       *pArgTcRequest);

extern IFX_int32_t MEI_InternalTcReset(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              MEI_TC_Reset_t         *pArgTcReset);

extern IFX_int32_t MEI_InternalErbAddrUpdate(
                                  MEI_DYN_CNTRL_T        *pMeiDynCntrl);

#ifdef PPA_SUPPORTS_CALLBACKS
extern int ppa_callback_set(e_ltq_mei_cb_type type, void *func);
extern void* ppa_callback_get(e_ltq_mei_cb_type type);

int ltq_mei_showtime_check(
                              const unsigned char line_idx,
                              int *is_showtime,
                              struct port_cell_info *port_cell,
                              void **xdata_addr);
#endif /* #ifdef PPA_SUPPORTS_CALLBACKS */

#endif      /* #if (MEI_EXPORT_INTERNAL_API == 1) && (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1) */

#endif      /* #ifndef _DRV_MEI_CPE_API_ATM_PTM_INTERN_H */

