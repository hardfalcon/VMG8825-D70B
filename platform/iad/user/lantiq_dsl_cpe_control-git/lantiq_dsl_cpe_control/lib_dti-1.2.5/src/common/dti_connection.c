/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Connection handling.
*/

/* ============================================================================
   Includes
   ========================================================================= */
#include "dti_osmap.h"
#include "ifxos_socket.h"

#include "dti_connection_interface.h"

/* ============================================================================
   Defines
   ========================================================================= */
#ifdef DTI_STATIC
#  undef DTI_STATIC
#endif

#ifdef DTI_DEBUG
#  define DTI_STATIC
#else
#  define DTI_STATIC   static
#endif

/* sockets */
#define DTI_SocketInit              IFXOS_SocketInit
#define DTI_SocketCleanup           (void)IFXOS_SocketCleanup
#define DTI_SocketCreate            IFXOS_SocketCreate
#define DTI_SocketConnect           IFXOS_SocketConnect

#if ( defined(IFXOS_HAVE_SOCKET_SHUTDOWN) && (IFXOS_HAVE_SOCKET_SHUTDOWN == 1) )
#  define DTI_SocketClose(socFd) \
                        (void)IFXOS_SocketShutdown(socFd, IFXOS_SOCKET_SHUTDOWN_RD); \
                        (void)IFXOS_SocketClose(socFd); \
                        socFd = -1
#else
#  define DTI_SocketClose(socFd) \
                        (void)IFXOS_SocketClose(socFd); \
                        socFd = -1

#endif

#define DTI_SocketAccept            IFXOS_SocketAccept
#define DTI_SocketRecv              IFXOS_SocketRecv
#define DTI_SocketSend              IFXOS_SocketSend
#define DTI_SocketBind              IFXOS_SocketBind
#define DTI_SocketListen            IFXOS_SocketListen
#define DTI_SocketSelect            IFXOS_SocketSelect
#define DTI_SocketNtoa              IFXOS_SocketNtoa
#define DTI_SOC_ADDR_PORT_SET       IFXOS_SOC_ADDR_PORT_SET
#define DTI_SOC_ADDR_SET            IFXOS_SOC_ADDR_SET
#define DTI_SocFdZero               IFXOS_SocFdZero
#define DTI_SocFdSet                IFXOS_SocFdSet
#define DTI_SocFdClr                IFXOS_SocFdClr
#define DTI_SocFdIsSet              IFXOS_SocFdIsSet
#define DTI_SocketAton              (void)IFXOS_SocketAton

#define DTI_SOC_TYPE_STREAM         IFXOS_SOC_TYPE_STREAM
#define DTI_SOC_AF_INET             IFXOS_SOC_AF_INET
#define DTI_SOC_WAIT_FOREVER        IFXOS_SOC_WAIT_FOREVER



/* ============================================================================
   DTI Packet Handling - Local Function Declaration
   ========================================================================= */

DTI_STATIC DTI_PacketError_t DTI_packetSwap (
                        DTI_Packet_t   *pDtiPacket,
                        IFX_boolean_t  bCurrOrder_net,
                        IFX_boolean_t  bSwapPayload);


/* ============================================================================
   Variables
   ========================================================================= */

IFXOS_PRN_USR_MODULE_CREATE(DTI_CON, IFXOS_PRN_LEVEL_HIGH);

/* ============================================================================
   Connection Handling - Local Function defintions
   ========================================================================= */


/* ============================================================================
   Connection Handling - Global Function defintions
   ========================================================================= */

