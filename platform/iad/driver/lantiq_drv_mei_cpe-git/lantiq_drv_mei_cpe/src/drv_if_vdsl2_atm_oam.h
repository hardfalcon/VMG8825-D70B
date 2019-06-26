/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _DRV_IF_VDSL2_ATM_OAM_H_
#define _DRV_IF_VDSL2_ATM_OAM_H_
/** \file
   ATM OAM message definitions for VRX Rev 1.
*/

#if defined (__GNUC__) || defined (__GNUG__)
   /* GNU C or C++ compiler */
   #define __PACKED__ __attribute__ ((packed))
#else
   /* Byte alignment adjustment */
   #pragma pack(2)
   #if !defined (_PACKED_)
      #define __PACKED__      /* nothing */
   #endif
#endif


/** @defgroup _DRV_IF_VDSL2_ATM_OAM_H_ */

/** @{ */

#ifdef __cplusplus
   extern "C" {
#endif

#include "ifx_types.h"

/** Defines for ACK_AtmCellLineInsertStatusGet */
#define ACK_ATMCELLLINEINSERTSTATUSGET_INSERT_PROG      0x1
#define ACK_ATMCELLLINEINSERTSTATUSGET_INSERT_DONE      0x2
#define ACK_ATMCELLLINEINSERTSTATUSGET_INSERT_ERR       0x3

/** Defines for EVT_AtmCellLineInsertStatusGet */
#define EVT_ATMCELLLINEINSERTSTATUSGET_INSERT_DONE      0x2
#define EVT_ATMCELLLINEINSERTSTATUSGET_INSERT_ERR       0x3

/** Message ID for CMD_AtmInsertExtract_Control */
#define CMD_ATMINSERTEXTRACT_CONTROL            0x5051
typedef struct CMD_AtmInsertExtract_Control     CMD_AtmInsertExtract_Control_t;

/** Message ID for ACK_AtmInsertExtract_Control */
#define ACK_ATMINSERTEXTRACT_CONTROL            0x5051
typedef struct ACK_AtmInsertExtract_Control     ACK_AtmInsertExtract_Control_t;

/** Message ID for CMD_AtmCellLineInsert */
#define CMD_ATMCELLLINEINSERT                   0x5151
typedef struct CMD_AtmCellLineInsert            CMD_AtmCellLineInsert_t;

/** Message ID for ACK_AtmCellLineInsert */
#define ACK_ATMCELLLINEINSERT                   0x5151
typedef struct ACK_AtmCellLineInsert            ACK_AtmCellLineInsert_t;

/** Message ID for CMD_AtmCellLineInsertStatusGet */
#define CMD_ATMCELLLINEINSERTSTATUSGET          0x5511
typedef struct CMD_AtmCellLineInsertStatusGet   CMD_AtmCellLineInsertStatusGet_t;

/** Message ID for ACK_AtmCellLineInsertStatusGet */
#define ACK_ATMCELLLINEINSERTSTATUSGET          0x5511
typedef struct ACK_AtmCellLineInsertStatusGet   ACK_AtmCellLineInsertStatusGet_t;

/** Message ID for EVT_AtmCellLineInsertStatusGet */
#define EVT_ATMCELLLINEINSERTSTATUSGET          0x5511
typedef struct EVT_AtmCellLineInsertStatusGet   EVT_AtmCellLineInsertStatusGet_t;

/** Message ID for EVT_AtmCellLineExtract */
#define EVT_ATMCELLLINEEXTRACT                  0x5211
typedef struct EVT_AtmCellLineExtract           EVT_AtmCellLineExtract_t;

/** Message ID for ALM_AtmCellExtractFailed */
#define ALM_ATMCELLEXTRACTFAILED                0x5311
typedef struct ALM_AtmCellExtractFailed         ALM_AtmCellExtractFailed_t;

/** Message ID for CMD_AtmInsertExtractStatsGet */
#define CMD_ATMINSERTEXTRACTSTATSGET            0x5411
typedef struct CMD_AtmInsertExtractStatsGet     CMD_AtmInsertExtractStatsGet_t;

/** Message ID for ACK_AtmInsertExtractStatsGet */
#define ACK_ATMINSERTEXTRACTSTATSGET            0x5411
typedef struct ACK_AtmInsertExtractStatsGet     ACK_AtmInsertExtractStatsGet_t;


/**
   ATM cell - type
*/
typedef struct MEI_ATMcell MEI_ATMcell_t;
struct MEI_ATMcell
{
   IFX_uint8_t ATMcell[53];
   IFX_uint8_t Res[3];
} __PACKED__ ;


/** Message CMD_AtmInsertExtract_Control */
struct CMD_AtmInsertExtract_Control
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
   /* Parameter 2 */
   IFX_uint32_t LinkNo;
   /* Parameter 3 */
   IFX_uint32_t Control;
   /* Parameter 4 */
   IFX_uint32_t direction;
   /* Parameter 5 */
   IFX_uint32_t failMsgControl;
   /* Parameter 6 */
   IFX_uint32_t insertEVT_Status;
} __PACKED__ ;

