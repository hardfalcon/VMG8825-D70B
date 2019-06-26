/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _CMV_MESSAGE_FORMAT_H
#define _CMV_MESSAGE_FORMAT_H
/* ==========================================================================
   Description : VR9 CMV interface - Message definitions and constants.
   ========================================================================= */
#ifdef __cplusplus
extern "C"
{
#endif
/* ============================================================================
   Includes
   ========================================================================= */


/* ============================================================================
   Global CMV message definitions
   ========================================================================= */

/**
   Max and used CMV message payload size [8 bit]
*/
#define CMV_MAX_PAYLOAD_8BIT_SIZE      2048
#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
#define CMV_USED_PAYLOAD_8BIT_SIZE     260
#else
#define CMV_USED_PAYLOAD_8BIT_SIZE     256
#endif

/**
   Max and used CMV message payload size [16 bit]
*/
#define CMV_MAX_PAYLOAD_16BIT_SIZE     1024
#define CMV_USED_PAYLOAD_16BIT_SIZE    CMV_USED_PAYLOAD_8BIT_SIZE / 2

/**
   Max and used CMV message payload size [32 bit]
*/
#define CMV_MAX_PAYLOAD_32BIT_SIZE     512
#define CMV_USED_PAYLOAD_32BIT_SIZE    CMV_USED_PAYLOAD_8BIT_SIZE / 4

/** CMV message header [16 bit]:
      - mailbox code
      - functional field (opcode, bit size)
      - payload properties (size, msg idx)
      - IFX message ID / CMV address, CMV group
      - CMV index
      - CMV length
*/
#define CMV_HEADER_32BIT_SIZE    (sizeof(CMV_STD_MESSAGE_HEADER_T) / 4)
#define CMV_HEADER_16BIT_SIZE    (sizeof(CMV_STD_MESSAGE_HEADER_T) / 2)
#define CMV_HEADER_8BIT_SIZE     (sizeof(CMV_STD_MESSAGE_HEADER_T))

/** CMV message size (byte) */
#define CMV_MESSAGE_SIZE         (sizeof(CMV_MESSAGE_ALL_T))

/* ============================================================================
   CMV standard message definitions
   ========================================================================= */

/* ============================================================================
   CMV Message field: offset 0 [16 bit] - mailbox code
   ========================================================================= */

/** Byte offset: mailbox code field within a modem message */
#define CMV_MSGHDR_MBCODE_OFF_8BIT           0

/* ============================================================================
   CMV Message field: offset 1 [16 bit] - function
   ========================================================================= */

/** Byte offset: functional op code field within a modem message */
#define CMV_MSGHDR_FCT_OPCODE_OFF_8BIT       2

/*
   Function - BIT SIZE
      possible values of the function BIT_SIZE field
*/

/**  8 Bit */
#define CMV_MSG_BIT_SIZE_8BIT                0x00
/** 16 Bit */
#define CMV_MSG_BIT_SIZE_16BIT               0x01
/** 32 Bit */
#define CMV_MSG_BIT_SIZE_32BIT               0x02
/** reserved */
#define CMV_MSG_BIT_SIZE_RES                 0x03


/**
   Function -  FUNCTION OPCODE
   NOTE:
      The FUNCTION OPCODE is a combination of the function fields:
      OPCODE, MSG MODE and DIRECTION
*/
#define CMV_MSGHDR_FCT_OPCODE_POS         4
#define CMV_MSGHDR_FCT_OPCODE_MASK        (0x00FF << CMV_MSGHDR_FCT_OPCODE_POS)

/* functional opcode: sub-field - direction */
#define CMV_MSGHDR_FCT_OPCODE_DIR_POS     4
#define CMV_MSGHDR_FCT_OPCODE_DIR_MASK    (0x0001 << CMV_MSGHDR_FCT_OPCODE_DIR_POS)

/* functional opcode: sub-field - mode */
#define CMV_MSGHDR_FCT_OPCODE_MODE_POS    5
#define CMV_MSGHDR_FCT_OPCODE_MODE_MASK   (0x0001 << CMV_MSGHDR_FCT_OPCODE_MODE_POS)

/* functional opcode: sub-field - code (type) */
#define CMV_MSGHDR_FCT_OPCODE_TYPE_POS    6
#define CMV_MSGHDR_FCT_OPCODE_TYPE_MASK   (0x003F << CMV_MSGHDR_FCT_OPCODE_TYPE_POS)

/* CMV message: bit size */
#define CMV_MSGHDR_BIT_SIZE_POS           14
#define CMV_MSGHDR_BIT_SIZE_MASK          (0x0003 << CMV_MSGHDR_BIT_SIZE_POS)


/*
   Access function field Offset 1
*/
#define P_CMV_MSGHDR_FCT_OPCODE_GET(pmsg) \
         ((pmsg->header.fctCode & CMV_MSGHDR_FCT_OPCODE_MASK) >> CMV_MSGHDR_FCT_OPCODE_POS)
#define CMV_MSGHDR_FCT_OPCODE_GET(msg) \
         ((msg.header.fctCode & CMV_MSGHDR_FCT_OPCODE_MASK) >> CMV_MSGHDR_FCT_OPCODE_POS)

#define P_CMV_MSGHDR_FCT_OPCODE_SET(pmsg, val) pmsg->header.fctCode = \
         ( (pmsg->header.fctCode & ~(CMV_MSGHDR_FCT_OPCODE_MASK)) | \
           ((val << CMV_MSGHDR_FCT_OPCODE_POS) & (CMV_MSGHDR_FCT_OPCODE_MASK)) )
#define CMV_MSGHDR_FCT_OPCODE_SET(msg, val) msg.header.fctCode = \
         ( (msg.header.fctCode & ~(CMV_MSGHDR_FCT_OPCODE_MASK)) | \
           ((val << CMV_MSGHDR_FCT_OPCODE_POS) & (CMV_MSGHDR_FCT_OPCODE_MASK)) )


#define P_CMV_MSGHDR_FCT_OPCODE_DIR_GET(pmsg) \
         ((pmsg->header.fctCode & (CMV_MSGHDR_FCT_OPCODE_DIR_MASK)) >> CMV_MSGHDR_FCT_OPCODE_DIR_POS )
#define CMV_MSG_FCT_OPCODE_DIR_GET(msg) \
         ((msg.header.fctCode & CMV_MSGHDR_FCT_OPCODE_DIR_MASK) >> CMV_MSGHDR_FCT_OPCODE_DIR_POS)

#define P_CMV_MSGHDR_FCT_OPCODE_DIR_SET(pmsg, val) pmsg->header.fctCode = \
         ( (pmsg->header.fctCode & ~(CMV_MSGHDR_FCT_OPCODE_DIR_MASK)) | \
           ((val << CMV_MSGHDR_FCT_OPCODE_DIR_POS) & (CMV_MSGHDR_FCT_OPCODE_DIR_MASK)) )
#define CMV_MSG_FCT_OPCODE_DIR_SET(msg, val) msg.header.fctCode = \
         ( (msg.header.fctCode & ~(CMV_MSGHDR_FCT_OPCODE_DIR_MASK)) | \
           ((val << CMV_MSGHDR_FCT_OPCODE_DIR_POS) & (CMV_MSGHDR_FCT_OPCODE_DIR_MASK)) )


#define P_CMV_MSGHDR_FCT_OPCODE_MODE_GET(pmsg) \
         ((pmsg->header.fctCode & CMV_MSGHDR_FCT_OPCODE_MODE_MASK) >> CMV_MSGHDR_FCT_OPCODE_MODE_POS)
#define CMV_MSGHDR_FCT_OPCODE_MODE_GET(msg) \
         ((msg.header.fctCode & CMV_MSGHDR_FCT_OPCODE_MODE_MASK) >> CMV_MSGHDR_FCT_OPCODE_MODE_POS)

#define P_CMV_MSGHDR_FCT_OPCODE_MODE_SET(pmsg, val) pmsg->header.fctCode =\
         ( (pmsg->header.fctCode & ~(CMV_MSGHDR_FCT_OPCODE_MODE_MASK)) | \
           ((val << CMV_MSGHDR_FCT_OPCODE_MODE_POS) & (CMV_MSGHDR_FCT_OPCODE_MODE_MASK)) )
#define CMV_MSGHDR_FCT_OPCODE_MODE_SET(msg, val) msg.header.fctCode = \
         ( (msg.header.fctCode & ~(CMV_MSGHDR_FCT_OPCODE_MODE_MASK)) | \
           ((val << CMV_MSGHDR_FCT_OPCODE_MODE_POS) & (CMV_MSGHDR_FCT_OPCODE_MODE_MASK)) )


#define P_CMV_MSGHDR_FCT_OPCODE_TYPE_GET(pmsg) \
         ((pmsg->header.fctCode & CMV_MSGHDR_FCT_OPCODE_TYPE_MASK) >> CMV_MSGHDR_FCT_OPCODE_TYPE_POS)
#define CMV_MSGHDR_FCT_OPCODE_TYPE_GET(msg) \
         ((msg.header.fctCode & CMV_MSGHDR_FCT_OPCODE_TYPE_MASK) >> CMV_MSGHDR_FCT_OPCODE_TYPE_POS)

#define P_CMV_MSGHDR_FCT_OPCODE_TYPE_SET(pmsg, val) pmsg->header.fctCode = \
         ( (pmsg->header.fctCode & ~(CMV_MSGHDR_FCT_OPCODE_TYPE_MASK)) | \
           ((val << CMV_MSGHDR_FCT_OPCODE_TYPE_POS) & (CMV_MSGHDR_FCT_OPCODE_TYPE_MASK)) )
#define CMV_MSGHDR_FCT_OPCODE_TYPE_SET(msg, val) msg.header.fctCode = \
         ( (msg.header.fctCode & ~(CMV_MSGHDR_FCT_OPCODE_TYPE_MASK)) | \
           ((val << CMV_MSGHDR_FCT_OPCODE_TYPE_POS) & (CMV_MSGHDR_FCT_OPCODE_TYPE_MASK)) )


#define P_CMV_MSGHDR_BIT_SIZE_GET(pmsg) \
         ((pmsg->header.fctCode & CMV_MSGHDR_BIT_SIZE_MASK) >> CMV_MSGHDR_BIT_SIZE_POS)
#define CMV_MSGHDR_BIT_SIZE_GET(msg) \
         ((msg.header.fctCode & CMV_MSGHDR_BIT_SIZE_MASK) >> CMV_MSGHDR_BIT_SIZE_POS)

#define P_CMV_MSGHDR_BIT_SIZE_SET(pmsg, val) pmsg->header.fctCode = \
         ( (pmsg->header.fctCode & ~(CMV_MSGHDR_BIT_SIZE_MASK)) | \
           ((val << CMV_MSGHDR_BIT_SIZE_POS) & (CMV_MSGHDR_BIT_SIZE_MASK)) )
#define CMV_MSGHDR_BIT_SIZE_SET(msg, val) msg.header.fctCode = \
         ( (msg.header.fctCode & ~(CMV_MSGHDR_BIT_SIZE_MASK)) | \
           ((val << CMV_MSGHDR_BIT_SIZE_POS) & (CMV_MSGHDR_BIT_SIZE_MASK)) )


/* ============================================================================
   CMV Message field: offset 2 [16 bit] - payload control
   ========================================================================= */

/** Byte offset: payload control field within a modem message */
#define CMV_MSGHDR_PAYL_CNTRL_OFF_8BIT       4

/* CMV message: payload size */
#define CMV_MSGHDR_PAYLOAD_SIZE_POS    0
#define CMV_MSGHDR_PAYLOAD_SIZE_MASK   (0x03FF << CMV_MSGHDR_PAYLOAD_SIZE_POS)

/* CMV message: payload size */
#define CMV_MSGHDR_MSG_IDX_POS         13
#define CMV_MSGHDR_MSG_IDX_MASK        (0x0007 << CMV_MSGHDR_MSG_IDX_POS)


#define P_CMV_MSGHDR_PAYLOAD_SIZE_GET(pmsg) \
         ((pmsg->header.paylCntrl & CMV_MSGHDR_PAYLOAD_SIZE_MASK) >> CMV_MSGHDR_PAYLOAD_SIZE_POS)
#define CMV_MSGHDR_PAYLOAD_SIZE_GET(msg) \
         ((msg.header.paylCntrl & CMV_MSGHDR_PAYLOAD_SIZE_MASK) >> CMV_MSGHDR_PAYLOAD_SIZE_POS)

#define P_CMV_MSGHDR_PAYLOAD_SIZE_SET(pmsg, val) pmsg->header.paylCntrl = \
         ( (pmsg->header.paylCntrl & ~(CMV_MSGHDR_PAYLOAD_SIZE_MASK)) | \
           ((val << CMV_MSGHDR_PAYLOAD_SIZE_POS) & (CMV_MSGHDR_PAYLOAD_SIZE_MASK)) )
#define CMV_MSGHDR_PAYLOAD_SIZE_SET(msg, val) msg.header.paylCntrl = \
         ( (msg.header.paylCntrl & ~(CMV_MSGHDR_PAYLOAD_SIZE_MASK)) | \
           ((val << CMV_MSGHDR_PAYLOAD_SIZE_POS) & (CMV_MSGHDR_PAYLOAD_SIZE_MASK)) )


#define P_CMV_MSGHDR_MSGIDX_GET(pmsg) \
         ((pmsg->header.paylCntrl & CMV_MSGHDR_MSG_IDX_MASK) >> CMV_MSGHDR_MSG_IDX_POS)
#define CMV_MSGHDR_MSGIDX_GET(msg) \
         ((msg.header.paylCntrl & CMV_MSGHDR_MSG_IDX_MASK) >> CMV_MSGHDR_MSG_IDX_POS)

#define P_CMV_MSGHDR_MSGIDX_SET(pmsg, val) pmsg->header.paylCntrl = \
         ( (pmsg->header.paylCntrl & ~(CMV_MSGHDR_MSG_IDX_MASK)) | \
           ((val << CMV_MSGHDR_MSG_IDX_POS) & (CMV_MSGHDR_MSG_IDX_MASK)) )
#define CMV_MSGHDR_MSGIDX_SET(msg, val) msg.header.paylCntrl = \
         ( (msg.header.paylCntrl & ~(CMV_MSGHDR_MSG_IDX_MASK)) | \
           ((val << CMV_MSGHDR_MSG_IDX_POS) & (CMV_MSGHDR_MSG_IDX_MASK)) )

/* ============================================================================
   CMV Message field: offset 3 [16 bit] - message ID
   ========================================================================= */

/* Byte offset: message ID field within a modem message */
#define CMV_MSGHDR_MSG_ID_OFF_8BIT           6

#define CMV_MSGHDR_MSG_ID_ADDRESS_POS   8
#define CMV_MSGHDR_MSG_ID_ADDRESS_MASK  (0x00FF << CMV_MSGHDR_MSG_ID_ADDRESS_POS)

#define CMV_MSGHDR_MSG_ID_GROUP_POS     0
#define CMV_MSGHDR_MSG_ID_GROUP_MASK    (0x003F << CMV_MSGHDR_MSG_ID_GROUP_POS)

#define P_CMV_MSGHDR_MSG_ID_ADDRESS_GET(pmsg) \
         ((pmsg->header.MessageID & CMV_MSGHDR_MSG_ID_ADDRESS_MASK) >> CMV_MSGHDR_MSG_ID_ADDRESS_POS)

#define P_CMV_MSGHDR_MSG_ID_ADDRESS_SET(pmsg, val) pmsg->header.MessageID = \
         ( (pmsg->header.MessageID & ~(CMV_MSGHDR_MSG_ID_ADDRESS_MASK)) | \
           ((val << CMV_MSGHDR_MSG_ID_ADDRESS_POS) & (CMV_MSGHDR_MSG_ID_ADDRESS_MASK)) )

#define P_CMV_MSGHDR_MSG_ID_GROUP_GET(pmsg) \
         ((pmsg->header.MessageID & CMV_MSGHDR_MSG_ID_GROUP_MASK) >> CMV_MSGHDR_MSG_ID_GROUP_POS)

#define P_CMV_MSGHDR_MSG_ID_GROUP_SET(pmsg, val) pmsg->header.MessageID = \
         ( (pmsg->header.MessageID & ~(CMV_MSGHDR_MSG_ID_GROUP_MASK)) | \
           ((val << CMV_MSGHDR_MSG_ID_GROUP_POS) & (CMV_MSGHDR_MSG_ID_GROUP_MASK)) )

/* CMV header message Id field */
#define P_CMV_MSGHDR_MSGID_GET(pmsg)         pmsg->header.MessageID
#define CMV_MSGHDR_MSGID_GET(msg)            msg.header.MessageID

/* ============================================================================
   CMV Message field: offset 4 [16 bit] - index
   ========================================================================= */

/* Byte offset: index field within a modem message */
#define CMV_MSGHDR_INDEX_OFF_8BIT            8

/* CMV header index field */
#define P_CMV_MSGHDR_INDEX_GET(pmsg)         pmsg->header.index
#define CMV_MSGHDR_INDEX_GET(msg)            msg.header.index

#define P_CMV_MSGHDR_INDEX_SET(pmsg, val)    pmsg->header.index = (val & 0xFFFF)
#define CMV_MSGHDR_INDEX_SET(msg, val)       msg.header.index = (val & 0xFFFF)

/* ============================================================================
   CMV Message field: offset 5 [16 bit] - length
   ========================================================================= */

/* Byte offset: length field within a modem message */
#define CMV_MSGHDR_LENGTH_OFF_8BIT           10

/* CMV header length field */
#define P_CMV_MSGHDR_LENGTH_GET(pmsg)        pmsg->header.length
#define CMV_MSGHDR_LENGTH_GET(msg)           msg.header.length

#define P_CMV_MSGHDR_LENGTH_SET(pmsg, val)   pmsg->header.length = (val & 0xFFFF)
#define CMV_MSGHDR_LENGTH_SET(msg, val)      msg.header.length = (val & 0xFFFF)


/**
   CMV standard message header.
*/
typedef struct cmv_std_message_header_s
{
   /** Mailbox Code */
   unsigned short mbxCode;
   /** Function/Direction Opcode */
   unsigned short fctCode;
   /** payload control fields: size, msg index */
   unsigned short paylCntrl;
   /** IFX msgId / CMV address, group */
   unsigned short MessageID;
   /** CMV index */
   unsigned short index;
   /** CMV length */
   unsigned short length;
} CMV_STD_MESSAGE_HEADER_T;

/**
   CMV payload paramaters (8, 16, 32 Bit)
*/
typedef union cmv_std_message_payload_s
{
   unsigned char  params_8Bit[CMV_USED_PAYLOAD_8BIT_SIZE];
   unsigned short params_16Bit[CMV_USED_PAYLOAD_16BIT_SIZE];
   unsigned int   params_32Bit[CMV_USED_PAYLOAD_32BIT_SIZE];
} CMV_STD_MESSAGE_PAYLOAD_T;

/**
   CMV Standard message.
*/
typedef struct cmv_std_message_s
{
   CMV_STD_MESSAGE_HEADER_T   header;
   CMV_STD_MESSAGE_PAYLOAD_T  payload;
} CMV_STD_MESSAGE_T;


/* ============================================================================
   CMV modem ready message definitions
   ========================================================================= */

/**
   CMV modem ready message.
*/
typedef struct cmv_message_modem_rdy_s
{
   CMV_STD_MESSAGE_HEADER_T   header;
   CMV_STD_MESSAGE_PAYLOAD_T  modemRdyParams;
} CMV_MESSAGE_MODEM_RDY_T;


/* ============================================================================
   CMV CodeSwap request message definitions
   ========================================================================= */
/**
   Max number of swap pages.
*/
#define MEI_CMV_CODESWAP_MAX_PAGES   0x10

/**
   CMV codeswap message payload (static).
*/
typedef struct cmv_message_cs_static_params_s
{
   unsigned short pageIdx[MEI_CMV_CODESWAP_MAX_PAGES];
} CMV_MESSAGE_CS_STATIC_PARAMS_T;


typedef struct cmv_dyn_codeswap_page_info_s
{
   unsigned short pageIdx;
   unsigned short h_destAddr;
   unsigned short l_destAddr;
} CMV_DYN_CODESWAP_PAGE_INFO_T;

/**
   CMV codeswap message payload (dynamic).
*/
typedef struct cmv_message_cs_dyn_params_s
{
   CMV_DYN_CODESWAP_PAGE_INFO_T pageInfo[MEI_CMV_CODESWAP_MAX_PAGES];
} CMV_MESSAGE_CS_DYN_PARAMS_T;


/**
   CMV codeswap message.
*/
typedef struct cmv_message_cs_s
{
   CMV_STD_MESSAGE_HEADER_T   header;
   union
   {
      CMV_MESSAGE_CS_STATIC_PARAMS_T   csStaticParams;
      CMV_MESSAGE_CS_DYN_PARAMS_T      csDynParams;
   } params;
} CMV_MESSAGE_CS_T;

/* ============================================================================
   CMV Fast Read request message definitions
   ========================================================================= */
/**
   Max number of swap pages.
*/
#define MEI_CMV_FAST_READ_MAX_PAGES   8

/**
   Fast read message params definition.
*/
typedef struct cmv_fast_read_params_s
{
   unsigned short addrMSW;
   unsigned short addrLSW;
   unsigned short size_16bit;
} CMV_FAST_READ_PARAMS_T;

/**
   CMV fast read message payload.
*/
typedef struct cmv_message_fast_rd_params_s
{
   unsigned short fastRdpage[MEI_CMV_FAST_READ_MAX_PAGES];
} CMV_MESSAGE_FAST_RD_PARAMS_T;

/**
   CMV codeswap message.
*/
typedef struct cmv_message_fast_rd_s
{
   CMV_STD_MESSAGE_HEADER_T      header;
   CMV_MESSAGE_FAST_RD_PARAMS_T  fastRdParams;
} CMV_MESSAGE_FAST_RD_T;

/**
   CMV messages
*/
typedef union cmv_message_all_s
{
   CMV_STD_MESSAGE_T       cmv;
   CMV_MESSAGE_MODEM_RDY_T ModemRdy;
   CMV_MESSAGE_CS_T        codeSwap;
   CMV_MESSAGE_FAST_RD_T   fastRd;
   unsigned short          rawMsg[CMV_HEADER_16BIT_SIZE + CMV_USED_PAYLOAD_16BIT_SIZE];
} CMV_MESSAGE_ALL_T;



/* ============================================================================
   Definition of the MESSAGE OPCODES.
   ========================================================================= */
/*
   NOTE:
      The MESSAGE OPCODE is a combination of the function fields:
      OPCODE, MSG MODE and DIRECTION
*/

/* CMV Message Codes: (WinHost-to-)ME-to-ARC */
#define H2D_CMV_READ                0x00
#define H2D_CMV_WRITE               0x04


/* CMV Message Codes: ARC-to-ME(-to-WinHost) */
#define D2H_CMV_READ_REPLY                0x01
#define D2H_CMV_WRITE_REPLY               0x05
#define D2H_CMV_INDICATE                  0x11
#define D2H_ERROR_OPCODE_UNKNOWN          0x21
#define D2H_ERROR_CMV_UNKNOWN             0x31
#define D2H_ERROR_CMV_READ_NOT_AVAILABLE  0x41
#define D2H_ERROR_CMV_WRITE_ONLY          0x51
#define D2H_ERROR_CMV_READ_ONLY           0x61
#define D2H_ERROR_CMV_UNINITIALIZED_ENTRY 0x71
#define D2H_ERROR_ILLEGAL_PAYLOAD_SIZE    0x81
#define D2H_ERROR_MSG_TEMP_NOT_AVAIL      0x91


/* Peek/Poke Message Codes: (WinHost-to-)ME-to-ARC */
#define H2D_DEBUG_READ_DM           0x02
#define H2D_DEBUG_READ_PM           0x06
#define H2D_DEBUG_WRITE_DM          0x0A
#define H2D_DEBUG_WRITE_PM          0x0E


/* Peek/Poke Message Codes: ARC-to-ME(-to-WinHost) */
#define D2H_DEBUG_READ_DM_REPLY     0x03
#define D2H_DEBUG_READ_PM_REPLY     0x07
#define D2H_DEBUG_WRITE_DM_REPLY    0x0B
#define D2H_DEBUG_WRITE_PM_REPLY    0x0F
#define D2H_ERROR_ADDR_UNKNOWN      0x33


/* Modem Ready Message Codes: ARC-to-ME */
#define D2DCE_AUTONOMOUS_MODEM_READY         0xF1
#define RESULT_AUTO_MODEM_READY_SUCCESS      0x0
#define RESULT_AUTO_MODEM_READY_ERROR_AC_CLK 0x101

/* Device functional opcodes */
#define D2H_ERROR_BKGD_TASK_WAITING       0xE1 /* 1110 0001b */
#define D2H_ERROR_GENERAL_FAILURE         0xE2 /* 1110 0010b */
#define D2H_ERROR_CMD_NOT_ALLOWED         0xE3 /* 1110 0011b */
#define D2H_ERROR_PARAMETER_WRONG         0xE4 /* 1110 0100b */
#define D2H_ERROR_RESOURCE_OCCUPIED       0xE5 /* 1110 0101b */
#define D2H_DBG_VISP_RUNNING              0xE6 /* 1110 0110b */
#define D2H_DBG_VISP_STOPPED              0xE7 /* 1110 0111b */


#define D2H_CMV_MSGID_MODEM_READY         0xFF02
#define D2H_CMV_MSGID_MODEM_FSM_STATE     0x0002

#define D2H_CMV_MSGID_MODEM_TC_STATUS     0x0E22

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #define _cmv_message_format_h */