/**
   Init the connection specific parts (OS specific).

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_conInit(void)
{
   if (DTI_SocketInit() != IFX_SUCCESS)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: DTI Connection Init - failed."IFXOS_CRLF));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/**
   Cleanup the connection specific parts (OS specific).

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_conCleanup(void)
{

   DTI_SocketCleanup();

   return IFX_SUCCESS;
}

/**
   Reset / init connection control structure.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_void_t DTI_conStructInit(
                        DTI_Connection_t  *pDtiCon)
{
   DTI_MemSet(pDtiCon, 0x0, sizeof(DTI_Connection_t));
   pDtiCon->nFd = -1;
   return;
}

/**
   Reset / init a connection control structure.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_void_t DTI_conCntrlStructInit(
                        DTI_ConnectionCntrl_t *pDtiConCntrl)
{
   DTI_MemSet(pDtiConCntrl, 0x0, sizeof(DTI_ConnectionCntrl_t));

   return;
}

/**
   Setup / takeover connection data.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_void_t DTI_conConSet(
                        DTI_Connection_t        *pDtiCon,
                        DTI_Connection_t        *pNewDtiCon)
{
   if (pNewDtiCon)
   {
      pDtiCon->nFd = pNewDtiCon->nFd;
      DTI_MemCpy(&pDtiCon->sockAddr, &pNewDtiCon->sockAddr, sizeof(IFXOS_sockAddr_t));
   }
   else
   {
      DTI_conStructInit(pDtiCon);
   }
   return;
}

/**
   Setup connection control data.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_void_t DTI_conCntrlConSet(
                        DTI_Connection_t        *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl)
{
   DTI_SocketNtoa(&pDtiCon->sockAddr, pDtiConCntrl->ipAddr);

   return;
}

/**
   Establish/Setup a Client/Server DTI connection

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_conOpen(
                        DTI_Connection_t  *pDtiCon,
                        IFX_boolean_t     bClient,
                        IFX_uint16_t      ipPort,
                        IFX_char_t        *pIpStr)
{
   IFX_int_t retVal = IFX_SUCCESS;

   if ( (pDtiCon == IFX_NULL) ||
        ((bClient == IFX_TRUE) && (pIpStr == IFX_NULL)) )
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: DTI Connection Open - missing args, <%s>."IFXOS_CRLF,
           (pDtiCon == IFX_NULL) ? "context" : "ipStr"));

      return IFX_ERROR;
   }
   else
   {
      if ((bClient == IFX_TRUE) && (DTI_StrLen(pIpStr) == 0))
      {
         IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
            ("Error: DTI Connection Open - missing args, <ipAddr>."IFXOS_CRLF));

         return IFX_ERROR;
      }
   }

   DTI_MemSet(&pDtiCon->sockAddr, 0x00, sizeof(IFXOS_sockAddr_t));

   if (ipPort)
   {
      IFXOS_SOC_ADDR_PORT_SET(&pDtiCon->sockAddr, DTI_htons(ipPort));
   }
   else
   {
      IFXOS_SOC_ADDR_PORT_SET(&pDtiCon->sockAddr, DTI_htons(DTI_TCP_PORT));
   }

#if (defined(IFXOS_VERSION_CHECK_EG_THAN) && IFXOS_VERSION_CHECK_EG_THAN(1, 1, 2))
   IFXOS_SOC_ADDR_FAMILY_SET(&pDtiCon->sockAddr, IFXOS_SOC_AF_INET);
#else
   pDtiCon->sockAddr.sin_family = IFXOS_SOC_AF_INET;
#endif

   if ( (pIpStr) && (DTI_StrLen(pIpStr)) )
   {
      DTI_SocketAton(pIpStr, &pDtiCon->sockAddr);

      IFXOS_PRN_USR_DBG_NL(DTI_CON, IFXOS_PRN_LEVEL_HIGH,
         ("DTI Connection Open - %s %s:%d."IFXOS_CRLF,
         (bClient == IFX_TRUE) ? "CLIENT" : "SERVER", pIpStr, ipPort ));
   }
   else
   {
      IFXOS_SOC_ADDR_SET(&pDtiCon->sockAddr, DTI_htonl(IFXOS_SOC_INADDR_ANY));

      IFXOS_PRN_USR_DBG_NL(DTI_CON, IFXOS_PRN_LEVEL_HIGH,
         ("DTI Connection Open - %s %s:%d."IFXOS_CRLF,
           (bClient == IFX_TRUE) ? "CLIENT" : "SERVER",
           "<local ip>", ipPort ));
   }

   retVal = DTI_SocketCreate(IFXOS_SOC_TYPE_STREAM, &pDtiCon->nFd);
   if (retVal != IFX_SUCCESS)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: DTI Connection Open - create socket."IFXOS_CRLF));

      return IFX_ERROR;
   }

   if (bClient == IFX_TRUE)
   {
      /* CLIENT: do connect */
      if (DTI_SocketConnect(
                     pDtiCon->nFd,
                     &pDtiCon->sockAddr,
                     sizeof(IFXOS_sockAddr_t)) != IFX_SUCCESS)
      {
         (void)DTI_conClose(pDtiCon);

         IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
            ("Error: DTI Connection Open - CLIENT connect remote %s:%d."IFXOS_CRLF,
              pIpStr, ipPort));

         return IFX_ERROR;
      }
   }
   else
   {
      /* SERVER: setup listener port */
      if (DTI_SocketBind(
                     pDtiCon->nFd,
                     &pDtiCon->sockAddr) != IFX_SUCCESS)
      {
         (void)DTI_conClose(pDtiCon);

         IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
            ("Error: DTI Connection Open - SERVER bind %s:%d."IFXOS_CRLF,
              (pIpStr) ? pIpStr : "<local ip>", ipPort));

         return IFX_ERROR;
      }

      if (DTI_SocketListen(
                     pDtiCon->nFd, 1) != IFX_SUCCESS)
      {
         (void)DTI_conClose(pDtiCon);

         IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
            ("Error: DTI Connection Open - SERVER listen %s:%d."IFXOS_CRLF,
              (pIpStr) ? pIpStr : "<local ip>", ipPort));

         return IFX_ERROR;
      }
   }

   return IFX_SUCCESS;
}


