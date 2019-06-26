#ifndef _DRV_MEI_CPE_MEI_VRX_H
#define _DRV_MEI_CPE_MEI_VRX_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : MEI driver - MEI definitions for the VR9 device
   Remarks:
   ========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

/* ============================================================================
   Includes
   ========================================================================= */


/* ============================================================================
   Macros
   ========================================================================= */

#ifndef __PACKED__

#if defined (__GNUC__) || defined (__GNUG__)
/* GNU C or C++ compiler */
#define __PACKED__ __attribute__ ((packed))
#elif !defined (__PACKED__)
#define __PACKED__ /* nothing */
#endif
#endif

/* ============================================================================
   Module      :  PCIe mapping offsets
   ========================================================================= */
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
/* In case of VR11 NO offset has to be used here !*/
#  define MEI_OUTBOUND_ADDRESS_BASE   0x0
#  define MEI_PDBRAM_OFFSET           0x200000
#  define MEI_RCU_OFFSET              0x8000
#  define MEI_CHIPID_EFUSE_OFFSET     0x18000
#  define MEI_DSL_MEI_OFFSET          0x2C0000
#  define MEI_CGU_OFFSET              0x0
#  define MEI_PLL_OMCFG_OFFSET        0x64
#else
#  define MEI_INTERNAL_ADDRESS_BASE   0x1E000000
#  define MEI_OUTBOUND_ADDRESS_BASE   0x20000000
#  define MEI_PDBRAM_OFFSET           0x80000
#  define MEI_RCU_OFFSET              0x2000
#  define MEI_DSL_MEI_OFFSET          0x116000
#endif

#define MEI_GPIO_OFFSET             0x102B00
#define MEI_PPE_OFFSET              0x200000

/* ============================================================================
   VR10 peripherial modules
   Module      :  PPE
   Module      :  GPIO
   ========================================================================= */
#define MEI_PCIE_PERIPHERIAL(mod_offset)  \
(MEI_DRV_PCIE_VIRT_MEMBASE_GET(&pMeiDev->meiDrvCntrl) | (mod_offset))
#define PPE_SB_OFFSET                    0x12000
#define PPE_SB_RAM_BLOCK_4_OFFSET        0x6000
#define PPE_FORCE_LINK_DOWN              0x7DC1
#define PPE_S_44K_OWN                    0x7DC2
#define MEI_PPE_U32REG(addr)              \
((volatile IFX_uint32_t*)(MEI_PCIE_PERIPHERIAL(MEI_PPE_OFFSET + PPE_SB_OFFSET + PPE_SB_RAM_BLOCK_4_OFFSET) + 4*(addr)))
#define GPIO_P0_IN                       0x14
#define GPIO_P0_ALSEL0                   0x1C
#define GPIO_P0_ALSEL1                   0x20
#define MEI_GPIO_U32REG(addr)            \
((volatile IFX_uint32_t*)(MEI_PCIE_PERIPHERIAL(MEI_GPIO_OFFSET) + (addr)))

/* ============================================================================
   Module      :  ARC AUX space register addresses
   ========================================================================= */
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
#  define MEI_REG_ARC_CRI_35B_CTRL       0x30C510
#endif


/* ============================================================================
   Module      :  ARC Load/Store register addresses
   ========================================================================= */
#define MEI_REG_ARC_STATUS             0x0
#define MEI_REG_ARC_LP_START           0x2
#define MEI_REG_ARC_LP_END             0x3
#define MEI_REG_ARC_DEBUG              0x5
#define MEI_REG_ARC_INT_MASK           0x10A


/* ============================================================================
   Macros - Interface definitions
   ========================================================================= */

