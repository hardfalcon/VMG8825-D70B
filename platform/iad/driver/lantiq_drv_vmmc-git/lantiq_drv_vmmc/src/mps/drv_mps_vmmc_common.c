/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_mps_vmmc_common.c MPS driver main implementation file.
   This file contains the implementation of the common MPS driver functions.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_config.h"
#include "drv_mps_vmmc_bsp.h"

#ifdef VMMC_WITH_MPS
   #include "drv_api.h" /* configurations from VMMC driver */
#endif /* VMMC_WITH_MPS */

#if (defined(GENERIC_OS) && defined(GREENHILLS_CHECK))
   #include "drv_vmmc_ghs.h"
#endif /* (defined(GENERIC_OS) && defined(GREENHILLS_CHECK)) */

#undef USE_PLAIN_VOICE_FIRMWARE
#undef PRINT_ON_ERR_INTERRUPT
#undef FAIL_ON_ERR_INTERRUPT

#ifdef LINUX
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
   #include <linux/autoconf.h>
#else
   #include <generated/autoconf.h>
#endif /* < Linux 2.6.33 */
#include <linux/interrupt.h>
#include <linux/delay.h>
#endif /* LINUX */

/* lib_ifxos headers */
#include "ifx_types.h"
#include "ifxos_linux_drv.h"
#include "ifxos_lock.h"
#include "ifxos_select.h"
#include "ifxos_event.h"
#include "ifxos_memory_alloc.h"
#include "ifxos_interrupt.h"
#include "ifxos_time.h"

#ifdef LINUX
#ifdef SYSTEM_FALCON
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,28))
#include <lantiq.h>
#include <irq.h>
#else
#include <asm/ifx/ifx_regs.h>
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
#include <sys1_reg.h>
#endif
#include <sysctrl.h>
#endif /* SYSTEM_FALCON */
#endif /* LINUX */

#include "drv_mps_vmmc.h"
#include "drv_mps_vmmc_dbg.h"
#include "drv_mps_vmmc_device.h"
#include "drv_mps_vmmc_crc32.h"

#ifdef VMMC_WITH_MPS
#ifdef TAPI_PACKET_OWNID
#include "drv_tapi_kpi_io.h"
#include "drv_vmmc_fw_data.h"
#endif /* TAPI_PACKET_OWNID */
#endif /* VMMC_WITH_MPS */

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
#define IFX_MPS_UNUSED(var) ((IFX_void_t)(var))

#define MPS_DEBUG_MBOX_DESCR_SHOW 0
#define MPS_DEBUG_FCT_ENTER_SHOW 0
#define MPS_DEBUG_FCT_LEAVE_SHOW 0

#if defined (MPS_DEBUG_FCT_ENTER_SHOW) && (MPS_DEBUG_FCT_ENTER_SHOW == 1)
#  define MPS_DEBUG_FCT_ENTER\
      TRACE (MPS, DBG_LEVEL_HIGH, (">>> --> %s()\n", __FUNCTION__))
#else
#  define MPS_DEBUG_FCT_ENTER
#endif

#if defined (MPS_DEBUG_FCT_LEAVE_SHOW) && (MPS_DEBUG_FCT_LEAVE_SHOW == 1)
#  define MPS_DEBUG_FCT_LEAVE\
      TRACE (MPS, DBG_LEVEL_HIGH, ("<<< --> %s()\n", __FUNCTION__))
#else
#  define MPS_DEBUG_FCT_LEAVE
#endif


/* ============================= */
/* Global variable definition    */
/* ============================= */
mps_comm_dev ifx_mps_dev;

/** This variable holds the actual base address of the MPS register block. */
IFX_uint32_t ifx_mps_reg_base = IFX_MPS_BASE_ADDR;
/** This variable holds the actual base address of the MPS SRAM area. */
IFX_uint32_t ifx_mps_ram_base = IFX_MPS_SRAM;
/** This variable holds the interrupt number of the IRQ associated with the
    MPS status register 4 which is used for AFE and DFE 0 status. */
IFX_uint32_t ifx_mps_ir4 = MPS_IR4;

/* Variable indicating the identified chip family. */
enum mps_chip ifx_mps_chip_family = MPS_CHIP_UNKNOWN;


/* ============================= */
/* Global function declaration   */
/* ============================= */
IFX_int32_t ifx_mps_bufman_buf_provide(
   IFX_uint_t n_segs,
   IFX_uint_t seg_size_byte,
   IFX_uint_t n_segs_per_msg);

IFX_int32_t ifx_mps_bufman_close (void);
IFX_void_t ifx_mps_mbx_data_upstream (IFX_ulong_t dummy);
IFX_void_t ifx_mps_mbx_cmd_upstream (IFX_ulong_t dummy);
IFX_int32_t ifx_mps_mbx_read_message (mps_fifo * fifo, MbxMsg_s * msg,
                                      IFX_uint32_t * bytes);

IFX_void_t ifx_mps_mbx_event_upstream (IFX_ulong_t dummy);

IFX_void_t *ifx_mps_fastbuf_malloc (IFX_size_t size, IFX_int32_t priority);
IFX_void_t ifx_mps_fastbuf_free (const IFX_void_t * ptr);
IFX_int32_t ifx_mps_fastbuf_create (IFX_uint_t fb_seg_size__byte);
IFX_int32_t ifx_mps_fastbuf_release (void);

extern IFX_uint32_t danube_get_cpu_ver (void);
extern mps_mbx_dev *ifx_mps_get_device (mps_devices type);

#if   (BSP_API_VERSION == 1)
extern IFX_void_t mask_and_ack_danube_irq (IFX_uint32_t irq_nr);
#elif (BSP_API_VERSION == 2)
extern IFX_void_t bsp_mask_and_ack_irq (IFX_uint32_t irq_nr);
#endif

extern void sys_hw_setup (void);
extern IFX_void_t ifx_mps_HwSetupXRX100(IFX_enDis_t bEnable);
extern IFX_int32_t ifx_mps_HwProbeXRX100(void);

extern IFXOS_event_t fw_ready_evt;
/* callback function to free all data buffers currently used by voice FW */
IFX_void_t (*ifx_mps_bufman_freeall)(void) = IFX_NULL;
/* ============================= */
/* Local function declaration    */
/* ============================= */
extern IFX_int32_t ifx_mps_mbx_write_message (
                        mps_mbx_dev * pMBDev,
                        IFX_uint8_t * msg_ptr,
                        IFX_uint32_t msg_bytes);

/* ============================= */
/* Local variable definition     */
/* ============================= */

mps_comm_dev *pMPSDev = &ifx_mps_dev;

#if CONFIG_MPS_HISTORY_SIZE > 0
#if CONFIG_MPS_HISTORY_SIZE > 512
#error "MPS history buffer > 512 words (2kB)"
#endif /* */
#define MPS_HISTORY_BUFFER_SIZE (CONFIG_MPS_HISTORY_SIZE)
IFX_int32_t ifx_mps_history_buffer_freeze = 0;
IFX_uint32_t ifx_mps_history_buffer[MPS_HISTORY_BUFFER_SIZE] = { 0 };
IFX_int32_t ifx_mps_history_buffer_words = 0;

#ifdef DEBUG
IFX_int32_t ifx_mps_history_buffer_words_total = 0;
#endif /* */

IFX_int32_t ifx_mps_history_buffer_overflowed = 0;

#endif /* CONFIG_MPS_HISTORY_SIZE > 0 */

atomic_t ifx_mps_write_blocked = ATOMIC_INIT (0);
atomic_t ifx_mps_dd_mbx_int_enabled = ATOMIC_INIT (0);

/******************************************************************************
 * Fast bufferpool
 ******************************************************************************/

/* internal Buf Mngt */
#define MPS_FASTBUF_INIT_INTERNAL  0x00000001
/* external Buf Mngt */
#define MPS_FASTBUF_INIT_EXTERNAL  0x00000002

/*
   Defines for FW Mailbox Layout 1
*/

/* number of initial buffer elements delivered to the FW after startup */
#define MPS_BUFFER_INITIAL_PROVISION_L1   36
/* number of available buffer elements */
#define MPS_BUFFER_NUM_OF_ELEMENTS_L1     63
/* number of buffer elements per FW message */
#define MPS_BUFFER_PROV_PER_MSG_L1        12
/* buffer segment size */
#define MPS_BUFFER_SEG_SIZE_L1            512
/* FW threshold used to deliver additional buffers. (Refill threshold) */
#define MPS_BUFFER_THRESHOLD_L1           24
/* buffer management runs in overflow state */
#define MPS_BUFFER_THRESHOLD_OVLOAD_L1    63

/*
   Defines for FW Mailbox Layout 2
*/
#define MPS_BUFFER_INITIAL_PROVISION_L2   128
#define MPS_BUFFER_NUM_OF_ELEMENTS_L2     255
#define MPS_BUFFER_PROV_PER_MSG_L2        12
#define MPS_BUFFER_SEG_SIZE_L2            512
#define MPS_BUFFER_THRESHOLD_L2           MPS_BUFFER_INITIAL_PROVISION_L2
#define MPS_BUFFER_THRESHOLD_OVLOAD_L2    255

/* Marker for buffer pool */
#define FASTBUF_USED     0x00000001
#define FASTBUF_FW_OWNED 0x00000002
#define FASTBUF_CMD_OWNED 0x00000004
#define FASTBUF_EVENT_OWNED 0x00000008
#define FASTBUF_WRITE_OWNED 0x00000010
#define FASTBUF_OWNED_MASK \
   (  FASTBUF_FW_OWNED \
    | FASTBUF_CMD_OWNED \
    | FASTBUF_EVENT_OWNED \
    | FASTBUF_WRITE_OWNED)

/**< mps buffer management structure */
typedef struct
{
   IFX_uint32_t fb_init;
   IFX_uint32_t fb_num_of_elem;
   IFX_uint32_t fb_seg_size_byte;          /**< Buffer size for voice cpu */
   IFX_uint32_t fb_num_of_prov_init;       /**< Initial provision buffer count */
   IFX_uint32_t fb_num_of_prov_per_msg;    /**< segments per message */
   IFX_uint32_t fb_threshold;              /**< Minimum buffer count */
   IFX_uint32_t fb_threshold_ov;           /**< max threshold --> overload */

   IFX_void_t *(*fb_malloc) (IFX_size_t size, IFX_int32_t priority); /**< Buffer alloc function (def. kmalloc) */
   IFX_void_t  (*fb_free) (const IFX_void_t *ptr);  /**< Buffer free  function (def. kfree) */
   IFX_int32_t (*fb_create) (IFX_uint_t fb_seg_size__byte);   /** Manager init function */
   IFX_int32_t (*fb_release) (void);  /** Manager shutdown function */

   IFX_void_t (*fb_freeall)(void);    /** external setup: free all buffer */

   /* dynamic data - local fast buffer management */
   IFX_int32_t fb_level;                    /**< Current bufffer level */
   mps_buffer_state_e e_fb_state;

   IFX_uint32_t fb_idx;
   IFX_uint32_t *p_fb;
   volatile IFX_uint32_t *p_fb_pool;
} mps_buf_mng_t;


/* global structure that holds VCPU buffer management data */
mps_buf_mng_t mps_buffer = {
   .fb_init = 0,
   .fb_num_of_elem = 0,
   .fb_seg_size_byte = 0,
   .fb_num_of_prov_init =  0,
   .fb_num_of_prov_per_msg =  0,
   .fb_threshold = 0,
   .fb_threshold_ov = 0,

   /* fast buffer manager */
   .fb_malloc = IFX_NULL,     /*&ifx_mps_fastbuf_malloc */
   .fb_free   = IFX_NULL,     /* &ifx_mps_fastbuf_free */
   .fb_create = IFX_NULL,     /* &ifx_mps_fastbuf_create */
   .fb_release  = IFX_NULL,   /* &ifx_mps_fastbuf_release */
   .fb_freeall  = IFX_NULL,

   /* dynamic data - local fast buffer management */
   .fb_level = 0,
   .e_fb_state = MPS_BUF_UNINITIALIZED, /* MPS_BUF_EMPTY */

   .fb_idx = 0,
   .p_fb = IFX_NULL,
   .p_fb_pool = IFX_NULL
};

/* firmware image footer */
FW_image_ftr_t *pFW_img_data = IFX_NULL;

/* cache operations */
#if defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)
#define CACHE_LINE_SZ 32
IFX_boolean_t bDoCacheOps = IFX_FALSE;
static IFX_void_t ifx_mps_cache_wb_inv (IFX_ulong_t addr, IFX_uint32_t len);
#endif /*defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)*/

/* ============================= */
/* Local function definition     */
/* ============================= */


/**
 * External buffer management check
 * Checks for external buffer manager (e.g. lib_bufferpool).
 *
 * \param   none
 *
 * \return  IFX_TRUE    External buffer manager is used (e.g. lib_bufferpool)
 * \return  IFX_FALSE   MPS internal buffer manager is used (fastbuf)
 * \ingroup Internal
 */
IFX_boolean_t ifx_mps_ext_bufman(void)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   return (p_fb_mng->fb_init == MPS_FASTBUF_INIT_EXTERNAL) ? IFX_TRUE : IFX_FALSE;
}

IFX_int_t ifx_mps_fb_cntrl_reset(mps_buf_mng_t *p_fb_mng)
{
   switch (p_fb_mng->fb_init)
   {
   case 0: /* still not initialized */
      break;
   case MPS_FASTBUF_INIT_INTERNAL:
   case MPS_FASTBUF_INIT_EXTERNAL:
      p_fb_mng->e_fb_state = MPS_BUF_EMPTY;
      p_fb_mng->fb_level = 0;
      p_fb_mng->fb_idx = 0;
      break;
   default:
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s() - error, invalid cfg state = 0x%X\n",
         __FUNCTION__, p_fb_mng->fb_init));
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

IFX_int_t ifx_mps_fastbuf_cfg_setup(
   IFX_uint_t fb_num_of_elem,
   IFX_uint_t fb_seg_size__byte,
   IFX_uint_t fb_num_of_prov_init,
   IFX_uint_t fb_num_of_prov_per_msg,
   IFX_uint_t fb_threshold,
   IFX_uint_t fb_threshold_ov)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;
   mps_comm_dev *p_dev = pMPSDev;

   /* ToDo: MA
      - Check limits, max seg size, max segs
      - Check - already exists
   */

   if (p_fb_mng->fb_init)
   {
      if (p_fb_mng->fb_init == MPS_FASTBUF_INIT_EXTERNAL)
      {
         TRACE (MPS, DBG_LEVEL_HIGH, ("%s() - warning, keep external setup\n", __FUNCTION__));
      }
      else
      {
         TRACE (MPS, DBG_LEVEL_HIGH, ("%s() - error, already inuse\n", __FUNCTION__));
         return IFX_ERROR;
      }
   }

   switch (p_dev->mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_ALL)
   {
   case LTQ_VOICE_MPS_MBOX_TYPE_L1:
      if (p_fb_mng->fb_init == 0)
      {
         p_fb_mng->fb_seg_size_byte =
            fb_seg_size__byte ? fb_seg_size__byte : MPS_BUFFER_SEG_SIZE_L1;
         p_fb_mng->fb_num_of_elem =
            fb_num_of_elem ? fb_num_of_elem : MPS_BUFFER_NUM_OF_ELEMENTS_L1;
         p_fb_mng->fb_num_of_prov_init =
            fb_num_of_prov_init ? fb_num_of_prov_init : MPS_BUFFER_INITIAL_PROVISION_L1;
         p_fb_mng->fb_num_of_prov_per_msg =
            fb_num_of_prov_per_msg ? fb_num_of_prov_per_msg : MPS_BUFFER_PROV_PER_MSG_L1;
         p_fb_mng->fb_threshold =
            fb_threshold ? fb_threshold : MPS_BUFFER_THRESHOLD_L1;
         p_fb_mng->fb_threshold_ov =
            fb_threshold_ov ? fb_threshold_ov : MPS_BUFFER_THRESHOLD_OVLOAD_L1;
      }
      else
      {
         /* keep external SEG Size */
         if (p_fb_mng->fb_seg_size_byte == 0)
            {p_fb_mng->fb_seg_size_byte =
                    fb_seg_size__byte ? fb_seg_size__byte : MPS_BUFFER_SEG_SIZE_L1;}
         if (p_fb_mng->fb_num_of_elem == 0)
            {p_fb_mng->fb_num_of_elem =
                    fb_num_of_elem ? fb_num_of_elem : MPS_BUFFER_NUM_OF_ELEMENTS_L1;}
         if (p_fb_mng->fb_num_of_prov_init == 0)
            {p_fb_mng->fb_num_of_prov_init =
                    fb_num_of_prov_init ? fb_num_of_prov_init : MPS_BUFFER_INITIAL_PROVISION_L1;}
         if (p_fb_mng->fb_num_of_prov_per_msg == 0)
            {p_fb_mng->fb_num_of_prov_per_msg =
                    fb_num_of_prov_per_msg ? fb_num_of_prov_per_msg : MPS_BUFFER_PROV_PER_MSG_L1;}
         if (p_fb_mng->fb_threshold == 0)
            {p_fb_mng->fb_threshold =
                    fb_threshold ? fb_threshold : MPS_BUFFER_THRESHOLD_L1;}
         if (p_fb_mng->fb_threshold_ov == 0)
            {p_fb_mng->fb_threshold_ov =
                    fb_threshold_ov ? fb_threshold_ov : MPS_BUFFER_THRESHOLD_OVLOAD_L1;}
      }
      break;

   case LTQ_VOICE_MPS_MBOX_TYPE_L2:
      if (p_fb_mng->fb_init == 0)
      {
         p_fb_mng->fb_seg_size_byte =
            fb_seg_size__byte ? fb_seg_size__byte : MPS_BUFFER_SEG_SIZE_L2;
         p_fb_mng->fb_num_of_elem =
            fb_num_of_elem ? fb_num_of_elem : MPS_BUFFER_NUM_OF_ELEMENTS_L2;
         p_fb_mng->fb_threshold =
            fb_threshold ? fb_threshold : MPS_BUFFER_THRESHOLD_L2;
         p_fb_mng->fb_num_of_prov_init =
            fb_num_of_prov_init ? fb_num_of_prov_init : MPS_BUFFER_INITIAL_PROVISION_L2;
         p_fb_mng->fb_num_of_prov_per_msg =
            fb_num_of_prov_per_msg ? fb_num_of_prov_per_msg : MPS_BUFFER_PROV_PER_MSG_L2;
         p_fb_mng->fb_threshold_ov =
            fb_threshold_ov ? fb_threshold_ov : MPS_BUFFER_THRESHOLD_OVLOAD_L2;
      }
      else
      {
         /* external setup: keep SEG Size */
         if (p_fb_mng->fb_seg_size_byte == 0)
            {p_fb_mng->fb_seg_size_byte =
                    fb_seg_size__byte ? fb_seg_size__byte : MPS_BUFFER_SEG_SIZE_L2;}
         if (p_fb_mng->fb_num_of_elem == 0)
            {p_fb_mng->fb_num_of_elem =
                    fb_num_of_elem ? fb_num_of_elem : MPS_BUFFER_NUM_OF_ELEMENTS_L2;}
         if (p_fb_mng->fb_num_of_prov_init == 0)
            {p_fb_mng->fb_num_of_prov_init =
                    fb_num_of_prov_init ? fb_num_of_prov_init : MPS_BUFFER_INITIAL_PROVISION_L2;}
         if (p_fb_mng->fb_num_of_prov_per_msg == 0)
            {p_fb_mng->fb_num_of_prov_per_msg =
                    fb_num_of_prov_per_msg ? fb_num_of_prov_per_msg : MPS_BUFFER_PROV_PER_MSG_L2;}
         if (p_fb_mng->fb_threshold == 0)
            {p_fb_mng->fb_threshold =
                    fb_threshold ? fb_threshold : MPS_BUFFER_THRESHOLD_L2;}
         if (p_fb_mng->fb_threshold_ov == 0)
            {p_fb_mng->fb_threshold_ov =
                    fb_threshold_ov ? fb_threshold_ov : MPS_BUFFER_THRESHOLD_OVLOAD_L2;}
      }
      break;

   default:
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - missing init, MBox descr = 0x%X (unknown)\r\n",
              __FUNCTION__, p_dev->mbx_descr));
      return IFX_ERROR;
   }

   /* external setup: keep SEG Size */
   if (p_fb_mng->fb_init == 0)
   {
      p_fb_mng->e_fb_state = MPS_BUF_UNINITIALIZED;
      p_fb_mng->fb_level = 0;
      p_fb_mng->fb_idx = 0;

      p_fb_mng->fb_malloc = &ifx_mps_fastbuf_malloc;
      p_fb_mng->fb_free   = &ifx_mps_fastbuf_free;
      p_fb_mng->fb_create = &ifx_mps_fastbuf_create;
      p_fb_mng->fb_release  = &ifx_mps_fastbuf_release;
   }

   return IFX_SUCCESS;
}

IFX_int_t ifx_mps_fastbuf_cfg_release(void)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   /* ToDo: MA
      - Check limits, max seg size, max segs
      - Check - already exists
   */
   if (p_fb_mng->fb_init == 0)
      {return IFX_SUCCESS;}

   if (p_fb_mng->fb_init == MPS_FASTBUF_INIT_EXTERNAL)
   {
      TRACE (MPS, DBG_LEVEL_NORMAL,
             ("%s(): info - release fast buffer external cfg\r\n", __FUNCTION__));
   }

   p_fb_mng->fb_num_of_elem = 0;
   p_fb_mng->fb_seg_size_byte = 0;
   p_fb_mng->fb_num_of_prov_init = 0;
   p_fb_mng->fb_num_of_prov_per_msg = 0;
   p_fb_mng->fb_threshold = 0;
   p_fb_mng->fb_threshold_ov = 0;

   p_fb_mng->e_fb_state = MPS_BUF_UNINITIALIZED;
   p_fb_mng->fb_level = 0;
   p_fb_mng->fb_idx = 0;

   p_fb_mng->fb_malloc = IFX_NULL;
   p_fb_mng->fb_free   = IFX_NULL;
   p_fb_mng->fb_create = IFX_NULL;
   p_fb_mng->fb_release  = IFX_NULL;

   return IFX_SUCCESS;
}

/**
 * Buffer allocate
 * Allocates and returns a buffer from the buffer pool.
 *
 * \param   size        Size of requested buffer
 * \param   priority    Ignored, always atomic
 *
 * \return  ptr    Address of the allocated buffer
 * \return  NULL   No buffer available
 * \ingroup Internal
 */