/**
   Close DTI connection

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_conClose(
                        DTI_Connection_t  *pDtiCon)
{
   if (pDtiCon == IFX_NULL)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: DTI Connection Close - missing context."IFXOS_CRLF));

      return IFX_ERROR;
   }

   if (pDtiCon->nFd >= 0)
   {
      DTI_SocketClose(pDtiCon->nFd);
   }

   return IFX_SUCCESS;
}

/**
   Add the given connection to the control structure for receive data.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_void_t DTI_conAddForRecvWait(
                        const DTI_Connection_t  *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl)
{
   DTI_SocFdSet(pDtiCon->nFd, &pDtiConCntrl->rdFdSet);

   if ((IFX_int_t)pDtiCon->nFd > pDtiConCntrl->socFdMax)
   {
      pDtiConCntrl->socFdMax = (IFX_int_t)pDtiCon->nFd + 1;
   }

   return;
}

/**
   Check if new data are available.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_conDataRecvCheck(
                        DTI_ConnectionCntrl_t *pDtiConCntrl,
                        IFX_int_t             waitTime_ms)
{
   IFX_int_t   retVal = 0;

   DTI_MemCpy(&pDtiConCntrl->tmpFdSet, &pDtiConCntrl->rdFdSet, sizeof(IFXOS_socFd_set_t));

   retVal = DTI_SocketSelect(
                  pDtiConCntrl->socFdMax,
                  &pDtiConCntrl->tmpFdSet, IFX_NULL, IFX_NULL,
                  (waitTime_ms == -1) ? (IFXOS_SOC_WAIT_FOREVER) :
                                        ((waitTime_ms == 0) ? (IFXOS_SOC_NO_WAIT) : waitTime_ms) );

   if (retVal < 0)
   {
      DTI_PRN_USR_ERR_NL(DTI_CON, DTI_PRN_LEVEL_ERR,
         ("ERROR: data revc check - select error = %d."DTI_CRLF, retVal));

      return IFX_ERROR;
   }

   return retVal;
}

/**
   Check if new data are available for the given connection.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_conDataRecvMyConCheck(
                        const DTI_Connection_t  *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl)
{
   if (DTI_SocFdIsSet(pDtiCon->nFd, &pDtiConCntrl->tmpFdSet) != 0)
   {
      return 1;
   }

   return 0;
}


/**
   Wait for a new connection and accept it.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_conNewConnectGet(
                        const DTI_Connection_t  *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl,
                        IFX_int_t               waitTime_ms,
                        DTI_Connection_t        *pDtiConNew)
{
   IFX_int_t   retVal = 0;
   IFX_char_t  addrStr[IFXOS_SOC_ADDR_LEN_BYTE] = {0};

   DTI_MemCpy(&pDtiConCntrl->tmpFdSet, &pDtiConCntrl->rdFdSet, sizeof(IFXOS_socFd_set_t));

   retVal = DTI_conDataRecvCheck(pDtiConCntrl, waitTime_ms);

   if (retVal > 0)
   {
      if (DTI_conDataRecvMyConCheck(pDtiCon, pDtiConCntrl) == 1)
      {
         pDtiConNew->nFd = DTI_SocketAccept(pDtiCon->nFd, &pDtiConNew->sockAddr);
         if (pDtiConNew->nFd < 0)
         {
            DTI_PRN_USR_ERR_NL(DTI_CON, DTI_PRN_LEVEL_ERR,
               ("WARNING: new connection get - accept, discard connection."DTI_CRLF));

            return IFX_ERROR;
         }
         DTI_SocketNtoa(&pDtiConNew->sockAddr, addrStr);
         DTI_PRN_USR_DBG_NL(DTI_CON, DTI_PRN_LEVEL_HIGH,
            ("new connection get - connection from %s, sock = %d."DTI_CRLF,
            addrStr, pDtiConNew->nFd));

         return 1;
      }
   }
   else
   {
      if (retVal == 0)
      {
         return 0;
      }
   }

   return retVal;
}

/**
   Read data from a connection.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_conDataRead(
                        const DTI_Connection_t  *pDtiCon,
                        IFX_boolean_t           bBlocking,
                        IFX_int_t               nReadBytes,
                        IFX_char_t              *pBufIn)
{
   IFX_int_t   recvBytes = 0, totalBytes = 0, remainBytes = nReadBytes;
   IFX_char_t  *pCurrBufIn = IFX_NULL;

   while (remainBytes > 0)
   {
      pCurrBufIn = &pBufIn[totalBytes];

      /* read remaining bytes */
      recvBytes = DTI_SocketRecv(pDtiCon->nFd, pCurrBufIn, remainBytes);
      if (recvBytes > 0)
      {
         totalBytes  += recvBytes;
         remainBytes -= recvBytes;
      }
      else
      {
         if ((recvBytes == 0) || (recvBytes == -1))
         {
            DTI_PRN_USR_ERR_NL(DTI_CON, DTI_PRN_LEVEL_ERR,
               ("WARNING: data recv - sock read, connection close."DTI_CRLF));

            return DTI_CONNECTION_CLOSED;
         }
         else
         {
            DTI_PRN_USR_ERR_NL(DTI_CON, DTI_PRN_LEVEL_ERR,
               ("ERROR: data recv - sock read, ret = %d (requ = %d, done = %d)."DTI_CRLF,
                 recvBytes, nReadBytes, (nReadBytes - remainBytes)));

            return IFX_ERROR;
         }
      }

      if (bBlocking == IFX_FALSE)
      {
         break;
      }
   }

   return totalBytes;
}

/**
   Check for and read data from a connection

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_conDataCheckAndRecv(
                        const DTI_Connection_t  *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl,
                        IFX_boolean_t           bBlocking,
                        IFX_int_t               waitTime_ms,
                        IFX_int_t               nReadBytes,
                        IFX_char_t              *pBufIn)
{
   IFX_int_t   retVal = 0, totalBytes = 0, remainBytes = nReadBytes;
   IFX_char_t  *pCurrBufIn = IFX_NULL;

   retVal = DTI_conDataRecvCheck(pDtiConCntrl, waitTime_ms);
   if (retVal > 0)
   {
      if (DTI_conDataRecvMyConCheck(pDtiCon, pDtiConCntrl) == 1)
      {
         /* read remaining bytes */
         pCurrBufIn = &pBufIn[totalBytes];
         retVal = DTI_conDataRead(pDtiCon, IFX_FALSE, remainBytes, pCurrBufIn);
         if (retVal > 0)
         {
            totalBytes  += retVal;
            remainBytes -= retVal;
         }
         else
         {
            return retVal;
         }

         while( (bBlocking == IFX_TRUE) && (remainBytes > 0) )
         {
            retVal = DTI_conDataRecvCheck(pDtiConCntrl, waitTime_ms);
            if (retVal > 0)
            {
               if (DTI_conDataRecvMyConCheck(pDtiCon, pDtiConCntrl) == 1)
               {
                  /* read remaining bytes */
                  pCurrBufIn = &pBufIn[totalBytes];
                  retVal = DTI_conDataRead(pDtiCon, IFX_FALSE, remainBytes, pCurrBufIn);
                  if (retVal > 0)
                  {
                     totalBytes  += retVal;
                     remainBytes -= retVal;
                  }
                  else
                  {
                     return retVal;
                  }
               }
            }
         }        /* while( (bBlocking == IFX_TRUE) && (remainBytes > 0) ) */
      }
   }

   return totalBytes;
}

