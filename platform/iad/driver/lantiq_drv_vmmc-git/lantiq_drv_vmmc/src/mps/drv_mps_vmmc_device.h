#ifndef _DRV_MPS_VMMC_DEVICE_H
#define _DRV_MPS_VMMC_DEVICE_H
/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_mps_vmmc_device.h Global header file of the MPS driver.
   This file contains the defines, the structures declarations, the table
   declarations and the device specific functions declarations.
*/

#if !defined(SYSTEM_FALCON)
   #ifdef LINUX
      #include "drv_mps_vmmc_bsp.h"

      #if (BSP_API_VERSION < 3)
         #include <asm/ifx_vpe.h>
      #elif (BSP_API_VERSION < 5)
         #include <asm/ltq_vpe.h>
      #else
         #include <asm/ltq_vpe.h>
         #include <irq.h>
         #include <lantiq_irq.h>
      #endif /* BSP_API_VERSION */
   #endif /* LINUX */
#else /* SYSTEM_FALCON */
   #include <lantiq.h>
   #include <irq.h>
   #include <gpio.h>
   #include <falcon_irq.h>
#endif /* SYSTEM_FALCON */

/** This variable holds the actual base address of the MPS register block. */
extern IFX_uint32_t ifx_mps_reg_base;  /* MPS registers */
/** This variable holds the actual base address of the MPS SRAM area. */
extern IFX_uint32_t ifx_mps_ram_base;  /* MPS memory */
/** This variable holds the interrupt number of the IRQ associated with the
    MPS status register 4 which is used for AFE and DFE 0 status. */
extern IFX_uint32_t ifx_mps_ir4;       /* MPS AD0 register interrupt */

/* ============================================ */
/* Default values for platform depending values */
/* ============================================ */
#if !defined(SYSTEM_FALCON)
   #define IFX_MPS_BASE_ADDR        (KSEG1 | 0x1F107000)
   #define IFX_MPS_SRAM             (KSEG1 | 0x1F200000)
#else /* SYSTEM_FALCON */
   #define IFX_MPS_BASE_ADDR        (KSEG1 | 0x1D004000)
   #define IFX_MPS_SRAM             (KSEG1 | 0x1F200000)
#endif /* SYSTEM_FALCON */

/* ============================= */
/* MPS registers                 */
/* ============================= */
#define IFX_MPS_RAD0SR              ((u32 *)(ifx_mps_reg_base + 0x0040))
#define IFX_MPS_SAD0SR              ((u32 *)(ifx_mps_reg_base + 0x0048))
#define IFX_MPS_CAD0SR              ((u32 *)(ifx_mps_reg_base + 0x0050))
#define IFX_MPS_AD0ENR              ((u32 *)(ifx_mps_reg_base + 0x0058))

#define IFX_MPS_CHIPID              ((u32 *)(ifx_mps_reg_base + 0x0344))
#define IFX_MPS_GRX330_CHIPID       ((u32 *)(ifx_mps_reg_base + 0x0348))

#define IFX_MPS_CHIPID_VERSION_GET(value) (((value) >> 28) & ((1 << 4) - 1))
#define IFX_MPS_CHIPID_PARTNUM_GET(value) (((value) >> 12) & ((1 << 16) - 1))

/* ============================= */
/* Interrupt vectors             */
/* ============================= */
#define MPS_IR4  /* AD0 */          (INT_NUM_IM4_IRL0 + 18)

/* ============================= */
/* ICU registers                 */
/* ============================= */
#if !defined(CONFIG_LANTIQ)
   /* Today request_irq() calls the BSP to enable the interrupts. This code
      is only kept for legacy BSPs and may be removed without notice. */
   #define IFX_ICU0_BASE            (KSEG1 | 0x1F880200)
   #define IFX_ICU0_IM4_IER         ((u32 *)(IFX_ICU0_BASE + 0x00A8))
   /* All MPS interrupts are located in IM4 (interrupt module 4) */
   #define ICU0_IM4_IRQ_ENABLE(X) \
           *((volatile IFX_uint32_t*)IFX_ICU0_IM4_IER) |= X;
   #define ICU0_IM4_IRQ_DISABLE(X) \
           *((volatile IFX_uint32_t*)IFX_ICU0_IM4_IER) &= ~X;
   /* Interrupt register values */
   #define IMx_IR18 (1 << 18)       /* AFE/DFE 0 */
#endif /* !defined(CONFIG_LANTIQ) */

/* ============================= */
/* MPS Common defines            */
/* ============================= */


/*---------------------------------------------------------------------------*/
/* Mailbox definitions                                                       */
/*---------------------------------------------------------------------------*/

/* The mailbox is located in SRAM which layout depends on the system. */
#if !defined(SYSTEM_FALCON)
   #define IFX_MPS_MBX_BASE         (ifx_mps_ram_base)