IFX_void_t *ifx_mps_fastbuf_malloc(
   IFX_size_t size__byte,
   IFX_int32_t priority)
{
   IFXOS_INTSTAT flags;
   IFX_uint32_t fb_addr32;
   IFX_int32_t fb_idx;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   if (p_fb_mng->fb_init != MPS_FASTBUF_INIT_INTERNAL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s() - error, fast buffer init missing / not internal (fb_init = %d)\n",
              __FUNCTION__, p_fb_mng->fb_init));
      return IFX_NULL;
   }

   if (size__byte > p_fb_mng->fb_seg_size_byte)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s() - error, fast buffer (internal) size too large (user_size = %d > fb_seg_size_byte =%d)\n",
              __FUNCTION__, (int)size__byte, p_fb_mng->fb_seg_size_byte));
      return IFX_NULL;
   }

   IFXOS_LOCKINT (flags);
   fb_idx = p_fb_mng->fb_idx;
   do
   {
      if (fb_idx == p_fb_mng->fb_num_of_elem)
         {fb_idx = 0;}
      if (p_fb_mng->p_fb_pool[fb_idx] & FASTBUF_USED)
         {continue;}

      fb_addr32 = p_fb_mng->p_fb_pool[fb_idx];
      p_fb_mng->p_fb_pool[fb_idx] |= FASTBUF_USED;
      if ((priority == FASTBUF_FW_OWNED) || (priority == FASTBUF_CMD_OWNED) ||
          (priority == FASTBUF_EVENT_OWNED) ||
          (priority == FASTBUF_WRITE_OWNED))
      {
              p_fb_mng->p_fb_pool[fb_idx] |= (priority & FASTBUF_OWNED_MASK);
      }
      p_fb_mng->fb_idx = fb_idx;
      IFXOS_UNLOCKINT (flags);
      return (IFX_void_t *) fb_addr32;
   } while (++fb_idx != p_fb_mng->fb_idx);
   IFXOS_UNLOCKINT (flags);

   TRACE (MPS, DBG_LEVEL_HIGH,
          ("%s() - error, buffer pool (internal) empty\n", __FUNCTION__));

   return IFX_NULL;
}



/**
 * Buffer free
 * Returns a buffer to the buffer pool.
 *
 * \param   ptr    Address of the allocated buffer
 *
 * \return  none
 * \ingroup Internal
 */
IFX_void_t ifx_mps_fastbuf_free(
   const IFX_void_t *ptr)
{
   IFXOS_INTSTAT flags;
   IFX_uint32_t fb_addr32 = (IFX_uint32_t)ptr;
   IFX_int_t fb_idx;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   if (p_fb_mng->fb_init != MPS_FASTBUF_INIT_INTERNAL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s() - error, fast buffer init missing / not internal\n", __FUNCTION__));
      return;
   }

   IFXOS_LOCKINT (flags);
   fb_idx = p_fb_mng->fb_idx;
   do
   {
      if (fb_idx < 0)
         {fb_idx = p_fb_mng->fb_num_of_elem - 1;}

      if ((p_fb_mng->p_fb_pool[fb_idx] & ~FASTBUF_OWNED_MASK) == (fb_addr32 | FASTBUF_USED))
      {
         p_fb_mng->p_fb_pool[fb_idx] &= ~FASTBUF_OWNED_MASK;
         p_fb_mng->p_fb_pool[fb_idx] &= ~FASTBUF_USED;
         IFXOS_UNLOCKINT (flags);
         return;
      }
   } while (--fb_idx != p_fb_mng->fb_idx);
   IFXOS_UNLOCKINT (flags);

   TRACE (MPS, DBG_LEVEL_HIGH,
          ("%s() - error, buffer not inside pool (0x%p, internal)\n", __FUNCTION__, ptr));

   return;
}

/*
   Note: internal fast buffer management
   The following setup values are FW related (Layout 1/2):
   - fb_num_of_elem, number of fast buffer elements
   - fb_threshold, treshold when additional buffers are send to the FW
   - fb_num_of_prov_init, number of buffers provided at the startup
*/

/**
 * Bufferpool init
 * Initializes a buffer pool with the previos configured values (depends on the used FW image)
 * and separates it into an index array which is part of the buffer management.
 * The 32byte alignment of the chunks is guaranteed by increasing the buffer size accordingly.
 * Bit 0 of the address in msg_buffer.p_fb_pool[x] is used as busy indicator.
 *
 * \return -ENOMEM  Memory allocation failed
 * \return  IFX_SUCCESS      Buffer pool initialized
 * \ingroup Internal
 */
IFX_int_t ifx_mps_fastbuf_create(
   IFX_uint_t fb_seg_size__byte)
{
   IFX_uint_t i;
   IFX_uint32_t *p_fb = IFX_NULL, *p_fb_pool = IFX_NULL;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;
   /* mps_comm_dev *p_dev = pMPSDev; */

   /* ToDo: MA
      - Check limits, max seg size, max segs
   */
   MPS_DEBUG_FCT_ENTER;

   if (p_fb_mng->fb_init)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s() - error, already inuse\n", __FUNCTION__));
      return IFX_ERROR;
   }

   if (fb_seg_size__byte)
   {
      p_fb_mng->fb_seg_size_byte = (fb_seg_size__byte + (fb_seg_size__byte % 32));
   }

   p_fb = IFXOS_BlockAlloc((p_fb_mng->fb_seg_size_byte * p_fb_mng->fb_num_of_elem));
   p_fb_pool = IFXOS_BlockAlloc(p_fb_mng->fb_num_of_elem * sizeof(IFX_uint32_t));
   if ((p_fb == IFX_NULL) || (p_fb_pool == IFX_NULL))
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s() - error, mem alloc\n", __FUNCTION__));
      if (p_fb != IFX_NULL) {IFXOS_BlockFree(p_fb);}
      if (p_fb_pool != IFX_NULL) {IFXOS_BlockFree(p_fb_pool);}
      return -ENOMEM;
   }

   memset(p_fb, 0x0, (p_fb_mng->fb_seg_size_byte * p_fb_mng->fb_num_of_elem));
   for (i = 0; i < p_fb_mng->fb_num_of_elem; i++)
      {p_fb_pool[i] = (IFX_uint32_t)&p_fb[i * p_fb_mng->fb_seg_size_byte];}

   p_fb_mng->fb_idx = 0;
   p_fb_mng->p_fb = p_fb;
   p_fb_mng->p_fb_pool = p_fb_pool;

   p_fb_mng->fb_level = 0;
   p_fb_mng->e_fb_state = MPS_BUF_EMPTY;
   p_fb_mng->fb_init = MPS_FASTBUF_INIT_INTERNAL;

   return IFX_SUCCESS;
}


/**
 * Bufferpool close
 * Frees the buffer pool allocated by ifx_mps_fastbuf_create and clears the
 * buffer pool.
 *
 * \return -ENOMEM  Memory allocation failed
 * \return  IFX_SUCCESS      Buffer pool initialized
 * \ingroup Internal
 */
IFX_int_t ifx_mps_fastbuf_release(void)
{
   IFX_uint32_t *p_fb = IFX_NULL;
   volatile IFX_uint32_t *p_fb_pool = IFX_NULL;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   MPS_DEBUG_FCT_ENTER;

   if (!p_fb_mng->fb_init)
      {return IFX_SUCCESS;}

   p_fb = p_fb_mng->p_fb;
   p_fb_pool = p_fb_mng->p_fb_pool;

   p_fb_mng->fb_init = 0;
   p_fb_mng->fb_idx = 0;

   p_fb_mng->p_fb = IFX_NULL;
   p_fb_mng->p_fb_pool = IFX_NULL;

   if (p_fb != IFX_NULL) {IFXOS_BlockFree(p_fb);}
   if (p_fb_pool != IFX_NULL) {IFXOS_BlockFree((void *)p_fb_pool);}

   return IFX_SUCCESS;
}



/******************************************************************************
 * Buffer manager
 ******************************************************************************/

/**
 * Get buffer fill level
 * This function return the current number of buffers provided to CPU1
 *
 * \return  level    The current number of buffers
 * \return  -1       The buffer state indicates an error
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_bufman_get_level (void)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   if ((p_fb_mng->e_fb_state != MPS_BUF_ERR) &&
       (p_fb_mng->e_fb_state != MPS_BUF_UNINITIALIZED))
      {return p_fb_mng->fb_level;}

   return -1;
}


/**
 * Update buffer state
 * This function will set the buffer state according to the current buffer level
 * and the previous state.
 *
 * \return  state    The new buffer state
 * \ingroup Internal
 */
static mps_buffer_state_e ifx_mps_bufman_update_state (void)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   if ((p_fb_mng->e_fb_state != MPS_BUF_ERR) && (p_fb_mng->e_fb_state != MPS_BUF_UNINITIALIZED))
   {
      if (p_fb_mng->fb_level == 0)
         p_fb_mng->e_fb_state = MPS_BUF_EMPTY;
      if ((p_fb_mng->fb_level > 0) &&
          (p_fb_mng->fb_level < mps_buffer.fb_threshold))
         p_fb_mng->e_fb_state = MPS_BUF_LOW;
      if ((p_fb_mng->fb_level >= p_fb_mng->fb_threshold) &&
          (p_fb_mng->fb_level <= p_fb_mng->fb_threshold_ov))
         p_fb_mng->e_fb_state = MPS_BUF_OK;
      if (p_fb_mng->fb_level > p_fb_mng->fb_threshold_ov)
         p_fb_mng->e_fb_state = MPS_BUF_OV;
   }
   return p_fb_mng->e_fb_state;
}


/**
   Increase buffer level
   This function increments the buffer level with the passed value.

   \param   value       Increment value.
   \return  level       The new buffer level.

   \ingroup Internal
*/
static IFX_int32_t ifx_mps_bufman_inc_level (IFX_uint32_t value)
{
   IFXOS_INTSTAT flags;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   IFXOS_LOCKINT (flags);
   p_fb_mng->fb_level += value;
   ifx_mps_bufman_update_state ();
   IFXOS_UNLOCKINT (flags);

   return p_fb_mng->fb_level;
}


/**
   Decrease buffer level
   This function decrements the buffer level with the passed value. A lowest
   level of zero is ensured.

   \param   value       Decrement value.
   \return  level       The new buffer level.

   \ingroup Internal
*/
static IFX_int32_t ifx_mps_bufman_dec_level (IFX_uint32_t value)
{
   IFXOS_INTSTAT flags;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   IFXOS_LOCKINT (flags);
   if (p_fb_mng->fb_level < value)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): Decrement %u exceeds current level %u!\n",
             __FUNCTION__, value, p_fb_mng->fb_level));
      p_fb_mng->fb_level = 0;
   }
   else
      {p_fb_mng->fb_level -= value;}

   ifx_mps_bufman_update_state ();
   IFXOS_UNLOCKINT (flags);

   return p_fb_mng->fb_level;
}


/**
 * Init buffer management
 * This function initializes the buffer management data structures and
 * provides buffer segments to CPU1.
 *
 * \return  0        IFX_SUCCESS, initialized and message sent
 * \return  -1       Error during message transmission
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_bufman_init(void)
{
   IFX_int32_t ret = IFX_SUCCESS;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   MPS_DEBUG_FCT_ENTER;

   switch (p_fb_mng->fb_init)
   {
   case 0: /* still not initialized */
      /* setup (use intenal default values) and create */
      if ((ret = ifx_mps_fastbuf_cfg_setup(0, 0, 0, 0, 0, 0)) == IFX_SUCCESS)
         {ret = p_fb_mng->fb_create(0);}
      break;
   case MPS_FASTBUF_INIT_INTERNAL:
      /* keep current config, reset current setup */
      ret = ifx_mps_fb_cntrl_reset(p_fb_mng);
      break;
   case MPS_FASTBUF_INIT_EXTERNAL:
      TRACE (MPS, DBG_LEVEL_NORMAL, ("%s() - info, external, provide buffer\n",
         __FUNCTION__));
      ret = ifx_mps_fb_cntrl_reset(p_fb_mng);
      break;
   default:
      TRACE (MPS, DBG_LEVEL_NORMAL, ("%s() - error, invalid cfg state = 0x%X\n",
         __FUNCTION__, p_fb_mng->fb_init));
      ret = IFX_ERROR;
      break;
   }

   if (ret == IFX_SUCCESS)
   {
      ret = ifx_mps_bufman_buf_provide(
              p_fb_mng->fb_num_of_prov_init, /* init - provide n startup buffer */
              0, 0);    /* use default values */
   }

   return ret;
}


/**
 * Close buffer management
 * This function is called on termination of voice CPU firmware. The registered
 * fb_release function has to take care of freeing buffers still left in VCPU.
 *
 * \return  0        IFX_SUCCESS, buffer manage shutdown correctly
 * \return  -1       Error during shutdown
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_bufman_close(void)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   MPS_DEBUG_FCT_ENTER;

   switch (p_fb_mng->fb_init)
   {
   case 0:
      break;
   case MPS_FASTBUF_INIT_INTERNAL:
      (void)p_fb_mng->fb_release();
      (void)ifx_mps_fastbuf_cfg_release();
      break;
   case MPS_FASTBUF_INIT_EXTERNAL:
      /* cannot close - external buffer management */
      TRACE (MPS, DBG_LEVEL_NORMAL,
             ("%s() - info, external config (reset control)\n", __FUNCTION__));
      (void)ifx_mps_fb_cntrl_reset(p_fb_mng);
      break;
   default:
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s() - error, invalid cfg state = 0x%X\n",
         __FUNCTION__, p_fb_mng->fb_init));
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/**
 * Free buffer
 *
 * \ingroup Internal
 */
IFX_void_t ifx_mps_bufman_free (const IFX_void_t * ptr)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   if (!p_fb_mng->fb_free)
      {return;}

   if ((IFX_void_t *)CPHYSADDR(ptr) != NULL)
      {p_fb_mng->fb_free ((IFX_void_t *) KSEG0ADDR (ptr));}

   return;
}

/**
 * Allocate buffer
 *
 * \ingroup Internal
 */
IFX_void_t *ifx_mps_bufman_malloc (IFX_size_t size, IFX_int32_t priority)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;
   IFX_void_t *ptr;

   if (!p_fb_mng->fb_malloc)
      {return IFX_NULL;}

   ptr = p_fb_mng->fb_malloc (size, priority);
   return ptr;
}

IFX_void_t ifx_mps_bufman_register_new(
   IFX_void_t *(*fb_ext_malloc) (IFX_size_t size, IFX_int32_t priority),
   IFX_void_t (*fb_ext_free) (const IFX_void_t * ptr),
   IFX_void_t (*fb_ext_freeall)(void),
   IFX_uint32_t fb_num_of_elem,
   IFX_uint32_t fb_seg_size_byte,
   IFX_uint32_t fb_treshold)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   MPS_DEBUG_FCT_ENTER;

   if (p_fb_mng->fb_init)
   {
      if (p_fb_mng->fb_init == MPS_FASTBUF_INIT_EXTERNAL)
      {
         TRACE (MPS, DBG_LEVEL_HIGH, ("%s() - warning, overwrite current setup\n", __FUNCTION__));
      }
      else
      {
         TRACE (MPS, DBG_LEVEL_HIGH, ("%s() - error, already inuse\n", __FUNCTION__));
         return;
      }
   }

   if (fb_seg_size_byte)
      {p_fb_mng->fb_seg_size_byte = fb_seg_size_byte;}
   if (fb_num_of_elem)
      {p_fb_mng->fb_num_of_elem = fb_num_of_elem;}


/*
#define MPS_DEFAULT_PROVISION_SEGMENTS_PER_MSG   12
   p_fb_mng->fb_threshold = fb_treshold;
   p_fb_mng->fb_num_of_prov_init = treshold + MPS_DEFAULT_PROVISION_SEGMENTS_PER_MSG;
*/
   p_fb_mng->fb_malloc = fb_ext_malloc;
   p_fb_mng->fb_free   = fb_ext_free;
   if (fb_ext_freeall != IFX_NULL)
      {p_fb_mng->fb_freeall = fb_ext_freeall;}
   p_fb_mng->e_fb_state = MPS_BUF_EMPTY;
   p_fb_mng->fb_init    = MPS_FASTBUF_INIT_EXTERNAL;

   return;
}

/**
 * Overwrite buffer management
 * Allows the upper layer to register its own fb_malloc/fb_free functions in order to do
 * its own buffer managment. To unregister driver needs to be re-initialized.
 *
 * \param   fb_malloc      Buffer allocation - arguments and return value as kmalloc
 * \param   fb_free        Buffer de-allocation - arguments and return value as kmalloc
 * \param   buf_size    Size of buffers provided to voice CPU
 * \param   treshold    Count of buffers provided to voice CPU
 */
IFX_void_t ifx_mps_bufman_register(
   IFX_void_t *(*fb_malloc) (IFX_size_t size, IFX_int32_t priority),
   IFX_void_t (*fb_free) (const IFX_void_t * ptr),
   IFX_uint32_t fb_seg_size_byte,
   IFX_uint32_t treshold)
{
   ifx_mps_bufman_register_new(
      fb_malloc, fb_free, IFX_NULL,
      0, fb_seg_size_byte, treshold);

   return;
}


IFX_void_t ifx_mps_bufman_unregister(void)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;
   mps_comm_dev *p_dev = pMPSDev;

   MPS_DEBUG_FCT_ENTER;

   if (p_fb_mng->fb_init == MPS_FASTBUF_INIT_EXTERNAL)
   {
      /* Check - release all external buffer
      if (fb_ext_freeall != IFX_NULL)
         {p_fb_mng->fb_freeall();}
      */
      p_fb_mng->fb_init= 0;
      if (p_dev->mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_ALL)
      {
         /* reset internal default values */
         (void)ifx_mps_fastbuf_cfg_setup(0, 0, 0, 0, 0, 0);
      }
      else
      {
         p_fb_mng->fb_seg_size_byte = 0;
         p_fb_mng->fb_num_of_elem = 0;
         p_fb_mng->e_fb_state = MPS_BUF_UNINITIALIZED;
         p_fb_mng->fb_malloc = IFX_NULL;
         p_fb_mng->fb_free   = IFX_NULL;
         p_fb_mng->fb_freeall = IFX_NULL;
      }
   }

   return;
}


/**
 * Register callback function to free all data buffers currently used by
 * voice firmware. Called by VMMC driver.
 * \param   none
 * \return  none
 *
 * \ingroup Internal
 */
IFX_void_t ifx_mps_register_bufman_freeall_callback(IFX_void_t (*pfn)(void))
{
   MPS_DEBUG_FCT_ENTER;
   ifx_mps_bufman_freeall = pfn;
}

IFX_void_t ifx_mps_bufman_freeall_xx(void)
{
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   if ((p_fb_mng->fb_init == MPS_FASTBUF_INIT_EXTERNAL) &&
       (p_fb_mng->fb_freeall != IFX_NULL))
      {p_fb_mng->fb_freeall();}

   return;
}

/**
 * Send buffer provisioning message
 * This function sends a buffer provisioning message to CPU1 using the passed
 * segment parameters.
 *
 * \param   segments     Number of memory segments to be provided to CPU1
 * \param   segment_size Size of each memory segment in bytes
 * \return  0            IFX_SUCCESS, message sent
 * \return  -1           IFX_ERROR, if message could not be sent
 * \ingroup Internal
 */

IFX_int32_t ifx_mps_bufman_buf_provide(
   IFX_uint_t n_segs,
   IFX_uint_t seg_size_byte,
   IFX_uint_t n_segs_per_msg)
{
   IFX_int_t retval = IFX_SUCCESS, rettmp = IFX_SUCCESS;

   IFX_uint_t i, n_segs_processed = 0, n_segs_free, n_segs_send;
   IFX_uint32_t mem_avail_32 = 0;
   MbxMsg_s msg;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   MPS_DEBUG_FCT_ENTER;

   if (!p_fb_mng->fb_init)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s(): missing fast buf init\n", __FUNCTION__));
      return IFX_ERROR;
   }

   /* take over internal default values if not set */
   seg_size_byte = (seg_size_byte) ? seg_size_byte : p_fb_mng->fb_seg_size_byte;
   n_segs_per_msg = (n_segs_per_msg) ?
      ((n_segs_per_msg < p_fb_mng->fb_num_of_prov_per_msg) ? n_segs_per_msg : p_fb_mng->fb_num_of_prov_per_msg) :
         p_fb_mng->fb_num_of_prov_per_msg;
   /* if n_segs it not set, assume we have to provide default provision buffers per message */
   n_segs = (n_segs) ? n_segs : n_segs_per_msg;

   /*
   TRACE (MPS, DBG_LEVEL_LOW,
      ("%s(): n_segs = %d, n_segs_per_msg = %d, seg_size_byte = %d\n",
      __FUNCTION__, n_segs, n_segs_per_msg, seg_size_byte));
   */

   while (n_segs_processed < n_segs)
   {
      n_segs_send = 0;
      n_segs_processed += n_segs_per_msg;

      /* Check available mailbox memory and adjust number of segments,
         if necessary */
      mem_avail_32 = ifx_mps_fifo_mem_available (&pMPSDev->voice_dwstrm_fifo) / sizeof(IFX_uint32_t);
      if (mem_avail_32 < (n_segs_per_msg + 2))
         {n_segs_free = (mem_avail_32 >= 3) ? (mem_avail_32 - 2) : 0;}
      else
         {n_segs_free = n_segs_per_msg;}

      if (n_segs_free)
      {
         memset (&msg, 0, sizeof (msg));
         for (i = 0; i < n_segs_free; i++)
         {
            msg.data[i] = (IFX_uint32_t)
               CPHYSADDR((IFX_uint32_t)p_fb_mng->fb_malloc(seg_size_byte, FASTBUF_FW_OWNED));
            if (msg.data[i] == (IFX_uint32_t)CPHYSADDR(IFX_NULL))
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                   ("%s(): cannot allocate buffer\n", __FUNCTION__));

               rettmp = IFX_ERROR;
               break;
            }
            n_segs_send++;
            /* invalidate cache */
            ifx_mps_cache_inv (KSEG0ADDR((IFX_uint32_t *)msg.data[i]), seg_size_byte);
         }

         if (n_segs_send)
         {
            /* Header: number of provision buffers + size field */
            msg.data[n_segs_send] = seg_size_byte;
            msg.header.hd.plength = (n_segs_send * sizeof(IFX_uint32_t)) + sizeof(IFX_uint32_t);
            msg.header.hd.type = CMD_ADDRESS_PACKET;
            rettmp = ifx_mps_mbx_write_message(
               (&pMPSDev->voice_mb[0]), (IFX_uint8_t *)&msg,
               (IFX_uint32_t)((n_segs_send + 2) * sizeof(IFX_uint32_t)));
            if (rettmp != IFX_SUCCESS)
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                      ("%s() - failed to write message\n", __FUNCTION__));

               atomic_inc (&ifx_mps_write_blocked);
               /* enable data downstream mailbox interrupt */
               ifx_mps_dd_mbx_int_enable ();
               for (i = 0; i < n_segs_send; i++)
                  {p_fb_mng->fb_free((IFX_void_t *)KSEG0ADDR((IFX_uint32_t *)msg.data[i]));}

               retval = (retval != IFX_SUCCESS) ? retval : rettmp;
            }
            else
            {
               ifx_mps_bufman_inc_level (n_segs_send);
               /* disable data downstream mailbox interrupt */
               ifx_mps_dd_mbx_int_disable ();
               atomic_set (&ifx_mps_write_blocked, 0);
            }
         }
      }   /* if (n_segs_free) */
   }    /* while (n_segs_processed < n_segs) */

   if (retval != IFX_SUCCESS)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
         ("%s() - with error\n", __FUNCTION__));
   }

   return retval;
}