/**
   Send data to a connection.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_conDataSend(
                        const DTI_Connection_t  *pDtiCon,
                        IFX_int_t               nSendBytes,
                        IFX_char_t              *pBufOut)
{
   IFX_int_t   retVal;

   if ( (retVal = IFXOS_SocketSend(pDtiCon->nFd, pBufOut, nSendBytes)) != nSendBytes)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: data send - Socket send, ret = %d (0x%X)."IFXOS_CRLF, retVal, retVal));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}



/* ============================================================================
   DTI Packet Handling - Local Function defintions
   ========================================================================= */

/**
   Transform and check the DTI packet from host to net and net to host order.

\param
   pDtiPacket     - points to the DTI packet.
\param
   bCurrOrder_net - if set the current order is "net order"
\param
   bSwapPayload   - if set, swap also the payload.

\return
   IFX_SUCCESS if swap done.
   IFX_ERROR   if something was wrong.
*/
DTI_STATIC DTI_PacketError_t DTI_packetSwap (
                        DTI_Packet_t   *pDtiPacket,
                        IFX_boolean_t  bCurrOrder_net,
                        IFX_boolean_t  bSwapPayload)
{
   IFX_boolean_t        bSwapPayloadIntern = IFX_TRUE;
   DTI_PacketError_t    retVal  = DTI_eNoError;
   DTI_PacketHeader_t   *pHeader;
   DTI_PTR_U            uPayload;

   if ( (pDtiPacket == IFX_NULL) )
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Swap - NULL ptr args."IFXOS_CRLF));

      return DTI_eErrUnknown;
   }

   pHeader = &pDtiPacket->header;

   if (bCurrOrder_net == IFX_TRUE)
   {
      /* packet is in net order --> at first swap DTI header */
      pHeader->magic          = (IFX_uint32_t)DTI_ntohl(pHeader->magic);
      pHeader->packetType     = (IFX_uint32_t)DTI_ntohl(pHeader->packetType);
      pHeader->packetOptions  = (IFX_uint32_t)DTI_ntohl(pHeader->packetOptions);
      pHeader->port           = (IFX_uint32_t)DTI_ntohl(pHeader->port);
      pHeader->tan            = (IFX_uint32_t)DTI_ntohl(pHeader->tan);
      pHeader->error          = (IFX_uint32_t)DTI_ntohl(pHeader->error);
      pHeader->payloadSize    = (IFX_uint32_t)DTI_ntohl(pHeader->payloadSize);
   }

   /*
      some checks
   */
   if (pHeader->magic != DTI_MAGIC)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Swap - invalid magic."IFXOS_CRLF));

      bSwapPayloadIntern = IFX_FALSE;
      retVal             = DTI_eErrMalformedPacket;
   }
   else
   {
      switch(pHeader->packetOptions & DTI_HDR_OPTION_MASK_PAYL_TYPE)
      {
         case DTI_eMixed:
         case DTI_e8Bit:
            bSwapPayloadIntern = IFX_FALSE;
            break;
         case DTI_e16Bit:
            if (pHeader->payloadSize % 2)
            {
               IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
                  ("Error: Packet Swap - missaligned data (DTI_e16Bit)."IFXOS_CRLF));

               bSwapPayloadIntern = IFX_FALSE;
               retVal       = DTI_eErrMalformedPacket;
            }
            break;
         case DTI_e32Bit:
            if (pHeader->payloadSize % 4)
            {
               IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
                  ("Error: Packet Swap - missaligned data (DTI_e32Bit)."IFXOS_CRLF));

               bSwapPayloadIntern = IFX_FALSE;
               retVal       = DTI_eErrMalformedPacket;
            }
            break;

         default:
            IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
               ("Error: Packet Swap - unknown type (%d)."IFXOS_CRLF,
                 pHeader->packetType));

            bSwapPayloadIntern = IFX_FALSE;
            retVal       = DTI_eErrMalformedPacket;
      }

      if (pHeader->payloadSize == 0)
      {
         bSwapPayloadIntern = IFX_FALSE;
      }
   }

   /*
      Swap payload
   */
   if ((bSwapPayloadIntern == IFX_TRUE) && (bSwapPayload == IFX_TRUE))
   {
      IFX_int_t    i;

      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacket->payload;

      switch(pHeader->packetOptions & DTI_HDR_OPTION_MASK_PAYL_TYPE)
      {
         case DTI_e16Bit:
            {
               IFX_uint16_t *pPayload_16 = (IFX_uint16_t *)DTI_PTR_CAST_GET_UINT16(uPayload);

               if (pPayload_16 == IFX_NULL)
               {

                  IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
                     ("Error: Packet Swap - data16 missaligned."IFXOS_CRLF));

                  retVal = DTI_eErrMalformedPacket;
               }
               else
               {
                  if (bCurrOrder_net == IFX_TRUE)
                  {
                     for (i = 0; i < (IFX_int_t)(pHeader->payloadSize / 2); i++)
                     {
                        pPayload_16[i] = (IFX_uint16_t)DTI_ntohs(pPayload_16[i]);
                     }
                  }
                  else
                  {
                     for (i = 0; i < (IFX_int_t)(pHeader->payloadSize / 2); i++)
                     {
                        pPayload_16[i] = (IFX_uint16_t)DTI_htons(pPayload_16[i]);
                     }
                  }
               }
            }
            break;
         case DTI_e32Bit:
            {
               IFX_uint32_t *pPayload_32 = (IFX_uint32_t *)DTI_PTR_CAST_GET_UINT32(uPayload);

               if (pPayload_32 == IFX_NULL)
               {

                  IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
                     ("Error: Packet Swap - data32 missaligned."IFXOS_CRLF));

                  retVal = DTI_eErrMalformedPacket;
               }
               else
               {
                  if (bCurrOrder_net == IFX_TRUE)
                  {
                     for (i = 0; i < (IFX_int_t)(pHeader->payloadSize / 4); i++)
                     {
                        pPayload_32[i] = (IFX_uint32_t)DTI_ntohl(pPayload_32[i]);
                     }
                  }
                  else
                  {
                     for (i = 0; i < (IFX_int_t)(pHeader->payloadSize / 4); i++)
                     {
                        pPayload_32[i] = (IFX_uint32_t)DTI_htonl(pPayload_32[i]);
                     }
                  }
               }
            }
            break;

         case DTI_eMixed:
         case DTI_e8Bit:
         default:
            break;
      }
   }


   if (bCurrOrder_net != IFX_TRUE)
   {
      /* packet is in host order --> final swap header */
      pHeader->magic          = (IFX_uint32_t)DTI_htonl(pHeader->magic);
      pHeader->packetType     = (IFX_uint32_t)DTI_htonl(pHeader->packetType);
      pHeader->packetOptions  = (IFX_uint32_t)DTI_htonl(pHeader->packetOptions);
      pHeader->port           = (IFX_uint32_t)DTI_htonl(pHeader->port);
      pHeader->tan            = (IFX_uint32_t)DTI_htonl(pHeader->tan);
      pHeader->error          = (IFX_uint32_t)DTI_htonl(pHeader->error);
      pHeader->payloadSize    = (IFX_uint32_t)DTI_htonl(pHeader->payloadSize);
   }

   return retVal;
}

