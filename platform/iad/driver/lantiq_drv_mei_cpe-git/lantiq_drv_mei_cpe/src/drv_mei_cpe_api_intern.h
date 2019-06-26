#ifndef _DRV_MEI_CPE_API_INTERN_H
#define _DRV_MEI_CPE_API_INTERN_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ============================================================================
   Description : VRX Driver internal API
   ========================================================================= */

/**
\attention
   All functions expect that the arguments and data are already in kernel space.
*/


/* ==========================================================================
   includes
   ========================================================================== */
#include "ifx_types.h"

#include "drv_mei_cpe_config.h"
#include "drv_mei_cpe_interface.h"

/* ==========================================================================
   Internal API - Macro Definitions
   ========================================================================== */


/* ==========================================================================
   Internal API - Type Definitions
   ========================================================================== */

/** \addtogroup MEI_COMMON_INTERN
 @{ */

/**
   Forward declaration
*/
typedef struct MEI_dyn_cntrl_s    MEI_DYN_CNTRL_T;
typedef struct MEI_dev_s MEI_DEV_T;


/*
   NFC received - call back.
*/
typedef void (*MEI_InternalMsgRecvCallBack)(void *);

typedef IFX_int32_t (*MEI_InternalTcLayerRequestCallback_t) (IFX_int32_t, IFX_uint32_t);

#if (MEI_EXPORT_INTERNAL_API == 1)

/* ==========================================================================
   Internal API - Functions Definitions
   ========================================================================== */

/**
   Open a device (driver layer internal)

   \param nLine
      Specifies the line number to open. The definition of the max. possible
      value is done by definition of (MEI_MAX_DFE_CHAN_DEVICES - 1) and is
      currently set to 0.

   \param ppMeiDynCntrl
      Reference to a pointer that will return the the dynamic MEI control
      structure.
      This pointer has to be stored within callig software and has to be used
      later on within access to driver layer internal functions.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalDevOpen(
                              IFX_uint16_t      nLine,
                              MEI_DYN_CNTRL_T **ppMeiDynCntrl);

/**
   Close a device (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalDevClose(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl);

/**
   Returns the driver version (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgDrvVersion_out
      points to the ioctl driver version struct.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalDrvVersionGet(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_drvVersion_t *pArgDrvVersion_out);

/**
   Set the Driver debug level (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgDbgLevel
      Points to the ioctl driver debug level struct.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalDebugLevelSet(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgLevel_t  *pArgDbgLevel);

/**
   Do the basic init of the device (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgInitDev
      Points to the ioctl driver basic init struct.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalInitDevice(
                              MEI_DYN_CNTRL_T     *pMeiDynCntrl,
                              IOCTL_MEI_devInit_t *pArgInitDev);

/**
   Do a reset (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgRstArgs
      Points to the ioctl driver reset struct.

   \param rstSrc
      Indicates the reset source (internal/external).

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalDevReset(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              IOCTL_MEI_reset_t  *pArgRstArgs,
                              IFX_int32_t          rstSrc);

/**
   Request the configuration of the driver (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgDevCfg_out
      Points to the return struct.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      The online chip parameters (chip id, bootmode, online mailbox params)
      are only returned if the modem is in online mode (modem ready).
*/
extern IFX_int32_t MEI_InternalRequestConfig(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              IOCTL_MEI_reqCfg_t *pArgDevCfg_out);

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
/**
   Swap the VRX between VDSL2 and ADSL (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgFwMode
      Points to the ioctl driver basic init struct.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalDevCfgFwModeSwap(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              IOCTL_MEI_fwMode_t *pArgFwMode);
#endif

#if (MEI_SUPPORT_STATISTICS == 1)
/**
   Request the statistics of the driver (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgDevStat_out
      Points to the return struct. Also refer to details of according structure
      that is used.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalRequestStat(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_statistic_t *pArgDevStat_out );
#endif

#if (MEI_SUPPORT_REGISTER == 1)
/**
   Set a MEI interface register (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgRegInOut
      Points to the return struct. Also refer to details of according structure
      that is used.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalSetRegister(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_regInOut_t  *pArgRegInOut);

/**
   Get a MEI interface register (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgRegInOut
      Points to the return struct. Also refer to details of according structure
      that is used.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalGetRegister(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_regInOut_t  *pArgRegInOut);
#endif

/**
   Do an FW Download (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgFwDl
      Points to the FW download information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalFirmwareDownload(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_fwDownLoad_t *pArgFwDl);

/**
   Do an Optimized FW Download (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgFwDl
      Points to the FW download information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalOptFirmwareDownload(
                              MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                              IOCTL_MEI_fwOptDownLoad_t *pArgFwDl);

/**
   Set FW xDSL/DualPort mode control.

\param
   pMeiDynCntrl - private dynamic comtrol data (per open instance)
\param
   pArgFwModeCtrl - points to the xDSL/DualPort mode control data

\return
   IFX_SUCCESS: if the FW Mode Control setting was successful.
   negative value if something went wrong.
*/
extern IFX_int32_t MEI_InternalFwModeCtrlSet(
                              MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                              IOCTL_MEI_FwModeCtrlSet_t *pArgFwModeCtrl);

