#ifndef _MEI_CPE_mailbox_h
#define _MEI_CPE_mailbox_h
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VDLS2 Mailbox interface.
   ========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif
/* ============================================================================
   Includes
   ========================================================================= */

/* include the IFX type definitions */
#include "ifx_types.h"
/* get the driver configuration */
#include "drv_mei_cpe_config.h"

/* get the AWARE CMV message format */
#include "cmv_message_format.h"

#if (MEI_SUPPORT_ROM_CODE == 1)
/* get the boot loader message format */
#include "user_if_vdsl2_boot_messages.h"
#endif

/* ============================================================================
   Mailbox definiton
   ========================================================================= */

/**
   Identify a raw mailbox message.
   The CMV message header is already set.
*/
#define MEI_DEF_MSGTYPE_RAW      0x01

/**
   Identify a IFX message.
   Extract the CMV message header informations from the given message ID.
*/
#define MEI_DEF_MSGTYPE_IFX      0x00


/** indicates an IFX message */
#define MEI_MSGID_BIT_IND_IFX       (1 << 4)

/** indicates the message action: read (0) / write (1) */
#define MEI_MSGID_BIT_IND_WR_CMD    (1 << 6)


/* =============================================================
   ARC memory map + mailbox settings
   ========================================================== */

/**
   Location of the mailbox description table.
*/
#define MEI_MAILBOX_DESCR_TABLE_ADDR    0x40000
/*
   Boot Mailbox address within the ARC controller memory.
*/

/* TODO: Check the values below*/

/** Mailbox address within the ARC Bulk memory: ARC to ME */
#define MEI_BOOT_MAILBOX_ARC2ME_ADDR    0x40120
#define MEI_BOOT_MAILBOX_ARC2ME_LEN     0x14
/** Mailbox address within the ARC Bulk memory: ME to ARC */
#define MEI_BOOT_MAILBOX_ME2ARC_ADDR    0x40014
#define MEI_BOOT_MAILBOX_ME2ARC_LEN     0x14

extern IFX_uint32_t MEI_MailboxBase_ME2ARC;
#define MEI_MAILBOX_BASE_ME2ARC   MEI_MailboxBase_ME2ARC

extern IFX_uint32_t MEI_MailboxBase_ARC2ME;
#define MEI_MAILBOX_BASE_ARC2ME   MEI_MailboxBase_ARC2ME


/* ===================================================
   Local MEI Control Settings
*/

/* POLL count for debug done identification */
/* #define MEI_DBG_POLL_WAIT_COUNT  1000 */
#define MEI_MEI_DBG_POLL_WAIT_COUNT     1000

/* Timeout for poll mode [ms] of the MEI mailbox */
#define MEI_MIN_MAILBOX_POLL_TIME_MS    10

#if (MEI_EMULATION_CONFIGURATION == 1)
/* Interval cycle to print status for waiting on MODEM_READY */
#define MEI_MODEM_READY_STATUS_PRINT_MS   15000
#endif /* (MEI_EMULATION_CONFIGURATION == 1) */

/* ===================================================
   Mailbox Codes
   =================================================== */

/** ME to ARC message */
#define MEI_MBOX_CODE_MSG_WRITE     0x00
/** ARC to ME response */
#define MEI_MBOX_CODE_MSG_ACK       0x00
/** ARC to ME indication (NFC) */
#define MEI_MBOX_CODE_NFC_REQ       0x01
/** ARC to ME event (EVT) */
#define MEI_MBOX_CODE_EVT_REQ       0x02
/** ARC to ME event (ALM) */
#define MEI_MBOX_CODE_ALM_REQ       0x04
/** ARC to ME Debug (DBG) */
#define MEI_MBOX_CODE_DBG_REQ       0x08
/** Static Code Swap request */
#define MEI_MBOX_CODE_CS_STAT_REQ   0x10
/** Dynamic Code Swap request */
#define MEI_MBOX_CODE_CS_DYN_REQ    0x11

/** Fast Read request */
#define MEI_MBOX_CODE_FAST_RD_REQ   0x20
/** BOOT indication */
#define MEI_MBOX_CODE_BOOT          0x40


