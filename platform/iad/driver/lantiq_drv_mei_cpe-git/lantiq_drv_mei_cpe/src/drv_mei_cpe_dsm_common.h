/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _DRV_MEI_CPE_DSM_COMMON_H
#define _DRV_MEI_CPE_DSM_COMMON_H

/* ==========================================================================
   Description : Common DSM Vectoring definitions.
   ========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

/* get config */
#include "drv_mei_cpe_config.h"

/* ============================================================================
   Inlcudes
   ========================================================================= */

#include "ifx_types.h"
#include "drv_mei_cpe_api.h"

#define MEI_DSM_VECTOR_ERB_SIZE_BYTE       (64*1024)

/**
   Defines vectoring support level by firmware
*/
typedef enum
{
   /** firmware do not support vectoring */
   e_MEI_DSM_VECTOR_FW_SUPPORT_MODE_NONE   = 0,
   /** firmware support vectoring partly */
   e_MEI_DSM_VECTOR_FW_SUPPORT_MODE_REDUCE,
   /** firmware support vectoring almost */
   e_MEI_DSM_VECTOR_FW_SUPPORT_MODE_FULL
} MEI_DSM_VECTOR_FW_SUPPORT_MODE_E;

#if MEI_SUPPORT_DEVICE_VR11 != 1
/** Error Reported Block - dynamic buffer */
typedef struct
{
   /** Error Reported Block virtual address */
   IFX_uint8_t *pERB_virt;

   /** Error Reported Block physical address */
   IFX_uint8_t *pERB_phy;

   /** Error Reported Block length in bytes   */
   IFX_uint32_t nERBsize_byte;

   /** PP driver call back function */
   mei_dsm_cb_func_t pCallBackFunc;
} MEI_DSM_VECTOR_DYN_ERB;
#endif /* (MEI_SUPPORT_DEVICE_VR11 != 1) */

/** Message ID for CMD_DSM_Control */
#define CMD_DSM_CONTROL 0x5248

/**
   Enables/Disables support for full vectoring (G.993.5) and full vector-
   friendly operation (G.993.2 Annex O). In case of ADSL, this only means the
   indication of the (VDSL) vectoring capabilities during G.Handshake.
*/
typedef struct CMD_DSM_Control CMD_DSM_Control_t;

/** Message ID for ACK_DSM_Control */
#define ACK_DSM_CONTROL 0x5248

/**
   Acknowledgement to CMD_DSM_Control.
*/
typedef struct ACK_DSM_Control ACK_DSM_Control_t;

/**
   Enables/Disables support for full vectoring (G.993.5) and full vector-
   friendly operation (G.993.2 Annex O). In case of ADSL, this only means the
   indication of the (VDSL) vectoring capabilities during G.Handshake.
*/
struct CMD_DSM_Control
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Reserved */
   IFX_uint16_t Res0 : 14;
   /** Supported Vectoring Mode */
   IFX_uint16_t VectoringMode : 2;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Supported Vectoring Mode */
   IFX_uint16_t VectoringMode : 2;
   /** Reserved */
   IFX_uint16_t Res0 : 14;
#endif
} __PACKED__ ;

/**
   Acknowledgement to CMD_DSM_Control.
*/
struct ACK_DSM_Control
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#endif
} __PACKED__ ;

/** Message ID for CMD_MAC_FrameConfigure */
#define CMD_MAC_FRAMECONFIGURE 0x5348

/**
   The message configures Ethernet properties, currently the MAC address of the
   device only.The MAC address is needed in the FW as "MAC source address"for
   PDUs not built by the host, like e.g. for the Ethernet encapsulated
   Backchannel Data ERB in Vectoring.
*/
typedef struct CMD_MAC_FrameConfigure CMD_MAC_FrameConfigure_t;

/** Message ID for ACK_MAC_FrameConfigure */
#define ACK_MAC_FRAMECONFIGURE 0x5348

/**
   Acknowledgement to CMD_MAC_FrameConfigure.
*/
typedef struct ACK_MAC_FrameConfigure ACK_MAC_FrameConfigure_t;

