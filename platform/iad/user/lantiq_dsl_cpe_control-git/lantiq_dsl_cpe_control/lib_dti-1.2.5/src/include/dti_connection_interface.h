#ifndef _DTI_CONNECTION_INTERFACE_H
#define _DTI_CONNECTION_INTERFACE_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DTI Connection library API.
*/


/** \mainpage  Debug and Trace Interface (DTI) Overview
   The Debug and Trace Interface (DTI) is a communication interface to
   access and communicate with target systems (embedded) for debug and
   test purposals.

   Therefore on the target system a so called DTI Agent must running.
   The communication is currently TCP/IP based and a corresponding protocol
   is used.
   On host side a so called DTI Client is required which fullfills the
   DTI protocol.
   The DTI Protocol is application independent but provides special features
   for debugging and testing of a target system.

   The DTI Protocol is splitted into functional modules:
   - Common: provides the common DTI features.
   - Device: covers device specific handling.
   - Command Line Interface (CLI): handles string commands.
   - DTI-CLI: handles DTI specific string commands.

   The following pages provides more detailed informations

   - \subpage DTI_SYSTEM_OVERVIEW_PAGE
   - \subpage DTI_AGENT_OVERVIEW_PAGE
   - \subpage DTI_LIBRARY_OVERVIEW_PAGE

   For more informations concerning the split see \subpage DTI_PROTOCOL_MODULE_PAGE.
*/

/** \page  DTI_SYSTEM_OVERVIEW_PAGE DTI System Overview
   The following picture gives an overview of a system setup for an existing
   VDSL2 sytem. Here the concept of DTI Agent and Client is shown and also the
   possibility of an replacement of the existing SOAP communication by the DTI
   concept.

   \image html dti_system_overview.png "DTI System Overview"

   On the target system a DTI Agent mus be available. There the DTI Agent can be
   a standalone application (for device testing) or it can be linked in form of
   a library to an already existing application (example the DTI Agent library
   can be linked to our DSL Deamon).


*/

/** \page DTI_AGENT_OVERVIEW_PAGE DTI Agent Overview

   The DTI Agent on the target provides a server client concept.
   Therefore the agent starts an listener server which will listen on the
   wellknown DTI port and will accept DTI connections form a remote client.
   On a connection requests the listener will start a corresponding worker
   which will take over the request.
   So the connection between a client and the agent will be handled on agent
   side always by a connection related worker with its own resources. This will
   make each connection independent.

   \image html dti_agent_overview.png "DTI Agent Overview"

   Out from the picture you can see there are shared resources like uploaded
   files and CLI interfaces while the device specific resources are local to the
   corresponding worker thread.

   The shared resources are common within the DTI Agent and can be used by all the
   worker threads. Therefore a corresponding protection is implemented within
   the DTI Agent.
   - With a file upload a remote file (for example a FW binary) will be transfered
     to the DTI Agent. The DTI Agent will keep the file within its internal
     control structure. Per default (compile option) the DTI Agent allows to
     upload 3 different files. On the target a file is identified by its
     image number which is simple the index within the control structure.
   - The CLI control structeres allows to setup and configure a CLI interface
     to the linked user application. Therefore a user application has to register
     the required callback functions for send a CLI command.
     Per default the DTI Agent supports 4 CLI interfaces (4 different user
     application in parallel). A CLI interface is identified by its interface
     number which is simple the index within the control structure.




*/