/******************************************************************************
 * FIFO Managment
 ******************************************************************************/

/**
 * Clear FIFO
 * This function clears the FIFO by resetting the pointers. The data itself is
 * not cleared.
 *
 * \param   fifo    Pointer to FIFO structure
 * \ingroup Internal
 */
IFX_void_t ifx_mps_fifo_clear (mps_fifo * fifo)
{
   if (fifo == IFX_NULL)
      {return;}

   if ((fifo->pread_off == IFX_NULL) ||
       (fifo->pwrite_off != IFX_NULL) ||
       (fifo->size < 4))
      {return;}

   *fifo->pread_off = fifo->size - 4;
   *fifo->pwrite_off = fifo->size - 4;

   return;
}


/**
 * Check FIFO for being not empty
 * This function checks whether the referenced FIFO contains at least
 * one unread data byte.
 *
 * \param   fifo     Pointer to FIFO structure
 * \return  1        TRUE if data to be read is available in FIFO,
 * \return  0        FALSE if FIFO is empty.
 * \ingroup Internal
 */
IFX_boolean_t ifx_mps_fifo_not_empty (mps_fifo * fifo)
{
   if (fifo == IFX_NULL)
      {return IFX_FALSE;}
   if ((fifo->pwrite_off == IFX_NULL) || (fifo->pread_off == IFX_NULL))
      {return IFX_FALSE;}

   if (*fifo->pwrite_off == *fifo->pread_off)
      return IFX_FALSE;
   else
      return IFX_TRUE;
}


/**
 * Check FIFO for free memory
 * This function returns the amount of free bytes in FIFO.
 *
 * \param   fifo     Pointer to FIFO structure
 * \return  0        The FIFO is full,
 * \return  count    The number of available bytes
 * \ingroup Internal
 */
IFX_uint32_t ifx_mps_fifo_mem_available (mps_fifo * fifo)
{
   IFX_uint32_t retval;

   if (fifo == IFX_NULL)
      {return 0;}

   if ((fifo->pread_off == IFX_NULL) ||
       (fifo->pwrite_off == IFX_NULL) ||
       (fifo->size == 0))
      {return 0;}

   /* The order of the terms is important to stay positive (unsigned). */
   retval = fifo->size + *fifo->pwrite_off - *fifo->pread_off - 1;
   /* Discard the carry (result-domain overflow) from the calculation. */
   if (retval >= fifo->size)
   {
      retval -= fifo->size;
   }

   return retval;
}


/**
 * Check FIFO for requested amount of memory
 * This function checks whether the requested FIFO is capable to store
 * the requested amount of data bytes.
 * The selected Fifo should be a downstream direction Fifo.
 *
 * \param   fifo     Pointer to mailbox structure to be checked
 * \param   bytes    Requested data bytes
 * \return  1        TRUE if space is available in FIFO,
 * \return  0        FALSE if not enough space in FIFO.
 * \ingroup Internal
 */
IFX_boolean_t ifx_mps_fifo_mem_request (mps_fifo * fifo, IFX_uint32_t bytes)
{
   IFX_uint32_t bytes_avail = ifx_mps_fifo_mem_available (fifo);

   if (bytes_avail > bytes)
   {
      return IFX_TRUE;
   }
   else
   {
      return IFX_FALSE;
   }
}


/**
 * Update FIFO read pointer
 * This function updates the position of the referenced FIFO.In case of
 * reaching the FIFO's end the pointer is set to the start position.
 *
 * \param   fifo      Pointer to FIFO structure
 * \param   increment Increment for read index
 * \ingroup Internal
 */
IFX_void_t ifx_mps_fifo_read_ptr_inc (mps_fifo * fifo, IFX_uint8_t increment)
{
   IFX_int32_t new_read_index;

   if (fifo == IFX_NULL)
      {return;}
   if (fifo->pread_off == IFX_NULL)
      {return;}
   if ((IFX_uint32_t) increment > fifo->size)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): Invalid offset passed: %d (fifo size = %d)!\n",
              __FUNCTION__, increment, fifo->size));
      return;
   }

   new_read_index = (IFX_int32_t)(*fifo->pread_off) - (IFX_int32_t)increment;

   if (new_read_index >= 0)
   {
      *(fifo->pread_off) = (IFX_uint32_t) new_read_index; /* no overflow */
   }
   else
   {
      *(fifo->pread_off) = (IFX_uint32_t) (new_read_index + (IFX_int32_t)(fifo->size));
   }

   return;
}


/**
 * Update FIFO write pointer
 * This function updates the position of the write pointer of the referenced FIFO.
 * In case of reaching the FIFO's end the pointer is set to the start position.
 *
 * \param   fifo      Pointer to FIFO structure
 * \param   increment Increment of write index
 * \ingroup Internal
 */
IFX_void_t ifx_mps_fifo_write_ptr_inc (mps_fifo * fifo, u16 increment)
{
   /* calculate new index ignoring ring buffer overflow */
   IFX_int32_t new_write_index;

   if (fifo == IFX_NULL)
      {return;}
   if (fifo->pwrite_off == IFX_NULL)
      {return;}
   if ((IFX_uint32_t) increment > fifo->size)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): Invalid offset passed: %d (fifo size = %d)!\n",
              __FUNCTION__, increment, fifo->size));
      return;
   }

   new_write_index = (IFX_int32_t)(*fifo->pwrite_off) - (IFX_int32_t)increment;

   if (new_write_index >= 0)
   {
      *fifo->pwrite_off = (IFX_uint32_t) new_write_index; /* no overflow */
   }
   else
   {
      *fifo->pwrite_off =
         (IFX_uint32_t) (new_write_index + (IFX_int32_t) (fifo->size));
   }

   return;
}

/**
 * Write data word to FIFO
 * This function writes a data word (32bit) to the referenced FIFO. The word is
 * written to the position defined by the current write pointer index and the
 * offset being passed.
 *
 * \param   fifo           Pointer to FIFO structure
 * \param   data           Data word to be written
 * \param   offset         Byte offset to be added to write pointer position
 * \return  0              IFX_SUCCESS, word written
 * \return  -1             Invalid offset.
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_fifo_write (mps_fifo * fifo, IFX_uint32_t data,
                                IFX_uint8_t offset)
{
   /* calculate write position */
   IFX_int32_t new_write_index;
   IFX_uint32_t write_address;

   if (fifo == IFX_NULL)
      {return IFX_ERROR;}
   if ((fifo->pwrite_off == IFX_NULL) || (fifo->pend == IFX_NULL))
      {return IFX_ERROR;}
   if (offset > fifo->size)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): Invalid offset passed - offset = %d (fifo size = %d)!\n",
              __FUNCTION__, offset, fifo->size));
      return -1;
   }

   new_write_index = (IFX_int32_t)(*fifo->pwrite_off) - (IFX_int32_t)offset;

   write_address =
      (IFX_uint32_t)fifo->pend + *fifo->pwrite_off - (IFX_uint32_t)offset;
   if (new_write_index < 0)
   {
      write_address += fifo->size;
   }

   *(IFX_uint32_t *) write_address = data;

   return 0;
}


#ifndef MODULE
/* extern IFX_void_t show_trace(long *sp); */
#endif /* */

/**
 * Read data word from FIFO
 * This function reads a data word (32bit) from the referenced FIFO. It first
 * calculates and checks the address defined by the FIFO's read index and passed
 * offset. The read pointer is not updated by this function.
 * It has to be updated after the complete message has been read.
 *
 * \param   fifo          Pointer to FIFO structure
 * \param   offset        Offset to read pointer position to be read from
 * \return  count         Number of data words read.
 * \return  -1            Invalid offset
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_fifo_read (mps_fifo * fifo, IFX_uint8_t offset,
                               IFX_uint32_t * pData)
{
   IFX_uint32_t read_address;
   IFX_int32_t new_read_index;
   IFX_int32_t ret;

   if ((fifo == IFX_NULL) || (pData == IFX_NULL))
      {return IFX_ERROR;}
   if ((fifo->pread_off == IFX_NULL) || (fifo->pend == IFX_NULL))
      {return IFX_ERROR;}

   if (!ifx_mps_fifo_not_empty (fifo))
   {
#ifndef MODULE
/*         long *sp; */
#endif /* */
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): FIFO %p is empty\n", __FUNCTION__, fifo));
#ifndef MODULE
/*
   __asm__("move %0, $29;":"=r"(sp));
   show_trace(sp);
*/
#endif /* */
      ret = IFX_ERROR;
   }
   else
   {
      if (offset > fifo->size)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): Invalid offset passed - offset = %d (fifo size = %d)!\n",
                 __FUNCTION__, offset, fifo->size));
         return -1;
      }
      new_read_index =
         (IFX_int32_t)(*fifo->pread_off) - (IFX_int32_t)offset;
      read_address =
         (IFX_uint32_t)fifo->pend + (IFX_uint32_t)(*fifo->pread_off) -
         (IFX_uint32_t)offset;
      if (new_read_index < 0)
      {
         read_address += fifo->size;
      }

      *pData = *(IFX_uint32_t *) read_address;

      ret = IFX_SUCCESS;
   }

   return (ret);
}


/******************************************************************************
 * Global Routines
 ******************************************************************************/

/**
 * Open MPS device
 * Open routine for the MPS device driver.
 *
 * \param   mps_device  MPS communication device structure
 * \param   pMBDev      Pointer to mailbox device structure
 * \param   bcommand    voice/command selector, 1 -> command mailbox,
 *                      2 -> voice, 3 -> event mailbox
 * \return  0           IFX_SUCCESS, successfully opened
 * \return  -1          IFX_ERROR, Driver already installed
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_common_open (mps_comm_dev * mps_device,
                                 mps_mbx_dev * pMBDev, IFX_int32_t bcommand,
                                 IFX_boolean_t from_kernel)
{
   IFXOS_INTSTAT flags;

   IFXOS_LOCKINT (flags);
   /* device is already installed or unknown device ID used */
   if (MPS_MBX_DEV_INSTALL_INST_ISSET(pMBDev) || !E_MPS_DEVICE_TYPE_IN_RANGE(pMBDev->devID))
   {
      IFXOS_UNLOCKINT (flags);
      return (IFX_ERROR);
   }
   MPS_MBX_DEV_INSTALL_INST_SET(pMBDev);
   IFXOS_UNLOCKINT (flags);

   if (bcommand == 2)           /* voice */
   {
      if (from_kernel)
      {
         MPS_MBX_DEV_INSTALL_KERNEL_SET(pMBDev);
         pMBDev->upstrm_fifo = &mps_device->voice_upstrm_fifo;
         pMBDev->dwstrm_fifo = &mps_device->voice_dwstrm_fifo;
      }
      else
      {
         MPS_MBX_DEV_INSTALL_KERNEL_CLEAR(pMBDev);
         pMBDev->upstrm_fifo = &mps_device->sw_upstrm_fifo[pMBDev->devID - 1];
         pMBDev->dwstrm_fifo = &mps_device->voice_dwstrm_fifo;
      }
   }

   else if (bcommand == 3)      /* event mailbox */
   {
      if (from_kernel)
      {
         pMBDev->upstrm_fifo = &mps_device->event_upstrm_fifo;
         pMBDev->dwstrm_fifo = IFX_NULL;
      }
      else
      {
         pMBDev->upstrm_fifo = &mps_device->sw_event_upstrm_fifo;
         pMBDev->dwstrm_fifo = IFX_NULL;
      }
   }
   return (IFX_SUCCESS);
}


/**
 * Close routine for MPS device driver
 * This function closes the channel assigned to the passed mailbox
 * device structure.
 *
 * \param   pMBDev   Pointer to mailbox device structure
 * \return  0        IFX_SUCCESS, will never fail
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_common_close (mps_mbx_dev * pMBDev,
                                  IFX_boolean_t from_kernel)
{
   IFX_MPS_UNUSED(from_kernel);

   /* clean data structures */
   if (!MPS_MBX_DEV_INSTALL_INST_ISSET(pMBDev))
   {
      return (IFX_ERROR);
   }
   MPS_MBX_DEV_INSTALL_RESET(pMBDev);

   /* Clear the downstream queues for voice fds only */
   if ((pMBDev->devID != command) && (pMBDev->devID != event_mbx))
   {
#ifdef CONFIG_PROC_FS
      pMBDev->upstrm_fifo->min_space = pMBDev->upstrm_fifo->size;
      pMBDev->dwstrm_fifo->min_space = pMBDev->dwstrm_fifo->size;
#endif /* */
      /* clean-up messages left in software fifo... */
      while (ifx_mps_fifo_not_empty (pMBDev->upstrm_fifo))
      {
         IFX_uint32_t bytes_read;
         MbxMsg_s msg;
         ifx_mps_mbx_read_message (pMBDev->upstrm_fifo, &msg, &bytes_read);
         ifx_mps_bufman_free ((IFX_void_t *)
                              KSEG0ADDR ((IFX_uint8_t *) msg.data[0]));
         pMBDev->upstrm_fifo->discards++;
      }
      /* reset software fifo... */
      *pMBDev->upstrm_fifo->pwrite_off = (pMBDev->upstrm_fifo->size - 4);
      *pMBDev->upstrm_fifo->pread_off = (pMBDev->upstrm_fifo->size - 4);
   }
#ifdef CONFIG_PROC_FS
   else
   {
      if (pMBDev->devID != event_mbx)
      {
         pMBDev->upstrm_fifo->min_space = pMBDev->upstrm_fifo->size;
         pMBDev->dwstrm_fifo->min_space = pMBDev->dwstrm_fifo->size;
      }
   }
#endif /* CONFIG_PROC_FS */

   return IFX_SUCCESS;
}

IFX_int_t ifx_mps_fifo_dyn_data_reset(mps_fifo *p_mps_fifo)
{
   if (p_mps_fifo != IFX_NULL)
   {
      p_mps_fifo->min_space = p_mps_fifo->size;
      p_mps_fifo->bytes = 0;
      p_mps_fifo->pkts = 0;
      p_mps_fifo->discards = 0;
   }

   return IFX_SUCCESS;
}


#if (MPS_DEBUG_MBOX_DESCR_SHOW == 1)
static IFX_void_t mps_init_dev_mbox_descrp_l1_show(
   mps_comm_dev * pDev,
   const IFX_char_t *p_desc)
{
   mps_mbx_reg *MBX_Memory;

   if (pDev->base_global == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
         ("[dbg] MBox Descr Layout1 (%s): missing init\n\r",
          (p_desc) ? p_desc : "unknown"));
      return;
   }
   MBX_Memory = pDev->base_global;

   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg] MBox Descr Layout1 (%s):\n\r",
       (p_desc) ? p_desc : "unknown"));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg]    MBX_UPSTR_CMD_BASE=0x%08X, MBX_UPSTR_CMD_SIZE=0x%04X\n\r",
       (IFX_uint_t)MBX_Memory->MBX_UPSTR_CMD_BASE, MBX_Memory->MBX_UPSTR_CMD_SIZE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg]    MBX_DNSTR_CMD_BASE=0x%08X, MBX_DNSTR_CMD_SIZE=0x%04X\n\r",
       (IFX_uint_t)MBX_Memory->MBX_DNSTR_CMD_BASE, MBX_Memory->MBX_DNSTR_CMD_SIZE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg]    MBX_UPSTR_DATA_BASE=0x%08X, MBX_UPSTR_DATA_SIZE=0x%04X\n\r",
       (IFX_uint_t)MBX_Memory->MBX_UPSTR_DATA_BASE, MBX_Memory->MBX_UPSTR_DATA_SIZE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg]    MBX_DNSTR_DATA_BASE=0x%08X, MBX_DNSTR_DATA_SIZE=0x%04X\n\r",
       (IFX_uint_t)MBX_Memory->MBX_DNSTR_DATA_BASE, MBX_Memory->MBX_DNSTR_DATA_SIZE));

   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg]    MBX_UPSTR_CMD_READ=0x%08X, MBX_UPSTR_CMD_WRITE=0x%04X\n\r",
       MBX_Memory->MBX_UPSTR_CMD_READ, MBX_Memory->MBX_UPSTR_CMD_WRITE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg]    MBX_DNSTR_CMD_READ=0x%08X, MBX_DNSTR_CMD_WRITE=0x%04X\n\r",
       MBX_Memory->MBX_DNSTR_CMD_READ, MBX_Memory->MBX_DNSTR_CMD_WRITE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg]    MBX_UPSTR_DATA_READ=0x%08X, MBX_UPSTR_DATA_WRITE=0x%04X\n\r",
       MBX_Memory->MBX_UPSTR_DATA_READ, MBX_Memory->MBX_UPSTR_DATA_WRITE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg]    MBX_DNSTR_DATA_READ=0x%08X, MBX_DNSTR_DATA_WRITE=0x%04X\n\r",
       MBX_Memory->MBX_DNSTR_DATA_READ, MBX_Memory->MBX_DNSTR_DATA_WRITE));

   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg]    MBX_UPSTR_EVENT_BASE=0x%08X, MBX_UPSTR_EVENT_SIZE=0x%04X\n\r",
       (IFX_uint_t)MBX_Memory->MBX_UPSTR_EVENT_BASE, MBX_Memory->MBX_UPSTR_EVENT_SIZE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[dbg]    MBX_UPSTR_EVENT_READ=0x%08X, MBX_UPSTR_EVENT_WRITE=0x%04X\n\r",
       MBX_Memory->MBX_UPSTR_EVENT_READ, MBX_Memory->MBX_UPSTR_EVENT_WRITE));

   return;
}

static IFX_void_t mps_init_dev_mbox_descrp_l2_show(
   mps_comm_dev * pDev,
   const IFX_char_t *p_desc)
{
   mps_mbx_reg_defs_l2 *p_mbx_defs;


   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]: ifx_mps_get_fw_version - start\n"));

   if (pDev->base_global_l2 == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
         ("[DBG] MBox Descr Layout2 (%s): missing init\n\r",
          (p_desc) ? p_desc : "unknown"));
      return;
   }
   p_mbx_defs = &pDev->base_global_l2->mbx_reg_defs_l2;


   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG] MBox Descr [0x%08X] Layout2 (%s):\n\r",
       (IFX_uint32_t)p_mbx_defs, (p_desc) ? p_desc : "unknown"));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MPS configuration options=0x%04X\n\r",
       VOICE_MBX_MPS_OPT_GET(p_mbx_defs)));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MBX_UPSTR_CMD_BASE=0x%08X, MBX_UPSTR_CMD_SIZE=0x%04X\n\r",
       (IFX_uint_t)p_mbx_defs->MBX_UPSTR_CMD_BASE,
       VOICE_MBX_US_CMD_SIZE_GET(p_mbx_defs)));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MBX_DNSTR_CMD_BASE=0x%08X, MBX_DNSTR_CMD_SIZE=0x%04X\n\r",
       (IFX_uint_t)p_mbx_defs->MBX_DNSTR_CMD_BASE, p_mbx_defs->MBX_DNSTR_CMD_SIZE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MBX_UPSTR_DATA_BASE=0x%08X, MBX_UPSTR_DATA_SIZE=0x%04X\n\r",
       (IFX_uint_t)p_mbx_defs->MBX_UPSTR_DATA_BASE, p_mbx_defs->MBX_UPSTR_DATA_SIZE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MBX_DNSTR_DATA_BASE=0x%08X, MBX_DNSTR_DATA_SIZE=0x%04X\n\r",
       (IFX_uint_t)p_mbx_defs->MBX_DNSTR_DATA_BASE, p_mbx_defs->MBX_DNSTR_DATA_SIZE));

   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MBX_UPSTR_CMD_READ=0x%08X, MBX_UPSTR_CMD_WRITE=0x%04X\n\r",
       p_mbx_defs->MBX_UPSTR_CMD_READ, p_mbx_defs->MBX_UPSTR_CMD_WRITE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MBX_DNSTR_CMD_READ=0x%08X, MBX_DNSTR_CMD_WRITE=0x%04X\n\r",
       p_mbx_defs->MBX_DNSTR_CMD_READ, p_mbx_defs->MBX_DNSTR_CMD_WRITE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MBX_UPSTR_DATA_READ=0x%08X, MBX_UPSTR_DATA_WRITE=0x%04X\n\r",
       p_mbx_defs->MBX_UPSTR_DATA_READ, p_mbx_defs->MBX_UPSTR_DATA_WRITE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MBX_DNSTR_DATA_READ=0x%08X, MBX_DNSTR_DATA_WRITE=0x%04X\n\r",
       p_mbx_defs->MBX_DNSTR_DATA_READ, p_mbx_defs->MBX_DNSTR_DATA_WRITE));

   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MBX_UPSTR_EVENT_BASE=0x%08X, MBX_UPSTR_EVENT_SIZE=0x%04X\n\r",
       (IFX_uint_t)p_mbx_defs->MBX_UPSTR_EVENT_BASE, p_mbx_defs->MBX_UPSTR_EVENT_SIZE));
   TRACE (MPS, DBG_LEVEL_HIGH,
      ("[DBG]    MBX_UPSTR_EVENT_READ=0x%08X, MBX_UPSTR_EVENT_WRITE=0x%04X\n\r",
       p_mbx_defs->MBX_UPSTR_EVENT_READ, p_mbx_defs->MBX_UPSTR_EVENT_WRITE));

   return;
}

#endif

#define DEEP_EVT_SW_FIFO_SIZE 512