/**
   The message configures Ethernet properties, currently the MAC address of the
   device only.The MAC address is needed in the FW as "MAC source address"for
   PDUs not built by the host, like e.g. for the Ethernet encapsulated
   Backchannel Data ERB in Vectoring.
*/
struct CMD_MAC_FrameConfigure
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Source MAC Address Octets 0 to 1 */
   IFX_uint16_t SrcMacAddrB0_1;
   /** Source MAC Address Octets 2 to 3 */
   IFX_uint16_t SrcMacAddrB2_3;
   /** Source MAC Address Octets 4 to 5 */
   IFX_uint16_t SrcMacAddrB4_5;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Source MAC Address Octets 0 to 1 */
   IFX_uint16_t SrcMacAddrB0_1;
   /** Source MAC Address Octets 2 to 3 */
   IFX_uint16_t SrcMacAddrB2_3;
   /** Source MAC Address Octets 4 to 5 */
   IFX_uint16_t SrcMacAddrB4_5;
#endif
} __PACKED__ ;

/**
   Acknowledgement to CMD_MAC_FrameConfigure.
*/
struct ACK_MAC_FrameConfigure
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#endif
} __PACKED__ ;

/** Message ID for EVT_DSM_ErrorVectorReady */
#define EVT_DSM_ERRORVECTORREADY 0x1109

/**
   This autononmous message indicates that a new downstream DSM error vector was
   written by the DSL FW into the SDRAM. Generation of this EVT message is
   enabled/disabled together with the G.993.5 vectoring functionality itself, by
   means of CMD_DSM_Control, parameter "Vector".
*/
typedef struct EVT_DSM_ErrorVectorReady EVT_DSM_ErrorVectorReady_t;

/**
   This autononmous message indicates that a new downstream DSM error vector was
   written by the DSL FW into the SDRAM. Generation of this EVT message is
   enabled/disabled together with the G.993.5 vectoring functionality itself, by
   means of CMD_DSM_Control, parameter "Vector".
*/
struct EVT_DSM_ErrorVectorReady
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** ERB Event Result Code */
   IFX_uint16_t ErrVecProcResult;
   /** L2 Backchannel Error Vector Date Size */
   IFX_uint16_t ErrVecSize;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** ERB Event Result Code */
   IFX_uint16_t ErrVecProcResult;
   /** L2 Backchannel Error Vector Date Size */
   IFX_uint16_t ErrVecSize;
#endif
} __PACKED__ ;

/** Message ID for CMD_DSM_StatsGet */
#define CMD_DSM_STATSGET 0x370A

/**
   Requests vectoring debug counter values for the number of discarded error
   vector packets. (It increments when the error vector data was not processed
   by the PP driver before being overwritten by the DSL FW with the next data).
*/
typedef struct CMD_DSM_StatsGet CMD_DSM_StatsGet_t;

/** Message ID for ACK_DSM_StatsGet */
#define ACK_DSM_STATSGET 0x370A

/**
   Delivers vectoring debug counter values: the number of discarded error vector
   packets. (It increments when the error vector data was not processed by the
   PP driver before being overwritten by the DSL FW with the next data). It is a
   wrap-around counter which is not affected by the TR1-period and only reset on
   FW download. Recognition of the counted event: If the error vector data was
   not processed by the PP driver, then the first 32-bit value [size] of the
   error vector information in the SDRAM is NOT set to zero on processing the
   next error vector by the DSL FW.
*/
typedef struct ACK_DSM_StatsGet ACK_DSM_StatsGet_t;

/**
   Requests vectoring debug counter values for the number of discarded error
   vector packets. (It increments when the error vector data was not processed
   by the PP driver before being overwritten by the DSL FW with the next data).
*/
struct CMD_DSM_StatsGet
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#endif
} __PACKED__ ;