#else /* SYSTEM_FALCON */
   #define IFX_MPS_MBX_BASE         (ifx_mps_ram_base + 0x0040)
#endif /* SYSTEM_FALCON */

#define MBX_CMD_FIFO_SIZE  64 /**< Size of command FIFO in bytes */
#define MBX_DATA_UPSTRM_FIFO_SIZE 64
#define MBX_DATA_DNSTRM_FIFO_SIZE 128
#define MBX_EVENT_FIFO_SIZE 32
#define MBX_DEFINITION_AREA_SIZE 32
#define MBX_RW_POINTER_AREA_SIZE 32
#define MBX_EVENT_POINTER_AREA_SIZE 16

/* base addresses for command and voice mailboxes (upstream and downstream ) */
#define MBX_UPSTRM_CMD_FIFO_BASE   (IFX_MPS_MBX_BASE + MBX_DEFINITION_AREA_SIZE + MBX_RW_POINTER_AREA_SIZE)
#define MBX_DNSTRM_CMD_FIFO_BASE   (MBX_UPSTRM_CMD_FIFO_BASE + MBX_CMD_FIFO_SIZE)
#define MBX_UPSTRM_DATA_FIFO_BASE  (MBX_DNSTRM_CMD_FIFO_BASE + MBX_CMD_FIFO_SIZE)
#define MBX_DNSTRM_DATA_FIFO_BASE  (MBX_UPSTRM_DATA_FIFO_BASE + MBX_DATA_UPSTRM_FIFO_SIZE)
#define MBX_UPSTRM_EVENT_FIFO_BASE (MBX_DNSTRM_DATA_FIFO_BASE + MBX_DATA_DNSTRM_FIFO_SIZE + MBX_EVENT_POINTER_AREA_SIZE)

#define MBX_DATA_WORDS 80
#define MBX_EVENT_DATA_WORDS 8

#define NUM_VOICE_CHANNEL    14 /**< nr of voice channels  */
#define NR_CMD_MAILBOXES      2 /**< nr of command mailboxes  */

#define MAX_UPSTRM_DATAWORDS   15
#define MAX_FIFO_WRITE_RETRIES 80

/*
   Defines for the new mailbox format descriptor
*/
/** Mailbox Register format 1: current format used */
#define LTQ_VOICE_MPS_MBOX_TYPE_L1   0x1
/** Mailbox Register format 2: new format */
#define LTQ_VOICE_MPS_MBOX_TYPE_L2   0x2

#define LTQ_VOICE_MPS_MBOX_TYPE_ALL \
   (LTQ_VOICE_MPS_MBOX_TYPE_L1 | LTQ_VOICE_MPS_MBOX_TYPE_L2)

/*
    MBX Layout 1 (old)                                 MBX Layout 2 (new)
   +----------------+  MBX Base                        +----------------+  MBX Base
   |0x000           |  Def Area (32 byte)              |                |  Def Area (32 byte)
   +----------------+                                  +----------------+
   |0x020           |  Ptr Area (32 byte)              |                |  Ptr Area (32 byte)
   +----------------+                                  +----------------+
   |0x040           |  UPSTRM_CMD_FIFO (64 byte)       |          0x040 |  UPSTRM_CMD_FIFO (64 byte)
   |                |                                  |                |
   |0x080           |  DNSTRM_CMD_FIFO (64 byte)       |          0x080 |  DNSTRM_CMD_FIFO (192 byte)
   |                |                                  |                |
   |0x0C0           |  UPSTRM_DATA_FIFO (64 byte)      |                |
   |                |                                  |                |
   |0x100           |  DNSTRM_DATA_FIFO (128 byte)     |          0x140 |  UPSTRM_EVENT_FIFO (64 byte)
   +----------------+                                  +----------------+
   |0x180           |  UPSTRM_EVENT_FIFO (Start)       |                |  UPSTRM_EVENT_FIFO (Start)
   +----------------+                                  +----------------+
   |                |  UPSTRM_EVENT_FIFO (Size)        |                |  UPSTRM_EVENT_FIFO (Size)
   +----------------+                                  +----------------+
   |                |  UPSTRM_EVENT_FIFO (Rd Idx)      |                |  UPSTRM_EVENT_FIFO (Rd Idx)
   +----------------+                                  +----------------+
   |                |  UPSTRM_EVENT_FIFO (Wr Idx)      |                |  UPSTRM_EVENT_FIFO (Wr Idx)
   +----------------+                                  +----------------+
   |0x190           |  UPSTRM_EVENT_FIFO (32 byte)     |                |  Reserved (32 byte)
   +----------------+                                  +----------------+

*/

#define MBX_UPSTRM_CMD_FIFO_SIZE_L2      64
#define MBX_UPSTRM_CMD_FIFO_SIZE32_L2    (MBX_UPSTRM_CMD_FIFO_SIZE_L2 / 4)
#define MBX_UPSTRM_CMD_FIFO_BASE_L2      (IFX_MPS_MBX_BASE + MBX_DEFINITION_AREA_SIZE + MBX_RW_POINTER_AREA_SIZE)