/** \page DTI_LIBRARY_OVERVIEW_PAGE DTI Library

   The DTI implementation provides a library for the basic protocol and connection
   handling on agent and client side. The functions are splitted into a
   - DTI protocol specific layer
   - DTI connection specific layer

   This split has been introduced to keep the protocol and the underlaying
   communication layer independent.

   \section DTI_PROTOCOL_IF_SECTION DTI Protocol Functions
   This part contains functions to setup and process DTI packets in a DTI protocol
   defined way. They can be used to check a received packet also to setup the
   corresponding responce.
   For more informations see \ref DTI_PROTOCOL_IF

   \section DTI_CONNECTION_IF_SECTION DTI Connection Functions
   Like mentioned above the TCP/IP protocol is used for communication.
   This layer provides an interface independent from the the underlaying
   communication protocol to setup a DTI connection and to send and receive
   DTI packets.
   If required this will allow (hopefully) to replace the underlaying protocol
   without changing the upperlayer DTI Agent concept.

   For more informations see \ref DTI_CONNECTION_IF

*/

/** \page DTI_SYSTEM_PROTOCOL_PAGE DTI Protocol

   For the communication between DTI Agent and DTI Client a TCP/IP based protocol
   is used with the following format:

   \image html dti_protocol_0.png "DTI Protocol"

   Out from this format a DTI packet will always have a header of the following
   form:

   \image html dti_protocol_1.png "DTI Protocol Header"

   The DTI packets are splitted within the following functional packet types.
   - \subpage DTI_PROTOCOL_MODULE_PAGE

*/

/** \page DTI_PROTOCOL_MODULE_PAGE DTI Protocol Modules

   The protocol is splitted in 3 (+1) operativ blocks with the corresponding DTI packet types.

   \section DTI_PROTOCOL_COMMON_SECTION DTI Protocol - Common Part
   The common part of the DTI packets are used to the general work. They cover
   DTI packets to:
   - request the system information
   - upload and manage files.
   - loop back handling.

   For detailed info see \ref DTI_PROTOCOL_COMMON

   \section DTI_PROTOCOL_DEVICE_SECTION DTI Protocol - Device Part
   The device specific part covers all packets used to access and manage a
   device on the target system. This are packets for:
   - direct device access (register access)
   - message exchange
   - device management like reset, FW download.

   The device communication is done local within the DTI Worker thread
   (see DTI Agent Overview, Listener and Worker). Therefore an DTI Worker
   will open and manage the underlaying device with its own context. So another
   DTI Worker (connection) will not be able to access these device resources.

   \image html dti_agent_device.png "DTI Agent Device Handling"

   For detailed information see also \ref DTI_PROTOCOL_CLI

   \section DTI_PROTOCOL_CLI_SECTION DTI Protocol - Command Line Interface (CLI) Part
   This module allows to send and receive string based commands to the target system.
   Depending on the configuration on the target system a CLI command is forwarded
   to a registered application by the DTI Agent.
   A CLI interface wihtin the DTI Agent is a global resource and can be used by
   all running DTI Worker threads (establisched connections)

   \image html dti_agent_cli.png "DTI Agent CLI Handling".

   For more information concerning the CLI module please have a look to
   - \subpage DTI_CLI_IF_PAGE

   For detailed interface information see \ref DTI_PROTOCOL_DEVICE.

   \section DTI_PROTOCOL_DTI_CLI_SECTION DTI Protocol - DTI-CLI Part
   This module is similar to the above mentioned CLI module. But the CLI commands
   are handled by the DTI Agent internally. Therefore such a DTI-CLI command must
   start with the DTI prefix and the Agent catch such a CLI command and does
   an internal processing.

   For detailed info see \ref DTI_PROTOCOL_CLI.
*/


