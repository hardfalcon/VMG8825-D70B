#ifndef _DRV_TAPI_QOS_H
#define _DRV_TAPI_QOS_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_tapi_qos.h  TAPI interface to the QOS driver.
*/

#ifdef TAPI_FEAT_QOS

#if defined(LINUX) && !defined(__KERNEL__)
   #error QOS feature available only in kernel space
#endif /* defined(LINUX) && !defined(__KERNEL__) */

#ifdef TAPI_FEAT_PROCFS
   #include <linux/seq_file.h>
#endif /* TAPI_FEAT_PROCFS */

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Driver Global Structures      */
/* ============================= */

/* ============================= */
/* Driver function declaration   */
/* ============================= */

/** Initialise the QOS service */
extern IFX_return_t IFX_TAPI_QOS_Init           (void);

/** Clean-up the QOS service */
extern IFX_void_t   IFX_TAPI_QOS_Cleanup        (void);

/** QOS IOCTL handlers */
extern IFX_int32_t IFX_TAPI_QOS_Start (TAPI_CHANNEL* pCh,
   QOS_INIT_SESSION const *pInit);

extern IFX_int32_t IFX_TAPI_QOS_SessionOnSocketStart (TAPI_CHANNEL* pChannel,
   QOS_INIT_SESSION_ON_SOCKET const *pInit);

extern IFX_int32_t IFX_TAPI_QOS_Stop (TAPI_CHANNEL* pCh);

extern IFX_int32_t IFX_TAPI_QOS_Clean (void);

#ifdef TAPI_FEAT_PROCFS
/** Linux proc filesystem printout */
extern IFX_void_t IFX_TAPI_QOS_proc_read_registration (struct seq_file *s);
#endif /* TAPI_FEAT_PROCFS */

#endif /* TAPI_FEAT_QOS */

#endif /* _DRV_TAPI_QOS_H */