/* ============================================================================
   DTI Packet Handling - functions
   ========================================================================= */

/**
   Resynchronisation - Read and search for the "magic" key

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_packetReadResync (
                        DTI_Connection_t        *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl,
                        IFX_boolean_t           *pBResync,
                        IFX_char_t              *pBufIn,
                        IFX_uint32_t            magicNum,
                        IFX_int_t               waitTime_ms)
{
   IFX_int_t      tryCnt = 5, nRemainBytes, nCurrentReadBytes;
   IFX_uint32_t   localMagic = DTI_htonl(magicNum);
   DTI_PTR_U            uPacketHdr;
   DTI_PacketHeader_t   *pPacketHdr = IFX_NULL;

   uPacketHdr.pUInt8 = (IFX_uint8_t *)pBufIn;
   pPacketHdr = (DTI_PacketHeader_t *)DTI_PTR_CAST_GET_ULONG(uPacketHdr);
   if (pPacketHdr == IFX_NULL )
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Read Resync - struct missaligned."IFXOS_CRLF));

      return DTI_CON_RECV_STATUS_ERROR;
   }

   nRemainBytes = sizeof(IFX_uint32_t);

   while (nRemainBytes && tryCnt > 0)
   {
      nCurrentReadBytes = DTI_conDataCheckAndRecv(
                              pDtiCon, pDtiConCntrl, IFX_FALSE,
                              waitTime_ms, 1, &pBufIn[sizeof(IFX_uint32_t) - nRemainBytes]);

      if (nCurrentReadBytes < 0)
      {
         return (nCurrentReadBytes == DTI_CONNECTION_CLOSED) ?
                     DTI_CON_RECV_STATUS_CON_CLOSED : DTI_CON_RECV_STATUS_ERROR;
      }

      if (nCurrentReadBytes == 1)
      {
         tryCnt = 5;
         nRemainBytes--;

         switch(nRemainBytes)
         {
            case 3:
               if ((localMagic & 0xFF) != (pPacketHdr->magic & 0xFF))
               {
                  nRemainBytes = sizeof(IFX_uint32_t);
               }
               break;

            case 2:
               if ((localMagic & 0xFFFF) != (pPacketHdr->magic & 0xFFFF))
               {
                  nRemainBytes = sizeof(IFX_uint32_t);
               }
               break;

            case 1:
               if ((localMagic & 0xFFFFFF) != (pPacketHdr->magic & 0xFFFFFF))
               {
                  nRemainBytes = sizeof(IFX_uint32_t);
               }
               break;

            case 0:
               if ((localMagic & 0xFFFFFFFF) != (pPacketHdr->magic & 0xFFFFFFFF))
               {
                  nRemainBytes = sizeof(IFX_uint32_t);
               }
               break;

            default:
               break;
         }
      }
      else
      {
         tryCnt--;
      }
   }

   if (nRemainBytes == 0)
   {
      /* magic number found - read the rest of the header */
      nCurrentReadBytes = DTI_conDataCheckAndRecv(
                              pDtiCon, pDtiConCntrl, IFX_FALSE,
                              waitTime_ms,
                              sizeof(DTI_PacketHeader_t) - sizeof(IFX_uint32_t),
                              &pBufIn[sizeof(IFX_uint32_t)]);

      if (nCurrentReadBytes < 0)
      {
         return (nCurrentReadBytes == DTI_CONNECTION_CLOSED) ?
                     DTI_CON_RECV_STATUS_CON_CLOSED : DTI_CON_RECV_STATUS_ERROR;
      }
      else
      {
         if (pBResync)
         {
            *pBResync = IFX_FALSE;
         }

         return (sizeof(IFX_uint32_t) + nCurrentReadBytes);
      }
   }

   return DTI_CON_RECV_STATUS_STREAM_RESYNC;
}