#define MBX_DNSTRM_CMD_FIFO_SIZE_L2      192
#define MBX_DNSTRM_CMD_FIFO_SIZE32_L2    (MBX_DNSTRM_CMD_FIFO_SIZE_L2 / 4)
#define MBX_DNSTRM_CMD_FIFO_BASE_L2      (MBX_UPSTRM_CMD_FIFO_BASE_L2 + MBX_UPSTRM_CMD_FIFO_SIZE_L2)

#define MBX_UPSTRM_EVENT_FIFO_SIZE_L2    64
#define MBX_UPSTRM_EVENT_FIFO_SIZE32_L2  (MBX_UPSTRM_EVENT_FIFO_SIZE_L2 / 4)
#define MBX_UPSTRM_EVENT_FIFO_BASE_L2    (MBX_DNSTRM_CMD_FIFO_BASE_L2 + MBX_DNSTRM_CMD_FIFO_SIZE_L2)

#define MBX_UPSTRM_DATA_FIFO_SIZE_L2     1024
#define MBX_UPSTRM_DATA_FIFO_SIZE32_L2   (MBX_UPSTRM_DATA_FIFO_SIZE_L2 / 4)
#define MBX_UPSTRM_DATA_FIFO_BASE_L2     "get via kmalloc"
#define MBX_DNSTRM_DATA_FIFO_SIZE_L2     1024
#define MBX_DNSTRM_DATA_FIFO_SIZE32_L2   (MBX_DNSTRM_DATA_FIFO_SIZE_L2 / 4)
#define MBX_DNSTRM_DATA_FIFO_BASE_L2     "get via kmalloc"

#define MBX_RESRV_DATA_SIZE_L2           32
#define MBX_RESRV_DATA_SIZE32_L2         (MBX_RESRV_DATA_SIZE_L2 / 4)

/*---------------------------------------------------------------------------*/
/* MPS buffer provision management structure definitions                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* DEVICE DEPENDENCIES                                                       */
/*---------------------------------------------------------------------------*/
typedef enum
{
   MPS_BUF_UNINITIALIZED,
   MPS_BUF_EMPTY,
   MPS_BUF_LOW,
   MPS_BUF_OK,
   MPS_BUF_OV,
   MPS_BUF_ERR
} mps_buffer_state_e;

typedef struct /**< mps buffer monitoring structure */
{
   IFX_int32_t buf_level;                  /**< Current bufffer level */
   IFX_uint32_t buf_threshold;              /**< Minimum buffer count */
   IFX_uint32_t buf_initial;                /**< Initial buffer count */
   IFX_uint32_t buf_size;                   /**< Buffer size for voice cpu */
   mps_buffer_state_e buf_state;
   IFX_void_t *(*malloc) (IFX_size_t size, IFX_int32_t priority); /**< Buffer alloc function (def. kmalloc) */
   IFX_void_t (*free) (const IFX_void_t *ptr);  /**< Buffer free  function (def. kfree) */
   IFX_int32_t (*init) (void); /** Manager init function */
   IFX_int32_t (*close) (void); /** Manager shutdown function */
} mps_buf_mng_t_xx;

/*---------------------------------------------------------------------------*/
/* Register structure definitions                                            */
/*---------------------------------------------------------------------------*/
typedef enum
{
   COMMAND = 1,
   VOICE
} MbxMessageType_e;

typedef enum
{
   UPSTREAM,
   DOWNSTREAM
} MbxDirection_e;

typedef struct
{
   IFX_uint32_t rw:1;
   IFX_uint32_t res1:2;
   IFX_uint32_t type:5;
   IFX_uint32_t res2:4;
   IFX_uint32_t chan:4;
   IFX_uint32_t res3:1;
   IFX_uint32_t odd:2;
   IFX_uint32_t res4:5;
   IFX_uint32_t plength:8;
} MbxMsgHd_s;

typedef union
{
   IFX_uint32_t val;
   MbxMsgHd_s hd;
} MbxMsgHd_u;

typedef struct
{
   MbxMsgHd_u header;
   IFX_uint32_t data[MAX_UPSTRM_DATAWORDS];
} MbxMsg_s;

/*---------------------------------------------------------------------------*/
/* FIFO structure                                                            */
/*---------------------------------------------------------------------------*/
typedef struct
{
   volatile IFX_uint32_t *volatile pend;       /**< Pointer to FIFO's read/write end address */
   volatile IFX_uint32_t *volatile pwrite_off; /**< Pointer to FIFO's write index location */
   volatile IFX_uint32_t *volatile pread_off;  /**< Pointer to FIFO's read index location */
   volatile IFX_uint32_t size;        /**< FIFO size */
   volatile IFX_uint32_t min_space;   /**< FIFO size */
   volatile IFX_uint32_t bytes;
   volatile IFX_uint32_t pkts;
   volatile IFX_uint32_t discards;
} mps_fifo;

