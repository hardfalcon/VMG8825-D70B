/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file  drv_tapi_voice.c
   Contains TAPI Voice Services : Play, Recording, Conferencing.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"
#include "drv_tapi_errno.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */

#ifdef TAPI_FEAT_VOICE
/**
   This interface adds a data channel to an analog phone device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMap          Pointer to IFX_TAPI_MAP_DATA_t structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Data_Channel_Add (TAPI_CHANNEL *pChannel,
                                   IFX_TAPI_MAP_DATA_t const *pMap)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = TAPI_statusOk, retLL = TAPI_statusOk;

   switch (pMap->nPlayStart)
   {
      case IFX_TAPI_MAP_DATA_UNCHANGED:
         /* do nothing */
         break;
      case IFX_TAPI_MAP_DATA_START:
         if (IFX_TAPI_PtrChk (pDrvCtx->COD.DEC_Start))
            retLL = pDrvCtx->COD.DEC_Start (pChannel->pLLChannel);
         else
            RETURN_STATUS (TAPI_statusNotSupported, 0);
         break;
      case IFX_TAPI_MAP_DATA_STOP:
         if (IFX_TAPI_PtrChk (pDrvCtx->COD.DEC_Stop))
            retLL = pDrvCtx->COD.DEC_Stop (pChannel->pLLChannel);
         else
            RETURN_STATUS (TAPI_statusNotSupported, 0);
         break;
   }
   if (TAPI_SUCCESS(retLL))
   {
      switch (pMap->nRecStart)
      {
         case IFX_TAPI_MAP_DATA_UNCHANGED:
            /* do nothing */
            break;
         case IFX_TAPI_MAP_DATA_START:
            if (IFX_TAPI_PtrChk (pDrvCtx->COD.ENC_Start))
               retLL = pDrvCtx->COD.ENC_Start (pChannel->pLLChannel);
            else
               RETURN_STATUS (TAPI_statusNotSupported, 0);
            break;
         case IFX_TAPI_MAP_DATA_STOP:
            if (IFX_TAPI_PtrChk (pDrvCtx->COD.ENC_Stop))
               retLL = pDrvCtx->COD.ENC_Stop (pChannel->pLLChannel);
            else
               RETURN_STATUS (TAPI_statusNotSupported, 0);
            break;
      }
   }

   /* call low level function here */
   if (TAPI_SUCCESS (retLL))
   {
      if (IFX_TAPI_PtrChk (pDrvCtx->CON.Data_Channel_Add))
      {
         retLL = pDrvCtx->CON.Data_Channel_Add (pChannel->pLLChannel, pMap);
      }
      else
      {
         RETURN_STATUS (TAPI_statusNotSupported, 0);
      }
   }
   if (!TAPI_SUCCESS(retLL))
   {
      /* errmsg: LL driver returned an error */
      RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }

   return ret;
}
#endif /* TAPI_FEAT_VOICE */


#ifdef TAPI_FEAT_VOICE
/**
   This interface removes a data channel from an analog phone device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMap          Pointer to IFX_TAPI_MAP_DATA_t structure.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Data_Channel_Remove (TAPI_CHANNEL *pChannel,
                                      IFX_TAPI_MAP_DATA_t const *pMap)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = TAPI_statusOk;

   switch (pMap->nPlayStart)
   {
      case IFX_TAPI_MAP_DATA_UNCHANGED:
         /* do nothing */
         break;
      case IFX_TAPI_MAP_DATA_START:
         if (IFX_TAPI_PtrChk (pDrvCtx->COD.DEC_Start))
            ret = pDrvCtx->COD.DEC_Start (pChannel->pLLChannel);
         else
            RETURN_STATUS (TAPI_statusNotSupported, 0);
         break;
      case IFX_TAPI_MAP_DATA_STOP:
         if (IFX_TAPI_PtrChk (pDrvCtx->COD.DEC_Stop))
            ret = pDrvCtx->COD.DEC_Stop (pChannel->pLLChannel);
         else
            RETURN_STATUS (TAPI_statusNotSupported, 0);
         break;
   }
   if (TAPI_SUCCESS (ret))
   {
      switch (pMap->nRecStart)
      {
         case IFX_TAPI_MAP_DATA_UNCHANGED:
            /* do nothing */
            break;
         case IFX_TAPI_MAP_DATA_START:
            if (IFX_TAPI_PtrChk (pDrvCtx->COD.ENC_Start))
               ret = pDrvCtx->COD.ENC_Start (pChannel->pLLChannel);
            else
               RETURN_STATUS (TAPI_statusNotSupported, 0);
            break;
         case IFX_TAPI_MAP_DATA_STOP:
            if (IFX_TAPI_PtrChk (pDrvCtx->COD.ENC_Stop))
               ret = pDrvCtx->COD.ENC_Stop (pChannel->pLLChannel);
            else
               RETURN_STATUS (TAPI_statusNotSupported, 0);
            break;
      }
   }

   /* call low level function here */
   if (TAPI_SUCCESS (ret))
   {
      if (IFX_TAPI_PtrChk (pDrvCtx->CON.Data_Channel_Remove))
         ret = pDrvCtx->CON.Data_Channel_Remove (pChannel->pLLChannel, pMap);
      else
         RETURN_STATUS (TAPI_statusNotSupported, 0);
   }

   return ret;
}
#endif /* TAPI_FEAT_VOICE */