static IFX_int32_t mps_init_dev_mbox_descrp_l1(mps_comm_dev * pDev)
{
   mps_mbx_reg *MBX_Memory;

   MPS_DEBUG_FCT_ENTER;

   if ((pDev->base_global != IFX_NULL) || (pDev->base_global_l2 != IFX_NULL))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - base ptr (%s) already set\r\n",
              __FUNCTION__, (pDev->base_global != IFX_NULL) ? "L1" : "L2"));
      return IFX_ERROR;
   }

   pDev->base_global = (mps_mbx_reg *)ifx_mps_ram_base;
   MBX_Memory = pDev->base_global;

   /* * Initialize common mailbox definition area which is used by both CPUs
      for MBX communication. These are: mailbox base address, mailbox size, *
      mailbox read index and mailbox write index. for command and voice
      mailbox, * upstream and downstream direction. */
   memset((IFX_void_t *)&MBX_Memory->MBX_UPSTR_CMD_BASE,
      0x00, sizeof(mps_mbx_reg) - (2 * sizeof(mps_boot_cfg_reg)));

   MBX_Memory->MBX_UPSTR_CMD_BASE = (IFX_uint32_t *)CPHYSADDR((IFX_uint32_t)MBX_UPSTRM_CMD_FIFO_BASE);
   MBX_Memory->MBX_UPSTR_CMD_SIZE = MBX_CMD_FIFO_SIZE;
   MBX_Memory->MBX_DNSTR_CMD_BASE = (IFX_uint32_t *)CPHYSADDR((IFX_uint32_t)MBX_DNSTRM_CMD_FIFO_BASE);
   MBX_Memory->MBX_DNSTR_CMD_SIZE = MBX_CMD_FIFO_SIZE;
   MBX_Memory->MBX_UPSTR_DATA_BASE = (IFX_uint32_t *)CPHYSADDR((IFX_uint32_t)MBX_UPSTRM_DATA_FIFO_BASE);
   MBX_Memory->MBX_UPSTR_DATA_SIZE = MBX_DATA_UPSTRM_FIFO_SIZE;
   MBX_Memory->MBX_DNSTR_DATA_BASE = (IFX_uint32_t *)CPHYSADDR((IFX_uint32_t)MBX_DNSTRM_DATA_FIFO_BASE);
   MBX_Memory->MBX_DNSTR_DATA_SIZE = MBX_DATA_DNSTRM_FIFO_SIZE;

   /* set read and write pointers below to the FIFO's uppermost address */
   MBX_Memory->MBX_UPSTR_CMD_READ  = (MBX_Memory->MBX_UPSTR_CMD_SIZE - 4);
   MBX_Memory->MBX_UPSTR_CMD_WRITE = (MBX_Memory->MBX_UPSTR_CMD_SIZE - 4);
   MBX_Memory->MBX_DNSTR_CMD_READ  = (MBX_Memory->MBX_DNSTR_CMD_SIZE - 4);
   MBX_Memory->MBX_DNSTR_CMD_WRITE = (MBX_Memory->MBX_DNSTR_CMD_SIZE - 4);
   MBX_Memory->MBX_UPSTR_DATA_READ  = (MBX_Memory->MBX_UPSTR_DATA_SIZE - 4);
   MBX_Memory->MBX_UPSTR_DATA_WRITE = (MBX_Memory->MBX_UPSTR_DATA_SIZE - 4);
   MBX_Memory->MBX_DNSTR_DATA_READ  = (MBX_Memory->MBX_DNSTR_DATA_SIZE - 4);
   MBX_Memory->MBX_DNSTR_DATA_WRITE = (MBX_Memory->MBX_DNSTR_DATA_SIZE - 4);

   MBX_Memory->MBX_UPSTR_EVENT_BASE = (IFX_uint32_t *)CPHYSADDR((IFX_uint32_t)MBX_UPSTRM_EVENT_FIFO_BASE);
   MBX_Memory->MBX_UPSTR_EVENT_SIZE = MBX_EVENT_FIFO_SIZE;
   MBX_Memory->MBX_UPSTR_EVENT_READ = (MBX_Memory->MBX_UPSTR_EVENT_SIZE - 4);
   MBX_Memory->MBX_UPSTR_EVENT_WRITE = MBX_Memory->MBX_UPSTR_EVENT_READ;

   TRACE (MPS, DBG_LEVEL_LOW,
          ("%s(): done\r\n", __FUNCTION__));

   return IFX_SUCCESS;
}


static IFX_int32_t mps_release_dev_mbox_descrp_l1(mps_comm_dev * pDev)
{
   mps_mbx_reg *MBX_Memory;

   MPS_DEBUG_FCT_ENTER;

   if (pDev->base_global == IFX_NULL)
      {return IFX_SUCCESS;}

   MBX_Memory = pDev->base_global;

   pDev->base_global = IFX_NULL;
   pDev->mbx_descr = 0;

   /* * Initialize common mailbox definition area which is used by both CPUs
      for MBX communication. These are: mailbox base address, mailbox size, *
      mailbox read index and mailbox write index. for command and voice
      mailbox, * upstream and downstream direction. */
   memset((IFX_void_t *)&MBX_Memory->MBX_UPSTR_CMD_BASE,
      0x00, sizeof(mps_mbx_reg) - (2 * sizeof(mps_boot_cfg_reg)));

   return IFX_SUCCESS;
}


static IFX_int32_t mps_init_dev_mbox_descrp_l2 (mps_comm_dev * pDev)
{
   mps_mbx_reg_defs_l2 *p_mbx_defs;

   MPS_DEBUG_FCT_ENTER;

   if ((pDev->base_global != IFX_NULL) || (pDev->base_global_l2 != IFX_NULL))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - base ptr (%s) already set\r\n",
              __FUNCTION__, (pDev->base_global != IFX_NULL) ? "L1" : "L2"));
      return IFX_ERROR;
   }

   pDev->base_global_l2 = (mps_mbx_reg_l2 *)ifx_mps_ram_base;
   p_mbx_defs = &pDev->base_global_l2->mbx_reg_defs_l2;

   memset((IFX_void_t *)p_mbx_defs, 0x00, sizeof(mps_mbx_reg_defs_l2));

   /* The MPS configuration options field is read by the FW.
      Bit 0 indicates to not initialise the SDD functionality. With this
      also the ports for SSI bus to the SLIC are left untouched. */
#ifdef VMMC_FEAT_SLIC
   VOICE_MBX_MPS_OPT_SET(p_mbx_defs, 0);
#else /* VMMC_FEAT_SLIC */
   VOICE_MBX_MPS_OPT_SET(p_mbx_defs, 1);
#endif /* VMMC_FEAT_SLIC */

   /*
      US/DS command FIFO
   */
   p_mbx_defs->MBX_UPSTR_CMD_BASE = (IFX_uint32_t *)CPHYSADDR((IFX_uint32_t)MBX_UPSTRM_CMD_FIFO_BASE_L2);
   VOICE_MBX_US_CMD_SIZE_SET(p_mbx_defs, MBX_UPSTRM_CMD_FIFO_SIZE_L2);
   p_mbx_defs->MBX_DNSTR_CMD_BASE = (IFX_uint32_t *)CPHYSADDR((IFX_uint32_t)MBX_DNSTRM_CMD_FIFO_BASE_L2);
   p_mbx_defs->MBX_DNSTR_CMD_SIZE = MBX_DNSTRM_CMD_FIFO_SIZE_L2;
   /* set read and write pointers below to the FIFO's uppermost address */
   p_mbx_defs->MBX_UPSTR_CMD_READ  = VOICE_MBX_US_CMD_SIZE_GET(p_mbx_defs) - 4;
   p_mbx_defs->MBX_UPSTR_CMD_WRITE = VOICE_MBX_US_CMD_SIZE_GET(p_mbx_defs) - 4;
   p_mbx_defs->MBX_DNSTR_CMD_READ  = p_mbx_defs->MBX_DNSTR_CMD_SIZE - 4;
   p_mbx_defs->MBX_DNSTR_CMD_WRITE = p_mbx_defs->MBX_DNSTR_CMD_SIZE - 4;

   /*
      US event FIFO
   */
   p_mbx_defs->MBX_UPSTR_EVENT_BASE = (IFX_uint32_t *)CPHYSADDR((IFX_uint32_t)MBX_UPSTRM_EVENT_FIFO_BASE_L2);
   p_mbx_defs->MBX_UPSTR_EVENT_SIZE = MBX_UPSTRM_EVENT_FIFO_SIZE_L2;
   p_mbx_defs->MBX_UPSTR_EVENT_READ = (p_mbx_defs->MBX_UPSTR_EVENT_SIZE - 4);
   p_mbx_defs->MBX_UPSTR_EVENT_WRITE = p_mbx_defs->MBX_UPSTR_EVENT_READ;

   /*
      voice upstream FIFO
   */
   p_mbx_defs->MBX_UPSTR_DATA_BASE  =
      (IFX_uint32_t *)virt_to_phys((const void *)pDev->p_voice_upstrm_mailbox);
   p_mbx_defs->MBX_UPSTR_DATA_SIZE  = MBX_UPSTRM_DATA_FIFO_SIZE_L2;
   p_mbx_defs->MBX_UPSTR_DATA_READ  = p_mbx_defs->MBX_UPSTR_DATA_SIZE - 4;
   p_mbx_defs->MBX_UPSTR_DATA_WRITE = p_mbx_defs->MBX_UPSTR_DATA_SIZE - 4;

   /*
      voice downstream FIFO
   */
   p_mbx_defs->MBX_DNSTR_DATA_BASE =
      (IFX_uint32_t *)virt_to_phys((const void *)pDev->p_voice_dwstrm_mailbox);
   p_mbx_defs->MBX_DNSTR_DATA_SIZE  = MBX_DNSTRM_DATA_FIFO_SIZE_L2;
   p_mbx_defs->MBX_DNSTR_DATA_READ  = p_mbx_defs->MBX_DNSTR_DATA_SIZE - 4;
   p_mbx_defs->MBX_DNSTR_DATA_WRITE = p_mbx_defs->MBX_DNSTR_DATA_SIZE - 4;

   TRACE (MPS, DBG_LEVEL_LOW,
          ("%s(): done\r\n", __FUNCTION__));

   return IFX_SUCCESS;
}

static IFX_int32_t mps_release_dev_mbox_descrp_l2 (mps_comm_dev * pDev)
{
   mps_mbx_reg_defs_l2 *p_mbx_defs;

   MPS_DEBUG_FCT_ENTER;

   if (pDev->base_global_l2 == IFX_NULL)
      {return IFX_SUCCESS;}

   p_mbx_defs = &pDev->base_global_l2->mbx_reg_defs_l2;

   pDev->base_global_l2 = IFX_NULL;
   pDev->mbx_descr = 0;

   memset((IFX_void_t *)p_mbx_defs, 0x00, sizeof(mps_mbx_reg_defs_l2));

   return IFX_SUCCESS;
}


/**
 * MPS Structure Initialization
 * This function initializes the data structures of the Multi Processor System
 * that are necessary for inter processor communication
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
static IFX_int32_t mps_init_dev_mbox_descrp (mps_comm_dev * pDev)
{
   IFX_int32_t retVal = IFX_SUCCESS;

   if (pDev == IFX_NULL)
      {return IFX_ERROR;}

   switch (pDev->mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_ALL)
   {
   case LTQ_VOICE_MPS_MBOX_TYPE_L1:
      retVal = mps_init_dev_mbox_descrp_l1(pDev);
      break;
   case LTQ_VOICE_MPS_MBOX_TYPE_L2:
      retVal = mps_init_dev_mbox_descrp_l2(pDev);
      break;
   default:
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - MBox descr = 0x%X (unknown)\r\n", __FUNCTION__, pDev->mbx_descr));
      return IFX_ERROR;
   }

   return retVal;
}


/**
 * MPS Structure Initialization
 * This function initializes the data structures of the Multi Processor System
 * that are necessary for inter processor communication
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
static IFX_int32_t mps_release_dev_mbox_descrp (mps_comm_dev * pDev)
{
   IFX_int32_t retVal = IFX_SUCCESS;

   if (pDev == IFX_NULL)
      {return IFX_ERROR;}

   switch (pDev->mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_ALL)
   {
   case LTQ_VOICE_MPS_MBOX_TYPE_L1:
      retVal = mps_release_dev_mbox_descrp_l1(pDev);
      break;
   case LTQ_VOICE_MPS_MBOX_TYPE_L2:
      retVal = mps_release_dev_mbox_descrp_l2(pDev);
      break;
   default:
      /* mailbox descriptor format still not set */
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - MBox descr = 0x%X (unknown)\r\n",
              __FUNCTION__, pDev->mbx_descr));
      return IFX_ERROR;
   }

   return retVal;
}

static IFX_int32_t mps_init_dev_mbox_setup_l1(mps_comm_dev * pDev)
{
   IFX_int32_t i;
   mps_mbx_reg *MBX_Memory;

   if (pDev->base_global == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - base ptr missing\r\n", __FUNCTION__));
      return IFX_ERROR;
   }
   MBX_Memory = pDev->base_global;

   /* set command mailbox sub structure pointers to global mailbox register
      addresses */
   pDev->cmd_upstrm_fifo.pend = (IFX_uint32_t *)KSEG1ADDR(MBX_Memory->MBX_UPSTR_CMD_BASE);
   pDev->cmd_upstrm_fifo.pwrite_off = (IFX_uint32_t *) &MBX_Memory->MBX_UPSTR_CMD_WRITE;
   pDev->cmd_upstrm_fifo.pread_off  = (IFX_uint32_t *) &MBX_Memory->MBX_UPSTR_CMD_READ;
   pDev->cmd_upstrm_fifo.size = MBX_Memory->MBX_UPSTR_CMD_SIZE;
#ifdef CONFIG_PROC_FS
   (void)ifx_mps_fifo_dyn_data_reset(&pDev->cmd_upstrm_fifo);
#endif /* */

   pDev->cmd_dwstrm_fifo.pend = (IFX_uint32_t *)KSEG1ADDR (MBX_Memory->MBX_DNSTR_CMD_BASE);
   pDev->cmd_dwstrm_fifo.pwrite_off = (IFX_uint32_t *) &MBX_Memory->MBX_DNSTR_CMD_WRITE;
   pDev->cmd_dwstrm_fifo.pread_off  = (IFX_uint32_t *) &MBX_Memory->MBX_DNSTR_CMD_READ;
   pDev->cmd_dwstrm_fifo.size = MBX_Memory->MBX_DNSTR_CMD_SIZE;
#ifdef CONFIG_PROC_FS
   (void)ifx_mps_fifo_dyn_data_reset(&pDev->cmd_dwstrm_fifo);
#endif /* */

   /* event mailbox */
   pDev->event_upstrm_fifo.pend = (IFX_uint32_t *)KSEG1ADDR(MBX_Memory->MBX_UPSTR_EVENT_BASE);
   pDev->event_upstrm_fifo.pwrite_off = (IFX_uint32_t *) &MBX_Memory->MBX_UPSTR_EVENT_WRITE;
   pDev->event_upstrm_fifo.pread_off  = (IFX_uint32_t *) &MBX_Memory->MBX_UPSTR_EVENT_READ;
   pDev->event_upstrm_fifo.size = MBX_Memory->MBX_UPSTR_EVENT_SIZE;

#ifdef TEST_EVT_DISCARD
   if ((pDev->sw_event_upstrm_fifo.pend =
         IFXOS_BlockAlloc(DEEP_EVT_SW_FIFO_SIZE + 8)) == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("__FUNCTION__(): error - mem alloc(%d), sw_event_upstrm_fifo\r\n",
              __FUNCTION__, DEEP_EVT_SW_FIFO_SIZE + 8));
      return IFX_ERROR;
   }
   memset((IFX_void_t *)pDev->sw_event_upstrm_fifo.pend, 0x0,
          (DEEP_EVT_SW_FIFO_SIZE + 8));

   pDev->sw_event_upstrm_fifo.pwrite_off =
      (pDev->sw_event_upstrm_fifo.pend + ((DEEP_EVT_SW_FIFO_SIZE) >> 2));
   *pDev->sw_event_upstrm_fifo.pwrite_off = (DEEP_EVT_SW_FIFO_SIZE - 4);
   pDev->sw_event_upstrm_fifo.pread_off =
      (pDev->sw_event_upstrm_fifo.pend + ((DEEP_EVT_SW_FIFO_SIZE + 4) >> 2));
   *pDev->sw_event_upstrm_fifo.pread_off = (DEEP_EVT_SW_FIFO_SIZE - 4);
   pDev->sw_event_upstrm_fifo.size = DEEP_EVT_SW_FIFO_SIZE;
#else
   if ((pDev->sw_event_upstrm_fifo.pend =
         IFXOS_BlockAlloc(MBX_Memory->MBX_UPSTR_EVENT_SIZE + 8)) == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - mem alloc(%d), sw_event_upstrm_fifo\r\n",
              __FUNCTION__, MBX_Memory->MBX_UPSTR_EVENT_SIZE + 8));
      return IFX_ERROR;
   }
   memset((IFX_void_t *)pDev->sw_event_upstrm_fifo.pend, 0x0,
          (MBX_Memory->MBX_UPSTR_EVENT_SIZE + 8));

   pDev->sw_event_upstrm_fifo.pwrite_off =
      (pDev->sw_event_upstrm_fifo.pend + ((MBX_Memory->MBX_UPSTR_EVENT_SIZE) >> 2));
   *pDev->sw_event_upstrm_fifo.pwrite_off =
      (MBX_Memory->MBX_UPSTR_EVENT_SIZE - 4);
   pDev->sw_event_upstrm_fifo.pread_off =
      (pDev->sw_event_upstrm_fifo.pend + ((MBX_Memory->MBX_UPSTR_EVENT_SIZE + 4) >> 2));
   *pDev->sw_event_upstrm_fifo.pread_off =
      (MBX_Memory->MBX_UPSTR_EVENT_SIZE - 4);
   pDev->sw_event_upstrm_fifo.size = MBX_Memory->MBX_UPSTR_EVENT_SIZE;
#endif /* TEST_EVT_DISCARD */

   /* voice upstream data mailbox area */
   pDev->voice_upstrm_fifo.pend = (IFX_uint32_t *)KSEG1ADDR (MBX_Memory->MBX_UPSTR_DATA_BASE);
   pDev->voice_upstrm_fifo.pwrite_off = (IFX_uint32_t *)&MBX_Memory->MBX_UPSTR_DATA_WRITE;
   pDev->voice_upstrm_fifo.pread_off  = (IFX_uint32_t *)&MBX_Memory->MBX_UPSTR_DATA_READ;
   pDev->voice_upstrm_fifo.size = MBX_Memory->MBX_UPSTR_DATA_SIZE;
#ifdef CONFIG_PROC_FS
   (void)ifx_mps_fifo_dyn_data_reset(&pDev->voice_upstrm_fifo);
#endif /* */

   /* voice downstream data mailbox area */
   pDev->voice_dwstrm_fifo.pend = (IFX_uint32_t *)KSEG1ADDR (MBX_Memory->MBX_DNSTR_DATA_BASE);
   pDev->voice_dwstrm_fifo.pwrite_off = (IFX_uint32_t *)&MBX_Memory->MBX_DNSTR_DATA_WRITE;
   pDev->voice_dwstrm_fifo.pread_off  = (IFX_uint32_t *)&MBX_Memory->MBX_DNSTR_DATA_READ;
   pDev->voice_dwstrm_fifo.size = MBX_Memory->MBX_DNSTR_DATA_SIZE;
#ifdef CONFIG_PROC_FS
   (void)ifx_mps_fifo_dyn_data_reset(&pDev->voice_dwstrm_fifo);
#endif /* */

   /* configure voice channel communication structure fields that are common to
      all voice channels */
   for (i = 0; i < NUM_VOICE_CHANNEL; i++)
   {
      if ((pDev->sw_upstrm_fifo[i].pend =
            IFXOS_BlockAlloc(MBX_Memory->MBX_UPSTR_DATA_SIZE + 8)) == IFX_NULL)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): error - mem alloc(%d), sw_upstrm_fifo[%d]\r\n",
                 __FUNCTION__, MBX_Memory->MBX_UPSTR_DATA_SIZE + 8, i));
         return IFX_ERROR;
      }
      memset ((IFX_void_t *) pDev->sw_upstrm_fifo[i].pend, 0x00,
              MBX_Memory->MBX_UPSTR_DATA_SIZE + 8);

      pDev->sw_upstrm_fifo[i].pwrite_off =
         (pDev->sw_upstrm_fifo[i].pend + ((MBX_Memory->MBX_UPSTR_DATA_SIZE) >> 2));
      *pDev->sw_upstrm_fifo[i].pwrite_off = (MBX_Memory->MBX_UPSTR_DATA_SIZE - 4);
      pDev->sw_upstrm_fifo[i].pread_off =
         (pDev->sw_upstrm_fifo[i].pend + ((MBX_Memory->MBX_UPSTR_DATA_SIZE + 4) >> 2));
      *pDev->sw_upstrm_fifo[i].pread_off = (MBX_Memory->MBX_UPSTR_DATA_SIZE - 4);
      pDev->sw_upstrm_fifo[i].size = MBX_Memory->MBX_UPSTR_DATA_SIZE;
#ifdef CONFIG_PROC_FS
      (void)ifx_mps_fifo_dyn_data_reset(&pDev->sw_upstrm_fifo[i]);
#endif /* */

      if (MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&pDev->voice_mb[i]))
      {
         /* reconfig done - activate currend config again */
         TRACE (MPS, DBG_LEVEL_NORMAL,
            ("%s(): INFO - reactivate Voice MBox[%d]\r\n", __FUNCTION__, i));
         MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->voice_mb[i]);
         MPS_MBX_DEV_INSTALL_INST_SET(&pDev->voice_mb[i]);
      }
      else
      {
         /* upstrm fifo pointer might be changed on open... */
         MPS_MBX_DEV_INSTALL_RESET(&pDev->voice_mb[i]);  /* current mbx installation status */
         pDev->voice_mb[i].upstrm_fifo = &pDev->sw_upstrm_fifo[i];
         pDev->voice_mb[i].dwstrm_fifo = &pDev->voice_dwstrm_fifo;
         pDev->voice_mb[i].pVCPU_DEV = pDev;       /* global pointer reference */
         pDev->voice_mb[i].down_callback = IFX_NULL;
         pDev->voice_mb[i].up_callback = IFX_NULL;
         memset (&pDev->voice_mb[i].event_mask, 0, sizeof (MbxEventRegs_s));
      }
   }

   if (MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&pDev->event_mbx))
   {
      /* reconfig done - activate currend config again */
      TRACE (MPS, DBG_LEVEL_NORMAL,
         ("%s(): INFO - reactivate Event MBox\r\n", __FUNCTION__));
      MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->event_mbx);
      MPS_MBX_DEV_INSTALL_INST_SET(&pDev->event_mbx);
   }

   if (MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&pDev->command_mb))
   {
      /* reconfig done - activate currend config again */
      TRACE (MPS, DBG_LEVEL_NORMAL, ("%s(): INFO - reactivate Command MBox\r\n", __FUNCTION__));
      MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->command_mb);
      MPS_MBX_DEV_INSTALL_INST_SET(&pDev->command_mb);
   }
   else
   {
      pDev->command_mb.dwstrm_fifo = &pDev->cmd_dwstrm_fifo;
      pDev->command_mb.upstrm_fifo = &pDev->cmd_upstrm_fifo;
      pDev->command_mb.pVCPU_DEV = pDev;         /* global pointer reference */
      MPS_MBX_DEV_INSTALL_RESET(&pDev->command_mb);
   }

   TRACE (MPS, DBG_LEVEL_LOW,
      ("%s(): done\r\n", __FUNCTION__));

   return IFX_SUCCESS;
}