typedef struct
{
   volatile IFX_uint32_t MPS_BOOT_RVEC;    /**< CPU reset vector */
   volatile IFX_uint32_t MPS_BOOT_NVEC;    /**<  */
   volatile IFX_uint32_t MPS_BOOT_EVEC;    /**<  */
   volatile IFX_uint32_t MPS_CP0_STATUS;   /**<  */
   volatile IFX_uint32_t MPS_CP0_EEPC;     /**<  */
   volatile IFX_uint32_t MPS_CP0_EPC;      /**<  */
   volatile IFX_uint32_t MPS_BOOT_SIZE;    /**<  */
   volatile IFX_uint32_t MPS_CFG_STAT;     /**<  */
} mps_boot_cfg_reg;

/*
 * Descriptor Layout 2
 * This structure represents the MPS mailbox definition area that is shared
 * by CCPU and VCPU. It comprises the mailboxes' base addresses and sizes in bytes as well as the
 */
typedef struct
{
   /* Definition Area (32 Byte): US/DS Cmd and Data */
   volatile IFX_uint32_t *MBX_UPSTR_CMD_BASE;      /**< Upstream Command FIFO Base Address */
   volatile IFX_uint32_t MBX_UPSTR_CMD_SIZE_MPS_OPT;  /**< Upstream Command FIFO size in byte */
   volatile IFX_uint32_t *MBX_DNSTR_CMD_BASE;      /**< Downstream Command FIFO Base Address */
   volatile IFX_uint32_t MBX_DNSTR_CMD_SIZE;       /**< Downstream Command FIFO size in byte */
   volatile IFX_uint32_t *MBX_UPSTR_DATA_BASE;     /**< Upstream Data FIFO Base Address */
   volatile IFX_uint32_t MBX_UPSTR_DATA_SIZE;      /**< Upstream Data FIFO size in byte */
   volatile IFX_uint32_t *MBX_DNSTR_DATA_BASE;     /**< Downstream Data FIFO Base Address */
   volatile IFX_uint32_t MBX_DNSTR_DATA_SIZE;      /**< Downstream Data FIFO size in byte */
   /* Pointer Area (32 Byte): US/DS Cmd and Data */
   volatile IFX_uint32_t MBX_UPSTR_CMD_READ;       /**< Upstream Command FIFO Read Index */
   volatile IFX_uint32_t MBX_UPSTR_CMD_WRITE;      /**< Upstream Command FIFO Write Index */
   volatile IFX_uint32_t MBX_DNSTR_CMD_READ;       /**< Downstream Command FIFO Read Index */
   volatile IFX_uint32_t MBX_DNSTR_CMD_WRITE;      /**< Downstream Command FIFO Write Index */
   volatile IFX_uint32_t MBX_UPSTR_DATA_READ;      /**< Upstream Data FIFO Read Index */
   volatile IFX_uint32_t MBX_UPSTR_DATA_WRITE;     /**< Upstream Data FIFO Write Index */
   volatile IFX_uint32_t MBX_DNSTR_DATA_READ;      /**< Downstream Data FIFO Read Index */
   volatile IFX_uint32_t MBX_DNSTR_DATA_WRITE;     /**< Downstream Data FIFO Write Index */
   /* Buffers: US/DS CMD Fifo, Event Fifo */
   volatile IFX_uint32_t MBX_UPSTR_CMD_BUF[MBX_UPSTRM_CMD_FIFO_SIZE32_L2];
   volatile IFX_uint32_t MBX_DNSTR_CMD_BUF[MBX_DNSTRM_CMD_FIFO_SIZE32_L2];
   volatile IFX_uint32_t MBX_EVENT_BUF[MBX_UPSTRM_EVENT_FIFO_SIZE32_L2];
   /* Def/Ptr Area (32 Byte): Event */
   volatile IFX_uint32_t *MBX_UPSTR_EVENT_BASE;    /**< Upstream Event FIFO Base Address */
   volatile IFX_uint32_t MBX_UPSTR_EVENT_SIZE;     /**< Upstream Event FIFO size in byte */
   volatile IFX_uint32_t MBX_UPSTR_EVENT_READ;     /**< Upstream Event FIFO Read Index */
   volatile IFX_uint32_t MBX_UPSTR_EVENT_WRITE;    /**< Upstream Event FIFO Write Index */
   /* Misc */
   volatile IFX_uint32_t MBX_RESRV_DATA_0[MBX_RESRV_DATA_SIZE32_L2];
   volatile IFX_uint32_t MBX_WDT_COUNTER;
   volatile IFX_uint32_t reserved[3];
} mps_mbx_reg_defs_l2;