/**
   Get FW xDSL/DualPort mode control status.

\param
   pMeiDynCntrl - private dynamic comtrol data (per open instance)
\param
   pArgFwModeStat - points to the xDSL/DualPort mode status data

\return
   IFX_SUCCESS: if the FW Mode Control status get was successful.
   negative value if something went wrong.
*/
extern IFX_int32_t MEI_InternalFwModeStatGet(
                              MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                              IOCTL_MEI_FwModeStatGet_t *pArgFwModeStat);

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
/**
   General Purpose Access (GPA) Write (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgGpaInOut
      Points to the return struct. Also refer to details of according structure
      that is used.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalGpaWrAccess(
                              MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                              IOCTL_MEI_GPA_accessInOut_t *pArgGpaInOut);

/**
   General Purpose Access (GPA) Read (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgGpaInOut
      Points to the return struct. Also refer to details of according structure
      that is used.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalGpaRdAccess(
                              MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                              IOCTL_MEI_GPA_accessInOut_t *pArgGpaInOut);
#endif


#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)
/**
   MEI Debug Access - Write (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgDbgAccessInOut
      Points to the return struct. Also refer to details of according structure
      that is used.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalMeiDbgAccessWr(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgAccess_t *pArgDbgAccessInOut);

/**
   MEI Debug Access - Read (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgDbgAccessInOut
      Points to the return struct. Also refer to details of according structure
      that is used.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalMeiDbgAccessRd(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgAccess_t *pArgDbgAccessInOut);
#endif


/**
   Set a callback data block for NFC handling (currently only for linux)
   (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pCallBackFunc
      Please refer to details of according structure that is used.

   \param pNfcCallBackData
      Points to the return struct. Also refer to details of according structure
      that is used.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalNfcCallBackDataSet(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              MEI_InternalMsgRecvCallBack pCallBackFunc,
                              IFX_void_t *pNfcCallBackData);

/**
   Enable the autonomous messages for this line and for this open instance
   (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalNfcEnable(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl);

/**
   Disable the autonomous messages for this line and for this open instance
   (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalNfcDisable(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl);

/**
   Setup the autonomous message handling (device + driver)
   (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgAutoMsgCtrl
      ioctl struct for autonomous msg config

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      Via this command you can also do the NFC Enable/Disable.
*/
extern IFX_int32_t MEI_InternalAutoMsgCtlSet(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              IOCTL_MEI_autoMsgCtrl_t *pArgAutoMsgCtrl);

/**
   Return the current setup of the autonomous message handling (device + driver)
   (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \param pArgAutoMsgCtrl
      ioctl struct for autonomous msg config

   \remarks
      This function should be only used within driver layer, internal access
      context.
      Via this command you can also do the NFC Enable/Disable.
*/
extern IFX_int32_t MEI_InternalAutoMsgCtlGet(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              IOCTL_MEI_autoMsgCtrl_t *pArgAutoMsgCtrl);