#if defined(TAPI_FEAT_VOICE) || defined(TAPI_FEAT_PCM) || defined(TAPI_FEAT_DECT)
/**
   Add a connection between any two of the following modules:
   PCM, ALM, DECT.

   \param   pChannel Handle to TAPI channel structure.
   \param   nSrcType Source module type
   \param   nDstCh   Destination channel number
   \param   nDstType Destination module type

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
static IFX_int32_t TAPI_MapAdd (TAPI_CHANNEL *pChannel,
   IFX_TAPI_MAP_TYPE_t nSrcType, IFX_uint16_t nDstCh,
   IFX_TAPI_MAP_TYPE_t nDstType)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->CON.Module_Connect))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->CON.Module_Connect (pChannel->pLLChannel,
      nSrcType, nDstCh, nDstType);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* defined(TAPI_FEAT_VOICE) || defined(TAPI_FEAT_PCM) || defined(TAPI_FEAT_DECT) */


#if defined(TAPI_FEAT_VOICE) || defined(TAPI_FEAT_PCM) || defined(TAPI_FEAT_DECT)
/**
   Remove a connection between any two of the following modules:
   PCM, ALM, DECT.

   \param   pChannel Handle to TAPI channel structure.
   \param   nSrcType Source module type
   \param   nDstCh   Destination channel number
   \param   nDstType Destination module type

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
static IFX_int32_t TAPI_MapRemove (TAPI_CHANNEL *pChannel,
   IFX_TAPI_MAP_TYPE_t nSrcType, IFX_uint16_t nDstCh,
   IFX_TAPI_MAP_TYPE_t nDstType)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->CON.Module_Disconnect))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->CON.Module_Disconnect (pChannel->pLLChannel,
      nSrcType, nDstCh, nDstType);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* defined(TAPI_FEAT_VOICE) || defined(TAPI_FEAT_PCM) || defined(TAPI_FEAT_DECT) */


#ifdef TAPI_FEAT_VOICE
/**
   Add a connection between phone and any of the following modules:
   PCM, ALM, DECT.

   \param   pChannel Handle to TAPI channel structure.
   \param   pMap     Handle to the map descriptor

   \return
      TAPI_statusOk - success
*/
IFX_int32_t TAPI_Phone_MapAdd (TAPI_CHANNEL *pChannel,
   IFX_TAPI_MAP_PHONE_t const *pMap)
{
   return TAPI_MapAdd (pChannel, IFX_TAPI_MAP_TYPE_PHONE,
      pMap->nPhoneCh, pMap->nChType);
}
#endif /* TAPI_FEAT_VOICE */


#ifdef TAPI_FEAT_VOICE
/**
   Remove a connection between phone and any of the following modules:
   PCM, ALM, DECT.

   \param   pChannel Handle to TAPI channel structure.
   \param   pMap     Handle to the map descriptor

   \return
      TAPI_statusOk - success
*/
IFX_int32_t TAPI_Phone_MapRemove (TAPI_CHANNEL *pChannel,
   IFX_TAPI_MAP_PHONE_t const *pMap)
{
   return TAPI_MapRemove (pChannel, IFX_TAPI_MAP_TYPE_PHONE,
      pMap->nPhoneCh, pMap->nChType);
}
#endif /* TAPI_FEAT_VOICE */


#ifdef TAPI_FEAT_PCM
/**
   Add a connection between PCM and any of the following modules:
   PCM, ALM, DECT.

   \param   pChannel Handle to TAPI channel structure.
   \param   pMap     Handle to the map descriptor

   \return
      TAPI_statusOk - success
*/
IFX_int32_t TAPI_Phone_PCM_MapAdd (TAPI_CHANNEL *pChannel,
   IFX_TAPI_MAP_PCM_t const *pMap)
{
   return TAPI_MapAdd (pChannel, IFX_TAPI_MAP_TYPE_PCM,
      pMap->nDstCh, pMap->nChType);
}
#endif /* TAPI_FEAT_PCM */