/*
 * Descriptor Layout 2
 * access MBX_UPSTR_CMD_SIZE_MPS_OPT: MPS Option and US Cmd Size
 */
#define VOICE_MBX_US_CMD_SIZE_MASK      0x0000FFFF
#define VOICE_MBX_MPS_OPT_MASK          0xFFFF0000

#define VOICE_MBX_US_CMD_SIZE_SET(P_REG_BASE_F2, US_CMD_SIZE) \
   /*lint -e{717}*/ \
   do {(P_REG_BASE_F2)->MBX_UPSTR_CMD_SIZE_MPS_OPT = \
         ((P_REG_BASE_F2)->MBX_UPSTR_CMD_SIZE_MPS_OPT & VOICE_MBX_MPS_OPT_MASK) | \
         ((US_CMD_SIZE) & VOICE_MBX_US_CMD_SIZE_MASK); } while (0)
#define VOICE_MBX_US_CMD_SIZE_GET(P_REG_BASE_F2) \
   ((P_REG_BASE_F2)->MBX_UPSTR_CMD_SIZE_MPS_OPT & VOICE_MBX_US_CMD_SIZE_MASK)

#define VOICE_MBX_MPS_OPT_SET(P_REG_BASE_F2, MPS_OPT) \
   /*lint -e{717}*/ \
   do {(P_REG_BASE_F2)->MBX_UPSTR_CMD_SIZE_MPS_OPT = \
         ((P_REG_BASE_F2)->MBX_UPSTR_CMD_SIZE_MPS_OPT & VOICE_MBX_US_CMD_SIZE_MASK) | \
         (((MPS_OPT) <<16) & VOICE_MBX_MPS_OPT_MASK); } while (0)
#define VOICE_MBX_MPS_OPT_GET(P_REG_BASE_F2) \
   (((P_REG_BASE_F2)->MBX_UPSTR_CMD_SIZE_MPS_OPT & VOICE_MBX_MPS_OPT_MASK) >> 16)


/*
 * This structure represents the MPS mailbox definition area that is shared
 * by CCPU and VCPU. It comprises the mailboxes' base addresses and sizes in bytes as well as the
 *
 *
 */
typedef struct
{
#if defined(SYSTEM_FALCON)
   mps_boot_cfg_reg MBX_CPU0_BOOT_CFG; /**< CPU0 Boot Configuration */
   mps_boot_cfg_reg MBX_CPU1_BOOT_CFG; /**< CPU1 Boot Configuration */
#endif /* SYSTEM_FALCON */
   volatile IFX_uint32_t *MBX_UPSTR_CMD_BASE;  /**< Upstream Command FIFO Base Address */
   volatile IFX_uint32_t MBX_UPSTR_CMD_SIZE;   /**< Upstream Command FIFO size in byte */
   volatile IFX_uint32_t *MBX_DNSTR_CMD_BASE;  /**< Downstream Command FIFO Base Address */
   volatile IFX_uint32_t MBX_DNSTR_CMD_SIZE;   /**< Downstream Command FIFO size in byte */
   volatile IFX_uint32_t *MBX_UPSTR_DATA_BASE; /**< Upstream Data FIFO Base Address */
   volatile IFX_uint32_t MBX_UPSTR_DATA_SIZE;  /**< Upstream Data FIFO size in byte */
   volatile IFX_uint32_t *MBX_DNSTR_DATA_BASE; /**< Downstream Data FIFO Base Address */
   volatile IFX_uint32_t MBX_DNSTR_DATA_SIZE;  /**< Downstream Data FIFO size in byte */
   volatile IFX_uint32_t MBX_UPSTR_CMD_READ;   /**< Upstream Command FIFO Read Index */
   volatile IFX_uint32_t MBX_UPSTR_CMD_WRITE;  /**< Upstream Command FIFO Write Index */
   volatile IFX_uint32_t MBX_DNSTR_CMD_READ;   /**< Downstream Command FIFO Read Index */
   volatile IFX_uint32_t MBX_DNSTR_CMD_WRITE;  /**< Downstream Command FIFO Write Index */
   volatile IFX_uint32_t MBX_UPSTR_DATA_READ;   /**< Upstream Data FIFO Read Index */
   volatile IFX_uint32_t MBX_UPSTR_DATA_WRITE;  /**< Upstream Data FIFO Write Index */
   volatile IFX_uint32_t MBX_DNSTR_DATA_READ;   /**< Downstream Data FIFO Read Index */
   volatile IFX_uint32_t MBX_DNSTR_DATA_WRITE;  /**< Downstream Data FIFO Write Index */
   volatile IFX_uint32_t MBX_DATA[MBX_DATA_WORDS];
   volatile IFX_uint32_t *MBX_UPSTR_EVENT_BASE; /**< Upstream Event FIFO Base Address */
   volatile IFX_uint32_t MBX_UPSTR_EVENT_SIZE;  /**< Upstream Event FIFO size in byte */
   volatile IFX_uint32_t MBX_UPSTR_EVENT_READ;  /**< Upstream Event FIFO Read Index */
   volatile IFX_uint32_t MBX_UPSTR_EVENT_WRITE; /**< Upstream Event FIFO Write Index */
   volatile IFX_uint32_t MBX_EVENT[MBX_EVENT_DATA_WORDS];
   volatile IFX_uint32_t reserved[4];
#if !defined(SYSTEM_FALCON)
   mps_boot_cfg_reg MBX_CPU0_BOOT_CFG; /**< CPU0 Boot Configuration */
   mps_boot_cfg_reg MBX_CPU1_BOOT_CFG; /**< CPU1 Boot Configuration */
#endif /* !SYSTEM_FALCON */
} mps_mbx_reg;