/** \page DTI_CLI_IF_PAGE DTI Comand Line Interface Overview

   The DTI Agent provides a Command Line Interface (CLI) handling which allows
   a remote DTI Client to send a command in string format to the DTI Agent.

   On DTI Agent side a corresponding setup is required which configures the handling
   of incoming CLI commands to allow the forwarding to the corresponding user
   application.

   First there must be an user application which takes and processes a CLI command.
   Therefore this application has to register a corresponding CLI send function.
   This function will be called by the DTI Agent in case of incoming CLI packets
   and will forward the CLI command to the application. Together with the CLI
   command, the DTI agent will also provide a buffer for the command response.
   The responce will be send to the remote DTI Client.

   Additional the DTI Agent supports also event handling. Therefore the DTI Agent
   exports a corresponding event send function which can be used by the application
   to send autonomous messages via DTI to the remote side.

   For the CLI handling the Agent supports a configurable number of CLI interfaces
   (default 4). Each interface can be used and configured by one user application.

   The setup and configuration of the CLI interfaces are global within the DTI Agent.
   So each available interface can be used by any DTI worker thread. The usage
   of the interfaces is protected inside the DTI Agent.

   The event handling requires an additional setup. An incoming event must be
   forwarded to the corresponding DTI worker because only a DTI worker can
   communicate wiht the remote DTI Client.
   Per default no DTI Worker will be registered within the CLI interface. To
   allow the event handling for a specified connection, the corresponding worker
   must be registered for this CLI interface.

   \image html dti_agent_cli_event.png "DTI Agent CLI Event Handling"

*/


/** \defgroup DTI_CONNECTION_IF DTI Connection Interface

   This Group contains the definitions and functions of the Debug and Trace
   Connection Layer.

   This layer provides basic functions and definitions to setup and manage
   a DTI connection.

   The DTI connection is currently based on a TCP/IP connection.
   Therefore basic functions to setup and manage a connection are defined.

   Additional there are some basic functions to send and receive DTI packets.

*/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   Includes
   ========================================================================= */

#include "ifx_types.h"
#include "ifxos_common.h"
#include "ifxos_debug.h"

#include "ifxos_socket.h"
#include "ifx_dti_protocol.h"

/* ============================================================================
   Connection Handling - Macro Defines
   ========================================================================= */

/** \addtogroup DTI_CONNECTION_IF
@{ */

#define DTI_CONNECTION_CLOSED                         (-9999)

#ifndef DTI_TCP_PORT
#  define DTI_TCP_PORT                                9000
#endif

/* ============================================================================
   Connection Handling - Type Defines
   ========================================================================= */

/**
   DTI connection context
*/
typedef struct
{
   /** socket address struct */
   IFXOS_sockAddr_t  sockAddr;
   /** socket descriptor */
   IFXOS_socket_t    nFd;

} DTI_Connection_t;

/**
   DTI connection control context
*/
typedef struct
{
   /* IP address (if set) */
   IFX_char_t           ipAddr[IFXOS_SOC_ADDR_LEN_BYTE];
   /* TCP port */
   IFX_uint16_t         sockPort;

   /** max FD for select handling */
   IFX_int_t            socFdMax;

   /** FD SET - contains the selected connections */
   IFXOS_socFd_set_t    rdFdSet;
   /** FD SET - temporary used for work */
   IFXOS_socFd_set_t    tmpFdSet;

} DTI_ConnectionCntrl_t;


/* ============================================================================
   Connection Handling - Exports
   ========================================================================= */

IFXOS_PRN_USR_MODULE_DECL(DTI_CON);

/**
   Init the connection specific parts (OS specific).
   If neccessary the OS specific initialization will be done (see Win32, socket init).

\return
   IFX_SUCCESS OS specific init was successful, else
   IFX_ERROR in case of something has went wrong.
*/
extern IFX_int_t DTI_conInit(void);

/**
   Cleanup the connection specific parts (OS specific).

\return
   IFX_SUCCESS OS specific cleanup was successful.

*/
extern IFX_int_t DTI_conCleanup(void);

/**
   Reset / init connection control structure.

\param
   pDtiCon  - points to the connection data structure \ref DTI_Connection_t.

\return
   none
*/
extern IFX_void_t DTI_conStructInit(
                        DTI_Connection_t  *pDtiCon);

/**
   Reset / init a connection control structure.

\param
   pDtiConCntrl   - points to the connection control data structure \ref DTI_ConnectionCntrl_t.

\return
   none
*/
extern IFX_void_t DTI_conCntrlStructInit(
                        DTI_ConnectionCntrl_t *pDtiConCntrl);