/**
   Delivers vectoring debug counter values: the number of discarded error vector
   packets. (It increments when the error vector data was not processed by the
   PP driver before being overwritten by the DSL FW with the next data). It is a
   wrap-around counter which is not affected by the TR1-period and only reset on
   FW download. Recognition of the counted event: If the error vector data was
   not processed by the PP driver, then the first 32-bit value [size] of the
   error vector information in the SDRAM is NOT set to zero on processing the
   next error vector by the DSL FW.
*/
struct ACK_DSM_StatsGet
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Discarded Error Vectors, LSW */
   IFX_uint16_t ErrVecDiscard_LSW;
   /** Discarded Error Vectors, MSW */
   IFX_uint16_t ErrVecDiscard_MSW;
   /** Transmitted Error Vectors, LSW */
   IFX_uint16_t ErrVecTransmitted_LSW;
   /** Transmitted Error Vectors, MSW */
   IFX_uint16_t ErrVecTransmitted_MSW;
   /** Total Error Vectors, LSW */
   IFX_uint16_t ErrVecTotal_LSW;
   /** Total Error Vectors, MSW */
   IFX_uint16_t ErrVecTotal_MSW;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Discarded Error Vectors, LSW */
   IFX_uint16_t ErrVecDiscard_LSW;
   /** Discarded Error Vectors, MSW */
   IFX_uint16_t ErrVecDiscard_MSW;
   /** Transmitted Error Vectors, LSW */
   IFX_uint16_t ErrVecTransmitted_LSW;
   /** Transmitted Error Vectors, MSW */
   IFX_uint16_t ErrVecTransmitted_MSW;
   /** Total Error Vectors, LSW */
   IFX_uint16_t ErrVecTotal_LSW;
   /** Total Error Vectors, MSW */
   IFX_uint16_t ErrVecTotal_MSW;
#endif
} __PACKED__ ;

/** Message ID for CMD_HS_SelectedProfileVDSL2Get */
#define CMD_HS_SELECTEDPROFILEVDSL2GET 0xCD03

/**
   Requests the actual selected VDSL2 Profile (See G.994.1 Amendment 4 [10],
   NPAR(3) coding).
*/
typedef struct CMD_HS_SelectedProfileVDSL2Get CMD_HS_SelectedProfileVDSL2Get_t;

/** Message ID for ACK_HS_SelectedProfileVDSL2Get */
#define ACK_HS_SELECTEDPROFILEVDSL2GET 0xCD03

/**
   Provides the actual selected VDSL2 Profile as requested by
   CMD_HS_SelectedProfileVDSL2Get (see G.994.1 Amendment 4 [10], NPAR(3)
   coding).
*/
typedef struct ACK_HS_SelectedProfileVDSL2Get ACK_HS_SelectedProfileVDSL2Get_t;

/**
   Requests the actual selected VDSL2 Profile (See G.994.1 Amendment 4 [10],
   NPAR(3) coding).
*/
struct CMD_HS_SelectedProfileVDSL2Get
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t  Length;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t  Length;
#endif
} __PACKED__ ;