/*
 * This structure represents the MPS mailbox definition area that is shared
 * by CCPU and VCPU. It comprises the mailboxes' base addresses and sizes in bytes as well as the
 *
 *
 */
typedef struct
{
#if defined(SYSTEM_FALCON)
   mps_boot_cfg_reg MBX_CPU0_BOOT_CFG; /**< CPU0 Boot Configuration */
   mps_boot_cfg_reg MBX_CPU1_BOOT_CFG; /**< CPU1 Boot Configuration */
#endif /* SYSTEM_FALCON */
   /** defines the mailbox register description for new layout 2 */
   mps_mbx_reg_defs_l2 mbx_reg_defs_l2;
#if !defined(SYSTEM_FALCON)
   mps_boot_cfg_reg MBX_CPU0_BOOT_CFG; /**< CPU0 Boot Configuration */
   mps_boot_cfg_reg MBX_CPU1_BOOT_CFG; /**< CPU1 Boot Configuration */
#endif /* !SYSTEM_FALCON */
} mps_mbx_reg_l2;


typedef union
{
   mps_mbx_reg    mbx_reg_l1;
   mps_mbx_reg_l2 mbx_reg_l2;
} mps_mbx_reg_u;

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Device connection structure                                               */
/*---------------------------------------------------------------------------*/

/**
 * Mailbox Device Structure.
 * This Structure holds top level parameters of the mailboxes used to allow
 * the communication between the control CPU and the Voice CPU
 */
typedef struct
{
   /* void pointer to the base device driver structure */
   IFX_void_t *pVCPU_DEV;

   /* Mutex semaphore to access the device */
   IFXOS_lock_t *sem_dev;

   /* Mutex semaphore to access the device */
   IFXOS_lock_t *sem_read_fifo;

   /* Wakeuplist for the select mechanism */
   IFXOS_drvSelectQueue_t mps_wakeuplist;

   mps_fifo *upstrm_fifo;    /**< Data exchange FIFO for read (upstream) */
   mps_fifo *dwstrm_fifo;    /**< Data exchange FIFO for write (downstream) */

#ifdef MPS_FIFO_BLOCKING_WRITE
   IFXOS_lock_t *sem_write_fifo;
   volatile IFX_boolean_t full_write_fifo;

   /* variable if the driver should block on write to the transmit FIFO */
   IFX_boolean_t bBlockWriteMB;
#endif                          /* MPS_FIFO_BLOCKING_WRITE */

   mps_devices devID;   /**< Device ID  1->command
                                        2->voice chan 0
                                        3->voice chan 1
                                        4->voice chan 2
                                        5->voice chan 3
                                        6->voice chan 4
                                        7->voice chan 5
                                        8->voice chan 6
                                        9->voice chan 7
                                       10->event  */
   volatile IFX_int32_t Installed;
   IFX_void_t (*down_callback) (mps_devices type);
   IFX_void_t (*up_callback) (mps_devices type);
   MbxEventRegs_s event_mask;
   MbxEventRegs_s callback_event_mask;
   IFX_void_t (*event_callback) (MbxEventRegs_s *events);

   /* Event mailbox */
   IFX_uint32_t event_callback_handle;
   IFX_void_t (*event_mbx_callback) (IFX_uint32_t event_callback_handle,
                                     mps_event_msg *msg);

#ifdef LINUX
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   struct cdev *mps_cdev;
#endif
#endif /* LINUX */
} mps_mbx_dev;

#define MPS_MBX_DEV_NOT_INSTALLED   0x00
#define MPS_MBX_DEV_INSTALLED       0x01
#define MPS_MBX_DEV_FROM_KERNEL     0x02
#define MPS_MBX_DEV_RECONFIG        0x10

#define MPS_MBX_DEV_INSTALL_RESET(P_MPS_MBX) \
   /*lint -e{717}*/ \
   do {(P_MPS_MBX)->Installed = MPS_MBX_DEV_NOT_INSTALLED;} while (0)