#ifdef TAPI_FEAT_PCM
/**
   Remove a connection between PCM and any of the following modules:
   PCM, ALM, DECT.

   \param   pChannel Handle to TAPI channel structure.
   \param   pMap     Handle to the map descriptor

   \return
      TAPI_statusOk - success
*/
IFX_int32_t TAPI_Phone_PCM_MapRemove (TAPI_CHANNEL *pChannel,
   IFX_TAPI_MAP_PCM_t const *pMap)
{
   return TAPI_MapRemove (pChannel, IFX_TAPI_MAP_TYPE_PCM,
      pMap->nDstCh, pMap->nChType);
}
#endif /* TAPI_FEAT_PCM */


#ifdef TAPI_FEAT_DECT
/**
   Add a connection between DECT and any of the following modules:
   PCM, ALM, DECT.

   \param   pChannel Handle to TAPI channel structure.
   \param   pMap     Handle to the map descriptor

   \return
      TAPI_statusOk - success
*/
IFX_int32_t TAPI_Phone_DECT_MapAdd (TAPI_CHANNEL *pChannel,
   IFX_TAPI_MAP_DECT_t const *pMap)
{
   return TAPI_MapAdd (pChannel, IFX_TAPI_MAP_TYPE_DECT,
      pMap->nDstCh, pMap->nChType);
}
#endif /* TAPI_FEAT_DECT */


#ifdef TAPI_FEAT_DECT
/**
   Remove a connection between DECT and any of the following modules:
   PCM, ALM, DECT.

   \param   pChannel Handle to TAPI channel structure.
   \param   pMap     Handle to the map descriptor

   \return
      TAPI_statusOk - success
*/
IFX_int32_t TAPI_Phone_DECT_MapRemove (TAPI_CHANNEL *pChannel,
   IFX_TAPI_MAP_DECT_t const *pMap)
{
   return TAPI_MapRemove (pChannel, IFX_TAPI_MAP_TYPE_DECT,
      pMap->nDstCh, pMap->nChType);
}
#endif /* TAPI_FEAT_DECT */


#ifdef TAPI_FEAT_DECT
/** Configure the DECT channel.

   \param   pChannel Handle to TAPI channel structure.
   \param   pDect    Handle to \ref IFX_TAPI_DECT_CFG_t structure.

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
IFX_int32_t IFX_TAPI_DECT_Cfg_Set (TAPI_CHANNEL *pChannel,
   IFX_TAPI_DECT_CFG_t const *pDect)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->DECT.Ch_Cfg))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->DECT.Ch_Cfg(pChannel->pLLChannel,
      pDect->nEncDelay, pDect->nDecDelay);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_DECT */


#ifdef TAPI_FEAT_DECT
/** Selects DECT encoding and packetisation time.

   \param   pChannel Handle to TAPI channel structure.
   \param   pDect    Handle to \ref IFX_TAPI_DECT_ENC_CFG_t structure.

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
IFX_int32_t IFX_TAPI_DECT_Enc_Cfg_Set (TAPI_CHANNEL *pChannel,
   IFX_TAPI_DECT_ENC_CFG_t const *pDectEnc)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->DECT.ENC_Cfg))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->DECT.ENC_Cfg(pChannel->pLLChannel,
      pDectEnc->nEncType, pDectEnc->nFrameLen);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_DECT */


#ifdef TAPI_FEAT_DECT
/** Activates/decativates the DECT channel.

   \param   pChannel Handle to TAPI channel structure.
   \param   pDect    Handle to \ref IFX_TAPI_DECT_ACTIVATION_t structure.

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
IFX_int32_t IFX_TAPI_DECT_Activation_Set (
   TAPI_CHANNEL *pChannel,
   IFX_uint32_t nEnable)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->DECT.Enable))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->DECT.Enable (pChannel->pLLChannel,
      (nEnable == IFX_ENABLE) ? 1 : 0);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_DECT */


#ifdef TAPI_FEAT_VOICE
/**
   Sets the coder type and frame length for the DECT encoding path

   \param   pChannel Handle to the TAPI channel
   \param   pEncCfg  Handle to the encoder configuration

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
IFX_int32_t TAPI_COD_ENC_CFG_Set (TAPI_CHANNEL *pChannel,
   IFX_TAPI_ENC_CFG_SET_t const *pEncCfg)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->COD.ENC_Cfg))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->COD.ENC_Cfg (pChannel->pLLChannel, pEncCfg);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_VOICE */