#define MEI_BIT0     (1 << 0)
#define MEI_BIT1     (1 << 1)
#define MEI_BIT2     (1 << 2)
#define MEI_BIT3     (1 << 3)
#define MEI_BIT4     (1 << 4)
#define MEI_BIT5     (1 << 5)
#define MEI_BIT6     (1 << 6)
#define MEI_BIT7     (1 << 7)
#define MEI_BIT8     (1 << 8)
#define MEI_BIT9     (1 << 9)
#define MEI_BIT10    (1 << 10)
#define MEI_BIT11    (1 << 11)
#define MEI_BIT12    (1 << 12)
#define MEI_BIT13    (1 << 13)
#define MEI_BIT14    (1 << 14)
#define MEI_BIT15    (1 << 15)
#define MEI_BIT16    (1 << 16)
#define MEI_BIT17    (1 << 17)
#define MEI_BIT18    (1 << 18)
#define MEI_BIT19    (1 << 19)
#define MEI_BIT20    (1 << 20)
#define MEI_BIT21    (1 << 21)
#define MEI_BIT22    (1 << 22)
#define MEI_BIT23    (1 << 23)
#define MEI_BIT24    (1 << 24)
#define MEI_BIT25    (1 << 25)
#define MEI_BIT26    (1 << 26)
#define MEI_BIT27    (1 << 27)
#define MEI_BIT28    (1 << 28)
#define MEI_BIT29    (1 << 29)
#define MEI_BIT30    (1 << 30)
#define MEI_BIT31    (1 << 31)

/* ============================================================================
   Macros - MEI register set
   ========================================================================= */

/** Chip Version Number Register */
#define MEI_REG_OFF_ME_VERSION         0x00
/** ARC4 to ME Interrupt Status Register */
#define MEI_REG_OFF_ME_ARC2ME_STAT     0x01
/** ARC4 to ME Interrupt Mask Register */
#define MEI_REG_OFF_ME_ARC2ME_MASK     0x02
/** ME to ARC Interrupt Register */
#define MEI_REG_OFF_ME_ME2ARC_INT      0x03
/** Configuration Register */
#define MEI_REG_OFF_ME_ME2ARC_STAT     0x04
/** Clock Control Register */
#define MEI_REG_OFF_ME_CLK_CTRL        0x05
/** Reset Control Register */
#define MEI_REG_OFF_ME_RST_CTRL        0x06
/** Configuration Register */
#define MEI_REG_OFF_ME_CONFIG          0x07
/** Debug Master Register */
#define MEI_REG_OFF_ME_DBG_MASTER      0x08
/** Debug Decode Register */
#define MEI_REG_OFF_ME_DBG_DECODE      0x09
/** Debug Port Select Register */
#define MEI_REG_OFF_ME_DBG_PORT_SEL    0x0A
/** Debug Read Address Register*/
#define MEI_REG_OFF_ME_DBG_RD_AD       0x0B
/** Debug Write Address Register */
#define MEI_REG_OFF_ME_DBG_WR_AD       0x0C
/** Debug Data Register */
#define MEI_REG_OFF_ME_DBG_DATA        0x0D
/** Management Data Transfer Port Selevt Register */
#define MEI_REG_OFF_ME_DX_PORT_SEL     0x0E
/** Data Transfer Address Register */
#define MEI_REG_OFF_ME_DX_AD           0x0F
/** Data Transfer Data Register */
#define MEI_REG_OFF_ME_DX_DATA         0x10
/** Data Transfer Status Register */
#define MEI_REG_OFF_ME_DX_STAT         0x11
/** TBD */
#define MEI_REG_OFF_ME_DX_MWS          0x12
/** General Purpose Register from ARC */
#define MEI_REG_OFF_ME_ARC_GP_STAT     0x13
/** ??? Register */
#define MEI_REG_OFF_ME_XDATA_BASE_SH   0x14
/** ??? Register */
#define MEI_REG_OFF_ME_XDATA_BASE      0x15
/** BAR0 Register */
#define MEI_REG_OFF_ME_XMEM_BAR0       0x16
/** BAR1 Register */
#define MEI_REG_OFF_ME_XMEM_BAR1       0x17
/** BAR2 Register */
#define MEI_REG_OFF_ME_XMEM_BAR2       0x18
/** BAR3 Register */
#define MEI_REG_OFF_ME_XMEM_BAR3       0x19
/** BAR4 Register */
#define MEI_REG_OFF_ME_XMEM_BAR4       0x1A
/** BAR5 Register */
#define MEI_REG_OFF_ME_XMEM_BAR5       0x1B
/** BAR6 Register */
#define MEI_REG_OFF_ME_XMEM_BAR6       0x1C
/** BAR7 Register */
#define MEI_REG_OFF_ME_XMEM_BAR7       0x1D
/** BAR8 Register */
#define MEI_REG_OFF_ME_XMEM_BAR8       0x1E
/** BAR9 Register */
#define MEI_REG_OFF_ME_XMEM_BAR9       0x1F
/** BAR10 Register */
#define MEI_REG_OFF_ME_XMEM_BAR10      0x20
/** BAR11 Register */
#define MEI_REG_OFF_ME_XMEM_BAR11      0x21
/** BAR12 Register */
#define MEI_REG_OFF_ME_XMEM_BAR12      0x22
/** BAR13 Register */
#define MEI_REG_OFF_ME_XMEM_BAR13      0x23
/** BAR14 Register */
#define MEI_REG_OFF_ME_XMEM_BAR14      0x24
/** BAR15 Register */
#define MEI_REG_OFF_ME_XMEM_BAR15      0x25
/** BAR16 Register */
#define MEI_REG_OFF_ME_XMEM_BAR16      0x26
/** ARB Misc Register */
#define MEI_REG_OFF_ME_XMEM_ARB_MISC   0x27