/* ===================================================
   Definition Mailbox description struct
   =================================================== */

/**
   Get the mailbox version from the mailbox descriptor table.
*/
#define MEI_MB_DESCR_TABLE_VERS_POS        0
#define MEI_MB_DESCR_TABLE_VERS_MASK       0x000F
#define MEI_MB_DESCR_TABLE_VERS_GET(descr_cntrl_off1) \
            ( ((descr_cntrl_off1) & MEI_MB_DESCR_TABLE_VERS_MASK) \
              >> MEI_MB_DESCR_TABLE_VERS_POS )

/**
   Get the number of mailboxes from the mailbox descriptor table.
*/
#define MEI_MB_DESCR_TABLE_NUM_OF_MB_POS   8
#define MEI_MB_DESCR_TABLE_NUM_OF_MB_MASK  0x0F00
#define MEI_MB_DESCR_TABLE_NUM_OF_MB_GET(descr_cntrl_off1) \
            ( ((descr_cntrl_off1) & MEI_MB_DESCR_TABLE_NUM_OF_MB_MASK) \
              >> MEI_MB_DESCR_TABLE_NUM_OF_MB_POS )

/**
   Get the descriptor size of an mailbox descriptor from
   the mailbox descriptor table.
*/
#define MEI_MB_DESCR_TABLE_DESCR_SIZE_POS  0
#define MEI_MB_DESCR_TABLE_DESCR_SIZE_MASK 0x000F
#define MEI_MB_DESCR_TABLE_DESCR_SIZE_GET(descr_cntrl_off2) \
            ( ((descr_cntrl_off2) & MEI_MB_DESCR_TABLE_DESCR_SIZE_MASK) \
              >> MEI_MB_DESCR_TABLE_DESCR_SIZE_POS )

/**
   Get the offset of the first mailbox descriptor from
   the mailbox descriptor table.
*/
#define MEI_MB_DESCR_TABLE_OFFSET_1MB_POS     8
#define MEI_MB_DESCR_TABLE_OFFSET_1MB_MASK    0x0F00
#define MEI_MB_DESCR_TABLE_OFFSET_1MB_GET(descr_cntrl_off2) \
            ( ((descr_cntrl_off2) & MEI_MB_DESCR_TABLE_OFFSET_1MB_MASK) \
              >> MEI_MB_DESCR_TABLE_OFFSET_1MB_POS )


/**
   Mailbox descriptor
*/
typedef struct MEI_mailbox_descr_s
{
   /** location of the ARC to ME mailbox */
   IFX_uint32_t addrArc2Me;
   /** size of the ARC to ME mailbox */
   IFX_uint32_t lenArc2Me;
   /** location of the ME to ARC mailbox */
   IFX_uint32_t addrMe2Arc;
   /** size of the ME to ARC mailbox */
   IFX_uint32_t lenMe2Arc;
} MEI_MAILBOX_DESCR_T;


/**
   Definition of the CMV mailbox
*/
typedef union MEI_cmv_mailbox_s
{
   /** CMV standard message */
   CMV_STD_MESSAGE_T       cmv;
   /** CMV Modem Ready message */
   CMV_MESSAGE_MODEM_RDY_T ModemRdy;
   /** CMV Codeswap Request message */
   CMV_MESSAGE_CS_T        codeSwap;
   /** CMV Fast Read message */
   CMV_MESSAGE_FAST_RD_T   fastRd;
} MEI_CMV_MAILBOX_T;


/**

*/
typedef struct MEI_mei_mailbox_raw_s
{
   IFX_uint16_t   rawMsg[CMV_HEADER_16BIT_SIZE + CMV_USED_PAYLOAD_16BIT_SIZE];
} MEI_MEI_MAILBOX_RAW_T;


/**
   Mailbox struct
*/
typedef union MEI_mei_mailbox_s
{
   /** raw mailbox message */
   MEI_MEI_MAILBOX_RAW_T mbRaw;
   /** CMV message type */
   MEI_CMV_MAILBOX_T     mbCmv;
} MEI_MEI_MAILBOX_T;


#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #ifndef _MEI_CPE_mailbox_h */