static IFX_int32_t mps_release_dev_mbox_setup_l1(
   mps_comm_dev * pDev,
   IFX_int_t b_force)
{
   IFX_int32_t i;
   volatile IFX_uint32_t *p_tmp;

   if (pDev->base_global == IFX_NULL)
      {return IFX_SUCCESS;}

   if (b_force == 1)
   {
      MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->command_mb);
      MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->event_mbx);
   }
   else
   {
      /* disable install flag while reconfig */
      if (MPS_MBX_DEV_INSTALL_INST_ISSET(&pDev->command_mb))
      {
         MPS_MBX_DEV_INSTALL_INST_CLEAR(&pDev->command_mb);
         MPS_MBX_DEV_INSTALL_RECONFIG_SET(&pDev->command_mb);
      }
      if (MPS_MBX_DEV_INSTALL_INST_ISSET(&pDev->event_mbx))
      {
         MPS_MBX_DEV_INSTALL_INST_CLEAR(&pDev->event_mbx);
         MPS_MBX_DEV_INSTALL_RECONFIG_SET(&pDev->event_mbx);
      }
   }

   if (!(MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&pDev->command_mb)))
   {
      MPS_MBX_DEV_INSTALL_RESET(&pDev->command_mb);
      pDev->command_mb.pVCPU_DEV = IFX_NULL;
      pDev->command_mb.upstrm_fifo = IFX_NULL;
      pDev->command_mb.dwstrm_fifo = IFX_NULL;
   }

   for (i = 0; i < NUM_VOICE_CHANNEL; i++)
   {
      if (b_force == 1)
         {MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->voice_mb[i]);}
      else if (MPS_MBX_DEV_INSTALL_INST_ISSET(&pDev->voice_mb[i]))
         {MPS_MBX_DEV_INSTALL_RECONFIG_SET(&pDev->voice_mb[i]);}   /* keep current user config */

      if (MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&pDev->voice_mb[i]))
      {
         /* disable install flag while reconfig */
         MPS_MBX_DEV_INSTALL_INST_CLEAR(&pDev->voice_mb[i]);
      }
      else
      {
         MPS_MBX_DEV_INSTALL_RESET(&pDev->voice_mb[i]);
         memset (&pDev->voice_mb[i].event_mask, 0, sizeof (MbxEventRegs_s));
         pDev->voice_mb[i].pVCPU_DEV = IFX_NULL;
         pDev->voice_mb[i].down_callback = IFX_NULL;
         pDev->voice_mb[i].up_callback = IFX_NULL;

         pDev->voice_mb[i].upstrm_fifo = IFX_NULL;
         pDev->voice_mb[i].dwstrm_fifo = IFX_NULL;
      }

      p_tmp = pDev->sw_upstrm_fifo[i].pend;
      memset(&pDev->sw_upstrm_fifo[i], 0x0, sizeof(mps_fifo));
      if (p_tmp != IFX_NULL)
         {IFXOS_BlockFree((IFX_void_t *)p_tmp);}
   }
   memset(&pDev->voice_upstrm_fifo, 0x0, sizeof(mps_fifo));
   memset(&pDev->voice_dwstrm_fifo, 0x0, sizeof(mps_fifo));
   p_tmp = pDev->sw_event_upstrm_fifo.pend;
   memset(&pDev->event_upstrm_fifo, 0x0, sizeof(mps_fifo));
   if (p_tmp != IFX_NULL)
      {IFXOS_BlockFree((IFX_void_t *)p_tmp);}

   memset(&pDev->event_upstrm_fifo, 0x0, sizeof(mps_fifo));
   memset(&pDev->cmd_upstrm_fifo, 0x0, sizeof(mps_fifo));
   memset(&pDev->cmd_dwstrm_fifo, 0x0, sizeof(mps_fifo));

   TRACE (MPS, DBG_LEVEL_LOW,
      ("%s(): done\r\n", __FUNCTION__));

   return IFX_SUCCESS;
}


static IFX_int32_t mps_init_dev_mbox_setup_l2(mps_comm_dev * pDev)
{
   IFX_int32_t i;
   mps_mbx_reg_defs_l2 *p_mbx_defs;

   if (pDev->base_global_l2 == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - base ptr missing\r\n", __FUNCTION__));
      return IFX_ERROR;
   }
   p_mbx_defs = &pDev->base_global_l2->mbx_reg_defs_l2;

   /* set command mailbox sub structure pointers to global mailbox register
      addresses */
   pDev->cmd_upstrm_fifo.pend = (IFX_uint32_t *)KSEG1ADDR (p_mbx_defs->MBX_UPSTR_CMD_BASE);
   pDev->cmd_upstrm_fifo.pwrite_off = (IFX_uint32_t *) &p_mbx_defs->MBX_UPSTR_CMD_WRITE;
   pDev->cmd_upstrm_fifo.pread_off  = (IFX_uint32_t *) &p_mbx_defs->MBX_UPSTR_CMD_READ;
   pDev->cmd_upstrm_fifo.size = VOICE_MBX_US_CMD_SIZE_GET(p_mbx_defs);
#ifdef CONFIG_PROC_FS
   (void)ifx_mps_fifo_dyn_data_reset(&pDev->cmd_upstrm_fifo);
#endif /* */

   pDev->cmd_dwstrm_fifo.pend = (IFX_uint32_t *)KSEG1ADDR (p_mbx_defs->MBX_DNSTR_CMD_BASE);
   pDev->cmd_dwstrm_fifo.pwrite_off = (IFX_uint32_t *) &p_mbx_defs->MBX_DNSTR_CMD_WRITE;
   pDev->cmd_dwstrm_fifo.pread_off  = (IFX_uint32_t *) &p_mbx_defs->MBX_DNSTR_CMD_READ;
   pDev->cmd_dwstrm_fifo.size = p_mbx_defs->MBX_DNSTR_CMD_SIZE;
#ifdef CONFIG_PROC_FS
   (void)ifx_mps_fifo_dyn_data_reset(&pDev->cmd_dwstrm_fifo);
#endif /* */

   /* event mailbox */
   pDev->event_upstrm_fifo.pend = (IFX_uint32_t *)KSEG1ADDR (p_mbx_defs->MBX_UPSTR_EVENT_BASE);
   pDev->event_upstrm_fifo.pwrite_off = (IFX_uint32_t *) &p_mbx_defs->MBX_UPSTR_EVENT_WRITE;
   pDev->event_upstrm_fifo.pread_off  = (IFX_uint32_t *) &p_mbx_defs->MBX_UPSTR_EVENT_READ;
   pDev->event_upstrm_fifo.size = p_mbx_defs->MBX_UPSTR_EVENT_SIZE;

#ifdef TEST_EVT_DISCARD
   if ((pDev->sw_event_upstrm_fifo.pend =
         IFXOS_BlockAlloc(DEEP_EVT_SW_FIFO_SIZE + 8)) == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - mem alloc(%d), sw_event_upstrm_fifo\r\n",
              __FUNCTION__, DEEP_EVT_SW_FIFO_SIZE + 8));
      return IFX_ERROR;
   }
   memset((IFX_void_t *)pDev->sw_event_upstrm_fifo.pend, 0x0,
          (DEEP_EVT_SW_FIFO_SIZE + 8));

   pDev->sw_event_upstrm_fifo.pwrite_off =
      (pDev->sw_event_upstrm_fifo.pend + ((DEEP_EVT_SW_FIFO_SIZE) >> 2));
   *pDev->sw_event_upstrm_fifo.pwrite_off = (DEEP_EVT_SW_FIFO_SIZE - 4);
   pDev->sw_event_upstrm_fifo.pread_off =
      (pDev->sw_event_upstrm_fifo.pend + ((DEEP_EVT_SW_FIFO_SIZE + 4) >> 2));
   *pDev->sw_event_upstrm_fifo.pread_off = (DEEP_EVT_SW_FIFO_SIZE - 4);
   pDev->sw_event_upstrm_fifo.size = DEEP_EVT_SW_FIFO_SIZE;
#else
   if ((pDev->sw_event_upstrm_fifo.pend =
         IFXOS_BlockAlloc(p_mbx_defs->MBX_UPSTR_EVENT_SIZE + 8)) == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - mem alloc(%d), sw_event_upstrm_fifo\r\n",
              __FUNCTION__, p_mbx_defs->MBX_UPSTR_EVENT_SIZE + 8));
      return IFX_ERROR;
   }
   memset((IFX_void_t *)pDev->sw_event_upstrm_fifo.pend, 0x0,
          (p_mbx_defs->MBX_UPSTR_EVENT_SIZE + 8));

   pDev->sw_event_upstrm_fifo.pwrite_off =
      (pDev->sw_event_upstrm_fifo.pend + ((p_mbx_defs->MBX_UPSTR_EVENT_SIZE) >> 2));
   *pDev->sw_event_upstrm_fifo.pwrite_off = (p_mbx_defs->MBX_UPSTR_EVENT_SIZE - 4);
   pDev->sw_event_upstrm_fifo.pread_off =
      (pDev->sw_event_upstrm_fifo.pend + ((p_mbx_defs->MBX_UPSTR_EVENT_SIZE + 4) >> 2));
   *pDev->sw_event_upstrm_fifo.pread_off = (p_mbx_defs->MBX_UPSTR_EVENT_SIZE - 4);
   pDev->sw_event_upstrm_fifo.size = p_mbx_defs->MBX_UPSTR_EVENT_SIZE;
#endif /* TEST_EVT_DISCARD */

   /* voice upstream data mailbox area */
   pDev->voice_upstrm_fifo.pend = pDev->p_voice_upstrm_mailbox;
   pDev->voice_upstrm_fifo.pwrite_off =
      (IFX_uint32_t *)&p_mbx_defs->MBX_UPSTR_DATA_WRITE;
   pDev->voice_upstrm_fifo.pread_off  =
      (IFX_uint32_t *)&p_mbx_defs->MBX_UPSTR_DATA_READ;
   pDev->voice_upstrm_fifo.size = p_mbx_defs->MBX_UPSTR_DATA_SIZE;
#ifdef CONFIG_PROC_FS
   (void)ifx_mps_fifo_dyn_data_reset(&pDev->voice_upstrm_fifo);
#endif /* */

   /* voice downstream data mailbox area */
   pDev->voice_dwstrm_fifo.pend = pDev->p_voice_dwstrm_mailbox;
   pDev->voice_dwstrm_fifo.pwrite_off =
      (IFX_uint32_t *)&p_mbx_defs->MBX_DNSTR_DATA_WRITE;
   pDev->voice_dwstrm_fifo.pread_off =
      (IFX_uint32_t *)&p_mbx_defs->MBX_DNSTR_DATA_READ;
   pDev->voice_dwstrm_fifo.size = p_mbx_defs->MBX_DNSTR_DATA_SIZE;
#ifdef CONFIG_PROC_FS
   (void)ifx_mps_fifo_dyn_data_reset(&pDev->voice_dwstrm_fifo);
#endif /* */

   /* configure voice channel communication structure fields that are common to
      all voice channels */
   for (i = 0; i < NUM_VOICE_CHANNEL; i++)
   {
      if ((pDev->sw_upstrm_fifo[i].pend =
            IFXOS_BlockAlloc(p_mbx_defs->MBX_UPSTR_DATA_SIZE + 8)) == IFX_NULL)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): error - mem alloc(%d), sw_upstrm_fifo[%d]\r\n",
                 __FUNCTION__, p_mbx_defs->MBX_UPSTR_DATA_SIZE + 8, i));
         return IFX_ERROR;
      }
      memset ((IFX_void_t *) pDev->sw_upstrm_fifo[i].pend, 0x00,
              p_mbx_defs->MBX_UPSTR_DATA_SIZE + 8);

      pDev->sw_upstrm_fifo[i].pwrite_off =
         (pDev->sw_upstrm_fifo[i].pend + ((p_mbx_defs->MBX_UPSTR_DATA_SIZE) >> 2));
      *pDev->sw_upstrm_fifo[i].pwrite_off = (p_mbx_defs->MBX_UPSTR_DATA_SIZE - 4);
      pDev->sw_upstrm_fifo[i].pread_off =
         (pDev->sw_upstrm_fifo[i].pend + ((p_mbx_defs->MBX_UPSTR_DATA_SIZE + 4) >> 2));
      *pDev->sw_upstrm_fifo[i].pread_off = (p_mbx_defs->MBX_UPSTR_DATA_SIZE - 4);
      pDev->sw_upstrm_fifo[i].size = p_mbx_defs->MBX_UPSTR_DATA_SIZE;
#ifdef CONFIG_PROC_FS
      (void)ifx_mps_fifo_dyn_data_reset(&pDev->sw_upstrm_fifo[i]);
#endif /* */

      if (MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&pDev->voice_mb[i]))
      {
         /* reconfig done - activate currend config again */
         TRACE (MPS, DBG_LEVEL_NORMAL,
            ("%s(): INFO - reactivate Voice MBox[%d]\r\n", __FUNCTION__, i));
         MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->voice_mb[i]);
         MPS_MBX_DEV_INSTALL_INST_SET(&pDev->voice_mb[i]);
      }
      else
      {
         /* upstrm fifo pointer might be changed on open... */
         MPS_MBX_DEV_INSTALL_RESET(&pDev->voice_mb[i]);  /* current mbx installation status */
         pDev->voice_mb[i].upstrm_fifo = &pDev->sw_upstrm_fifo[i];
         pDev->voice_mb[i].dwstrm_fifo = &pDev->voice_dwstrm_fifo;
         pDev->voice_mb[i].pVCPU_DEV = pDev;       /* global pointer reference */
         pDev->voice_mb[i].down_callback = IFX_NULL;
         pDev->voice_mb[i].up_callback = IFX_NULL;
         memset (&pDev->voice_mb[i].event_mask, 0, sizeof (MbxEventRegs_s));
      }
   }

   if (MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&pDev->event_mbx))
   {
      /* reconfig done - activate current config again */
      TRACE (MPS, DBG_LEVEL_NORMAL,
         ("%s(): INFO - reactivate Event MBox\r\n", __FUNCTION__));
      MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->event_mbx);
      MPS_MBX_DEV_INSTALL_INST_SET(&pDev->event_mbx);
   }

   if (MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&pDev->command_mb))
   {
      /* reconfig done - activate current config again */
      TRACE (MPS, DBG_LEVEL_NORMAL,
         ("%s(): INFO - reactivate Command MBox\r\n", __FUNCTION__));
      MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->command_mb);
      MPS_MBX_DEV_INSTALL_INST_SET(&pDev->command_mb);
   }
   else
   {
      pDev->command_mb.dwstrm_fifo = &pDev->cmd_dwstrm_fifo;
      pDev->command_mb.upstrm_fifo = &pDev->cmd_upstrm_fifo;
      pDev->command_mb.pVCPU_DEV = pDev;         /* global pointer reference */
      MPS_MBX_DEV_INSTALL_RESET(&pDev->command_mb);
   }

   TRACE (MPS, DBG_LEVEL_LOW,
      ("%s(): done\r\n", __FUNCTION__));

   return IFX_SUCCESS;
}


static IFX_int32_t mps_release_dev_mbox_setup_l2(
   mps_comm_dev * pDev,
   IFX_int_t b_force)
{
   IFX_int32_t i;
   volatile IFX_uint32_t *p_tmp;
   mps_mbx_reg_defs_l2 *p_mbx_defs;

   if (pDev->base_global_l2 == IFX_NULL)
      {return IFX_SUCCESS;}
   p_mbx_defs = &pDev->base_global_l2->mbx_reg_defs_l2;

   if (b_force == 1)
   {
      MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->command_mb);
      MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->event_mbx);
   }
   else
   {
      /* disable install flag while reconfig */
      if (MPS_MBX_DEV_INSTALL_INST_ISSET(&pDev->command_mb))
      {
         MPS_MBX_DEV_INSTALL_INST_CLEAR(&pDev->command_mb);
         MPS_MBX_DEV_INSTALL_RECONFIG_SET(&pDev->command_mb);
      }
      if (MPS_MBX_DEV_INSTALL_INST_ISSET(&pDev->event_mbx))
      {
         MPS_MBX_DEV_INSTALL_INST_CLEAR(&pDev->event_mbx);
         MPS_MBX_DEV_INSTALL_RECONFIG_SET(&pDev->event_mbx);
      }
   }

   if (!(MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&pDev->command_mb)))
   {
      MPS_MBX_DEV_INSTALL_RESET(&pDev->command_mb);
      pDev->command_mb.pVCPU_DEV = IFX_NULL;
      pDev->command_mb.upstrm_fifo = IFX_NULL;
      pDev->command_mb.dwstrm_fifo = IFX_NULL;
   }

   for (i = 0; i < NUM_VOICE_CHANNEL; i++)
   {
      if (b_force == 1)
      {
         MPS_MBX_DEV_INSTALL_RECONFIG_CLEAR(&pDev->voice_mb[i]);
      }
      else
      {
         if (MPS_MBX_DEV_INSTALL_INST_ISSET(&pDev->voice_mb[i]))
         {
            /* disable install flag while reconfig */
            MPS_MBX_DEV_INSTALL_INST_CLEAR(&pDev->voice_mb[i]);
            MPS_MBX_DEV_INSTALL_RECONFIG_SET(&pDev->voice_mb[i]);
         }
      }

      if (!(MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&pDev->voice_mb[i])))
      {
         MPS_MBX_DEV_INSTALL_RESET(&pDev->voice_mb[i]);
         memset (&pDev->voice_mb[i].event_mask, 0, sizeof (MbxEventRegs_s));
         pDev->voice_mb[i].pVCPU_DEV = IFX_NULL;
         pDev->voice_mb[i].down_callback = IFX_NULL;
         pDev->voice_mb[i].up_callback = IFX_NULL;

         pDev->voice_mb[i].upstrm_fifo = IFX_NULL;
         pDev->voice_mb[i].dwstrm_fifo = IFX_NULL;
      }

      p_tmp = pDev->sw_upstrm_fifo[i].pend;
      memset(&pDev->sw_upstrm_fifo[i], 0x0, sizeof(mps_fifo));
      if (p_tmp != IFX_NULL)
         {IFXOS_BlockFree((IFX_void_t *)p_tmp);}
   }

   p_tmp = pDev->sw_event_upstrm_fifo.pend;
   memset(&pDev->sw_event_upstrm_fifo, 0x0, sizeof(mps_fifo));
   if (p_tmp != IFX_NULL)
      {IFXOS_BlockFree((IFX_void_t *)p_tmp);}

   memset(&pDev->voice_upstrm_fifo, 0x0, sizeof(mps_fifo));
   memset(&pDev->voice_dwstrm_fifo, 0x0, sizeof(mps_fifo));
   memset(&pDev->event_upstrm_fifo, 0x0, sizeof(mps_fifo));
   memset(&pDev->cmd_upstrm_fifo, 0x0, sizeof(mps_fifo));
   memset(&pDev->cmd_dwstrm_fifo, 0x0, sizeof(mps_fifo));

   TRACE (MPS, DBG_LEVEL_LOW, ("%s(): done\r\n", __FUNCTION__));

   return IFX_SUCCESS;
}


/**
 * MPS Structure Initialization
 * This function initializes the data structures of the Multi Processor System
 * that are necessary for inter processor communication
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
static IFX_int32_t mps_init_dev_mbox_setup (mps_comm_dev * pDev)
{
   IFX_int32_t retVal = IFX_SUCCESS;

   if (pDev == IFX_NULL)
      {return IFX_ERROR;}

   switch (pDev->mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_ALL)
   {
   case LTQ_VOICE_MPS_MBOX_TYPE_L1:
      retVal = mps_init_dev_mbox_setup_l1(pDev);
#if (MPS_DEBUG_MBOX_DESCR_SHOW == 1)
      mps_init_dev_mbox_descrp_l1_show(pDev, "mps_init_dev_mbox_setup");
#endif
      break;
   case LTQ_VOICE_MPS_MBOX_TYPE_L2:
      retVal = mps_init_dev_mbox_setup_l2(pDev);
#if (MPS_DEBUG_MBOX_DESCR_SHOW == 1)
      mps_init_dev_mbox_descrp_l2_show(pDev, "mps_init_dev_mbox_setup");
#endif
      break;
   default:
      TRACE (MPS, DBG_LEVEL_HIGH,
         ("%s(): error - MBox descr = 0x%X (unknown)\r\n",
          __FUNCTION__, pDev->mbx_descr));
      return IFX_ERROR;
   }

   return retVal;
}


/**
 * MPS Structure Initialization
 * This function initializes the data structures of the Multi Processor System
 * that are necessary for inter processor communication
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
static IFX_int32_t mps_release_dev_mbox_setup (
   mps_comm_dev * pDev,
   IFX_int_t b_force)
{
   IFX_int32_t retVal = IFX_SUCCESS;

   if (pDev == IFX_NULL)
      {return IFX_ERROR;}

   switch (pDev->mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_ALL)
   {
   case LTQ_VOICE_MPS_MBOX_TYPE_L1:
      retVal = mps_release_dev_mbox_setup_l1(pDev, b_force);
      break;
   case LTQ_VOICE_MPS_MBOX_TYPE_L2:
      retVal = mps_release_dev_mbox_setup_l2(pDev, b_force);
      break;
   default:
      TRACE (MPS, DBG_LEVEL_HIGH,
         ("%s(): error - MBox descr = 0x%X (unknown)\r\n",
          __FUNCTION__, pDev->mbx_descr));
      return IFX_ERROR;
   }

   return retVal;
}


/**
 * MPS Structure Initialization
 * This function releases the OS objects of the Multi Processor System
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
static IFX_int32_t mps_release_dev_os_objects (mps_comm_dev * pDev)
{
   IFX_int32_t i;

   if (pDev == IFX_NULL)
      {return IFX_ERROR;}

   if (pDev->event_mbx.sem_dev != IFX_NULL)
   {
      IFXOS_LockRelease(pDev->event_mbx.sem_dev);
      IFXOS_BlockFree(pDev->event_mbx.sem_dev);
      pDev->event_mbx.sem_dev = IFX_NULL;
   }
   if (pDev->command_mb.sem_dev != IFX_NULL)
   {
      IFXOS_LockRelease(pDev->command_mb.sem_dev);
      IFXOS_BlockFree(pDev->command_mb.sem_dev);
      pDev->command_mb.sem_dev = IFX_NULL;
   }

   /* configure voice channel communication structure fields that are common to
      all voice channels */
   for (i = 0; i < NUM_VOICE_CHANNEL; i++)
   {
      if (pDev->voice_mb[i].sem_dev != IFX_NULL)
      {
         IFXOS_LockRelease(pDev->voice_mb[i].sem_dev);
         IFXOS_BlockFree(pDev->voice_mb[i].sem_dev);
         pDev->voice_mb[i].sem_dev = IFX_NULL;
      }
      if (pDev->voice_mb[i].sem_read_fifo != IFX_NULL)
      {
         IFXOS_LockRelease(pDev->voice_mb[i].sem_read_fifo);
         IFXOS_BlockFree(pDev->voice_mb[i].sem_read_fifo);
         pDev->voice_mb[i].sem_read_fifo = IFX_NULL;
      }
#ifdef MPS_FIFO_BLOCKING_WRITE
      pDev->voice_mb[i].bBlockWriteMB = FALSE;
      if (pDev->voice_mb[i].sem_write_fifo != IFX_NULL)
      {
         IFXOS_LockRelease(pDev->voice_mb[i].sem_write_fifo);
         IFXOS_BlockFree(pDev->voice_mb[i].sem_write_fifo);
         pDev->voice_mb[i].sem_write_fifo = IFX_NULL;
      }
#endif /* MPS_FIFO_BLOCKING_WRITE */
   }

   if (pDev->provide_buffer != IFX_NULL)
   {
      IFXOS_LockRelease(pDev->provide_buffer);
      IFXOS_BlockFree(pDev->provide_buffer);
      pDev->provide_buffer = IFX_NULL;
   }

   TRACE (MPS, DBG_LEVEL_LOW, ("%s(): done\r\n", __FUNCTION__));

   return IFX_SUCCESS;
}