#if (MEI_SUPPORT_DEVICE_VR11 == 1)
/** BAR17 Register */
#define MEI_REG_OFF_ME_XMEM_BAR17      0x28
/** BAR18 Register */
#define MEI_REG_OFF_ME_XMEM_BAR18      0x29
/** BAR19 Register */
#define MEI_REG_OFF_ME_XMEM_BAR19      0x2A
/** BAR20 Register */
#define MEI_REG_OFF_ME_XMEM_BAR20      0x2B
/** BAR21 Register */
#define MEI_REG_OFF_ME_XMEM_BAR21      0x2C
/** BAR22 Register */
#define MEI_REG_OFF_ME_XMEM_BAR22      0x2D
/** BAR23 Register */
#define MEI_REG_OFF_ME_XMEM_BAR23      0x2E
/** BAR24 Register */
#define MEI_REG_OFF_ME_XMEM_BAR24      0x2F

/** first MEI regsiter set offset */
#define MEI_REG_OFFSET_FIRST           0x00
/** last MEI regsiter set offset */
#define MEI_REG_OFFSET_LAST            0x2F

#else

/** first MEI regsiter set offset */
#define MEI_REG_OFFSET_FIRST           0x00
/** last MEI regsiter set offset */
#define MEI_REG_OFFSET_LAST            0x27

#endif

/* =============================================================
   MEI register definitions
*/
/* Definition:  Chip Version Number Register ========== */
#define MEI_ME_VERSION_REV             0x000000FF
#define MEI_ME_VERSION_JTAG_REV        0x0000FF00

/* Definition: ARC4 to ME Interrupt Status Register === */
#define ME_ARC2ME_STAT_ARC_MSGAV0      MEI_BIT0
#define ME_ARC2ME_STAT_ARC_MSGAV1      MEI_BIT1
#define ME_ARC2ME_STAT_ARC_GP_INT0     MEI_BIT2
#define ME_ARC2ME_STAT_DBG_DONE        MEI_BIT4
#define ME_ARC2ME_STAT_DBG_ERR         MEI_BIT5
/* Select port#0 access*/
#define ME_ARC2ME_STAT_ARC_MSGAV       ME_ARC2ME_STAT_ARC_MSGAV0

/** Definition of the interrupt mask ARC to ME */
#define ME_ARC2ME_INTERRUPT_ALL        (   ME_ARC2ME_STAT_ARC_MSGAV0 \
                                         | ME_ARC2ME_STAT_ARC_MSGAV1 \
                                         | ME_ARC2ME_STAT_ARC_GP_INT0 \
                                         | ME_ARC2ME_STAT_DBG_DONE \
                                         | ME_ARC2ME_STAT_DBG_ERR )



/* Definition: ARC4 to ME Interrupt Mask Register ===== */
#define ME_ARC2ME_MASK_ARC_MSGAV0_ENA     MEI_BIT0
#define ME_ARC2ME_MASK_ARC_MSGAV1_ENA     MEI_BIT1
#define ME_ARC2ME_MASK_ARC_GP_INT0_ENA    MEI_BIT2
#define ME_ARC2ME_MASK_DBG_DONE_ENA       MEI_BIT4
#define ME_ARC2ME_MASK_DBG_ERR_ENA        MEI_BIT5
/* Select port#0 access*/
#define ME_ARC2ME_MASK_ARC_MSGAV_ENA      ME_ARC2ME_MASK_ARC_MSGAV0_ENA

