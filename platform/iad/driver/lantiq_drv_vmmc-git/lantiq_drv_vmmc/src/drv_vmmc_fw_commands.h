#ifndef _DRV_VMMC_FW_COMMANDS_H
#define _DRV_VMMC_FW_COMMANDS_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_fw_commands.h
   This file contains the definitions of the common parts of the voice-FW
   command interface.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

/* ============================= */
/* Global Defines                */
/* ============================= */

#ifndef __PACKED__
   #if defined (__GNUC__) || defined (__GNUG__)
      /* GNU C or C++ compiler */
      #define __PACKED__ __attribute__ ((packed))
   #elif !defined (__PACKED__)
      #define __PACKED__      /* nothing */
   #endif
#endif

/* ============================= */
/* Global Types                  */
/* ============================= */

/** @defgroup _VMMC_FW_SPEC_COMMANDS_ Command Messages
 *  @{
 */

#ifdef __cplusplus
   extern "C" {
#endif

/* ----- Common Command Header ----- */
#define CMD_HDR_CNT 4

#define CMDREAD 1
#define CMDWRITE 0

#define CMD_ALI 1
#define CMD_DECT 3
#define CMD_SDD 4
#define CMD_EOP 6
#define CMD_RTCP_XR 7

/* Subcommand coding for ALI commands */
#define MOD_DCCTL 0

/* Subcommand coding for SDD commands */
#define MOD_SDD 0
#define MOD_SDD1 1

/* Subcommand coding for EOP commands */
#define MOD_PCM 0
#define MOD_ALI 1
#define MOD_SIGNALING 2
#define MOD_CODER 3
#define MOD_RESOURCE 6
#define MOD_SYSTEM 7

#define CMD_LEN 0x000000ff

#define CMD_HEAD_BE           \
   /** Read/Write */          \
   IFX_uint32_t RW : 1;       \
   /** Short Command */       \
   IFX_uint32_t SC : 1;       \
   /** Broadcast */           \
   IFX_uint32_t BC : 1;       \
   /** Command */             \
   IFX_uint32_t CMD : 5;      \
   /** Channel */             \
   IFX_uint32_t CHAN : 8;     \
   /** Mode of Subcommand */  \
   IFX_uint32_t MOD : 3;      \
   /** EDSP Command */        \
   IFX_uint32_t ECMD : 5;     \
   /** Length */              \
   IFX_uint32_t LENGTH : 8

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* _DRV_VMMC_FW_COMMANDS_H */