/**
   Setup / takeover connection data.
   Setup the given "pDtiCon" data. If the "pNewDtiCon" connection is given, this
   will be used as a reference else a default init will be done.

\param
   pDtiCon     - points to the \ref DTI_Connection_t structure which has to setup.
\param
   pNewDtiCon  - if set points to the \ref DTI_Connection_t structure used for reference.

\return
   none
*/
extern IFX_void_t DTI_conConSet(
                        DTI_Connection_t  *pDtiCon,
                        DTI_Connection_t  *pNewDtiCon);

/**
   Setup connection control data.
   Therefore the following control data will be setup with the connection data:
   - IP Address (ntoa() convertion).

\param
   pDtiCon        - points to the connection data structure \ref DTI_Connection_t.
\param
   pDtiConCntrl   - points to the connection control data structure \ref DTI_ConnectionCntrl_t.

\return
   none
*/
extern IFX_void_t DTI_conCntrlConSet(
                        DTI_Connection_t        *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl);

/**
   Establisch/Setup a Client/Server DTI connection

   Depending on the bClient indication a client or server connections is opened.

   Server Connection:
      - Create Socket
      - Bind to the given IP address or to the system default address.
      - Listen

   Client Connection:
      - Create Socket
      - try to connect to the remote given server address.

\param
   pDtiCon  - points to the connection data structure \ref DTI_Connection_t.
\param
   bClient  - indication to setup a client or server connection.
\param
   ipPort   - used IP port.
\param
   pIpStr   - points to the used IP address.

\return
   IFX_SUCCESS connection has been opened and establisched, else
   IFX_ERROR in case of something has went wrong.
*/
extern IFX_int_t DTI_conOpen(
                        DTI_Connection_t  *pDtiCon,
                        IFX_boolean_t     bClient,
                        IFX_uint16_t      ipPort,
                        IFX_char_t        *pIpStr);

/**
   Close DTI connection

\param
   pDtiCon  - points to the connection data structure \ref DTI_Connection_t.

\return
   IFX_SUCCESS connection has been closed successfully, else
   IFX_ERROR in case of something has went wrong.
*/
extern IFX_int_t DTI_conClose(
                        DTI_Connection_t  *pDtiCon);


/**
   Add the given connection to the control structure for receive data.
   Therefore the connection will be added to the control FD Set structure.

\param
   pDtiCon        - points to the connection data structure \ref DTI_Connection_t.
\param
   pDtiConCntrl   - points to the connection control data structure \ref DTI_ConnectionCntrl_t.

\return
   none
*/
extern IFX_void_t DTI_conAddForRecvWait(
                        const DTI_Connection_t  *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl);


/**
   Check if new data are available.
   This functions check all connections within this control structure for new
   data. If data are available the corresponding connection will be kept wihtin
   an internal temporary FD Set structure.

\param
   pDtiConCntrl   - points to the connection control data structure \ref DTI_ConnectionCntrl_t.
\param
   waitTime_ms    - time to wait for new data [ms].

\return
   0: timeout, no new connection request received.
   1: new data received.
   IFX_ERROR: error occurred.

*/
extern IFX_int_t DTI_conDataRecvCheck(
                        DTI_ConnectionCntrl_t *pDtiConCntrl,
                        IFX_int_t             waitTime_ms);



/**
   Check if new data are available for the given connection.
   Therefore the internal temporary FD Set structure will be checked for the
   given connection.

\param
   pDtiCon        - points to the connection data structure \ref DTI_Connection_t.
\param
   pDtiConCntrl   - points to the connection control data structure \ref DTI_ConnectionCntrl_t.

\return
   0: no new data available for this connection.
   1: new data received.
*/
extern IFX_int_t DTI_conDataRecvMyConCheck(
                        const DTI_Connection_t  *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl);