/**
   Provides the actual selected VDSL2 Profile as requested by
   CMD_HS_SelectedProfileVDSL2Get (see G.994.1 Amendment 4 [10], NPAR(3)
   coding).
*/
struct ACK_HS_SelectedProfileVDSL2Get
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t  Length;
   /** Reserved */
   IFX_uint16_t Res0 : 4;
   /** O-P-VECTOR1 Flag Tones Only, Selected Bit 11 */
   IFX_uint16_t dsmSel11 : 1;
   /** Extended O-P-VECTOR1,Selected Bit 10 */
   IFX_uint16_t dsmSel10 : 1;
   /** Upstream FDPS, Selected Bit 9 */
   IFX_uint16_t dsmSel9 : 1;
   /** Pilot Sequence Length Multiple of 4, Selected Bit 8 */
   IFX_uint16_t dsmSel8 : 1;
   /** Reserved */
   IFX_uint16_t Res1 : 4;
   /** G.993.2 Annex X, Selected Bit 3 */
   IFX_uint16_t dsmSel3 : 1;
   /** G.993.2 Annex Y, Selected Bit 2 */
   IFX_uint16_t dsmSel2 : 1;
   /** G.993.5 DS Vectoring, Selected Bit 1 */
   IFX_uint16_t dsmSel1 : 1;
   /** G.993.5 DS+US Vectoring, Selected Bit 0 */
   IFX_uint16_t dsmSel0 : 1;
   /** Reserved */
   IFX_uint16_t Res2 : 7;
   /** 35b, Profile Selected Bit 8 */
   IFX_uint16_t profileSel8 : 1;
   /** 30a, Profile Selected Bit 7 */
   IFX_uint16_t profileSel7 : 1;
   /** 17a, Profile Selected Bit 6 */
   IFX_uint16_t profileSel6 : 1;
   /** 12b, Profile Selected Bit 5 */
   IFX_uint16_t profileSel5 : 1;
   /** 12a, Profile Selected Bit 4 */
   IFX_uint16_t profileSel4 : 1;
   /** 8d, Profile Selected Bit 3 */
   IFX_uint16_t profileSel3 : 1;
   /** 8c, Profile Selected Bit 2 */
   IFX_uint16_t profileSel2 : 1;
   /** 8b, Profile Selected Bit 1 */
   IFX_uint16_t profileSel1 : 1;
   /** 8a, Profile Selected Bit 0 */
   IFX_uint16_t profileSel0 : 1;
   /** ADLU-56, Annex A US0 PSDs Selected Bit  15 */
   IFX_uint16_t A_US0PsdSel15 : 1;
   /** ADLU-52, Annex A US0 PSDs Selected Bit  14 */
   IFX_uint16_t A_US0PsdSel14 : 1;
   /** ADLU-48, Annex A US0 PSDs Selected Bit  13 */
   IFX_uint16_t A_US0PsdSel13 : 1;
   /** ADLU-44, Annex A US0 PSDs Selected Bit  12 */
   IFX_uint16_t A_US0PsdSel12 : 1;
   /** ADLU-40, Annex A US0 PSDs Selected Bit  11 */
   IFX_uint16_t A_US0PsdSel11 : 1;
   /** ADLU-36, Annex A US0 PSDs Selected Bit  10 */
   IFX_uint16_t A_US0PsdSel10 : 1;
   /** ADLU-32, Annex A US0 PSDs Selected Bit  9 */
   IFX_uint16_t A_US0PsdSel9 : 1;
   /** EU-64, Annex A US0 PSDs Selected Bit  8 */
   IFX_uint16_t A_US0PsdSel8 : 1;
   /** EU-60, Annex A US0 PSDs Selected Bit  7 */
   IFX_uint16_t A_US0PsdSel7 : 1;
   /** EU-56, Annex A US0 PSDs Selected Bit  6 */
   IFX_uint16_t A_US0PsdSel6 : 1;
   /** EU-52, Annex A US0 PSDs Selected Bit  5 */
   IFX_uint16_t A_US0PsdSel5 : 1;
   /** EU-48, Annex A US0 PSDs Selected Bit  4 */
   IFX_uint16_t A_US0PsdSel4 : 1;
   /** EU-44, Annex A US0 PSDs Selected Bit  3 */
   IFX_uint16_t A_US0PsdSel3 : 1;
   /** EU-40, Annex A US0 PSDs Selected Bit  2 */
   IFX_uint16_t A_US0PsdSel2 : 1;
   /** EU-36, Annex A US0 PSDs Selected Bit  1 */
   IFX_uint16_t A_US0PsdSel1 : 1;
   /** EU-32, Annex A US0 PSDs Selected Bit  0 */
   IFX_uint16_t A_US0PsdSel0 : 1;
   /** Reserved */
   IFX_uint16_t Res3 : 10;
   /** ADLU-128 Annex A US0 PSDs Selected Bit 21 */
   IFX_uint16_t A_US0PsdSel21 : 1;
   /** EU-128, Annex A US0 PSDs Selected Bit 20 */
   IFX_uint16_t A_US0PsdSel20 : 1;
   /** Reserved */
   IFX_uint16_t Res4 : 2;
   /** ADLU-64, Annex A US0 PSDs Selected Bit 17 */
   IFX_uint16_t A_US0PsdSel17 : 1;
   /** ADLU-60, Annex A US0 PSDs Selected Bit 16 */
   IFX_uint16_t A_US0PsdSel16 : 1;
   /** Reserved */
   IFX_uint16_t Res5 : 13;
   /** US0 In 120 to 276 kHz, Annex B US0 PSDs Selected Bit 2 */
   IFX_uint16_t B_US0PsdSel2 : 1;
   /** US0 In 25 to 276 kHz, Annex B US0 PSDs Selected Bit 1 */
   IFX_uint16_t B_US0PsdSel1 : 1;
   /** US0 In 25 to 138 kHz, Annex B US0 PSDs Selected Bit 0 */
   IFX_uint16_t B_US0PsdSel0 : 1;
   /** Reserved */
   IFX_uint16_t Res6 : 10;
   /** US0 In 25 to 276 kHz, Annex C US0 PSDs Selected Bit 5 */
   IFX_uint16_t C_US0PsdSel5 : 1;
   /** US0 In 25 to 138 kHz, Annex C US0 PSDs Selected Bit 4 */
   IFX_uint16_t C_US0PsdSel4 : 1;
   /** Reserved */
   IFX_uint16_t Res7 : 2;
   /** US0 In 25 to 276kHz, Annex C US0 PSDs Selected Bit 1 */
   IFX_uint16_t C_US0PsdSel1 : 1;
   /** US0 In 25 to 138 kHz, Annex C US0 PSDs Selected Bit 0 */
   IFX_uint16_t C_US0PsdSel0 : 1;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t  Length;
   /** G.993.5 DS+US Vectoring, Selected Bit 0 */
   IFX_uint16_t dsmSel0 : 1;
   /** G.993.5 DS Vectoring, Selected Bit 1 */
   IFX_uint16_t dsmSel1 : 1;
   /** G.993.2 Annex Y, Selected Bit 2 */
   IFX_uint16_t dsmSel2 : 1;
   /** G.993.2 Annex X, Selected Bit 3 */
   IFX_uint16_t dsmSel3 : 1;
   /** Reserved */
   IFX_uint16_t Res1 : 4;
   /** Pilot Sequence Length Multiple of 4, Selected Bit 8 */
   IFX_uint16_t dsmSel8 : 1;
   /** Upstream FDPS, Selected Bit 9 */
   IFX_uint16_t dsmSel9 : 1;
   /** Extended O-P-VECTOR1,Selected Bit 10 */
   IFX_uint16_t dsmSel10 : 1;
   /** O-P-VECTOR1 Flag Tones Only, Selected Bit 11 */
   IFX_uint16_t dsmSel11 : 1;
   /** Reserved */
   IFX_uint16_t Res0 : 4;
   /** 8a, Profile Selected Bit 0 */
   IFX_uint16_t profileSel0 : 1;
   /** 8b, Profile Selected Bit 1 */
   IFX_uint16_t profileSel1 : 1;
   /** 8c, Profile Selected Bit 2 */
   IFX_uint16_t profileSel2 : 1;
   /** 8d, Profile Selected Bit 3 */
   IFX_uint16_t profileSel3 : 1;
   /** 12a, Profile Selected Bit 4 */
   IFX_uint16_t profileSel4 : 1;
   /** 12b, Profile Selected Bit 5 */
   IFX_uint16_t profileSel5 : 1;
   /** 17a, Profile Selected Bit 6 */
   IFX_uint16_t profileSel6 : 1;
   /** 30a, Profile Selected Bit 7 */
   IFX_uint16_t profileSel7 : 1;
   /** 35b, Profile Selected Bit 8 */
   IFX_uint16_t profileSel8 : 1;
   /** Reserved */
   IFX_uint16_t Res2 : 7;
   /** EU-32, Annex A US0 PSDs Selected Bit  0 */
   IFX_uint16_t A_US0PsdSel0 : 1;
   /** EU-36, Annex A US0 PSDs Selected Bit  1 */
   IFX_uint16_t A_US0PsdSel1 : 1;
   /** EU-40, Annex A US0 PSDs Selected Bit  2 */
   IFX_uint16_t A_US0PsdSel2 : 1;
   /** EU-44, Annex A US0 PSDs Selected Bit  3 */
   IFX_uint16_t A_US0PsdSel3 : 1;
   /** EU-48, Annex A US0 PSDs Selected Bit  4 */
   IFX_uint16_t A_US0PsdSel4 : 1;
   /** EU-52, Annex A US0 PSDs Selected Bit  5 */
   IFX_uint16_t A_US0PsdSel5 : 1;
   /** EU-56, Annex A US0 PSDs Selected Bit  6 */
   IFX_uint16_t A_US0PsdSel6 : 1;
   /** EU-60, Annex A US0 PSDs Selected Bit  7 */
   IFX_uint16_t A_US0PsdSel7 : 1;
   /** EU-64, Annex A US0 PSDs Selected Bit  8 */
   IFX_uint16_t A_US0PsdSel8 : 1;
   /** ADLU-32, Annex A US0 PSDs Selected Bit  9 */
   IFX_uint16_t A_US0PsdSel9 : 1;
   /** ADLU-36, Annex A US0 PSDs Selected Bit  10 */
   IFX_uint16_t A_US0PsdSel10 : 1;
   /** ADLU-40, Annex A US0 PSDs Selected Bit  11 */
   IFX_uint16_t A_US0PsdSel11 : 1;
   /** ADLU-44, Annex A US0 PSDs Selected Bit  12 */
   IFX_uint16_t A_US0PsdSel12 : 1;
   /** ADLU-48, Annex A US0 PSDs Selected Bit  13 */
   IFX_uint16_t A_US0PsdSel13 : 1;
   /** ADLU-52, Annex A US0 PSDs Selected Bit  14 */
   IFX_uint16_t A_US0PsdSel14 : 1;
   /** ADLU-56, Annex A US0 PSDs Selected Bit  15 */
   IFX_uint16_t A_US0PsdSel15 : 1;
   /** ADLU-60, Annex A US0 PSDs Selected Bit 16 */
   IFX_uint16_t A_US0PsdSel16 : 1;
   /** ADLU-64, Annex A US0 PSDs Selected Bit 17 */
   IFX_uint16_t A_US0PsdSel17 : 1;
   /** Reserved */
   IFX_uint16_t Res4 : 2;
   /** EU-128, Annex A US0 PSDs Selected Bit 20 */
   IFX_uint16_t A_US0PsdSel20 : 1;
   /** ADLU-128 Annex A US0 PSDs Selected Bit 21 */
   IFX_uint16_t A_US0PsdSel21 : 1;
   /** Reserved */
   IFX_uint16_t Res3 : 10;
   /** US0 In 25 to 138 kHz, Annex B US0 PSDs Selected Bit 0 */
   IFX_uint16_t B_US0PsdSel0 : 1;
   /** US0 In 25 to 276 kHz, Annex B US0 PSDs Selected Bit 1 */
   IFX_uint16_t B_US0PsdSel1 : 1;
   /** US0 In 120 to 276 kHz, Annex B US0 PSDs Selected Bit 2 */
   IFX_uint16_t B_US0PsdSel2 : 1;
   /** Reserved */
   IFX_uint16_t Res5 : 13;
   /** US0 In 25 to 138 kHz, Annex C US0 PSDs Selected Bit 0 */
   IFX_uint16_t C_US0PsdSel0 : 1;
   /** US0 In 25 to 276kHz, Annex C US0 PSDs Selected Bit 1 */
   IFX_uint16_t C_US0PsdSel1 : 1;
   /** Reserved */
   IFX_uint16_t Res7 : 2;
   /** US0 In 25 to 138 kHz, Annex C US0 PSDs Selected Bit 4 */
   IFX_uint16_t C_US0PsdSel4 : 1;
   /** US0 In 25 to 276 kHz, Annex C US0 PSDs Selected Bit 5 */
   IFX_uint16_t C_US0PsdSel5 : 1;
   /** Reserved */
   IFX_uint16_t Res6 : 10;
#endif
} __PACKED__ ;

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif   /* #ifndef _DRV_MEI_CPE_DSM_COMMON_H */