#define MPS_MBX_DEV_INSTALL_INST_SET(P_MPS_MBX) \
   /*lint -e{717}*/ \
   do {(P_MPS_MBX)->Installed |= MPS_MBX_DEV_INSTALLED;} while (0)
#define MPS_MBX_DEV_INSTALL_INST_CLEAR(P_MPS_MBX) \
   /*lint -e{717}*/ \
   do {(P_MPS_MBX)->Installed &= ~(MPS_MBX_DEV_INSTALLED);} while (0)

#define MPS_MBX_DEV_INSTALL_KERNEL_SET(P_MPS_MBX) \
   /*lint -e{717}*/ \
   do {(P_MPS_MBX)->Installed |= MPS_MBX_DEV_FROM_KERNEL;} while (0)
#define MPS_MBX_DEV_INSTALL_KERNEL_CLEAR(P_MPS_MBX) \
   /*lint -e{717}*/ \
   do {(P_MPS_MBX)->Installed &= ~(MPS_MBX_DEV_FROM_KERNEL);} while (0)

#define MPS_MBX_DEV_INSTALL_RECONFIG_SET(P_MPS_MBX) \
   /*lint -e{717}*/ \
   do {(P_MPS_MBX)->Installed |= MPS_MBX_DEV_RECONFIG;} while (0)
#define MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(P_MPS_MBX) \
   /*lint -e{717}*/ \
   do {(P_MPS_MBX)->Installed &= ~(MPS_MBX_DEV_RECONFIG);} while (0)

#define MPS_MBX_DEV_INSTALL_INST_ISSET(P_MPS_MBX) \
   (((P_MPS_MBX)->Installed & MPS_MBX_DEV_INSTALLED) ? 1 : 0)
#define MPS_MBX_DEV_INSTALL_KERNEL_ISSET(P_MPS_MBX) \
   (((P_MPS_MBX)->Installed & MPS_MBX_DEV_FROM_KERNEL) ? 1 : 0)
#define MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(P_MPS_MBX) \
   (((P_MPS_MBX)->Installed & MPS_MBX_DEV_RECONFIG) ? 1 : 0)

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Device structure                                                          */
/*---------------------------------------------------------------------------*/

/**
 * Mailbox Device Structure.
 * This Structure represents the communication device that provides the resources
 * for the communication between CPU0 and CPU1
 */
typedef struct
{
   IFX_uint_t mbx_descr;   /**< selects the used mailbox register format (see \ref mps_mbx_reg) */
   mps_mbx_reg *base_global;   /**< global register pointer for the ISR */
   mps_mbx_reg_l2 *base_global_l2;   /**< global register pointer for the ISR */
   IFX_uint32_t flags;                   /**< Pointer to private date of the specific handler */
   mps_mbx_dev voice_mb[NUM_VOICE_CHANNEL];     /**< Data upstream and downstream mailboxes */
   mps_mbx_dev command_mb;                      /**< Command upstream and downstream mailbox */
   MbxEventRegs_s event;  /**< global structure holding the interrupt status */
   mps_fifo cmd_upstrm_fifo;
   mps_fifo cmd_dwstrm_fifo;
   mps_fifo voice_upstrm_fifo;
   mps_fifo voice_dwstrm_fifo;
   mps_fifo sw_upstrm_fifo[NUM_VOICE_CHANNEL];
   IFXOS_lock_t *provide_buffer;

   /* Used for L2 only. Pointer to mailbox memory shared with the FW. */
   IFX_uint32_t *p_voice_upstrm_mailbox;
   /* Used for L2 only. Pointer to mailbox memory shared with the FW. */
   IFX_uint32_t *p_voice_dwstrm_mailbox;

   mps_mbx_dev event_mbx;         /**< Event upstream mailbox */
   mps_fifo event_upstrm_fifo;    /**< Mailbox FIFO structure */
   mps_fifo sw_event_upstrm_fifo; /** Software FIFO used for user space access */
} mps_comm_dev;

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* VoIP firmware CRC-32 checksum calculation                                 */
/*---------------------------------------------------------------------------*/
/* VoIP firmware image footer format */
typedef struct
{
   /* CRC-32 checksum */
   IFX_uint32_t crc32;
   /* start address for CRC32 checksum calculation */
   IFX_uint32_t st_addr_crc;
   /* end address for CRC32 checksum calculation */
   IFX_uint32_t en_addr_crc;
   /* firmware version */
   IFX_uint32_t fw_vers;
   /* plain/encrypted: 0-plain, 1-encrypted */
   IFX_uint32_t enc;
   /* magic word 0xCC123456 */
   IFX_uint32_t magic;
   /* firmware memory requirement */
   IFX_uint32_t mem;
} FW_image_ftr_t;
extern FW_image_ftr_t *pFW_img_data;