/** Definition of the interrupt mask ARC to ME */
#define ME_ARC2ME_INTERRUPT_MASK_ALL      (   ME_ARC2ME_MASK_ARC_MSGAV0_ENA \
                                            | ME_ARC2ME_MASK_ARC_MSGAV1_ENA \
                                            | ME_ARC2ME_MASK_ARC_GP_INT0_ENA \
                                            | ME_ARC2ME_MASK_DBG_DONE_ENA \
                                            | ME_ARC2ME_MASK_DBG_ERR_ENA )

/** Definition of the interrupt un-mask ARC to ME
   \Note the Debug Done and Debug Error will be handled via polling
*/
#define ME_ARC2ME_INTERRUPT_UNMASK_ALL    (   ME_ARC2ME_MASK_ARC_MSGAV0_ENA \
                                            | ME_ARC2ME_MASK_ARC_MSGAV1_ENA \
                                            | ME_ARC2ME_MASK_ARC_GP_INT0_ENA)

/* Definition: Reset Control Register ================= */
#define ME_RST_CTRL_DSP_RST            MEI_BIT0
#define ME_RST_CTRL_XDSL_RST           MEI_BIT1
#define ME_RST_CTRL_AHB_SOFT_RST       MEI_BIT2

/** reset all sub modules */
#define ME_RST_CTRL_ALL                (   ME_RST_CTRL_DSP_RST \
                                         | ME_RST_CTRL_XDSL_RST \
                                         | ME_RST_CTRL_AHB_SOFT_RST)


/* Definition: Configuration Register ================= */
#define ME_CONFIG_INT_LEVEL            MEI_BIT0


/* Definition: ME to ARC4 Interrupt Register ========== */
/** Mailbox Message Interrupt to ARC for port 0*/
#define ME_ME2ARC_INT_ME_MSGAV0         MEI_BIT0
/** Mailbox Message Interrupt to ARC for port 1*/
#define ME_ME2ARC_INT_ME_MSGAV1         MEI_BIT1
/** General Purpose Interrupt to ARC */
#define ME_ME2ARC_INT_ME_GP_INT0        MEI_BIT2
/* Select port#0 access*/
#define ME_ME2ARC_INT_ME_MSGAV          ME_ME2ARC_INT_ME_MSGAV0


/* Definition: ME to ARC4 Interrupt Status Register === */
/** Mailbox Message Interrupt to ARC Status for port 0*/
#define ME_ME2ARC_STAT_ME_MSGAV0        MEI_BIT0
/** Mailbox Message Interrupt to ARC Status for port 1*/
#define ME_ME2ARC_STAT_ME_MSGAV1        MEI_BIT1
/** General Purpose Interrupt to ARC Status */
#define ME_ME2ARC_STAT_ME_GP_INT0       MEI_BIT2
/* Select port#0 access*/
#define ME_ME2ARC_STAT_ME_MSGAV         ME_ME2ARC_STAT_ME_MSGAV0

/* Definition: Debug Decode Register ================== */
/**
   DEBUG_DEC:
   - 00 = Auxiliary address01,
   - 10 = LD/ST address
   - 11 = Core address
*/
#define ME_DBG_DECODE_DEBUG_DEC        MEI_BIT0 | MEI_BIT1
#define ME_DBG_DECODE_DEBUG_DEC_AUX    0x0
#define ME_DBG_DECODE_DEBUG_DEC_LDST   0x2
#define ME_DBG_DECODE_DEBUG_DEC_CORE   0x3

/* Definition: Debug Port Select Register ================== */
#define ME_DBG_PORT_SELECT             MEI_BIT0

/* Definition: Debug Master Register ================== */
/**
   HOST_MSTR:
   - If bit = 0: JTAG is master of host/debug bus.
   - If bit = 1: ME is master of host/debug bus.
*/
#define ME_DBG_MASTER_HOST_MSTR        MEI_BIT0

/**
   Definition: Data Transfer Status Register
*/
#define ME_DX_STAT_DX_ERR              MEI_BIT0
#define ME_DX_STAT_ACC_ERR             MEI_BIT1