/** Message ACK_AtmInsertExtract_Control */
struct ACK_AtmInsertExtract_Control
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
} __PACKED__ ;


/** Message CMD_AtmCellLineInsert */
struct CMD_AtmCellLineInsert
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
   /* Parameter 2 */
   IFX_uint32_t LinkNo;
   /* Parameter 3 to (Length +1) */
   MEI_ATMcell_t ATMCells[4];

} __PACKED__ ;

/** Message ACK_AtmCellLineInsert */
struct ACK_AtmCellLineInsert
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
} __PACKED__ ;


/** Message CMD_AtmCellLineInsertStatusGet */
struct CMD_AtmCellLineInsertStatusGet
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
   /* Parameter 2 */
   IFX_uint32_t LinkNo;
} __PACKED__ ;

/** Message ACK_AtmCellLineInsertStatusGet */
struct ACK_AtmCellLineInsertStatusGet
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
   /* Parameter 2 */
   IFX_uint32_t LinkNo;
   /* Parameter 3 */
   IFX_uint32_t insertStatus;
} __PACKED__ ;

/** Message EVT_AtmCellLineInsertStatusGet */
struct EVT_AtmCellLineInsertStatusGet
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
   /* Parameter 2 */
   IFX_uint32_t LinkNo;
   /* Parameter 3 */
   IFX_uint32_t insertStatus;
} __PACKED__ ;


/** Message EVT_AtmCellLineExtract */
struct EVT_AtmCellLineExtract
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
   /* Parameter 2 */
   IFX_uint32_t LinkNo;
   /* Parameter 3 */
   IFX_uint32_t Reserved00;
   /* Parameter 4 to (Length +1) */
   MEI_ATMcell_t  ATMcells[4];
} __PACKED__ ;


/** Message ALM_AtmCellExtractFailed */
struct ALM_AtmCellExtractFailed
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
   /* Parameter 2 */
   IFX_uint32_t LinkNo;
   /* Parameter 3 */
   IFX_uint32_t direction;
   /* Parameter 4 */
   IFX_uint32_t FailCount;
} __PACKED__ ;


/** Message CMD_AtmInsertExtractStatsGet */
struct CMD_AtmInsertExtractStatsGet
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
   /* Parameter 2 */
   IFX_uint32_t LinkNo;
   /* Parameter 3 */
   IFX_uint32_t direction;
} __PACKED__ ;

/** Message ACK_AtmInsertExtractStatsGet */
struct ACK_AtmInsertExtractStatsGet
{
   /* Parameter 0 */
   IFX_uint16_t Index;
   /* Parameter 1 */
   IFX_uint16_t Length;
   /* Parameter 2 */
   IFX_uint32_t LinkNo;
   /* Parameter 3 */
   IFX_uint32_t direction;
   /* Parameter 4 */
   IFX_uint32_t extractedCells;
   /* Parameter 5 */
   IFX_uint32_t failExtractCells;
   /* Parameter 6 */
   IFX_uint32_t Res00;
   /* Parameter 7 */
   IFX_uint32_t insertedCells;
} __PACKED__ ;

#ifdef __cplusplus
}
#endif

/** @} */
#endif      /* #ifndef _DRV_IF_VDSL2_ATM_OAM_H_ */

