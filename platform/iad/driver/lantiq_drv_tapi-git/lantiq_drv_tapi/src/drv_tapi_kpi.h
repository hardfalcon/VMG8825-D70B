#ifndef _DRV_TAPI_KPI_H
#define _DRV_TAPI_KPI_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_kpi.h
   This file contains the declaration of the "Kernel Packet Interface" (KPI).
   The KPI is used to exchange packetised data with other drivers.
*/

#ifdef TAPI_FEAT_KPI

#if defined(LINUX) && !defined(__KERNEL__)
   #error KPI feature available only in kernel space
#endif /* defined(LINUX) && !defined(__KERNEL__) */

/* ========================================================================== */
/*                               Includes                                     */
/* ========================================================================== */
#include <lib_fifo.h>
#include "drv_tapi_kpi_io.h"

/* ========================================================================== */
/*                               Configuration                                */
/* ========================================================================== */


/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */


/* ========================================================================== */
/*                             Type definitions                               */
/* ========================================================================== */

/** Struct used in the TAPI_CHANNEL for each stream to set the target.
    The values are set by the ioctl \ref IFX_TAPI_KPI_CH_CFG_SET. */
typedef struct
{
   /** KPI channel where the stream is sent to */
   IFX_TAPI_KPI_CH_t    nKpiCh;
   /** Direct reference to the egress fifo in the target KPI group */
   FIFO_ID             *pEgressFifo;
} IFX_TAPI_KPI_STREAM_SWITCH;

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */

/** Initialise the Kernel Packet Interface */
extern IFX_return_t IFX_TAPI_KPI_Init (void);

/** Shutdown the Kernel Packet Interface */
extern IFX_void_t   IFX_TAPI_KPI_Cleanup (void);

/** Handler for the ioctl IFX_TAPI_KPI_CH_CFG_SET */
extern IFX_int32_t  IFX_TAPI_KPI_ChCfgSet (TAPI_CHANNEL *pChannel,
                                           IFX_TAPI_KPI_CH_CFG_t const *pCfg);

/** Handler for the ioctl IFX_TAPI_KPI_GRP_CFG_SET */
extern IFX_int32_t  IFX_TAPI_KPI_GrpCfgSet (IFX_TAPI_KPI_GRP_CFG_t const *pCfg);

#endif /* TAPI_FEAT_KPI */

#endif /* _DRV_TAPI_KPI_H */