/**
   Read and swap a DTI packet from the TCP connection and setup
   the DTI packet structure.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_packetRead (
                        DTI_Connection_t        *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl,
                        IFX_boolean_t           *pBResync,
                        IFX_int_t               waitTime_ms,
                        IFX_int_t               nBufInLen,
                        IFX_char_t              *pBufIn,
                        IFX_int_t               *pCurrRecvBytes,
                        DTI_PacketError_t       *pPacketError,
                        DTI_Packet_t            **ppPacket)
{
   IFX_int_t         retVal, nRemainBytes, nBytesRead, nTotalRecvBytes;

   DTI_PacketError_t packetError = DTI_eNoError;
   DTI_Packet_t      *pDtiPacket = IFX_NULL;
   DTI_PTR_U         uPacket;

   if ( (pDtiCon == IFX_NULL) || (pDtiConCntrl == IFX_NULL) )
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Read - NULL ptr args."IFXOS_CRLF));

      if (pCurrRecvBytes)
      {
         *pCurrRecvBytes  = 0;
      }
      return DTI_CON_RECV_STATUS_ERROR;
   }

   if ( (pBufIn == IFX_NULL) || (pCurrRecvBytes == IFX_NULL) )
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Read - NULL ptr data args."IFXOS_CRLF));

      if (pCurrRecvBytes)
      {
         *pCurrRecvBytes  = 0;
      }
      return DTI_CON_RECV_STATUS_ERROR;
   }

   if (pBResync)
   {
      if (*pBResync == IFX_TRUE)
      {
         *pCurrRecvBytes = 0;
         retVal = DTI_packetReadResync(
                     pDtiCon, pDtiConCntrl, pBResync, pBufIn, DTI_MAGIC, waitTime_ms);
         if (retVal < 0)
         {
            return retVal;
         }
         else if( waitTime_ms >= 0 && retVal == 0)
         {
            return DTI_CON_RECV_STATUS_TIMEOUT;
         }
         else
         {
            *pCurrRecvBytes = retVal;
         }
      }
   }

   retVal = DTI_conDataRecvCheck(pDtiConCntrl, waitTime_ms);
   if (retVal <= 0)
   {
      if (retVal == 0)
      {
         /* nothing received or timeout is occurred */
         if( waitTime_ms >= 0 )
         {
            return DTI_CON_RECV_STATUS_TIMEOUT;
         }
         else
         {
            return DTI_CON_RECV_STATUS_PENDING;
         }
      }
      else
      {
         IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
            ("Error: Packet Read - data recv check."IFXOS_CRLF));

         *pCurrRecvBytes  = 0;
         return DTI_CON_RECV_STATUS_ERROR;
      }
   }

   if (DTI_conDataRecvMyConCheck(pDtiCon, pDtiConCntrl) == 0)
   {
      /* nothing received */
      return DTI_CON_RECV_STATUS_PENDING;
   }


   /*
      Data available for this connection.
   */
   uPacket.pUInt8 = (IFX_uint8_t *)pBufIn;
   pDtiPacket = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPacket);
   if (pDtiPacket == IFX_NULL )
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Recv - struct missaligned."IFXOS_CRLF));

      *pCurrRecvBytes  = 0;
      return DTI_CON_RECV_STATUS_ERROR;
   }

   nTotalRecvBytes = *pCurrRecvBytes;

   if (nTotalRecvBytes < sizeof(DTI_PacketHeader_t))
   {
      nRemainBytes = (IFX_int_t)sizeof(DTI_PacketHeader_t) - nTotalRecvBytes;
      nBytesRead = DTI_conDataRead(
                           pDtiCon, IFX_FALSE,
                           nRemainBytes, &pBufIn[nTotalRecvBytes]);
      if (nBytesRead < 0)
      {
         /* error read */
         *pCurrRecvBytes  = 0;
         return (nBytesRead == DTI_CONNECTION_CLOSED) ?
                     DTI_CON_RECV_STATUS_CON_CLOSED : DTI_CON_RECV_STATUS_ERROR;
      }

      nTotalRecvBytes += nBytesRead;
      nRemainBytes    -= nBytesRead;
   }

   if (nTotalRecvBytes >= sizeof(DTI_PacketHeader_t))
   {
      if (DTI_ntohl(pDtiPacket->header.magic) != DTI_MAGIC)
      {
         IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
            ("ERROR: Packet Recv - Header invalid MAGIC."IFXOS_CRLF));

         *pCurrRecvBytes  = 0;
         if (pBResync)
         {
            *pBResync = IFX_TRUE;
         }

         return DTI_CON_RECV_STATUS_STREAM_RESYNC;
      }
      else
      {
         nRemainBytes = (  (IFX_int_t)sizeof(DTI_PacketHeader_t) +
                         + (IFX_int_t)DTI_ntohl(pDtiPacket->header.payloadSize))
                        - nTotalRecvBytes;

         if ( (nRemainBytes + nTotalRecvBytes) > nBufInLen)
         {
            IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
               ("ERROR: Packet Recv - Header invalid MAGIC."IFXOS_CRLF));

            *pCurrRecvBytes  = 0;
            return DTI_CON_RECV_STATUS_STREAM_RESYNC;
         }

         nBytesRead = DTI_conDataRead(
                              pDtiCon, IFX_FALSE,
                              nRemainBytes, &pBufIn[nTotalRecvBytes]);
         if (nBytesRead < 0)
         {
            /* error read */
            *pCurrRecvBytes  = 0;
            return (nBytesRead == DTI_CONNECTION_CLOSED) ?
                        DTI_CON_RECV_STATUS_CON_CLOSED : DTI_CON_RECV_STATUS_ERROR;
         }

         nTotalRecvBytes += nBytesRead;
         nRemainBytes    -= nBytesRead;

         if (nRemainBytes == 0)
         {
            /* done */
            packetError = DTI_packetSwap(pDtiPacket, IFX_TRUE, IFX_TRUE);

            DTI_packetShow (
               pDtiPacket, IFX_TRUE, IFX_FALSE, "DTI Recv",
               (packetError == DTI_eNoError) ? IFXOS_PRN_LEVEL_LOW : DTI_PRN_LEVEL_HIGH);

            *pPacketError = packetError;
            if (ppPacket != IFX_NULL)
            {
               *ppPacket = pDtiPacket;
            }

            *pCurrRecvBytes  = nTotalRecvBytes;

            return DTI_CON_RECV_STATUS_DONE;
         }
      }
   }

   *pCurrRecvBytes = nTotalRecvBytes;
   return DTI_CON_RECV_STATUS_PENDING;
}