#define FW_MAGIC_NUM 0xcc123456
#define FW_FORMAT_NEW (pFW_img_data->magic==FW_MAGIC_NUM)
#define FW_AR9_OLD_FMT_XCPT_AREA_SZ 28
#define FW_DANUBE_OLD_FMT_XCPT_AREA_SZ 20
#define FW_XCPT_AREA_OFFSET 8
#define FW_PLT_INCA_IP2 0
#define FW_PLT_DANUBE   1
#define FW_PLT_XRX100   4
#define FW_PLT_XRX200   7
#define FW_PLT_XRX300   8

#define FW_ENC_ENCRYPT         0x1
#define FW_ENC_MBOX_LAYOUT_2   0x2
#define FW_ENC_FLAGS_ALL \
   (FW_ENC_ENCRYPT | FW_ENC_MBOX_LAYOUT_2)

/*---------------------------------------------------------------------------*/
IFX_int32_t ifx_mps_common_open (mps_comm_dev * pDev, mps_mbx_dev * pMBDev,
                                 IFX_int32_t bcommand,
                                 IFX_boolean_t from_kernel);
IFX_int32_t ifx_mps_common_close (mps_mbx_dev * pMBDev,
                                  IFX_boolean_t from_kernel);
IFX_int32_t ifx_mps_mbx_read (mps_mbx_dev * pMBDev, mps_message * pPkg,
                              IFX_int32_t timeout);
IFX_int32_t ifx_mps_mbx_write_cmd (mps_mbx_dev * pMBDev,
                                   mps_message * readWrite);
IFX_int32_t ifx_mps_mbx_write_data (mps_mbx_dev * pMBDev,
                                    mps_message * readWrite);
IFX_int_t ifx_mps_fifo_dyn_data_reset(mps_fifo *p_mps_fifo);
IFX_int32_t ifx_mps_init_mbox_setup(mps_comm_dev * pDev, IFX_uint_t mbx_descr);
IFX_int32_t ifx_mps_init_structures (mps_comm_dev * pDev);
IFX_void_t ifx_mps_release_structures (mps_comm_dev * pDev);
IFX_uint32_t ifx_mps_reset_structures (mps_comm_dev * pDev);

IFX_int32_t ifx_mps_restart (void);
IFX_void_t ifx_mps_reset (void);
IFX_int32_t ifx_mps_download_firmware (mps_mbx_dev * pDev, mps_fw * pFWDwnld);
IFX_uint32_t ifx_mps_fifo_mem_available (mps_fifo * mbx);

IFX_int_t ifx_mps_fastbuf_cfg_setup(
   IFX_uint_t fb_num_of_elem,
   IFX_uint_t fb_seg_size__byte,
   IFX_uint_t fb_num_of_prov_init,
   IFX_uint_t fb_num_of_prov_per_msg,
   IFX_uint_t fb_threshold,
   IFX_uint_t fb_threshold_ov);

IFX_int_t ifx_mps_fastbuf_cfg_release(void);
IFX_int32_t ifx_mps_bufman_init (void);
IFX_void_t ifx_mps_bufman_free (const IFX_void_t * ptr);
IFX_void_t *ifx_mps_bufman_malloc (IFX_size_t size, IFX_int32_t priority);

IFX_void_t ifx_mps_bufman_freeall_xx(void);
IFX_void_t ifx_mps_register_bufman_freeall_callback (IFX_void_t (*pfn)(void));
extern IFX_void_t (*ifx_mps_bufman_freeall)(void);

IFX_int32_t ifx_mps_get_fw_version (IFX_int32_t print);
IFX_void_t ifx_mps_disable_mailbox_int (void);
IFX_void_t ifx_mps_disable_all_int (void);
IFX_void_t ifx_mps_enable_mailbox_int (void);
#ifdef VMMC_FEAT_VPE1_SW_WD
extern IFX_int32_t ifx_mps_register_wdog_callback (
                        IFX_int32_t (*pfn) (IFX_uint32_t flags));
#endif /* VMMC_FEAT_VPE1_SW_WD */
DECLARE_TRACE_GROUP (MPS);
IFX_uint32_t ifx_mps_fw_crc32(
   volatile IFX_uint32_t *l_cpu1_base_addr,
   FW_image_ftr_t *l_pFW_img_data);
IFX_void_t ifx_mps_fw_crc_compare(
   volatile IFX_uint32_t *l_cpu1_base_addr,
   FW_image_ftr_t *l_pFW_img_data);
IFX_void_t ifx_mps_dump_fw_xcpt(
   volatile IFX_uint32_t *l_cpu1_base_addr,
   FW_image_ftr_t *l_pFW_img_data);
extern volatile IFX_uint32_t *cpu1_base_addr;

#if defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)
#define MVPCONTROL_CPA_BIT (1 << 3)
#define VPEOPT_DWX_MASK 0xf
extern IFX_boolean_t bDoCacheOps;
#endif /*defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)*/

#endif /* _DRV_MPS_VMMC_DEVICE_H */