/**
   Wait for a new connection and accept it.
   This is on server side, to wait for and accept new incoming connections.
   A new accepted connection will be returnd to the caller.

\param
   pDtiCon        - points to the connection data structure \ref DTI_Connection_t.
\param
   pDtiConCntrl   - points to the connection control data structure \ref DTI_ConnectionCntrl_t.
\param
   waitTime_ms    - time to wait for a new connection [ms].
\param
   pDtiConNew     - points to the new connection data structure \ref DTI_Connection_t.

\return
   0: timeout, no new connection request received.
   1: new connection received and accepted
   IFX_ERROR: error occurred.
*/
extern IFX_int_t DTI_conNewConnectGet(
                        const DTI_Connection_t  *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl,
                        IFX_int_t               waitTime_ms,
                        DTI_Connection_t        *pDtiConNew);

/**
   Read data from a connection.

\param
   pDtiCon     - points to the connection data structure \ref DTI_Connection_t.
\param
   bBlocking   - if set, wait for all requested bytes (blocking).
\param
   nReadBytes  - number of bytes to read.
\param
   pBufIn      - points to the receive buffer.

\return
   > 0 number of read bytes
   IFX_ERROR: in case of error
   DTI_CONNECTION_CLOSED: connection closed
*/
extern IFX_int_t DTI_conDataRead(
                        const DTI_Connection_t  *pDtiCon,
                        IFX_boolean_t           bBlocking,
                        IFX_int_t               nReadBytes,
                        IFX_char_t              *pBufIn);
/**
   Check for and read data from a connection

\param
   pDtiCon     - points to the connection data structure \ref DTI_Connection_t.
\param
   pDtiConCntrl   - points to the connection control data structure \ref DTI_ConnectionCntrl_t.
\param
   bBlocking   - if set, wait for all requested bytes (blocking).
\param
   waitTime_ms - time to wait for data [ms].
\param
   nReadBytes  - number of bytes to read.
\param
   pBufIn      - points to the receive buffer.

\return
   > 0                     number of read bytes
   0                       no data available
   IFX_ERROR               in case of error
   DTI_CONNECTION_CLOSED   connection closed
*/
extern IFX_int_t DTI_conDataCheckAndRecv(
                        const DTI_Connection_t  *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl,
                        IFX_boolean_t           bBlocking,
                        IFX_int_t               waitTime_ms,
                        IFX_int_t               nReadBytes,
                        IFX_char_t              *pBufIn);

/**
   Send data to a connection.

\param
   pDtiCon     - points to the connection data structure \ref DTI_Connection_t.
\param
   nSendBytes  - number of bytes to send.
\param
   pBufOut     - points to the buffer contaning the out data.

\return
   IFX_SUCCESS: operation done, or
   IFX_ERROR: in case of error
*/
extern IFX_int_t DTI_conDataSend(
                        const DTI_Connection_t  *pDtiCon,
                        IFX_int_t               nSendBytes,
                        IFX_char_t              *pBufOut);

/* ============================================================================
   DTI Packet Handling - Macro Defines
   ========================================================================= */

/** Receive Op - connection closed */
#define DTI_CON_RECV_STATUS_CON_CLOSED          DTI_CONNECTION_CLOSED
/** Receive Op - resync required */
#define DTI_CON_RECV_STATUS_STREAM_RESYNC       -2
/** Receive Op - error */
#define DTI_CON_RECV_STATUS_ERROR               IFX_ERROR
/** Receive Op - nothing to do, nothing received */
#define DTI_CON_RECV_STATUS_PENDING             0
/** Receive Op - packet received done */
#define DTI_CON_RECV_STATUS_DONE                1
/** Receive Op - partial packet received done */
#define DTI_CON_RECV_STATUS_PARTIAL_DONE        2
/** Receive Op - timeout is occurred */
#define DTI_CON_RECV_STATUS_TIMEOUT             3

