/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

 ****************************************************************************
   \file  drv_vmmc_event.c

   \remarks
      Handling of driver, firmware and device specific events.
 *******************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_api.h"

/* ============================= */
/* Local Macros  Definitions    */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Local function declaration    */
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


static IFX_void_t stripPathCpy (char* dst, const char* src)
{
   IFX_uint32_t nMax = strlen (src);
   IFX_uint32_t i = nMax;

   /* only name of file without path. \ and / */
   while (i > 0 && src[i] != 0x5C && src[i] != 0x2F)
   {
      i--;
   }
   if (i > 0)
   {
      nMax = nMax - i;
      /* ignore separator */
      i++;
   }
   if (nMax > (IFX_TAPI_MAX_FILENAME - 1))
   {
      nMax = IFX_TAPI_MAX_FILENAME - 2;
      dst[IFX_TAPI_MAX_FILENAME - 1] = 0;
   }
   memcpy (dst, &src[i], nMax+1);
}

/**
   Report a channel error to TAPI.

   \param  pCh          Pointer to VMMC channel.
   \param  err          Error number.
   \param  nLine        Line in source code where the error occured.
   \param  sFile        String with source code filename where the error occured.
   \param  info         Pointer to additional data to be logged.
   \param  len          Length of additional data to be logged.
*/
IFX_void_t VMMC_ChErrorEvent (VMMC_CHANNEL *pCh, IFX_uint16_t err,
                              IFX_uint32_t nLine, const IFX_char_t *sFile,
                              IFX_void_t* info, IFX_uint32_t len)
{
   IFX_TAPI_ErrorLine_t errLine;
   IFX_TAPI_EVENT_t tapiEvent;
   /* Get tapi channel context. */
   TAPI_CHANNEL *pChannel = (TAPI_CHANNEL *)pCh->pTapiCh;

   /* Fill event structure. */
   memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
   errLine.nLlCode = err;
   errLine.nHlCode = 0;
   errLine.nLine = nLine;
   if (info != IFX_NULL && len != 0)
   {
      if (len > sizeof(errLine.msg))
      {
         len = sizeof(errLine.msg);
         /* Warning: less data than given was copied into the report */
      }
      memcpy (errLine.msg, info, len);
   }
   stripPathCpy (errLine.sFile, sFile);
   tapiEvent.id = IFX_TAPI_EVENT_FAULT_GENERAL_CHINFO;
   tapiEvent.data.error = &errLine;

   IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);
}


/**
   Report a device error to TAPI.

   \param  pDev         Pointer to VMMC device.
   \param  err          Error number.
   \param  nLine        Line in source code where the error occured.
   \param  sFile        String with source code filename where the error occured.
   \param  info         Pointer to additional data to be logged.
   \param  len          Length of additional data to be logged.
*/
IFX_void_t VMMC_DevErrorEvent (VMMC_DEVICE *pDev, IFX_uint16_t err,
                               IFX_uint32_t nLine, const IFX_char_t* sFile,
                               IFX_void_t* info, IFX_uint32_t len)
{
   IFX_TAPI_ErrorLine_t errLine;
   IFX_TAPI_EVENT_t tapiEvent;

   /* Fill event structure. */
   memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
   errLine.nHlCode = 0;
   errLine.nLlCode = err;
   errLine.nLine = nLine;
   if (info != IFX_NULL && len != 0)
   {
      if (len > sizeof(errLine.msg))
      {
         len = sizeof(errLine.msg);
         /* Warning: less data than given was copied into the report */
      }
      memcpy (errLine.msg, info, len);
   }
   stripPathCpy (errLine.sFile, sFile);
   tapiEvent.id = IFX_TAPI_EVENT_FAULT_GENERAL_DEVINFO;
   tapiEvent.data.error = &errLine;

   IFX_TAPI_Event_Dispatch(pDev->pChannel[0].pTapiCh, &tapiEvent);
}