/* width of the MEI registers [16 bit] */
#define MEI_MEI_REGISTER_WIDTH    sizeof(IFX_uint32_t)

/* width of the DMA data access [16 bit] */
#define MEI_MEI_DMA_DATA_WIDTH    sizeof(IFX_uint32_t)

/* ============================================================================
   VR9 Management Entity Interface (MEI) - register defintions
   ========================================================================= */
/* Total number of the MEI BARx registers*/
#define MEI_TOTAL_BAR_REGISTER_COUNT   (17)
#if (MEI_SUPPORT_DEVICE_VR11 != 1)
/* Total number of the MEI BARy registers*/
#define MEI_TOTAL_BAR_REGISTER_COUNT2   (0)

/* BAR16 size [byte]*/
#define MEI_BAR16_SIZE_BYTE            (1024*1024)

#else

/* Total number of the MEI BARx registers*/
#define MEI_TOTAL_BAR_REGISTER_COUNT2   (8)

/* BAR16 size [byte]*/
#define MEI_BAR16_SIZE_BYTE            (512*1024)

#endif

typedef IFX_vuint32_t MEI_MeiReg_t;
typedef IFX_uint32_t  MEI_MeiRegVal_t;

typedef struct MEI_mei_register_set_s
{
   /** [00] Chip Version Number Register */
   IFX_vuint32_t ME_VERSION;
   /** [01] ARC4 to ME Interrupt Status Register */
   IFX_vuint32_t ME_ARC2ME_STAT;
   /** [02] ARC4 to ME Interrupt Mask Register */
   IFX_vuint32_t ME_ARC2ME_MASK;
   /** [03] ME to ARC4 Interrupt Register */
   IFX_vuint32_t ME_ME2ARC_INT;
   /** [04] ME to ARC4 Interrupt Status Register */
   IFX_vuint32_t ME_ME2ARC_STAT;
   /** [05] Clock Control Register */
   IFX_vuint32_t ME_CLK_CTRL;
   /** [06] Reset Control Register */
   IFX_vuint32_t ME_RST_CTRL;
   /** [07] Configuration Register */
   IFX_vuint32_t ME_CONFIG;
   /** [08] Debug Master Register */
   IFX_vuint32_t ME_DBG_MASTER;
   /** [09] Debug Decode Register */
   IFX_vuint32_t ME_DBG_DECODE;
   /** [0A] Debug Port Select Register */
   IFX_vuint32_t ME_DBG_PORT_SEL;
   /** [0B] Debug Read Address Register */
   IFX_vuint32_t ME_DBG_RD_AD;
   /** [0C] Debug Write Address Register */
   IFX_vuint32_t ME_DBG_WR_AD;
   /** [0D] Debug Data Register */
   IFX_vuint32_t ME_DBG_DATA;
   /** [0E] DX Port Select Register */
   IFX_vuint32_t ME_DX_PORT_SEL;
   /** [0F] Data Transfer Address Register */
   IFX_vuint32_t ME_DX_AD;
   /** [10] Data Transfer Data Register  */
   IFX_vuint32_t ME_DX_DATA;
   /** [11] Data Transfer Status Register */
   IFX_vuint32_t ME_DX_STAT;
   /** [12] TBD */
   IFX_vuint32_t ME_DX_MWS;
   /** [13] General Purpose Register from ARC */
   IFX_vuint32_t ME_ARC_GP_STAT;
   /** [14] Shadow Register for XDATA base address */
   IFX_vuint32_t ME_XDATA_BASE_SH;
   /** [15] Active Register for XDATA base address */
   IFX_vuint32_t ME_XDATA_BASE;
   /** [16-26] BARx Register */
   IFX_vuint32_t ME_XMEM_BARx[MEI_TOTAL_BAR_REGISTER_COUNT];
   /** [27] ARB Misc Register */
   IFX_vuint32_t ME_XMEM_ARB_MISC;
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   /** [28-37] BARy Register */
   IFX_vuint32_t ME_XMEM_BARy[MEI_TOTAL_BAR_REGISTER_COUNT2];
#endif
} __PACKED__ MEI_MEI_REG_IF_T;