/* ============================================================================
   DTI Packet Handling - Exports
   ========================================================================= */

/**
   Resynchronisation - Read and search for the "magic" key

\param
   pDtiCon        - points to the connection data structure \ref DTI_Connection_t.
\param
   pDtiConCntrl   - points to the connection control data structure \ref DTI_ConnectionCntrl_t.
\param
   pBResync       - points to the resync flag, reset to false if a DTI packet has been found.
\param
   pBufIn         - points to the recv buffer to store the packet
\param
   magicNum       - Magic number of the DTI protocol.
\param
   waitTime_ms    - time to wait for new data [ms].

\return
   DTI_CON_RECV_STATUS_CON_CLOSED
   DTI_CON_RECV_STATUS_ERROR
   DTI_CON_RECV_STATUS_STREAM_RESYNC
   number of read bytes (sizeof(DTI_PacketHeader_t))
*/
extern IFX_int_t DTI_packetReadResync (
                        DTI_Connection_t        *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl,
                        IFX_boolean_t           *pBResync,
                        IFX_char_t              *pBufIn,
                        IFX_uint32_t            magicNum,
                        IFX_int_t               waitTime_ms);

/**
   Read and swap a DTI packet from the TCP connection and setup
   the DTI packet structure.

\param
   pDtiCon        - points to the connection data structure \ref DTI_Connection_t.
\param
   pDtiConCntrl   - points to the connection control data structure \ref DTI_ConnectionCntrl_t.
\param
   pBResync       - points to the resync flag, set to IFX_FALSE if a DTI packet has been found.
\param
   waitTime_ms    - use waiting time for new data
\param
   nBufInLen      - size of the recv buffer
\param
   pBufIn         - points to the recv buffer to store the packet
\param
   pCurrRecvBytes - points location where the current number of received
                    bytes is stored
\param
   pPacketError   - return pointer.
\param
   ppPacket       - returns the DTI packet pointer (assigned to the recv buffer).

\return
   DTI_CON_RECV_STATUS_DONE         : packet receive complete
   DTI_CON_RECV_STATUS_PENDING      : packet receive still not complete.
   DTI_CON_RECV_STATUS_CON_CLOSED   : connection has been closed.
   DTI_CON_RECV_STATUS_ERROR        : Error while receive operation.
*/
extern IFX_int_t DTI_packetRead (
                        DTI_Connection_t        *pDtiCon,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl,
                        IFX_boolean_t           *pBResync,
                        IFX_int_t               waitTime_ms,
                        IFX_int_t               nBufInLen,
                        IFX_char_t              *pBufIn,
                        IFX_int_t               *pCurrRecvBytes,
                        DTI_PacketError_t       *pPacketError,
                        DTI_Packet_t            **ppPacket);

/**
   Swap and send a DTI packet to the TCP connection.

\param
   pDtiCon           - points the the contection info
\param
   pDtiPacket        - points to the the DTI packet to send.

\return
   IFX_SUCCESS - if no error occurred, else
   IFX_ERROR   - if something went wrong.
*/
extern IFX_int_t DTI_packetSend(
                        const DTI_Connection_t  *pDtiCon,
                        DTI_Packet_t            *pDtiPacket);

/**
   Show a given DTI packet.

\param
   pDtiPacket     - points to the DTI packet.
\param
   bIsInHostOrder - Set if the packet is already in host order.
\param
   bShowPayload   - If set show also payload data.
\param
   pDescr         - user description
\param
   dbgLevel       - used debug level for printout.
\return
   none
*/
extern IFX_void_t DTI_packetShow (
                        DTI_Packet_t      *pDtiPacket,
                        IFX_boolean_t     bIsInHostOrder,
                        IFX_boolean_t     bShowPayload,
                        const IFX_char_t  *pDescr,
                        IFX_uint_t        dbgLevel);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _DTI_CONNECTION_INTERFACE_H */