/**
 * MPS Structure Initialization
 * This function initializes the OS objects of the Multi Processor System
 * that are necessary for inter processor communication
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
static IFX_int32_t mps_init_dev_os_objects (mps_comm_dev * pDev)
{
   IFX_int32_t i;

   if (pDev == IFX_NULL)
      {return IFX_ERROR;}

   /* initialize the semaphores for multitasking access */
   if ((pDev->event_mbx.sem_dev = IFXOS_BlockAlloc (sizeof (IFXOS_lock_t))) == IFX_NULL)
      {return IFX_ERROR;}
   memset (pDev->event_mbx.sem_dev, 0, sizeof (IFXOS_lock_t));
   IFXOS_LockInit (pDev->event_mbx.sem_dev);

   if (IFX_SUCCESS !=
       IFXOS_DrvSelectQueueInit (&pDev->event_mbx.mps_wakeuplist))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - init_waitqueue_head(evt)\r\n", __FUNCTION__));
   }

   if ((pDev->command_mb.sem_dev = IFXOS_BlockAlloc(sizeof (IFXOS_lock_t))) == IFX_NULL)
      {return IFX_ERROR;}
   memset (pDev->command_mb.sem_dev, 0, sizeof (IFXOS_lock_t));
   IFXOS_LockInit (pDev->command_mb.sem_dev);

   /* select mechanism implemented for each queue */
   if (IFX_SUCCESS !=
       IFXOS_DrvSelectQueueInit (&pDev->command_mb.mps_wakeuplist))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): error - init_waitqueue_head(cmd)\r\n", __FUNCTION__));
   }

   /* configure voice channel communication structure fields that are common to
      all voice channels */
   for (i = 0; i < NUM_VOICE_CHANNEL; i++)
   {
      /* initialize the semaphores for multitasking access */
      if ((pDev->voice_mb[i].sem_dev = IFXOS_BlockAlloc(sizeof (IFXOS_lock_t))) == IFX_NULL)
         {return IFX_ERROR;}
      memset (pDev->voice_mb[i].sem_dev, 0, sizeof (IFXOS_lock_t));
      IFXOS_LockInit (pDev->voice_mb[i].sem_dev);

      /* initialize the semaphores to read from the fifo */
      if ((pDev->voice_mb[i].sem_read_fifo = IFXOS_BlockAlloc (sizeof (IFXOS_lock_t))) == IFX_NULL)
         {return IFX_ERROR;}
      memset (pDev->voice_mb[i].sem_read_fifo, 0, sizeof (IFXOS_lock_t));
      IFXOS_LockInit (pDev->voice_mb[i].sem_read_fifo);
      /* sem take - blocked */
      IFXOS_LockGet (pDev->voice_mb[i].sem_read_fifo);

#ifdef MPS_FIFO_BLOCKING_WRITE
      if ((pDev->voice_mb[i].sem_write_fifo = IFXOS_BlockAlloc (sizeof (IFXOS_lock_t))) == IFX_NULL)
         {return IFX_ERROR;}
      memset (pDev->voice_mb[i].sem_write_fifo, 0, sizeof (IFXOS_lock_t));
      IFXOS_LockInit (pDev->voice_mb[i].sem_write_fifo);
      /* sem take - blocked */
      IFXOS_LockGet (pDev->voice_mb[i].sem_write_fifo);
      pDev->voice_mb[i].bBlockWriteMB = TRUE;
#endif /* MPS_FIFO_BLOCKING_WRITE */

      /* select mechanism implemented for each queue */
      if (IFX_SUCCESS !=
          IFXOS_DrvSelectQueueInit (&pDev->voice_mb[i].mps_wakeuplist))
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
            ("%s(): error - init_waitqueue_head(voice_mb[%d])\r\n", __FUNCTION__, i));
      }
   }

   if ((pDev->provide_buffer = IFXOS_BlockAlloc (sizeof (IFXOS_lock_t))) == IFX_NULL)
      {return IFX_ERROR;}

   memset (pDev->provide_buffer, 0, sizeof (IFXOS_lock_t));
   IFXOS_LockInit (pDev->provide_buffer);
   /* sem take - blocked */
   IFXOS_LockGet (pDev->provide_buffer);

   TRACE (MPS, DBG_LEVEL_LOW, ("%s(): done\r\n", __FUNCTION__));

   return IFX_SUCCESS;
}

/**
 * MPS Structure Initialization - release FW image data
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_release_fw_image_data(mps_comm_dev * pDev)
{
   FW_image_ftr_t *p_tmp = IFX_NULL;

   if (pFW_img_data == IFX_NULL)
      {return IFX_SUCCESS;}

   p_tmp = pFW_img_data;
   pFW_img_data = IFX_NULL;
   IFXOS_BlockFree (p_tmp);

   return IFX_SUCCESS;
}


/**
 * MPS Structure Initialization - allocate FW image data
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_init_fw_image_data(mps_comm_dev * pDev)
{
   if (pFW_img_data != IFX_NULL)
      {return IFX_ERROR;}

   /* allocate buffer for firmware image data */
   if ((pFW_img_data = IFXOS_BlockAlloc(sizeof(*pFW_img_data))) == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
         ("%s(): error - mem alloc\r\n", __FUNCTION__));
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/**
 * MPS Structure Initialization
 * This function initializes the mailbox structures of the Multi Processor System
 * that are necessary for inter processor communication
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_release_mbox_setup(
   mps_comm_dev * pDev)
{
   IFX_int_t b_force = 0;

   if (pDev == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s(): error - missing dev\r\n", __FUNCTION__));
      return IFX_ERROR;
   }

   TRACE (MPS, DBG_LEVEL_LOW,
          ("%s(): %s release (MBox descr = 0x%X)\r\n",
           __FUNCTION__, (pDev->mbx_descr != 0) ? "do" : "skip", pDev->mbx_descr));

   if (pDev->mbx_descr != 0)
   {
      if (mps_release_dev_mbox_setup(pDev, b_force) != IFX_SUCCESS)
         {return IFX_ERROR;}
      if (mps_release_dev_mbox_descrp(pDev) != IFX_SUCCESS)
         {return IFX_ERROR;}
   }

   TRACE (MPS, DBG_LEVEL_LOW, ("%s(): done\r\n", __FUNCTION__));

   return IFX_SUCCESS;
}

/**
 * MPS Structure Initialization
 * This function initializes the mailbox structures of the Multi Processor System
 * that are necessary for inter processor communication
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_init_mbox_setup(
   mps_comm_dev * pDev,
   IFX_uint_t mbx_descr)
{
   MPS_DEBUG_FCT_ENTER;

   if (pDev == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s(): error - missing dev\r\n", __FUNCTION__));
      return IFX_ERROR;
   }

   if (pDev->mbx_descr == 0)
      {pDev->mbx_descr = mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_ALL;}
   else
   {
      if (pDev->mbx_descr != (mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_ALL))
      {
         if (ifx_mps_release_mbox_setup(pDev) != IFX_SUCCESS)
            {return IFX_ERROR;}
         pDev->mbx_descr = mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_ALL;
      }
      else
      {
         return IFX_SUCCESS;
      }
   }

   if (pDev->mbx_descr == LTQ_VOICE_MPS_MBOX_TYPE_L2)
   {
      /* In L2 this is the memory shared with the FW and used as mailbox.
         Allocating it from the DMA pool ensures that it comes from a memory
         range that can be accessed also from the FW side. */

      if (pDev->p_voice_upstrm_mailbox == IFX_NULL)
      {
         pDev->p_voice_upstrm_mailbox =
            kmalloc(MBX_UPSTRM_DATA_FIFO_SIZE_L2, GFP_KERNEL | GFP_DMA);
      }
      if (pDev->p_voice_upstrm_mailbox == IFX_NULL)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): error allocating voice upstream mbx of size %d\r\n",
                 __FUNCTION__, MBX_UPSTRM_DATA_FIFO_SIZE_L2));
         return IFX_ERROR;
      }
      memset((IFX_void_t *)pDev->p_voice_upstrm_mailbox, 0x0,
             MBX_UPSTRM_DATA_FIFO_SIZE_L2);


      if (pDev->p_voice_dwstrm_mailbox == IFX_NULL)
      {
         pDev->p_voice_dwstrm_mailbox =
            kmalloc(MBX_DNSTRM_DATA_FIFO_SIZE_L2, GFP_KERNEL | GFP_DMA);
      }
      if (pDev->p_voice_dwstrm_mailbox == IFX_NULL)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): error allocating voice downstream mbx of size %d\r\n",
                 __FUNCTION__, MBX_DNSTRM_DATA_FIFO_SIZE_L2));
         return IFX_ERROR;
      }
      memset((IFX_void_t *)pDev->p_voice_dwstrm_mailbox, 0x0,
             MBX_DNSTRM_DATA_FIFO_SIZE_L2);
   }

   if (pDev->mbx_descr != 0)
   {
      if (mps_init_dev_mbox_descrp(pDev) != IFX_SUCCESS)
         {return IFX_ERROR;}
      if (mps_init_dev_mbox_setup(pDev) != IFX_SUCCESS)
         {return IFX_ERROR;}
   }
   else
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): MBox descr = 0 (unknown)\r\n", __FUNCTION__));
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/**
 * MPS Structure Release
 * This function releases the entire MPS data structure used for communication
 * between the CPUs.
 *
 * \param   pDev     Poiter to MPS communication structure
 * \ingroup Internal
 */
IFX_void_t ifx_mps_release_structures (mps_comm_dev * pDev)
{
   IFX_int_t b_force = 0;
   IFXOS_INTSTAT flags;

   if (pDev == IFX_NULL)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s(): error - missing dev\r\n", __FUNCTION__));
      return;
   }

   IFXOS_LOCKINT (flags);

   (void)ifx_mps_release_fw_image_data(pDev);
   (void)mps_release_dev_mbox_setup(pDev, b_force);
   (void)mps_release_dev_mbox_descrp(pDev);
   (void)mps_release_dev_os_objects(pDev);

   /* In L2 this is the memory shared with the FW and used as mailbox. */
   if (pDev->p_voice_upstrm_mailbox != IFX_NULL)
   {
      kfree(pDev->p_voice_upstrm_mailbox);
      pDev->p_voice_upstrm_mailbox = IFX_NULL;
   }
   if (pDev->p_voice_dwstrm_mailbox != IFX_NULL)
   {
      kfree(pDev->p_voice_dwstrm_mailbox);
      pDev->p_voice_dwstrm_mailbox = IFX_NULL;
   }

   IFXOS_UNLOCKINT (flags);

   return;
}


/**
 * MPS Structure Initialization
 * This function initializes the data structures of the Multi Processor System
 * that are necessary for inter processor communication
 *
 * \param   pDev     Pointer to MPS device structure to be initialized
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_init_structures (mps_comm_dev * pDev)
{
   mps_devices e_mps_dev;

   MPS_DEBUG_FCT_ENTER;

   if (pDev == IFX_NULL)
      {return IFX_ERROR;}

   /* Initialize MPS main structure */
   memset ((IFX_void_t *)pDev, 0, sizeof (mps_comm_dev));
   pDev->flags = 0x00000000;

   if (mps_init_dev_os_objects(pDev) != IFX_SUCCESS)
      {return IFX_ERROR;}

   /* for testing - fixed FW format */
   if (ifx_mps_init_mbox_setup(pDev, LTQ_VOICE_MPS_MBOX_TYPE_L1) != IFX_SUCCESS)
         {return IFX_ERROR;}

   /* set channel identifiers */
   pDev->command_mb.devID = command;
   for (e_mps_dev = E_MPS_DEVICE_TYPE_VOICE_FIRST; e_mps_dev <= E_MPS_DEVICE_TYPE_VOICE_LAST; e_mps_dev++)
   {
      pDev->voice_mb[MPS_DEVICE_VOICE_TYPE_2_CHANNEL(e_mps_dev)].devID = e_mps_dev;
   }
   pDev->event_mbx.devID = event_mbx;

   if (ifx_mps_init_fw_image_data(pDev) != IFX_SUCCESS)
      {return IFX_ERROR;}

   return 0;
}


/**
 * MPS Structure Reset
 * This function resets the global structures into inital state
 *
 * \param   pDev     Pointer to MPS device structure
 * \return  0        IFX_SUCCESS, if initialization was successful
 * \return  -1       IFX_ERROR, allocation or semaphore access problem
 * \ingroup Internal
 */
IFX_uint32_t ifx_mps_reset_structures (mps_comm_dev * pDev)
{
   IFX_int32_t i;

#ifdef CONFIG_PROC_FS
   (void)ifx_mps_fifo_dyn_data_reset(&pDev->voice_dwstrm_fifo);
#endif /* */
   for (i = 0; i < NUM_VOICE_CHANNEL; i++)
   {
      if (pDev->voice_mb[i].dwstrm_fifo != IFX_NULL)
         {ifx_mps_fifo_clear (pDev->voice_mb[i].dwstrm_fifo);}
      if (pDev->voice_mb[i].upstrm_fifo != IFX_NULL)
      {
         ifx_mps_fifo_clear (pDev->voice_mb[i].upstrm_fifo);
         (void)ifx_mps_fifo_dyn_data_reset(pDev->voice_mb[i].upstrm_fifo);
      }
   }

   if (pDev->command_mb.dwstrm_fifo != IFX_NULL)
   {
      ifx_mps_fifo_clear (pDev->command_mb.dwstrm_fifo);
      (void)ifx_mps_fifo_dyn_data_reset(pDev->command_mb.dwstrm_fifo);
   }
   if (pDev->command_mb.upstrm_fifo != IFX_NULL)
   {
      ifx_mps_fifo_clear (pDev->command_mb.upstrm_fifo);
      (void)ifx_mps_fifo_dyn_data_reset(pDev->command_mb.upstrm_fifo);
   }
   ifx_mps_fifo_clear (&pDev->event_upstrm_fifo);

#if CONFIG_MPS_HISTORY_SIZE > 0
   ifx_mps_history_buffer_freeze = 0;
   ifx_mps_history_buffer_words = 0;
   ifx_mps_history_buffer_overflowed = 0;
#endif /* */
   IFXOS_LockTimedGet (pDev->provide_buffer, 0, IFX_NULL);
   return IFX_SUCCESS;
}


/******************************************************************************
 * Mailbox Managment
 ******************************************************************************/

/**
 * Gets channel ID field from message header
 * This function reads the data word at the read pointer position
 * of the mailbox FIFO pointed to by mbx and extracts the channel ID field
 * from the data word read.
 *
 * \param   mbx      Pointer to mailbox structure to be accessed
 * \return  ID       Voice channel identifier.
 * \ingroup Internal
 */
mps_devices ifx_mps_mbx_get_message_channel (mps_fifo * mbx)
{
   MbxMsgHd_u msg_hd;
   mps_devices retval = unknown;
   IFX_int32_t ret;

   ret = ifx_mps_fifo_read (mbx, 0, &msg_hd.val);
   if (ret == IFX_ERROR)
      return retval;

   if (MPS_DEVICE_CHANNEL_NUM_IS_VALID(msg_hd.hd.chan))
      {retval = MPS_DEVICE_VOICE_CHANNEL_2_TYPE(msg_hd.hd.chan);}

   if (retval == unknown)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): unknown channel ID %d\n", __FUNCTION__,
             msg_hd.hd.chan));
   }

   return retval;
}


/**
 * Get message length
 * This function returns the length in bytes of the message located at read pointer
 * position. It reads the plength field of the message header (length in bytes)
 * adds the header length and returns the complete length in bytes.
 *
 * \param   mbx      Pointer to mailbox structure to be accessed
 * \return  length   Length of message in bytes.
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_mbx_get_message_length (mps_fifo * mbx)
{
   MbxMsgHd_u msg_hd;
   IFX_int32_t ret;

   ret = ifx_mps_fifo_read (mbx, 0, &msg_hd.val);

   /* return payload + header length in bytes */
   if (ret == IFX_ERROR)        /* error */
      return 0;
   else
      return ((IFX_int32_t) msg_hd.hd.plength + 4);
}


/**
 * Read message from upstream data mailbox
 * This function reads a complete data message from the upstream data mailbox.
 * It reads the header checks how many payload words are included in the message
 * and reads the payload afterwards. The mailbox's read pointer is updated afterwards
 * by the amount of words read.
 *
 * \param   fifo        Pointer to mailbox structure to be read from
 * \param   msg         Pointer to message structure read from buffer
 * \param   bytes       Pointer to number of bytes included in read message
 * \return  0           IFX_SUCCESS, successful read operation,
 * \return  -1          Invalid length field read.
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_mbx_read_message (mps_fifo * fifo, MbxMsg_s * msg,
                                      IFX_uint32_t * bytes)
{
   IFX_int32_t i, ret;
   IFXOS_INTSTAT flags;

   IFXOS_LOCKINT (flags);

   /* read message header from buffer */
   ret = ifx_mps_fifo_read (fifo, 0, &msg->header.val);
   if (ret == IFX_ERROR)
   {
      IFXOS_UNLOCKINT (flags);
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): fifo read hdr failed\n", __FUNCTION__));
      return ret;
   }

   if ((msg->header.hd.plength % 4) != 0)       /* check payload length */
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): Odd payload length %d\n", __FUNCTION__,
              msg->header.hd.plength));
      IFXOS_UNLOCKINT (flags);
      return IFX_ERROR;
   }

   if ((msg->header.hd.plength / 4) > MAX_UPSTRM_DATAWORDS)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): header length %d exceeds the Mailbox size %d\n",
              __FUNCTION__, msg->header.hd.plength, MAX_UPSTRM_DATAWORDS*4));
      IFXOS_UNLOCKINT (flags);
      return IFX_ERROR;
   }

   for (i = 0; i < msg->header.hd.plength; i += 4)      /* read message payload
                                                         */
   {
      ret = ifx_mps_fifo_read (fifo, (IFX_uint8_t) (i + 4), &msg->data[i / 4]);
      if (ret == IFX_ERROR)
      {
         IFXOS_UNLOCKINT (flags);
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): fifo read payload failed\n", __FUNCTION__));

         return ret;
      }
   }
   *bytes = msg->header.hd.plength + 4;
   ifx_mps_fifo_read_ptr_inc (fifo, (msg->header.hd.plength + 4));
   IFXOS_UNLOCKINT (flags);
   return IFX_SUCCESS;
}


/**
 * Read message from FIFO
 * This function reads a message from the upstream data mailbox and passes it
 * to the calling function. A call to the notify_upstream function will trigger
 * another wakeup in case there is already more data available.
 *
 * \param   pMBDev   Pointer to mailbox device structure
 * \param   pPkg     Pointer to data transfer structure (output parameter)
 * \param   timeout  Currently unused
 * \return  0        IFX_SUCCESS, successful read operation,
 * \return  -1       IFX_ERROR, in case of read error.
 * \return  -ENODATA No data was available
 * \return  -EBADMSG Accidential read of buffer message
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_mbx_read (mps_mbx_dev * pMBDev, mps_message * pPkg,
                              IFX_int32_t timeout)
{
   MbxMsg_s msg;
   IFX_uint32_t bytes = 0;
   mps_fifo *fifo;
   IFX_int32_t retval = IFX_ERROR;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   if (!p_fb_mng->fb_init)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s(): missing fast buf init\n", __FUNCTION__));
      return retval;
   }

   IFX_MPS_UNUSED(timeout);

   fifo = pMBDev->upstrm_fifo;
   memset (&msg, 0, sizeof (msg));      /* initialize msg pointer */
   if (!ifx_mps_fifo_not_empty (fifo))
   {
      /* Nothing available for this channel... */
      return -ENODATA;
   }

   /* read message from mailbox */
   if (ifx_mps_mbx_read_message (fifo, &msg, &bytes) == 0)
   {
      switch (pMBDev->devID)
      {
         case command:

            /* command messages are completely passed to the caller. The
               mps_message structure comprises a pointer to the * message start
               and the message size in bytes */
            pPkg->pData = p_fb_mng->fb_malloc (bytes, FASTBUF_CMD_OWNED);
            if (pPkg->pData == IFX_NULL)
               return -1;
            memcpy ((IFX_uint8_t *) pPkg->pData, (IFX_uint8_t *) & msg, bytes);
            pPkg->cmd_type = msg.header.hd.type;
            pPkg->nDataBytes = bytes;
            pPkg->RTP_PaylOffset = 0;
            retval = IFX_SUCCESS;
#ifdef CONFIG_PROC_FS
            pMBDev->upstrm_fifo->bytes += bytes;
#endif /* */

            /* do another wakeup in case there is more data available... */
            ifx_mps_mbx_cmd_upstream (0);
            break;

         case event_mbx:

            /* event messages are completely passed to the caller. The
               mps_message structure comprises a pointer to the * message start
               and the message size in bytes */
            pPkg->pData = p_fb_mng->fb_malloc (bytes, FASTBUF_EVENT_OWNED);
            if (pPkg->pData == IFX_NULL)
               return -1;
            memcpy ((IFX_uint8_t *) pPkg->pData, (IFX_uint8_t *) & msg, bytes);
            pPkg->cmd_type = msg.header.hd.type;
            pPkg->nDataBytes = bytes;
            pPkg->RTP_PaylOffset = 0;
            retval = IFX_SUCCESS;

            /* do another wakeup in case there is more data available... */
            ifx_mps_mbx_event_upstream (0);
            break;

         default:
            if (E_MPS_DEVICE_TYPE_IS_VOICE(pMBDev->devID))
            {
               /* data messages are passed as mps_message pointer that comprises a
                  pointer to the payload start address and the payload size in
                  bytes. The message header is removed and the payload pointer,
                  payload size, payload type and and RTP payload offset are passed
                  to CPU0. */
               pPkg->cmd_type = msg.header.hd.type;
               pPkg->pData = (IFX_uint8_t *)KSEG0ADDR((IFX_uint8_t *)msg.data[0]);  /* get payload pointer */
               pPkg->nDataBytes = msg.data[1];     /* get payload size */
               ifx_mps_cache_inv ((IFX_ulong_t)pPkg->pData, pPkg->nDataBytes);
               /* set RTP payload offset for RTP messages to be clarified how this
                  should look like exactly */
               pPkg->RTP_PaylOffset = 0;
               retval = IFX_SUCCESS;
#ifdef CONFIG_PROC_FS
               pMBDev->upstrm_fifo->bytes += bytes;
#endif
               if (IFX_SUCCESS ==
                   IFXOS_LockTimedGet (pMPSDev->provide_buffer, 0, IFX_NULL))
               {
                  /* use the fastbuf setup values */
                  if (ifx_mps_bufman_buf_provide(0, 0, 0) != IFX_SUCCESS)
                  {
                     TRACE (MPS, DBG_LEVEL_HIGH,
                            ("%s(): Warning - provide buffer failed...\n",
                             __FUNCTION__));
                     IFXOS_LockRelease (pMPSDev->provide_buffer);
                  }
               }
            }
            break;
      }
   }
   return retval;
}


/**
 * Build 32 bit word starting at byte_ptr.
 * This function builds a 32 bit word out of 4 consecutive bytes
 * starting at byte_ptr position.
 *
 * \param   byte_ptr  Pointer to first byte (most significat 8 bits) of word calculation
 * \return  value     Returns value of word starting at byte_ptr position
 * \ingroup Internal
 */