typedef union
{
   MEI_MEI_REG_IF_T    regStruct;
   IFX_vuint32_t       regRaw[sizeof(MEI_MEI_REG_IF_T)/sizeof(IFX_vuint32_t)];
} MEI_MEI_REG_IF_U; /* old MEI_MEI_REGISTER_SET_T */


/* ************************************************************************** */
/*                         CMD_PLL_CLOCKSET 0x1962                            */
/* ************************************************************************** */

#define MEI_PLL_CLOCKSET_MIN    -32768
#define MEI_PLL_CLOCKSET_MAX     32767
#define MEI_PLL_DISABLED         32768

/** Message ID for CMD_PLL_ClockSet */
#define CMD_PLL_CLOCKSET 0x1962

/**
   Sets an offset for the PLL frequency compared to the crystals rated nominal
   frequency of 36 MHz. This allows e.g. to finetune the handshake tone
   frequencies to exactly match the standard values.
*/
typedef struct CMD_PLL_ClockSet CMD_PLL_ClockSet_t;

/** Message ID for ACK_PLL_ClockSet */
#define ACK_PLL_CLOCKSET 0x1962

/**
   This is the acknowledgement for CMD_PLL_ClockSet.
*/
typedef struct ACK_PLL_ClockSet ACK_PLL_ClockSet_t;

/**
   Sets an offset for the PLL frequency compared to the crystals rated nominal
   frequency of 36 MHz. This allows e.g. to finetune the handshake tone
   frequencies to exactly match the standard values.
*/
struct CMD_PLL_ClockSet
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** PLL Clock Offset in ppm */
   IFX_int16_t pllClockOffset;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** PLL Clock Offset in ppm */
   IFX_int16_t pllClockOffset;
#endif
} __PACKED__ ;


/**
   This is the acknowledgement for CMD_PLL_ClockSet.
*/
struct ACK_PLL_ClockSet
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


/* ************************************************************************** */
/*                       CMD_MODEMFSM_STATEGET 0x0002                         */
/* ************************************************************************** */

/** Message ID for CMD_ModemFSM_StateGet */
#define CMD_MODEMFSM_STATEGET 0x0002

/**
   Requests information about the current state of the modem state-machine.The
   command can be sent in all states of the modem state machine (see Figure 2).
*/
typedef struct CMD_ModemFSM_StateGet CMD_ModemFSM_StateGet_t;

/** Message ID for ACK_ModemFSM_StateGet */
#define ACK_MODEMFSM_STATEGET 0x0002

/**
   Returns information about the current state of the modem state-machine.
*/
typedef struct ACK_ModemFSM_StateGet ACK_ModemFSM_StateGet_t;

/**
   Requests information about the current state of the modem state-machine.The
   command can be sent in all states of the modem state machine (see Figure 2).
*/
struct CMD_ModemFSM_StateGet
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
   Returns information about the current state of the modem state-machine.
*/
struct ACK_ModemFSM_StateGet
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Modem Status */
   IFX_uint16_t ModemState;
   /** Reserved */
   IFX_uint16_t Res0 : 14;
   /** Line Power Management State */
   IFX_uint16_t LxState : 2;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Modem Status */
   IFX_uint16_t ModemState;
   /** Line Power Management State */
   IFX_uint16_t LxState : 2;
   /** Reserved */
   IFX_uint16_t Res0 : 14;
#endif
} __PACKED__ ;

/* ************************************************************************** */
/*                       API TC-Layer re-definitions                          */
/* ************************************************************************** */

#define DSL_TC_ATM  1
#define DSL_TC_EFM  2

/* ************************************************************************** */
/*                         EVT_TC_STATUSGET 0x0E22                            */
/* ************************************************************************** */

#define EVT_TC_StatusGet_EFM_TC 0x1
#define EVT_TC_StatusGet_ATM_TC 0x2

/** Message ID for EVT_TC_StatusGet */
#define EVT_TC_STATUSGET 0x0E22

/**
   This autonomous Event reports the to be used TC mode after being known.
*/
typedef struct EVT_TC_StatusGet EVT_TC_StatusGet_t;

/**
   This autonomous Event reports the to be used TC mode after being known.
*/
struct EVT_TC_StatusGet
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** TC Used */
   IFX_uint16_t TC;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** TC Used */
   IFX_uint16_t TC;
