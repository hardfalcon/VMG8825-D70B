#ifndef __DRV_TAPI_POLLING_H__
#define __DRV_TAPI_POLLING_H__
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef TAPI_FEAT_POLL

extern IFX_int32_t TAPI_IrqPollConf (IFX_TAPI_POLL_CONFIG_t const *pGlobalPollCfg);
extern IFX_int32_t TAPI_PoolDevAdd (TAPI_DEV *pTapiDev);
extern IFX_int32_t TAPI_PoolDevRem (TAPI_DEV *pTapiDev);
/* Just for testing purposes only. */
extern IFX_int32_t TAPI_Poll_Test (void);

#endif /* TAPI_FEAT_POLL */

#endif /* __DRV_TAPI_POLLING_H__ */