/**
   Write a CMD message to the device (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgMsg
      points to the user message information data.

   \return
      Success: Number of written bytes (message size: header + payload)
      Error:
         -e_MEI_ERR_MSG_PARAM: invalid message parameter
         -e_MEI_ERR_GET_ARG:   cannot get arguments

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalCmdMsgWrite(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_message_t   *pArgMsg);
/**
   Check for a available ack message and read (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgMsg
      points to the user message information data.

   \return
      Success: Number of read payload bytes
      Error:
         -e_MEI_ERR_RETURN_ARG:     cannot return arguments
         -e_MEI_ERR_DEV_NEG_RESP:   negative acknowledge
         -e_MEI_ERR_DEV_INVAL_RESP: invalid response.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalAckMsgRead(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_message_t   *pArgMsg);

/**
   Write a CMD message to the device and check for the corresponding ack
   (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgMsgs
      Points to the return struct. Also refer to details of according structure
      that is used.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalMsgSend(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t *pArgMsgs);

/**
   This function triggers to send a specified message which requests data from
   the local device and waits for appropriate answer.

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param nMsgID
      Specifies the message ID as defined in the VRX firmware
      message specification. It includes the message type and
      subtype.

   \param nLength
      Number of bytes of the message payload.

   \param nData
      Pointer to the message payload data.

   \param nLenAck
      Available buffer size for received ack.

   \param pDataAck
      Pointer to buffer for receiving ack message.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.
*/
extern IFX_int32_t MEI_InternalSendMessage(
                              MEI_DYN_CNTRL_T      *pMeiDynCntrl,
                              const IFX_uint16_t   nMsgID,
                              const IFX_uint16_t   nLength,
                              const IFX_uint8_t    *pData,
                              const IFX_uint16_t   nLenAck,
                              IFX_uint8_t          *pDataAck);

/**
   Check for a available autonomous (NFC) message and read.
   (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgMsg
      Points to the return struct. Also refer to details of according structure
      that is used.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
*/
extern IFX_int32_t MEI_InternalNfcMsgRead(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_message_t   *pArgMsg);

#if (MEI_SUPPORT_DSM == 1)
/**
   Specifies the callback function type that has to be used for registering (by
   PP driver) and signaling of interrupts (by the MEI Driver) for G.Vector
   related ERB ([E]rror [R]eported [B]lock) data availability.

   \param p_error_vector
      Pointer to base address of the memory that is allocated by MEI driver
      within SDRAM and shared between DSL Subsystem (Firmware and Software) and
      PP Subsystem (Software).
      The data consists of one 32 bit "err_vec_size" value followed by one
      complete Ethernet frame (ERB data plus header) according G.993.5 (refer
      to G.993.5, 04/2010, "Figure 7-9: Format of the Ethernet encapsulation of
      backchannel data message") but without FCS, which will be added by the
      PP related handling.
      The "err_vec_size" value gives the number of bytes for the complete
      Ethernet frame excluding the "err_vec_size" size (4 bytes) itself.
      This data is provided/written by the DSL Firmware. In case of ERB data
      exceeds 1019 bytes (max. "Protocol Payload Data" is defined as 1024 byte,
      which is a restriction that is of relevance for transmitting it
      alternatively via the EOC) the needed segmentation has to be done by the
      PP related handling, means segment the ERB data to fit it within multiple
      of 1019 bytes blocks and embedding them within multiple frames adjusting
      the "LENGTH" as well as the SEGMENT CODE" of each Ethernet frame
      accordingly.

   \return
      0 or positive if successful, negative value if an error has been occurred
      within context of G.Vector related handling of the PP driver.
      \todo Check if dedicated error codes should be defined from PP driver
            point of view.
*/
typedef IFX_int32_t (*mei_dsm_cb_func_t) (IFX_uint32_t *p_error_vector);

#endif /* (MEI_SUPPORT_DSM == 1) */

#if (MEI_DRV_ATM_OAM_ENABLE == 1)

/**
   Setup of the ATM OAM feature (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgAtmOamInit
      points to the ATM OAM init information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      For more information please refer to corresponding ioctl cmd
      \ref FIO_MEI_ATMOAM_INIT
      (see: MEI driver interface file "drv_mei_cpe_interface.h")
*/
extern IFX_int32_t MEI_InternalAtmOamInit(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_init_t  *pArgAtmOamInit);

/**
   Enable the ATM OAM on the device (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgAtmOamCntrl
      points to the ATM OAM control information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      For more information please refer to corresponding ioctl cmd
      \ref FIO_MEI_ATMOAM_CNTRL
      (see: MEI driver interface file "drv_mei_cpe_interface.h")
*/
extern IFX_int32_t MEI_InternalAtmOamCntrl(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_cntrl_t *pArgAtmOamCntrl);

