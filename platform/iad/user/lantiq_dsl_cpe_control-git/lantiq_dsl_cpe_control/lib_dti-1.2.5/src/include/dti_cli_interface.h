#ifndef _DTI_CLI_INTERFACE_H
#define _DTI_CLI_INTERFACE_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Interface definiton for the
      DTI Command Line Interface (CLI).
*/




/** \defgroup DTI_CLI_IF DTI Command Line Interface

   This Group contains the definitions and functions of the Debug and Trace
   Command Line Interface Layer.

   This layer provides basic functions and definitions to setup and manage
   a DTI CLI on agent side.
   For the management of a user CLI the DTI Agent must provide functions to
   - allow to register a corresponding user send function which is called
     by the agent to send a received CLI command to the user application
   - a function to stop the interface.
   - export a send event functions which will be called by the user application
     to send an upcoming event to the remote DTI client.

*/


#ifdef __cplusplus
   extern "C" {
#endif


/* ============================================================================
   Includes
   ========================================================================= */
#include "ifx_types.h"


/* ============================================================================
   Defines
   ========================================================================= */

/**
   Defines the callback function to send a string to a given CLI.

\param
   pCmdIn            - points to the CLI command string.
\param
   pResultOut        - points to the return buffer, filled by the CLI with
                       the result string
\param
   pResultSize_byte  - contains the provided buffer size,
                       returns the size of the result sting
\param
   pResultCode       - return pointer to return a CLI specific return code.

\return
   IFX_SUCCESS in case of success
   IFX_ERROR   in case of error
*/
typedef IFX_int_t (*DTI_CliSendCommandFunc_t)(
                              IFX_void_t        *pCallbackContext,
                              const IFX_char_t  *pCmdIn,
                              IFX_char_t        *pResultOut,
                              IFX_int_t         *pResultSize_byte,
                              IFX_int_t         *pResultCode);

/**
   Defines the callback function to send a partial response from CLI

\param
   pPartRespContext            - callback context
\param
   pBuffer                     - partial response data
\param
   len                         - size of partial response

\return
   size of data
*/
typedef IFX_uint32_t (*DTI_PartialResponseHandler)(
        IFX_void_t              *pPartRespContext,
        IFX_void_t              *pBuffer,
        IFX_uint32_t              len);


/**
   Defines the callback function to send a string to a given CLI. Fragmented response supported.

\param
   pPartRespHdlr     - points to function which handle partial response.
\param
   pPartRespCtx      - points to partial response context
\param
   pCmdIn            - points to the CLI command string.
\param
   pResultOut        - points to the return buffer, filled by the CLI with
                       the result string
\param
   pResultSize_byte  - contains the provided buffer size,
                       returns the size of the result sting
\param
   pResultCode       - return pointer to return a CLI specific return code.

\return
   IFX_SUCCESS in case of success
   IFX_ERROR   in case of error
*/
typedef IFX_int_t (*DTI_CliSendFragmentedCommandFunc_t)(
                              IFX_void_t                 *pCallbackContext,
                              DTI_PartialResponseHandler pPartRespHdlr,
                              IFX_void_t                 *pPartRespCtx,
                              const IFX_char_t           *pCmdIn,
                              IFX_char_t                 *pResultOut,
                              IFX_int_t                  *pResultSize_byte,
                              IFX_int_t                  *pResultCode);


/**
   Callback function to send a partial response from CLI

\param
   pPartRespContext            - callback context
\param
   pBuffer                     - partial response data
\param
   len                         - size of partial response

\return
   size of data
*/
IFX_uint32_t DTI_handlePartialResponse(IFX_void_t              *pPartRespContext,
                                       IFX_void_t              *pBuffer,
                                       IFX_uint32_t            len);


struct DTI_PartialResponceContext_s{
   /** In packet data*/
   DTI_Packet_t  *pPacketIn;
   /** Out packet data*/
   DTI_Packet_t  *pPacketOut;
   /** Out packet buffer size*/
   IFX_uint32_t iBufferOutLen;
   /** DTI Protocol Server Context (Agent side) */
   DTI_ProtocolServerCtx_t *pDtiProtServerCtx;
};
typedef struct DTI_PartialResponceContext_s DTI_PartialResponceContext_t;

/* ============================================================================
   Exports
   ========================================================================= */

/**
   This function registers a corresponding send function within the DTI.
   The function is used to send incoming command strings to the caller
   command line interface (CLI).
   The send function must have the format of \ref DTI_CliSendCommandFunc_t

\param
   pDtiAgentContext  - points to the DTI Agent context.
\param
   pCallbackContext  - points to the callback context, this context contains
                       user data used by the callback function. The callback
                       function is called with this pointer.
\param
   pIfName           - CLI name, via this name the CLI will be identified from
                       the remote client.
\param
   pSendFct          - function pointer to the send function to send a command
                       on this CLI.
                       Provide a NULL pointer to reserve the CLI interface and
                       to get a interface number (for example, only to register
                       an event call back).
\param
   responceBufferSize - the buffer size required by the callback function.

\return
   In case of success, the CLI interface is marked as reserved and the
   interface number is returned (>= 0)
   In case of error IFX_ERROR will be return.

\remarks
   The last available interface is the default loop back interface.

*/
extern IFX_int_t DTI_CLI_SendFunctionRegister(
                                 IFX_void_t                 *pDtiAgentContext,
                                 IFX_void_t                 *pCallbackContext,
                                 const IFX_char_t           *pIfName,
                                 DTI_CliSendCommandFunc_t   pSendFct,
                                 IFX_uint_t                 responceBufferSize);

/**
   This function registers a corresponding send function within the DTI.
   The function is used to send incoming command strings to the caller
   command line interface (CLI) with fragmented response support.
   The send function must have the format of \ref DTI_CliSendFragmentedCommandFunc_t

\param
   pDtiAgentContext  - points to the DTI Agent context.
\param
   pCallbackContext  - points to the callback context, this context contains
                       user data used by the callback function. The callback
                       function is called with this pointer.
\param
   pIfName           - CLI name, via this name the CLI will be identified from
                       the remote client.
\param
   pSendFragFct      - function pointer to the send function to send a command                     on this CLI.
                       Provide a NULL pointer to reserve the CLI interface and
                       to get a interface number (for example, only to register
                       an event call back).
\param
   responceBufferSize - the buffer size required by the callback function.

\return
   In case of success, the CLI interface is marked as reserved and the
   interface number is returned (>= 0)
   In case of error IFX_ERROR will be return.

\remarks
   The last available interface is the default loop back interface.

*/
extern IFX_int_t DTI_CLI_SendFragmentedFunctionRegister(
                                 IFX_void_t                           *pDtiAgentContext,
                                 IFX_void_t                           *pCallbackContext,
                                 const IFX_char_t                     *pIfName,
                                 DTI_CliSendFragmentedCommandFunc_t   pSendFragFct,
                                 IFX_uint_t                           responceBufferSize);
/**
   This function stops the given DTI CLI interface within the DTI Agent.
   - unregisters the send function.
   - disable the event callback.
   - free allocated memory.

\param
   pDtiAgentContext  - points to the DTI Agent context.
\param
   pIfNumber         - CLI interface number.

\return
   In case of success, the CLI Send Fct is unregistered.
   In case of error IFX_ERROR or a negative value will be return.

\remarks
   The last available interface is the default loop back interface.

*/
extern IFX_int_t DTI_CLI_InterfaceStop(
                                 IFX_void_t  *pDtiAgentContext,
                                 IFX_int_t   ifNumber);


/**
   This function writes the given event message to the DTI CLI Interface.
   - checks the given IF.
   - distributes the message to all listen worker.

\param
   pDtiAgentContext  - points to the DTI Agent context.
\param
   pIfNumber         - CLI interface number.
\param
   pEventOut         - points to the message to write
\param
   eventOutSize_byte - number of bytes to write.

\return
   In case of success, the CLI Send Fct is unregistered.
   In case of error IFX_ERROR or a negative value will be return.

\remarks
   The last available interface is the default loop back interface.

*/
extern IFX_int_t DTI_CLI_InterfaceEventSend(
                                 IFX_void_t  *pDtiAgentContext,
                                 IFX_int_t   ifNumber,
                                 IFX_char_t  *pEventOut,
                                 IFX_uint_t  eventOutSize_byte);

#ifdef __cplusplus
}
#endif

#endif   /* #ifndef _DTI_CLI_INTERFACE_H */