IFX_uint32_t ifx_mps_mbx_build_word (IFX_uint8_t * byte_ptr)
{
   IFX_uint32_t result = 0x00000000;
   IFX_int32_t i;

   for (i = 0; i < 4; i++)
   {
      result += (IFX_uint32_t) (*(byte_ptr + i)) << ((3 - i) * 8);
   }
   return (result);
}


/**
 * Write to Downstream Mailbox of MPS.
 * This function writes messages into the downstream mailbox to be read
 * by CPU1
 *
 * \param   pMBDev    Pointer to mailbox device structure
 * \param   msg_ptr   Pointer to message
 * \param   msg_bytes Number of bytes in message
 * \return  0         Returns IFX_SUCCESS in case of successful write operation
 * \return  -EAGAIN   in case of access fails with FIFO overflow while in irq
 * \return  -EIO      in case of access fails with FIFO overflow in task context
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_mbx_write_message (mps_mbx_dev * pMBDev,
                                       IFX_uint8_t * msg_ptr,
                                       IFX_uint32_t msg_bytes)
{
   mps_fifo *mbx;
   IFX_uint32_t i;
   IFXOS_INTSTAT flags;
   IFX_int32_t retval = -EAGAIN;
   IFX_int32_t retries = 0;
   IFX_uint32_t word = 0;
   IFX_boolean_t word_aligned = IFX_TRUE;
   static IFX_uint32_t trace_fag;

   IFXOS_LOCKINT (flags);
   if (pMBDev == IFX_NULL)
   {
      IFXOS_UNLOCKINT (flags);
      TRACE (MPS, DBG_LEVEL_HIGH,
            ("%s(): MBox Device - missing init !!!\n", __FUNCTION__));
      return IFX_ERROR;
   }
   mbx = pMBDev->dwstrm_fifo;
   if (mbx == IFX_NULL)
   {
      IFXOS_UNLOCKINT (flags);
      TRACE (MPS, DBG_LEVEL_HIGH,
            ("%s(): DS FIFO not set - missing init !!!\n", __FUNCTION__));
      return IFX_ERROR;
   }
   if ((IFX_uint32_t) msg_ptr & 0x00000003)
   {
      word_aligned = IFX_FALSE;
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): Passed message not word aligned !!!\n", __FUNCTION__));
   }

   /* request for downstream mailbox buffer memory, make MAX_FIFO_WRITE_RETRIES
      attempts in case not enough memory is not available */
   while (++retries <= MAX_FIFO_WRITE_RETRIES)
   {
      if (ifx_mps_fifo_mem_request (mbx, msg_bytes) == IFX_TRUE)
      {
         trace_fag = 0;
         break;
      }

      if (in_interrupt ())
      {
         retries = MAX_FIFO_WRITE_RETRIES + 1;
         break;
      }
      else
      {
#ifdef TEST_NO_CMD_MBX_WRITE_RETRIES
         if(pMBDev == &ifx_mps_dev.command_mb)
         {
            retries = MAX_FIFO_WRITE_RETRIES + 1;
            trace_fag = 1;
            break;
         }
         else
#endif /* TEST_NO_CMD_MBX_WRITE_RETRIES */
         {
            IFXOS_UNLOCKINT (flags);
            udelay (125);
            IFXOS_LOCKINT (flags);
         }
      }
   }

   if (retries <= MAX_FIFO_WRITE_RETRIES)
   {
      /* write message words to mailbox buffer starting at write pointer
         position and update the write pointer index by the amount of written
         data afterwards */
      for (i = 0; i < msg_bytes; i += 4)
      {
         if (word_aligned)
            ifx_mps_fifo_write (mbx, *(IFX_uint32_t *) (msg_ptr + i), i);
         else
         {
            word = ifx_mps_mbx_build_word (msg_ptr + i);
            ifx_mps_fifo_write (mbx, word, i);
         }
      }
#ifdef VMMC_WITH_MPS
#ifdef TAPI_PACKET_OWNID
      if(pMBDev != &(ifx_mps_dev.command_mb))
      {
         switch (((mps_message *)msg_ptr)->cmd_type)
         {
            case DAT_PAYL_PTR_MSG_HDLC_PACKET:
               IFX_TAPI_VoiceBufferChOwn (
                  (IFX_void_t *)KSEG0ADDR (((mps_message *)msg_ptr)->pData),
                  IFX_TAPI_BUFFER_OWNER_HDLC_FW);
               break;
            case DAT_PAYL_PTR_MSG_FAX_DATA_PACKET:
            case DAT_PAYL_PTR_MSG_VOICE_PACKET:
            case DAT_PAYL_PTR_MSG_EVENT_PACKET:
            case DAT_PAYL_PTR_MSG_FAX_T38_PACKET:
               IFX_TAPI_VoiceBufferChOwn (
                  (IFX_void_t *)KSEG0ADDR (((mps_message *)msg_ptr)->pData),
                  IFX_TAPI_BUFFER_OWNER_COD_FW);
               break;
         }
      }
#endif /* TAPI_PACKET_OWNID */
#endif /* VMMC_WITH_MPS */

      ifx_mps_fifo_write_ptr_inc (mbx, msg_bytes);

      retval = IFX_SUCCESS;

#ifdef CONFIG_PROC_FS
      mbx->pkts++;
      mbx->bytes += msg_bytes;
      if (mbx->min_space > ifx_mps_fifo_mem_available (mbx))
         mbx->min_space = ifx_mps_fifo_mem_available (mbx);
#endif /* CONFIG_PROC_FS */
   }
   else
   {
      /* insufficient space in the mailbox for writing the data */

      /** \todo update error statistics */

      if (!trace_fag)           /* protect from trace flood */
      {
         TRACE (MPS, DBG_LEVEL_LOW,
                ("%s(): write message timeout\n", __FUNCTION__));

         if (pMBDev->devID == command)
         {
            /* dump the command downstream mailbox */
            TRACE (MPS, DBG_LEVEL_HIGH,
                   (" (wr: 0x%08x, rd: 0x%08x)\n",
                    (IFX_uint32_t) ifx_mps_dev.cmd_dwstrm_fifo.pend +
                    (IFX_uint32_t) * ifx_mps_dev.cmd_dwstrm_fifo.pwrite_off,
                    (IFX_uint32_t) ifx_mps_dev.cmd_dwstrm_fifo.pend +
                    (IFX_uint32_t) * ifx_mps_dev.cmd_dwstrm_fifo.pread_off));
            for (i = 0; i < ifx_mps_dev.cmd_dwstrm_fifo.size; i += 16)
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                      ("   0x%08x: %08x %08x %08x %08x\n",
                       (IFX_uint32_t) (ifx_mps_dev.cmd_dwstrm_fifo.pend +
                                       (i / 4)),
                       *(ifx_mps_dev.cmd_dwstrm_fifo.pend + (i / 4)),
                       *(ifx_mps_dev.cmd_dwstrm_fifo.pend + 1 + (i / 4)),
                       *(ifx_mps_dev.cmd_dwstrm_fifo.pend + 2 + (i / 4)),
                       *(ifx_mps_dev.cmd_dwstrm_fifo.pend + 3 + (i / 4))));
            }
         }

         /* trace only once until write succeeds at least one time */
         trace_fag = 1;
      }

      /* If the command downstream mailbox stays full for several milliseconds,
         a fatal error has occurred and the voice CPU should be restarted */
      if (!in_interrupt ())
      {
         /* If not in interrupt we already waited some milliseconds for the
            voice firmware. Do a reset now. */

#if 0                           /* disabled until reset concept is implemented */
         IFX_int32_t status;

         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): Restarting the voice firmware now\n", __FUNCTION__));

         status = ifx_mps_restart ();
         if (status == IFX_SUCCESS)
         {
            status = ifx_mps_get_fw_version (1);
         }
         if (status == IFX_SUCCESS)
         {
            status = ifx_mps_bufman_init ();
         }
         if (status != IFX_SUCCESS)
         {
            TRACE (MPS, DBG_LEVEL_HIGH,
                   ("%s(): Restarting the voice firmware failed\n",
                    __FUNCTION__));
         }
         else
         {
            /* firmware was restarted so reset the trace output flag */
            trace_fag = 0;
         }
#endif

         /* -> return fatal error */
         retval = -EIO;
      }
   }
   IFXOS_UNLOCKINT (flags);
   return retval;
}


/**
 * Write to Downstream Data Mailbox of MPS.
 * This function writes the passed message into the downstream data mailbox.
 *
 * \param   pMBDev     Pointer to mailbox device structure
 * \param   readWrite  Pointer to message structure
 * \return  0          IFX_SUCCESS in case of successful write operation
 * \return  -1         IFX_ERROR in case of access fails with FIFO overflow
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_mbx_write_data (mps_mbx_dev * pMBDev,
                                    mps_message * readWrite)
{
   IFX_int32_t retval = IFX_ERROR;
   MbxMsg_s msg;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   if (!p_fb_mng->fb_init)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s(): missing fast buf init\n", __FUNCTION__));
      return retval;
   }

   if (E_MPS_DEVICE_TYPE_IS_VOICE(pMBDev->devID))
   {
#ifdef FAIL_ON_ERR_INTERRUPT
      /* check status not worth going on if voice CPU has indicated an error */
      if (pMPSDev->event.MPS_Ad0Reg.fld.data_err)
      {
         return retval;
      }
#endif /* */
      if (atomic_read (&ifx_mps_write_blocked) != 0)
      {
         /* no more messages can be sent until more buffers have been provided */
         return -1;
      }
      memset (&msg, 0, sizeof (msg));   /* initialize msg structure */

      /* build data message from passed payload data structure */
      msg.header.hd.plength = 0x8;
      msg.header.hd.chan = (IFX_uint32_t)MPS_DEVICE_VOICE_TYPE_2_CHANNEL(pMBDev->devID);
      msg.header.hd.type = readWrite->cmd_type;
      msg.data[0] = CPHYSADDR ((IFX_uint32_t) readWrite->pData);
      msg.data[1] = readWrite->nDataBytes;
#if defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)
      if (IFX_TRUE == bDoCacheOps)
      {
         ifx_mps_cache_wb_inv ((IFX_uint32_t) readWrite->pData, readWrite->nDataBytes);
      }
#endif /*defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)*/

      retval = ifx_mps_mbx_write_message (pMBDev, (IFX_uint8_t *) & msg, 12);

      if (retval == IFX_SUCCESS)
      {
         /* The FW with Layout 2 reuses the buffer after processing the
            content. Increase the number of provided buffers by one. */
         if ((pMPSDev->mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_L2) != 0)
         {
            ifx_mps_bufman_inc_level(1);
         }
      }
      else
      {
         TRACE (MPS, DBG_LEVEL_LOW,
                ("%s(): Writing data failed ! *\n", __FUNCTION__));
      }
   }
   else
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): Invalid device ID %d !\n", __FUNCTION__, pMBDev->devID));
   }

   if (IFX_SUCCESS == IFXOS_LockTimedGet (pMPSDev->provide_buffer, 0, IFX_NULL))
   {
      /* use the fastbuf setup values */
      if (ifx_mps_bufman_buf_provide(0, 0, 0) != IFX_SUCCESS)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): Warning - provide buffer failed...\n", __FUNCTION__));
         IFXOS_LockRelease (pMPSDev->provide_buffer);
      }
   }

   return retval;
}


/**
 * Write to downstream command mailbox.
 * This is the function to write commands into the downstream command mailbox
 * to be read by CPU1
 *
 * \param   pMBDev     Pointer to mailbox device structure
 * \param   readWrite  Pointer to transmission data container
 * \return  0          IFX_SUCCESS in case of successful write operation
 * \return  -1         IFX_ERROR in case of access fails with FIFO overflow
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_mbx_write_cmd (mps_mbx_dev * pMBDev,
                                   mps_message * readWrite)
{
   IFX_int32_t retval = IFX_ERROR;

   if (pMBDev->devID == command)
   {
#ifdef FAIL_ON_ERR_INTERRUPT
      /* check status not worth going on if voice CPU has indicated an error */
      if (pMPSDev->event.MPS_Ad0Reg.fld.cmd_err)
      {
         return retval;
      }
#endif /*DEBUG*/
         if ((readWrite->nDataBytes) % 4)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): invalid number of bytes %d\n", __FUNCTION__,
                 readWrite->nDataBytes));
      }
      if ((IFX_uint32_t) (readWrite->pData) & 0x00000003)
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): non word aligned data passed to mailbox\n",
                 __FUNCTION__));
      if (readWrite->nDataBytes > (MBX_CMD_FIFO_SIZE - 4))
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): command size too large!\n", __FUNCTION__));

#if CONFIG_MPS_HISTORY_SIZE > 0
      if (!ifx_mps_history_buffer_freeze)
      {
         IFX_int32_t i, pos;
         for (i = 0; i < (readWrite->nDataBytes / 4); i++)
         {
            pos = ifx_mps_history_buffer_words;
            ifx_mps_history_buffer[pos] =
               ((IFX_uint32_t *) readWrite->pData)[i];
            ifx_mps_history_buffer_words++;

#ifdef DEBUG
            ifx_mps_history_buffer_words_total++;
#endif /* */
            if (ifx_mps_history_buffer_words == MPS_HISTORY_BUFFER_SIZE)
            {
               ifx_mps_history_buffer_words = 0;
               ifx_mps_history_buffer_overflowed = 1;
            }
         }
      }
#endif /* */
      retval =
         ifx_mps_mbx_write_message (pMBDev, (IFX_uint8_t *) readWrite->pData,
                                    readWrite->nDataBytes);
      if (retval != IFX_SUCCESS)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s() - failed to write message!\n", __FUNCTION__));
      }
   }
   else
   {
      /* invalid device id read from mailbox FIFO structure */
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("%s(): Invalid device ID %d !\n", __FUNCTION__, pMBDev->devID));
   }
   return retval;
}


/**
 * Notify queue about upstream data reception
 * This function checks the channel identifier included in the header
 * of the message currently pointed to by the upstream data mailbox's
 * read pointer. It wakes up the related queue to read the received data message
 * out of the mailbox for further processing. The process is repeated
 * as long as upstream messages are available in the mailbox.
 * The function is attached to the driver's poll/select functionality.
 *
 * \param   dummy    Tasklet parameter, not used.
 * \ingroup Internal
 */
IFX_void_t ifx_mps_mbx_data_upstream (IFX_ulong_t dummy)
{
   mps_devices dev_voice_ch;
   mps_fifo *mbx;
   mps_mbx_dev *mbx_dev;
   MbxMsg_s msg;
   IFX_uint32_t bytes_read = 0;
   IFXOS_INTSTAT flags;
   IFX_int32_t ret;
   mps_buf_mng_t *p_fb_mng = &mps_buffer;

   IFX_MPS_UNUSED(dummy);

   if (!p_fb_mng->fb_init)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("%s(): missing fast buf init\n", __FUNCTION__));
      return;
   }

   /* set pointer to data upstream mailbox, no matter if 0,1,2 or 3 because
      they point to the same shared mailbox memory */
   mbx = &pMPSDev->voice_upstrm_fifo;
   while (ifx_mps_fifo_not_empty (mbx))
   {
      IFXOS_LOCKINT (flags);

      /* select mailbox device structure acc. to channel ID read from current msg */
      dev_voice_ch = ifx_mps_mbx_get_message_channel (mbx);
      if (!E_MPS_DEVICE_TYPE_IS_VOICE(dev_voice_ch))
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s(): Invalid voice channel ID %d read from mailbox\n",
                 __FUNCTION__, dev_voice_ch));
         IFXOS_UNLOCKINT (flags);
         return;
      }
      mbx_dev = (mps_mbx_dev *)&pMPSDev->voice_mb[MPS_DEVICE_VOICE_TYPE_2_CHANNEL(dev_voice_ch)];

#ifdef CONFIG_PROC_FS
      if (mbx->min_space > ifx_mps_fifo_mem_available (mbx))
         mbx->min_space = ifx_mps_fifo_mem_available (mbx);
#endif /* */

      /* read message header from buffer */
      ret = ifx_mps_fifo_read (mbx, 0, &msg.header.val);
      if (ret == IFX_ERROR)     /* fifo error (empty) */
         return;

#ifdef CONFIG_PROC_FS
      mbx->pkts++;
      mbx->bytes += msg.header.hd.plength + 4;
#endif /* */

      if (msg.header.hd.type == CMD_ADDRESS_PACKET)
      {
         IFX_int32_t i;
         ifx_mps_mbx_read_message (mbx, &msg, &bytes_read);
         for (i = 0; i < (msg.header.hd.plength / 4 - 1); i++)
         {
            p_fb_mng->fb_free ((IFX_void_t *) KSEG0ADDR (msg.data[i]));
            /* The FW with Layout 2 returns only buffer which cannot be put
               to the internal free buffer list. Since all buffers given are
               counted all returned buffers must be deducted from the count. */
            if ((pMPSDev->mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_L2) != 0)
            {
               ifx_mps_bufman_dec_level(1);
            }
         }
         IFXOS_UNLOCKINT (flags);
         continue;
      }
      else
      {
         ifx_mps_bufman_dec_level (1);

         /* discard packet in case no one is listening... */
         if (!MPS_MBX_DEV_INSTALL_INST_ISSET(mbx_dev))
         {
          data_upstream_discard:
            ifx_mps_mbx_read_message (mbx, &msg, &bytes_read);
            ifx_mps_bufman_free ((IFX_void_t *)
                                 KSEG0ADDR ((IFX_uint8_t *) msg.data[0]));
            mbx_dev->upstrm_fifo->discards++;

            IFXOS_UNLOCKINT (flags);
            continue;
         }

         if (mbx_dev->up_callback != IFX_NULL)
         {
            if ((ifx_mps_bufman_get_level () <= p_fb_mng->fb_threshold)
#ifdef LINUX
                 &&
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
                (atomic_read (&pMPSDev->provide_buffer->object.count) == 0)
#else
                ((volatile unsigned int)pMPSDev->provide_buffer->object.count == 0)
#endif
#endif /* LINUX */
            )
            {
               IFXOS_LockRelease (pMPSDev->provide_buffer);
            }

            /* use callback function to notify about data reception */
            mbx_dev->up_callback (dev_voice_ch);
            IFXOS_UNLOCKINT (flags);
            continue;
         }
         else
         {
            IFX_int32_t i, msg_bytes;

            msg_bytes = (msg.header.hd.plength + 4);
            if (ifx_mps_fifo_mem_request (mbx_dev->upstrm_fifo, msg_bytes) !=
                IFX_TRUE)
            {
               goto data_upstream_discard;
            }

            /* Copy message into sw fifo */
            for (i = 0; i < msg_bytes; i += 4)
            {
               IFX_uint32_t data;

               ret = ifx_mps_fifo_read (mbx, (IFX_uint8_t) i, &data);
               if (ret == IFX_ERROR)
                  return;
               ifx_mps_fifo_write (mbx_dev->upstrm_fifo, data, (IFX_uint8_t) i);
            }
            ifx_mps_fifo_read_ptr_inc (mbx, msg_bytes);
            ifx_mps_fifo_write_ptr_inc (mbx_dev->upstrm_fifo, msg_bytes);

#ifdef CONFIG_PROC_FS
            if (mbx_dev->upstrm_fifo->min_space >
                ifx_mps_fifo_mem_available (mbx_dev->upstrm_fifo))
               mbx_dev->upstrm_fifo->min_space =
                  ifx_mps_fifo_mem_available (mbx_dev->upstrm_fifo);
            mbx_dev->upstrm_fifo->pkts++;
#endif /* CONFIG_PROC_FS */

            if ((ifx_mps_bufman_get_level () <= p_fb_mng->fb_threshold)
#ifdef LINUX
                &&
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
                (atomic_read (&pMPSDev->provide_buffer->object.count) == 0)
#else
                (pMPSDev->provide_buffer->object.count == 0)
#endif
#endif /* LINUX */
            )
            {
               IFXOS_LockRelease (pMPSDev->provide_buffer);
            }

            /* use queue wake up to notify about data reception */
            IFXOS_DrvSelectQueueWakeUp (&(mbx_dev->mps_wakeuplist), 0);
            IFXOS_UNLOCKINT (flags);
         }
      }
   }
   return;
}


/**
 * Notify queue about upstream command reception
 * This function checks the channel identifier included in the header
 * of the message currently pointed to by the upstream command mailbox's
 * read pointer. It wakes up the related queue to read the received command
 * message out of the mailbox for further processing. The process is repeated
 * as long as upstream messages are avaiilable in the mailbox.
 * The function is attached to the driver's poll/select functionality.
 *
 * \param   dummy    Tasklet parameter, not used.
 * \ingroup Internal
 */
IFX_void_t ifx_mps_mbx_cmd_upstream (IFX_ulong_t dummy)
{
   mps_fifo *mbx;
   IFXOS_INTSTAT flags;

   IFX_MPS_UNUSED(dummy);

   /* set pointer to upstream command mailbox */
   mbx = &(pMPSDev->cmd_upstrm_fifo);
   IFXOS_LOCKINT (flags);
   if (ifx_mps_fifo_not_empty (mbx))
   {
#ifdef CONFIG_PROC_FS
      if (mbx->min_space > ifx_mps_fifo_mem_available (mbx))
         mbx->min_space = ifx_mps_fifo_mem_available (mbx);
#endif /* */
      if (!MPS_MBX_DEV_INSTALL_INST_ISSET(&pMPSDev->command_mb))
      {
         /* TODO: What to do with this?? */
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("%s() - wheee, unmaintained command message...\n",
                 __FUNCTION__));
      }
      if (pMPSDev->command_mb.up_callback != IFX_NULL)
      {
         pMPSDev->command_mb.up_callback (command);
      }
      else
      {
         /* wake up sleeping process for further processing of received command
          */
         IFXOS_DrvSelectQueueWakeUp (&(pMPSDev->command_mb.mps_wakeuplist), 0);
      }
   }
   IFXOS_UNLOCKINT (flags);
   return;
}


/**
 * Notify queue about upstream event reception
 * The function will deliver an incoming event to the registered handler.
 *
 * \param   dummy    Tasklet parameter, not used.
 * \ingroup Internal
 */