/**
   Request the ATM OAM service counter from the device (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgAtmOamCounter
      points to the ATM OAM counter information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      For more information please refer to corresponding ioctl cmd
      \ref FIO_MEI_ATMOAM_REQ_DEV_COUNTER
      (see: MEI driver interface file "drv_mei_cpe_interface.h")
*/
extern IFX_int32_t MEI_InternalAtmOamCounterGet(
                              MEI_DYN_CNTRL_T            *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_counter_t *pArgAtmOamCounter);

/**
   Request the ATM OAM status from the driver (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgAtmOamStatus#
      points to the ATM OAM driver status information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      For more information please refer to corresponding ioctl cmd
      \ref FIO_MEI_ATMOAM_REQ_DRV_STATUS
      (see: MEI driver interface file "drv_mei_cpe_interface.h")
*/
extern IFX_int32_t MEI_InternalAtmOamStatusGet(
                              MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_status_t *pArgAtmOamStatus);

/**
   Insert ATM OAM cells to the device (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgAtmOamCells
      points to the ATM OAM cell information.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      Via this command you can also do the NFC Enable/Disable.
      For more information please refer to corresponding ioctl cmd
      \ref FIO_MEI_ATMOAM_CELL_INSERT
      (see: MEI driver interface file "drv_mei_cpe_interface.h")
*/
extern IFX_int32_t MEI_InternalAtmOamCellInsert(
                              MEI_DYN_CNTRL_T                *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_drvAtmCells_t *pArgAtmOamCells);

#endif      /* #if (MEI_DRV_ATM_OAM_ENABLE == 1) */


#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)

/**
   Setup of the ClearEOC feature (driver layer internal)

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgCEocInit
      points to the Clear EOC init information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      For more information please refer to corresponding ioctl cmd
      \ref FIO_MEI_CEOC_INIT
      (see: MEI driver interface file "drv_mei_cpe_interface.h")
*/
extern IFX_int32_t MEI_InternalCEocInit(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_init_t  *pArgCEocInit);

/**
   Setup and configure the vrx device for the ClearEOC (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgCEocCntrl
      Points to the Clear EOC control information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      For more information please refer to corresponding ioctl cmd
      \ref FIO_MEI_CEOC_CNTRL
      (see: MEI driver interface file "drv_mei_cpe_interface.h")
*/
extern IFX_int32_t MEI_InternalCEocCntrl(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_cntrl_t *pArgCEocCntrl);

/**
   Request the Clear EOC statistics form the driver (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgCEocStats
      Points to the Clear EOC control information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      For more information please refer to corresponding ioctl cmd
      \ref FIO_MEI_CEOC_STATS
      (see: MEI driver interface file "drv_mei_cpe_interface.h")
*/
extern IFX_int32_t MEI_InternalCEocStats(
                              MEI_DYN_CNTRL_T            *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_statistic_t *pArgCEocStats);

/**
   Clear EOC frame write (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgCEocFrame
      Points to the Clear EOC control information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      For more information please refer to corresponding ioctl cmd
      \ref FIO_MEI_CEOC_FRAME_WR
      (see: MEI driver interface file "drv_mei_cpe_interface.h")
*/
extern IFX_int32_t MEI_InternalCEocFrameWr(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_frame_t *pArgCEocFrame);

/**
   Clear EOC frame read (driver layer internal).

   \param pMeiDynCntrl
      Pointer to the dynamic MEI control structure.

   \param pArgCEocFrame
      Points to the Clear EOC control information data.

   \return
      0 or positive if successful, negative value if an error has been occurred.
      In case of an error please refer to the definition of MEI_ERROR_CODES
      for details.

   \remarks
      This function should be only used within driver layer, internal access
      context.
      For more information please refer to corresponding ioctl cmd
      \ref FIO_MEI_CEOC_FRAME_RD
      (see: MEI driver interface file "drv_mei_cpe_interface.h")
*/
extern IFX_int32_t MEI_InternalCEocFrameRd(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_frame_t *pArgCEocFrame);

#endif      /* #if (MEI_DRV_CLEAR_EOC_ENABLE == 1) */

#endif      /* #if (MEI_EXPORT_INTERNAL_API == 1) */

#endif      /* #ifndef _DRV_MEI_CPE_API_INTERN_H */

/** @} MEI_COMMON_INTERN */