#endif
} __PACKED__ ;

/** Message ID for CMD_PAF_HS_StatusGet */
#define CMD_PAF_HS_STATUSGET 0xDD03

/**
   The message requests aggregation discovery and aggregation status information
   for EFM bonding.
*/
typedef struct CMD_PAF_HS_StatusGet CMD_PAF_HS_StatusGet_t;

/** Message ID for ACK_PAF_HS_StatusGet */
#define ACK_PAF_HS_STATUSGET 0xDD03

/**
   The message reports discovery and aggregation status information for EFM
   bonding after request by CMD_PAF_HS_StatusGet.At the CPE side, the Host SW
   concludes on a GET request if none of the following 4 bits are set:
   discoveryClearIfSame, discoverySetIfClear, aggregClear , aggregSet. It does
   not distinguish between "discovery GET" and "aggregation GET"; in case of any
   "GET" both the discovery and the aggregation code are sent to the CO with the
   CLR.
*/
typedef struct ACK_PAF_HS_StatusGet ACK_PAF_HS_StatusGet_t;

/**
   The message requests aggregation discovery and aggregation status information
   for EFM bonding.
*/
struct CMD_PAF_HS_StatusGet
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
   The message reports discovery and aggregation status information for EFM
   bonding after request by CMD_PAF_HS_StatusGet.At the CPE side, the Host SW
   concludes on a GET request if none of the following 4 bits are set:
   discoveryClearIfSame, discoverySetIfClear, aggregClear , aggregSet. It does
   not distinguish between "discovery GET" and "aggregation GET"; in case of any
   "GET" both the discovery and the aggregation code are sent to the CO with the
   CLR.
*/
struct ACK_PAF_HS_StatusGet
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Reserved */
   IFX_uint16_t Res0 : 9;
   /** Discovery Clear-If-Same */
   IFX_uint16_t discoveryClearIfSame : 1;
   /** Discovery Set-If-Clear */
   IFX_uint16_t discoverySetIfClear : 1;
   /** Aggregation Clear */
   IFX_uint16_t aggregClear : 1;
   /** Aggregation Set */
   IFX_uint16_t aggregSet : 1;
   /** Reserved */
   IFX_uint16_t Res1 : 2;
   /** PCS Control Register (Register 3.60, Bit 12) */
   IFX_uint16_t PAF_Enable : 1;
   /** Aggregation Discovery Code (Register 6.18) */
   IFX_uint16_t discoveryCode1;
   /** Aggregation Discovery Code (Registers 6.19) */
   IFX_uint16_t discoveryCode2;
   /** Aggregation Discovery Code (Registers 6.20) */
   IFX_uint16_t discoveryCode3;
   /** Partner PME Aggregate Data (Registers 6.22) */
   IFX_uint16_t aggregateData1;
   /** Partner PME Aggregate Data (Registers 6.23) */
   IFX_uint16_t aggregateData2;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** PCS Control Register (Register 3.60, Bit 12) */
   IFX_uint16_t PAF_Enable : 1;
   /** Reserved */
   IFX_uint16_t Res1 : 2;
   /** Aggregation Set */
   IFX_uint16_t aggregSet : 1;
   /** Aggregation Clear */
   IFX_uint16_t aggregClear : 1;
   /** Discovery Set-If-Clear */
   IFX_uint16_t discoverySetIfClear : 1;
   /** Discovery Clear-If-Same */
   IFX_uint16_t discoveryClearIfSame : 1;
   /** Reserved */
   IFX_uint16_t Res0 : 9;
   /** Aggregation Discovery Code (Register 6.18) */
   IFX_uint16_t discoveryCode1;
   /** Aggregation Discovery Code (Registers 6.19) */
   IFX_uint16_t discoveryCode2;
   /** Aggregation Discovery Code (Registers 6.20) */
   IFX_uint16_t discoveryCode3;
   /** Partner PME Aggregate Data (Registers 6.22) */
   IFX_uint16_t aggregateData1;
   /** Partner PME Aggregate Data (Registers 6.23) */
   IFX_uint16_t aggregateData2;
#endif
} __PACKED__ ;

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #define _DRV_MEI_CPE_MEI_VRX_H */