#ifdef TAPI_FEAT_VOICE
/**
   Put encoder into hold or unhold state.

   \param   pChannel Handle to the TAPI channel
   \param   nOnHold  Hold state (IFX_ENABLE - hold, IFX_DISABLE - unhold)

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
IFX_int32_t TAPI_COD_ENC_Hold (TAPI_CHANNEL *pChannel, IFX_int32_t nOnHold)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->COD.ENC_Hold))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->COD.ENC_Hold (pChannel->pLLChannel,
      (IFX_operation_t)nOnHold);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_VOICE */


#ifdef TAPI_FEAT_BABYPHONE
/**
   Turns the room noise detection mode on

   \param   pChannel Handle to the TAPI channel
   \param   pDet     Handle to the room noise configuration

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
IFX_int32_t TAPI_COD_ENC_Room_Noise_Start (TAPI_CHANNEL *pChannel,
   IFX_TAPI_ENC_ROOM_NOISE_DETECT_t const *pDet)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->COD.ENC_RoomNoise))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->COD.ENC_RoomNoise (pChannel->pLLChannel,
      IFX_TRUE, pDet->nThreshold, pDet->nVoicePktCnt,
      pDet->nSilencePktCnt);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_BABYPHONE */


#ifdef TAPI_FEAT_BABYPHONE
/**
   Turns the room noise detection mode off

   \param   pChannel Handle to the TAPI channel

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
IFX_int32_t TAPI_COD_ENC_Room_Noise_Stop (TAPI_CHANNEL *pChannel)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->COD.ENC_RoomNoise))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->COD.ENC_RoomNoise (pChannel->pLLChannel,
      IFX_FALSE, 0, 0, 0);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_BABYPHONE */


#ifdef TAPI_FEAT_VOICE
/**
   Switches on/off HP filter of decoder path

   \param   pChannel Handle to the TAPI channel
   \param   nHp      IFX_FALSE to switch HP off, IFX_TRUE to switch HP on

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
IFX_int32_t TAPI_COD_DEC_HP_Set (TAPI_CHANNEL *pChannel, IFX_int32_t nHp)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->COD.DEC_HP_Set))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->COD.DEC_HP_Set (pChannel->pLLChannel,
      (IFX_boolean_t)nHp);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_VOICE */


#ifdef TAPI_FEAT_CID
/**
   Find the data channel which gets it's main input from a given module
   and which provides it's input to the given module.

    There can be more than one data channel connected to a module. So the
    function acts as an iterator with pTapiCh as input and output parameter.

    If no LL function is implemented this function reports a single straight
    connection between module and data channel.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.
   \param  nModType     Type of the module where to start search.
   \param  pTapiCh      On calling specifies which was the last channel found
                        by this function. For the first call use IFX_NULL.
                        For iteration calls use the last output.
                        Returns pointer to the TAPI channel the found module
                        belongs to or IFX_NULL if no module is connected.

   \return
   TAPI_statusOk if successful, otherwise error code
*/
IFX_int32_t IFX_TAPI_Module_Find_Connected_Data_Channel (
                                          TAPI_CHANNEL *pChannel,
                                          IFX_TAPI_MAP_TYPE_t nModType,
                                          TAPI_CHANNEL **pTapiCh)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = TAPI_statusOk;

   if (IFX_TAPI_PtrChk (pDrvCtx->CON.Module_Find_Connected_Data_Channel))
   {
      /* Pass parameters to LL implementation. */
      ret = pDrvCtx->CON.Module_Find_Connected_Data_Channel(
                                       pChannel->pLLChannel, nModType, pTapiCh);
   }
   else
   {
      /* Report a single straight connection between module and data channel. */
      if (*pTapiCh == IFX_NULL)
      {
         *pTapiCh = pChannel;
      }
      else
      {
         *pTapiCh = IFX_NULL;
      }
   }

   return ret;
}
#endif /* TAPI_FEAT_CID */


#ifdef TAPI_FEAT_AMR
/**
   Get the AMR codec specific parameters.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.
   \param  pTapiAMR     Pointer to IFX_TAPI_COD_AMR_t structure.

   \return
   TAPI_statusOk if successful, otherwise error code
*/
IFX_int32_t IFX_TAPI_COD_AMR_Get(TAPI_CHANNEL *pChannel,
                                 IFX_TAPI_COD_AMR_t *pTapiAMR)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->COD.AMR_Get))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->COD.AMR_Get (pChannel->pLLChannel, pTapiAMR);

   if (!TAPI_SUCCESS(retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_AMR */