/**
   Swap and send a DTI packet to the TCP connection.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_int_t DTI_packetSend(
                        const DTI_Connection_t  *pDtiCon,
                        DTI_Packet_t            *pDtiPacket)
{
   IFX_int_t            nBytesToSend;
   DTI_PacketError_t    swapRet = DTI_eNoError;
   DTI_PacketHeader_t   *pHeader;

   if ( (pDtiCon == IFX_NULL)  || (pDtiPacket == IFX_NULL) )
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Send - NULL ptr args."IFXOS_CRLF));

      return IFX_ERROR;
   }

   if (pDtiCon->nFd < 0)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Send - invalid socket connection."IFXOS_CRLF));

      return IFX_ERROR;
   }

   pHeader            = &pDtiPacket->header;
   nBytesToSend       = (IFX_int_t)pHeader->payloadSize + sizeof(DTI_PacketHeader_t);

   swapRet = DTI_packetSwap(pDtiPacket, IFX_FALSE, IFX_TRUE);
   if (swapRet != DTI_eNoError)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Send - swap."IFXOS_CRLF));

      pHeader->error = (IFX_uint32_t)DTI_htons(swapRet);
   }

   if ( DTI_conDataSend(pDtiCon, nBytesToSend, (IFX_char_t *)pDtiPacket) != IFX_SUCCESS)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Send - Socket send %d (0x%X)."IFXOS_CRLF, nBytesToSend, nBytesToSend));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/**
   Show a given DTI packet.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_connection_interface.h'
*/
IFX_void_t DTI_packetShow (
                        DTI_Packet_t      *pDtiPacket,
                        IFX_boolean_t     bIsInHostOrder,
                        IFX_boolean_t     bShowPayload,
                        const IFX_char_t  *pDescr,
                        IFX_uint_t        dbgLevel)
{
   IFX_uint32_t         payloadSize;
   DTI_PacketHeader_t   *pHeader;

   if ( (pDtiPacket == IFX_NULL) )
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Show - NULL ptr args."IFXOS_CRLF));

      return;
   }

   pHeader     = &pDtiPacket->header;
   payloadSize = (IFX_uint32_t)
         ((bIsInHostOrder == IFX_TRUE) ? pHeader->payloadSize : DTI_ntohl(pHeader->payloadSize));

   IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
      ("DTI Packet: %s" IFXOS_CRLF,
        pDescr ? pDescr : "unknown"));
   IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
      ("\tmagic:  0x%08X    type:   0x%08X    option: 0x%08X" IFXOS_CRLF,
       (bIsInHostOrder == IFX_TRUE) ? pHeader->magic         : DTI_ntohl(pHeader->magic),
       (bIsInHostOrder == IFX_TRUE) ? pHeader->packetType    : DTI_ntohl(pHeader->packetType),
       (bIsInHostOrder == IFX_TRUE) ? pHeader->packetOptions : DTI_ntohl(pHeader->packetOptions)) );

   IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
      ("\tport:   0x%08X    tan:    0x%08X    error:  0x%08X" IFXOS_CRLF,
       (bIsInHostOrder == IFX_TRUE) ? pHeader->port  : DTI_ntohl(pHeader->port),
       (bIsInHostOrder == IFX_TRUE) ? pHeader->tan   : DTI_ntohl(pHeader->tan),
       (bIsInHostOrder == IFX_TRUE) ? pHeader->error : DTI_ntohl(pHeader->error) ));

   IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
      ("\tpaylSize: 0x%08X" IFXOS_CRLF,
       (bIsInHostOrder == IFX_TRUE) ? pHeader->payloadSize : DTI_ntohl(pHeader->payloadSize)));


   if (((bIsInHostOrder == IFX_TRUE) ? pHeader->magic : DTI_ntohl(pHeader->magic)) == DTI_MAGIC)
   {
      if ( (bShowPayload) && (payloadSize > 0))
      {
         IFX_int_t   i, j;
         DTI_PayloadType_t ePayloadType;
         DTI_PTR_U   uPayload;

         uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacket->payload;

         if (bIsInHostOrder == IFX_TRUE)
         {
            ePayloadType = (DTI_PayloadType_t)(pHeader->packetOptions & DTI_HDR_OPTION_MASK_PAYL_TYPE);
         }
         else
         {
            ePayloadType = (DTI_PayloadType_t)(DTI_ntohl(pHeader->packetOptions) & DTI_HDR_OPTION_MASK_PAYL_TYPE);
         }

         switch(ePayloadType)
         {
            case DTI_eMixed:
            case DTI_e8Bit:
               {
                  IFX_uint8_t *pPayload_8 = DTI_PTR_CAST_GET_UINT8(uPayload);

                  if (pPayload_8 != IFX_NULL)
                  {
                     for (i = 0, j=0 ; i < (IFX_int_t)payloadSize; i++, j++)
                     {
                        if (i == 0)
                        {
                           IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
                              ("Data [%s]:" IFXOS_CRLF "\t",
                                (ePayloadType == DTI_e8Bit) ? "DTI_e8Bit" : "DTI_eMixed"));

                        }
                        IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
                           ("%02x ", pPayload_8[i]));

                        if (j >= 15)
                        {
                           IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
                              (IFXOS_CRLF "\t"));
                        }
                     }

                     IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel, (IFXOS_CRLF));
                  }
                  else
                  {
                     IFXOS_PRN_USR_ERR_NL(DTI_CON, dbgLevel,
                        ("Error: Packet Show - missing data (DTI_e8Bit)."IFXOS_CRLF));
                  }
               }
               break;

            case DTI_e16Bit:
               {
                  IFX_uint16_t *pPayload_16 = DTI_PTR_CAST_GET_UINT16(uPayload);

                  if (payloadSize % 2)
                  {
                     IFXOS_PRN_USR_ERR_NL(DTI_CON, dbgLevel,
                        ("Error: Packet Show - unaligned size (DTI_e16Bit)."IFXOS_CRLF));

                     payloadSize &= ~(0x1);
                  }

                  payloadSize = payloadSize / 2;

                  if (pPayload_16 != IFX_NULL)
                  {
                     for (i = 0, j=0 ; i < (IFX_int_t)payloadSize; i++, j++)
                     {
                        if (i == 0)
                        {
                           IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
                              ("Data [DTI_e16Bit]:" IFXOS_CRLF "\t"));

                        }
                        IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
                           ("%04x ", ((bIsInHostOrder == IFX_TRUE) ? pPayload_16[i] : DTI_ntohs(pPayload_16[i]))));

                        if (j >= 8)
                        {
                           IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
                              (IFXOS_CRLF "\t"));
                        }
                     }

                     IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel, (IFXOS_CRLF));
                  }
                  else
                  {
                     IFXOS_PRN_USR_ERR_NL(DTI_CON, dbgLevel,
                        ("Error: Packet Show - missing / unaligned data (DTI_e16Bit)."IFXOS_CRLF));
                  }
               }
               break;

            case DTI_e32Bit:
               {
                  IFX_uint32_t *pPayload_32 = DTI_PTR_CAST_GET_UINT32(uPayload);

                  if (payloadSize % 4)
                  {
                     IFXOS_PRN_USR_ERR_NL(DTI_CON, dbgLevel,
                        ("Error: Packet Show - unaligned size (DTI_e32Bit)."IFXOS_CRLF));

                     payloadSize &= ~(0x3);
                  }

                  payloadSize = payloadSize / 4;

                  if (pPayload_32 != IFX_NULL)
                  {
                     for (i = 0, j=0 ; i < (IFX_int_t)payloadSize; i++, j++)
                     {
                        if (i == 0)
                        {
                           IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
                              ("Data [DTI_e32Bit]:" IFXOS_CRLF "\t"));

                        }
                        IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
                           ("%08X ", ((bIsInHostOrder == IFX_TRUE) ? pPayload_32[i] : DTI_ntohl(pPayload_32[i]))));

                        if (j >= 8)
                        {
                           IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel,
                              (IFXOS_CRLF "\t"));
                        }
                     }

                     IFXOS_PRN_USR_DBG_NL(DTI_CON, dbgLevel, (IFXOS_CRLF));
                  }
                  else
                  {
                     IFXOS_PRN_USR_ERR_NL(DTI_CON, dbgLevel,
                        ("Error: Packet Show - missing / unaligned data (DTI_e32Bit)."IFXOS_CRLF));
                  }

               }
               break;

            default:
               break;
         }
      }
   }
   else
   {
      IFXOS_PRN_USR_ERR_NL(DTI_CON, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Show - invalid header (magic)."IFXOS_CRLF));
   }

   return;
}