IFX_void_t ifx_mps_mbx_event_upstream (IFX_ulong_t dummy)
{
   mps_fifo *mbx;
   MbxMsg_s msg;
   mps_event_msg *p_evt_msg = (mps_event_msg *)&msg;
   IFX_int32_t length = 0;
   IFX_uint32_t read_length = 0;
   IFXOS_INTSTAT flags;

   IFX_MPS_UNUSED(dummy);

   /* set pointer to upstream event mailbox */
   mbx = &(pMPSDev->event_upstrm_fifo);
   IFXOS_LOCKINT (flags);
   while (ifx_mps_fifo_not_empty (mbx))
   {
      length = ifx_mps_mbx_get_message_length (mbx);
#ifdef TEST_EVT_DISCARD
      /* VoIP firmware test feature - if there is no space in event sw fifo,
         discard the event */
      if (length >= ifx_mps_fifo_mem_available (&pMPSDev->sw_event_upstrm_fifo))
      {
         IFXOS_UNLOCKINT (flags);
         return;
      }
#endif /* TEST_EVT_DISCARD */
      if (length > sizeof (mps_event_msg))
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("MPS Event message too large for buffer. Skipping\n"));
         ifx_mps_fifo_read_ptr_inc (mbx, length);
         IFXOS_UNLOCKINT (flags);
         return;
      }
      ifx_mps_mbx_read_message (mbx, &msg, &read_length);
      if (pMPSDev->event_mbx.event_mbx_callback != IFX_NULL)
      {
         pMPSDev->event_mbx.event_mbx_callback (pMPSDev->event_mbx.
                                                event_callback_handle, p_evt_msg);
      }
      else
      {
         if (ifx_mps_fifo_write (&pMPSDev->sw_event_upstrm_fifo, p_evt_msg->data[0], 0)
             == -1)
         {
            IFXOS_UNLOCKINT (flags);
            return;
         }
         if (ifx_mps_fifo_write (&pMPSDev->sw_event_upstrm_fifo, p_evt_msg->data[1], 4)
             == -1)
         {
            IFXOS_UNLOCKINT (flags);
            return;
         }
         ifx_mps_fifo_write_ptr_inc (&pMPSDev->sw_event_upstrm_fifo, length);

         /* wake up sleeping process for further processing of received event */
         IFXOS_DrvSelectQueueWakeUp (&(pMPSDev->event_mbx.mps_wakeuplist), 0);
      }
   }
   IFXOS_UNLOCKINT (flags);
   return;
}

IFX_int32_t ifx_mps_event_mbx_activation_poll (IFX_int32_t value)
{
   MPS_Ad0Reg_u Ad0Reg;

   /* Enable necessary MPS interrupts */
   Ad0Reg.val = *IFX_MPS_AD0ENR;
   Ad0Reg.fld.evt_mbx = value;
   *IFX_MPS_AD0ENR = Ad0Reg.val;
   return (IFX_SUCCESS);
}


/**
 * Change event interrupt activation.
 * Allows the upper layer enable or disable interrupt generation of event previously
 * registered. Note that
 *
 * \param   type   DSP device entity ( 1 - command, 2 - voice0, 3 - voice1,
 *                 4 - voice2, 5 - voice3, 6 - voice4 )
 * \param   act    Register values according to MbxEvent_Regs, whereas bit=1 means
 *                 active, bit=0 means inactive
 * \return  0      IFX_SUCCESS, interrupt masked changed accordingly
 * \return  ENXIO  Wrong DSP device entity (only 1-5 supported)
 * \return  EINVAL Callback value null
 * \ingroup API
 */
IFX_int32_t ifx_mps_event_activation_poll (mps_devices type,
                                           MbxEventRegs_s * act)
{
   mps_mbx_dev *pMBDev;
   MPS_Ad0Reg_u Ad0Reg;

   /* Get corresponding mailbox device structure */
   if ((pMBDev = ifx_mps_get_device (type)) == 0)
      return (-ENXIO);

   /* Enable necessary MPS interrupts */
   Ad0Reg.val = *IFX_MPS_AD0ENR;
   Ad0Reg.val =
      (Ad0Reg.val & ~pMBDev->event_mask.MPS_Ad0Reg.val) |
      (act->MPS_Ad0Reg.val & pMBDev->event_mask.MPS_Ad0Reg.val);
   *IFX_MPS_AD0ENR = Ad0Reg.val;

   atomic_set (&ifx_mps_dd_mbx_int_enabled, Ad0Reg.fld.dd_mbx);

   return (IFX_SUCCESS);
}


/**
 * Change event interrupt activation.
 * Allows the upper layer enable or disable interrupt generation of event previously
 * registered. Note that
 *
 * \param   type   DSP device entity ( 1 - command, 2 - voice0, 3 - voice1,
 *                 4 - voice2, 5 - voice3 )
 * \param   act    Register values according to MbxEvent_Regs, whereas bit=1 means
 *                 active, bit=0 means skip
 * \return  0      IFX_SUCCESS, interrupt masked changed accordingly
 * \return  ENXIO  Wrong DSP device entity (only 1-5 supported)
 * \return  EINVAL Callback value null
 * \ingroup API
 */
IFX_int32_t ifx_mps_event_activation (mps_devices type, MbxEventRegs_s * act)
{
   mps_mbx_dev *pMBDev;
   MPS_Ad0Reg_u Ad0Reg;

   /* Get corresponding mailbox device structure */
   if ((pMBDev = ifx_mps_get_device (type)) == 0)
      return (-ENXIO);

   /* Enable necessary MPS interrupts */
   Ad0Reg.val = *IFX_MPS_AD0ENR;
   Ad0Reg.val =
      (Ad0Reg.val & ~pMBDev->callback_event_mask.MPS_Ad0Reg.val) |
      (act->MPS_Ad0Reg.val & pMBDev->callback_event_mask.MPS_Ad0Reg.val);
   *IFX_MPS_AD0ENR = Ad0Reg.val;

   atomic_set (&ifx_mps_dd_mbx_int_enabled, Ad0Reg.fld.dd_mbx);

   return (IFX_SUCCESS);
}


/**
   This function enables mailbox interrupts on Danube.
\param
   None.
\return
   None.
*/
IFX_void_t ifx_mps_enable_mailbox_int ()
{
   MPS_Ad0Reg_u Ad0Reg;

   Ad0Reg.val = *IFX_MPS_AD0ENR;
   Ad0Reg.fld.cu_mbx = 1;
   Ad0Reg.fld.du_mbx = 1;
   Ad0Reg.fld.dl_end = 1;

   *IFX_MPS_AD0ENR = Ad0Reg.val;
}

/**
   This function disables mailbox interrupts on Danube.
\param
   None.
\return
   None.
*/
IFX_void_t ifx_mps_disable_mailbox_int ()
{
   MPS_Ad0Reg_u Ad0Reg;

   Ad0Reg.val = *IFX_MPS_AD0ENR;
   Ad0Reg.fld.cu_mbx = 0;
   Ad0Reg.fld.du_mbx = 0;
   *IFX_MPS_AD0ENR = Ad0Reg.val;
}

/**
   This function enables dd_mbx interrupts on Danube.
\param
   None.
\return
   None.
*/
IFX_void_t ifx_mps_dd_mbx_int_enable (void)
{
   IFXOS_INTSTAT flags;
   MPS_Ad0Reg_u Ad0Reg;

   IFXOS_LOCKINT (flags);

   if (atomic_read (&ifx_mps_dd_mbx_int_enabled) == 0)
   {
      Ad0Reg.val = *IFX_MPS_AD0ENR;
      Ad0Reg.fld.dd_mbx = 1;
      *IFX_MPS_AD0ENR = Ad0Reg.val;
   }

   atomic_inc (&ifx_mps_dd_mbx_int_enabled);

   IFXOS_UNLOCKINT (flags);
}

/**
   This function disables dd_mbx interrupts on Danube.
\param
   None.
\return
   None.
*/
IFX_void_t ifx_mps_dd_mbx_int_disable (void)
{
   IFXOS_INTSTAT flags;
   MPS_Ad0Reg_u Ad0Reg;

   IFXOS_LOCKINT (flags);

   if (atomic_read (&ifx_mps_dd_mbx_int_enabled) > 0)
   {
      atomic_dec (&ifx_mps_dd_mbx_int_enabled);

      if (atomic_read (&ifx_mps_dd_mbx_int_enabled) == 0)
      {
         Ad0Reg.val = *IFX_MPS_AD0ENR;
         Ad0Reg.fld.dd_mbx = 0;
         *IFX_MPS_AD0ENR = Ad0Reg.val;
      }
   }

   IFXOS_UNLOCKINT (flags);
}

/**
   This function disables all MPS interrupts on Danube.
\param
   None.
\return
   None.
*/
IFX_void_t ifx_mps_disable_all_int ()
{
   *IFX_MPS_SAD0SR = 0x00000000;
}

/******************************************************************************
 * Interrupt service routines
 ******************************************************************************/

/**
 * Upstream data interrupt handler
 * This function is called on occurence of an data upstream interrupt.
 * Depending on the occured interrupt either the command or data upstream
 * message processing is started via tasklet
 *
 * \param   irq      Interrupt number
 * \param   pDev     Pointer to MPS communication device structure
 * \ingroup Internal
 */
irqreturn_t ifx_mps_ad0_irq (IFX_int32_t irq, mps_comm_dev * pDev)
{
   MbxEventRegs_s events;
   MPS_Ad0Reg_u MPS_Ad0StatusReg;
   mps_mbx_dev *mbx_dev = (mps_mbx_dev *) & (pMPSDev->command_mb);

   IFX_MPS_UNUSED(pDev);

   /* read interrupt status */
   MPS_Ad0StatusReg.val = *IFX_MPS_RAD0SR;
   /* acknowledge interrupt */
   *IFX_MPS_CAD0SR = MPS_Ad0StatusReg.val;
   /* handle only enabled interrupts */
   MPS_Ad0StatusReg.val &= *IFX_MPS_AD0ENR;

#if !defined(SYSTEM_FALCON)
#if   (BSP_API_VERSION == 1)
   mask_and_ack_danube_irq (irq);
#elif (BSP_API_VERSION == 2)
   bsp_mask_and_ack_irq (irq);
#endif
#endif /* !defined(SYSTEM_FALCON) */

   /* FW is up and ready to process commands */
   if (MPS_Ad0StatusReg.fld.dl_end)
   {
      IFXOS_EventWakeUp (&fw_ready_evt);
   }

#ifdef PRINT_ON_ERR_INTERRUPT
   if (MPS_Ad0StatusReg.fld.data_err)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("\n%s() - data_err\n", __FUNCTION__));
   }

   if (MPS_Ad0StatusReg.fld.cmd_err)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("\n%s() - cmd_err\n", __FUNCTION__));
   }

   if (MPS_Ad0StatusReg.fld.pcm_crash)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("\n%s() - pcm_crash\n", __FUNCTION__));
   }

   if (MPS_Ad0StatusReg.fld.mips_ol)

   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("\n%s() - mips_ol\n", __FUNCTION__));
   }

   if (MPS_Ad0StatusReg.fld.evt_ovl)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("\n%s() - evt_ovl\n", __FUNCTION__));
   }

   if (MPS_Ad0StatusReg.fld.rcv_ov)
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("\n%s() - rcv_ov\n", __FUNCTION__));
   }
#endif /* */

   if (MPS_Ad0StatusReg.fld.dd_mbx)
   {
      if (atomic_read (&ifx_mps_write_blocked) != 0)
      {
         /* use the fastbuf setup values */
         if (ifx_mps_bufman_buf_provide(0, 0, 0) == IFX_SUCCESS)
         {
            /* mark interrupt as handled, skip handling on high level */
            MPS_Ad0StatusReg.fld.dd_mbx = 0;
         }
      }
   }

   if (MPS_Ad0StatusReg.fld.du_mbx)
   {
#ifdef CONFIG_PROC_FS
      pMPSDev->voice_mb[0].upstrm_fifo->pkts++;
#endif /* CONFIG_PROC_FS */
      ifx_mps_mbx_data_upstream (0);
   }

   if (MPS_Ad0StatusReg.fld.cu_mbx)
   {
#ifdef CONFIG_PROC_FS
      pMPSDev->command_mb.upstrm_fifo->pkts++;
#endif /* CONFIG_PROC_FS */
      ifx_mps_mbx_cmd_upstream (0);
   }

   if (MPS_Ad0StatusReg.fld.evt_mbx)
   {
      ifx_mps_mbx_event_upstream (0);
   }

#if CONFIG_MPS_HISTORY_SIZE > 0
   if (MPS_Ad0StatusReg.fld.cmd_err)
   {
      ifx_mps_history_buffer_freeze = 1;
      TRACE (MPS, DBG_LEVEL_HIGH, ("MPS cmd_err interrupt!\n"));
   }
#endif /* */
   pMPSDev->event.MPS_Ad0Reg.val =
      MPS_Ad0StatusReg.val | (pMPSDev->event.MPS_Ad0Reg.val & mbx_dev->
                              event_mask.MPS_Ad0Reg.val);

   /* use callback function or queue wake up to notify about data reception */
   if (mbx_dev->event_callback != IFX_NULL)
   {
      if (mbx_dev->callback_event_mask.MPS_Ad0Reg.val & MPS_Ad0StatusReg.val)
      {
         events.MPS_Ad0Reg.val = MPS_Ad0StatusReg.val;
         /* pass only requested bits */
         events.MPS_Ad0Reg.val &= mbx_dev->callback_event_mask.MPS_Ad0Reg.val;

         mbx_dev->event_callback (&events);
      }
   }

   if (mbx_dev->event_mask.MPS_Ad0Reg.val & MPS_Ad0StatusReg.val)
   {
      IFXOS_DrvSelectQueueWakeUp (&(mbx_dev->mps_wakeuplist), 0);
   }

   return IRQ_HANDLED;
}


/**
 * Print firmware version.
 * This function queries the current firmware version and prints it.
 *
 * \return  0        IFX_SUCCESS
 * \return  -EFAULT  Error while fetching version
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_get_fw_version (IFX_int32_t print)
{
   MbxMsg_s msg;
   MbxMsg_s msg2;
   IFX_uint32_t *ptmp;
   mps_fifo *fifo;
   IFX_int32_t timeout = 300;   /* 3s timeout */
   IFX_int32_t retval;
   IFX_uint32_t bytes_read = 0;

   fifo = &(ifx_mps_dev.cmd_upstrm_fifo);
   /* build message */
   ptmp = (IFX_uint32_t *) & msg;
   ptmp[0] = 0x8600e604;
   ptmp[1] = 0x00000000;

   /* send message */
   retval =
      ifx_mps_mbx_write_message (&(ifx_mps_dev.command_mb),
                                 (IFX_uint8_t *) & msg, 4);
   while (!ifx_mps_fifo_not_empty (fifo) && timeout > 0)
   {
      /* Sleep for 10ms */
      IFXOS_MSecSleep(10);
      timeout--;
   }
   if (timeout == 0)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("[%s %s %d]: timeout\n", __FILE__, __func__, __LINE__));
      return -EFAULT;
   }
   memset(&msg2, 0xCC, sizeof(MbxMsg_s));
   retval = ifx_mps_mbx_read_message (fifo, &msg2, &bytes_read);
   if ((retval != 0) || (bytes_read != 8))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("[%s %d]: error, retval=%d, bytes_read=%d!\n", __func__, __LINE__, retval, bytes_read));

      TRACE (MPS, DBG_LEVEL_HIGH,
             ("[Msg]: Hdr=0x%08X, Data=0x%08X\n", msg2.header.val, msg2.data[0]));

      return -EFAULT;
   }
   ptmp = (IFX_uint32_t *) & msg2;

   if (print)
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("ok!\nVersion %d.%d.%d.%d.%d up and running...\n",
              (ptmp[1] >> 24) & 0xff, (ptmp[1] >> 16) & 0xff,
              (ptmp[1] >> 12) & 0xf, (ptmp[1] >> 8) & 0xf, (ptmp[1] & 0xff)));

   return 0;
}

/**
 * Calculate CRC-32 checksum on voice EDSP firmware image located
 * at cpu1_base addr.
 *
 * \param   l_cpu1_base_addr Base address of CPU1, obtained from BSP.
 * \param   l_pFW_img_data   ptr to FW image metadata such as range
 *                           for checksum calculation etc.
 *
 * \return  crc32            The CRC-32 checksum.
 *
 * \ingroup Internal
 */
IFX_uint32_t ifx_mps_fw_crc32(volatile IFX_uint32_t *l_cpu1_base_addr,
                                   FW_image_ftr_t *l_pFW_img_data)
{
   IFX_uint32_t crc32;

   crc32 = ifx_mps_calc_crc32(0xffffffff, (IFX_uint8_t *)l_cpu1_base_addr,
                              2*sizeof(IFX_uint32_t), IFX_FALSE);
   crc32 = ifx_mps_calc_crc32(~crc32, ((IFX_uint8_t *)l_cpu1_base_addr) +
                                                      l_pFW_img_data->st_addr_crc,
                              l_pFW_img_data->en_addr_crc -
                                                      l_pFW_img_data->st_addr_crc,
                              IFX_TRUE);
   return crc32;
}

/**
 * Re-calculate and compare the calculated EDSP firmware checksum with
 * stored EDSP firmware checksum. Print on console whether verification
 * is passed or not. Called in case of FW watchdog or timeout waiting
 * for FW ready event.
 *
 * \param   l_cpu1_base_addr Base address of CPU1, obtained from BSP.
 * \param   l_pFW_img_data   ptr to FW image metadata such as range
 *                           for checksum calculation etc.
 * \return  none
 *
 * \ingroup Internal
 */
IFX_void_t ifx_mps_fw_crc_compare(volatile IFX_uint32_t *l_cpu1_base_addr,
                                  FW_image_ftr_t *l_pFW_img_data)
{
   IFX_uint32_t stored_cksum = l_pFW_img_data->crc32;
   IFX_uint32_t calc_cksum = ifx_mps_fw_crc32(l_cpu1_base_addr,
                                                   l_pFW_img_data);

   if (stored_cksum != calc_cksum)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("MPS: FW checksum error: calculated=0x%08x stored=0x%08x\r\n",
              calc_cksum, stored_cksum));
   }
   else
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("MPS: FW checksum OK\r\n"));
   }
}

/**
 * Dump the EDSP firmware exception area on console. The size of the
 * exception area is different on different platforms. The information
 * on exception area size is stored in pFW_img_data buffer.
 * Called in case of FW watchdog or timeout waiting for FW ready event.
 *
 * \param   l_cpu1_base_addr Base address of CPU1, obtained from BSP.
 * \param   l_pFW_img_data   ptr to FW image metadata such as range
 *                           for checksum calculation etc.
 * \return  none
 *
 * \ingroup Internal
 */
IFX_void_t ifx_mps_dump_fw_xcpt(volatile IFX_uint32_t *l_cpu1_base_addr,
                                FW_image_ftr_t *l_pFW_img_data)
{
   IFX_uint32_t offset, *pTmp;
   IFX_boolean_t bPrintout = IFX_FALSE;

   for(offset=FW_XCPT_AREA_OFFSET;
                offset<l_pFW_img_data->st_addr_crc; offset += 4)
   {
      pTmp = (IFX_uint32_t *)(((IFX_uint8_t*)l_cpu1_base_addr)+offset);
      if (0 != *pTmp)
      {
         bPrintout = IFX_TRUE;
         break;
      }
   }

   if (IFX_TRUE == bPrintout)
   {
      for(offset=FW_XCPT_AREA_OFFSET;
                   offset<l_pFW_img_data->st_addr_crc; offset += 4)
      {
         pTmp = (IFX_uint32_t *)(((IFX_uint8_t*)l_cpu1_base_addr)+offset);
         TRACE (MPS, DBG_LEVEL_HIGH, ("%08x: %08x\r\n", offset, *pTmp));
      }
   }
}

#ifndef VMMC_WITH_MPS
EXPORT_SYMBOL (ifx_mps_bufman_register);
EXPORT_SYMBOL (ifx_mps_bufman_unregister);
EXPORT_SYMBOL (ifx_mps_event_activation);
EXPORT_SYMBOL (ifx_mps_register_bufman_freeall_callback);
#endif

#if defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)
/**
 * Issue a Hit_Invalidate_D operation on a data segment of a
 * given size at given address. Address range is aligned to the
 * multiple of a cache line size.
 *
 * \param   addr    Address of a data segment
 * \param   len     Length in bytes of a data segment
 * \return  none
 * \ingroup Internal
 * \remarks addr parameter must be in KSEG0
 */
IFX_void_t ifx_mps_cache_inv (IFX_ulong_t addr, IFX_uint32_t len)
{
   IFX_ulong_t aline = addr & ~(CACHE_LINE_SZ - 1);
   IFX_ulong_t end   = (addr + len - 1) & ~(CACHE_LINE_SZ - 1);

   if (IFX_FALSE == bDoCacheOps)
      return;

   while (1)
   {
      __asm__ __volatile__(
         " .set    push        \n"
         " .set    noreorder   \n"
         " .set    mips3\n\t   \n"
         " cache   0x11, 0(%0) \n"
         " .set    pop         \n"
         : :"r"(aline));

      if (aline == end)
         break;

      aline += CACHE_LINE_SZ;
   }
}

/**
 * Issue a Hit_Writeback_Inv_D operation on a data segment of a
 * given size at given address. Address range is aligned to the
 * multiple of a cache line size.
 *
 * \param   addr    Address of a data segment
 * \param   len     Length in bytes of a data segment
 * \return  none
 * \ingroup Internal
 * \remarks addr parameter must be in KSEG0
 */
static IFX_void_t ifx_mps_cache_wb_inv (IFX_ulong_t addr, IFX_uint32_t len)
{
   IFX_ulong_t aline = addr & ~(CACHE_LINE_SZ - 1);
   IFX_ulong_t end   = (addr + len - 1) & ~(CACHE_LINE_SZ - 1);

   while (1)
   {
      __asm__ __volatile__(
         " .set    push            \n"
         " .set    noreorder       \n"
         " .set    mips3\n\t       \n"
         " cache   0x15, 0(%0)     \n"
         " .set    pop             \n"
         : :"r" (aline));

      if (aline == end)
         break;

      aline += CACHE_LINE_SZ;
   }
   /* MIPS multicore write reordering workaround:
      writing to on-chip SRAM and off-chip SDRAM can be reordered in time on
      MIPS multicore, in other words, there is no guarantee that write
      operation to SDRAM is finished at the moment of passing a data pointer to
      voice CPU  through data mailbox in SRAM.
      Workaround sequence:
      1) Write back (and invalidate) all used cache lines
      2) SYNC
      3) Read-back uncahed one word
      4) SYNC
      5) Write data pointer message to the mailbox in the on-chip SRAM */
   __asm__ __volatile__(" sync \n");
   /* dummy read back uncached */
   *((volatile IFX_uint32_t *)KSEG1ADDR(aline));
   __asm__ __volatile__(" sync \n");
}
#endif /*defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)*/


/**
 * Enable and test the HW
 *
 * This function is called to enable power-domains, clock-domains and other
 * HW needed to use the MPS. If possible it will also test the HW.
 *
 * \return
 * 0 on success or error code otherwise.
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_hw_enable(void)
{
#if defined(SYSTEM_FALCON)
   ifx_mps_chip_family = MPS_CHIP_FALCON;
   sys_hw_setup ();
   return 0;
#else
   IFX_int32_t ret;

   ret = ifx_mps_HwProbeXRX100();
   if (!ret)
   {
      ifx_mps_HwSetupXRX100(IFX_ENABLE);
   }

   return ret;
#endif
}


/**
 * Disable the HW
 * This function is called to disable power-domains, clock-domains and other
 * HW needed to use the MPS.
 *
 * \ingroup Internal
 */
IFX_void_t ifx_mps_hw_disable(void)
{
#if defined(SYSTEM_FALCON)
   #if   (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
      sys1_hw_deactivate (ACTS_MPS);
   #else
      ltq_sysctl_sys1_activate (SYSCTRL_SYS1_MPS);
   #endif
#else
   ifx_mps_HwSetupXRX100(IFX_DISABLE);
#endif
}
