#ifndef _DRV_MPS_VMMC_H
#define _DRV_MPS_VMMC_H
/******************************************************************************

                            Copyright (c) 2007-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_mps_vmmc.h
   This file contains the defines, the structures declarations,
   the tables declarations and the global functions declarations.
*/

/* ============================= */
/* MPS Common defines            */
/* ============================= */

enum mps_chip
{
   MPS_CHIP_UNKNOWN,
   MPS_CHIP_XRX100,
   MPS_CHIP_XRX200,
   MPS_CHIP_XRX300,
   MPS_CHIP_FALCON
};

/* Variable indicating the identified chip family. */
extern enum mps_chip ifx_mps_chip_family;

/**
 * MPS Mailbox Message.
 */
typedef struct
{
   IFX_uint8_t *pData;         /**< Pointer to data location in SDRAM to be passed to other CPU */
   IFX_uint32_t nDataBytes;     /**< Amount of valid data bytes in SDRAM starting at pData */
   IFX_uint32_t RTP_PaylOffset; /**< Byte offset to reserve space for RTP header */
   IFX_uint32_t cmd_type;       /**< Type of command */
} mps_message;

typedef enum
{
   unknown,
   command,
   voice0,
   voice1,
   voice2,
   voice3,
   voice4,
   voice5,
   voice6,
   voice7,
   voice8,
   voice9,
   voice10,
   voice11,
   voice12,
   voice13,
   event_mbx
} mps_devices;

#define E_MPS_DEVICE_TYPE_FIRST   command
#define E_MPS_DEVICE_TYPE_LAST    event_mbx

#define E_MPS_DEVICE_TYPE_VOICE_FIRST   voice0
#define E_MPS_DEVICE_TYPE_VOICE_LAST    voice13

#define MPS_DEVICE_MAX_VOICE_CHANNELS   ((int)(E_MPS_DEVICE_TYPE_VOICE_LAST - E_MPS_DEVICE_TYPE_VOICE_FIRST) + 1)
#define MPS_DEVICE_MAX_VOICE_CHANNEL_INVALID   (MPS_DEVICE_MAX_VOICE_CHANNELS + 1)

#define E_MPS_DEVICE_TYPE_IN_RANGE(E_MPS_DEV_TYPE) \
   ((((mps_devices)(E_MPS_DEV_TYPE) >= E_MPS_DEVICE_TYPE_FIRST) && \
     ((mps_devices)(E_MPS_DEV_TYPE) <= E_MPS_DEVICE_TYPE_LAST)) ? 1 : 0)

#define E_MPS_DEVICE_TYPE_IS_VOICE(E_MPS_DEV_TYPE) \
   ((((mps_devices)(E_MPS_DEV_TYPE) >= E_MPS_DEVICE_TYPE_VOICE_FIRST) && \
     ((mps_devices)(E_MPS_DEV_TYPE) <= E_MPS_DEVICE_TYPE_VOICE_LAST) ) ? 1 : 0)

#define MPS_DEVICE_CHANNEL_NUM_IS_VALID(CH_NUM) \
   ((((CH_NUM) >= 0) && ((CH_NUM) < MPS_DEVICE_MAX_VOICE_CHANNELS)) ? 1 : 0)

#define MPS_DEVICE_VOICE_CHANNEL_2_TYPE(CH_NUM) \
   ((MPS_DEVICE_CHANNEL_NUM_IS_VALID(CH_NUM)) ? \
    ((mps_devices)(CH_NUM) + E_MPS_DEVICE_TYPE_VOICE_FIRST) : unknown)

#define MPS_DEVICE_VOICE_TYPE_2_CHANNEL(VOICE_TYPE) \
   ((E_MPS_DEVICE_TYPE_IS_VOICE(VOICE_TYPE)) ? \
    ((VOICE_TYPE) - E_MPS_DEVICE_TYPE_VOICE_FIRST) : MPS_DEVICE_MAX_VOICE_CHANNEL_INVALID)


/**
 * Mailbox history structure.
 * This structure contains the history of messages sent to the mailbox.
 */
typedef struct
{
   IFX_uint32_t *buf;        /**< History buffer */
   IFX_int32_t len;          /**< Length of history buffer in words */
   IFX_int32_t total_words;  /**< Overall number of words sent to mailbox */
   IFX_int32_t freeze;       /**< Indication whether logging was stopped */
} mps_history;

/**
 * Firmware structure.
 * This structure contains a pointer to the firmware and its length.
 */
typedef struct
{
   IFX_uint32_t *data;  /**< Pointer to firmware image */
   IFX_uint32_t length;  /**< Length of firmware in bytes */
} mps_fw;

#define CMD_VOICEREC_STATUS_PACKET  0x0
#define CMD_VOICEREC_DATA_PACKET    0x1
#define CMD_RTP_VOICE_DATA_PACKET   0x4
#define CMD_RTP_EVENT_PACKET        0x5
#define CMD_ADDRESS_PACKET          0x8
#define CMD_FAX_DATA_PACKET         0x10
#define CMD_FAX_STATUS_PACKET       0x11
#define CMD_P_PHONE_DATA_PACKET     0x12
#define CMD_P_PHONE_STATUS_PACKET   0x13
#define CMD_CID_DATA_PACKET         0x14

#define CMD_ALI_PACKET              0x1
#define CMD_COP_PACKET              0x2
#define CMD_EOP_PACKET              0x6

/******************************************************************************
 * Exported IOCTLs
 ******************************************************************************/
/** magic number */
#define IFX_MPS_MAGIC 'O'

/**
 * Set event notification mask.
 * \param   arg Event mask
 * \ingroup IOCTL
 */
#define FIO_MPS_EVENT_REG _IOW(IFX_MPS_MAGIC, 1, IFX_uint32_t)
/**
 * Mask Event Notification.
 * \ingroup IOCTL
 */
#define FIO_MPS_EVENT_UNREG _IO(IFX_MPS_MAGIC, 2)
/**
 * Read Message from Mailbox.
 * \param   arg Pointer to structure #mps_message
 * \ingroup IOCTL
 */
#define FIO_MPS_MB_READ _IOR(IFX_MPS_MAGIC, 3, mps_message)
/**
 * Write Message to Mailbox.
 * \param   arg Pointer to structure #mps_message
 * \ingroup IOCTL
 */
#define FIO_MPS_MB_WRITE _IOW(IFX_MPS_MAGIC, 4, mps_message)
/**
 * Reset Voice CPU.
 * \ingroup IOCTL
 */
#define FIO_MPS_RESET _IO(IFX_MPS_MAGIC, 6)
/**
 * Restart Voice CPU.
 * \ingroup IOCTL
 */
#define FIO_MPS_RESTART _IO(IFX_MPS_MAGIC, 7)
/**
 * Read Version String.
 * \param   arg Pointer to version string.
 * \ingroup IOCTL
 */
#define FIO_MPS_GETVERSION      _IOR(IFX_MPS_MAGIC, 8, char*)
/**
 * Reset Mailbox Queue.
 * \ingroup IOCTL
 */
#define FIO_MPS_MB_RST_QUEUE _IO(IFX_MPS_MAGIC, 9)
/**
 * Download Firmware
 * \param   arg Pointer to structure #mps_fw
 * \ingroup IOCTL
 */
#define  FIO_MPS_DOWNLOAD _IO(IFX_MPS_MAGIC, 17)
/**
 * Set FIFO Blocking State.
 * \param   arg Boolean value [0=off,1=on]
 * \ingroup IOCTL
 */
#define  FIO_MPS_TXFIFO_SET _IOW(IFX_MPS_MAGIC, 18, bool_t)
/**
 * Read FIFO Blocking State.
 * \param   arg Boolean value [0=off,1=on]
 * \ingroup IOCTL
 */
#define  FIO_MPS_TXFIFO_GET _IOR(IFX_MPS_MAGIC, 19, bool_t)
/**
 * Read channel Status Register.
 * \param   arg Content of status register
 * \ingroup IOCTL
 */
#define  FIO_MPS_GET_STATUS _IOR(IFX_MPS_MAGIC, 20, IFX_uint32_t)
/**
 * Read command history buffer.
 * \param   arg Pointer to structure #mps_history
 * \ingroup IOCTL
 */
#define  FIO_MPS_GET_CMD_HISTORY _IOR(IFX_MPS_MAGIC, 21, IFX_uint32_t)

/**
 * Enable event notification for event mailbox.
 * \ingroup IOCTL
 */
#define FIO_MPS_EVENT_MBX_REG _IO(IFX_MPS_MAGIC, 22)
/**
 * Disable event notification for event mailbox.
 * \ingroup IOCTL
 */
#define FIO_MPS_EVENT_MBX_UNREG _IO(IFX_MPS_MAGIC, 23)

/******************************************************************************
 * Register structure definitions
 ******************************************************************************/
/** Register structure for AFE and DFE 0 Status Register.
    Registers: MPS_RAD0SR, MPS_SAD0SR, MPS_CAD0SR and MPS_AD0ENR. */
typedef struct
{
   IFX_uint32_t res1:16;
   IFX_uint32_t dl_end:1;
   IFX_uint32_t wd_fail:1;
   IFX_uint32_t res2:2;
   IFX_uint32_t mips_ol:1;
   IFX_uint32_t data_err:1;
   IFX_uint32_t pcm_crash:1;
   IFX_uint32_t cmd_err:1;
   IFX_uint32_t res3:1;
   IFX_uint32_t evt_ovl:1;
   IFX_uint32_t evt_mbx:1;
   IFX_uint32_t rcv_ov:1;
   IFX_uint32_t dd_mbx:1;
   IFX_uint32_t cd_mbx:1;
   IFX_uint32_t du_mbx:1;
   IFX_uint32_t cu_mbx:1;
} MPS_Ad0Reg_s;

typedef union
{
   IFX_uint32_t val;
   MPS_Ad0Reg_s fld;
} MPS_Ad0Reg_u;

typedef struct
{
   MPS_Ad0Reg_u MPS_Ad0Reg;
} MbxEventRegs_s;

#define MAX_EVENT_MSG_LENGTH 2
typedef struct
{
   volatile IFX_uint32_t data[MAX_EVENT_MSG_LENGTH];   /**< Data of event message */
} mps_event_msg;

/******************************************************************************
 * Exported functions
 ******************************************************************************/
#ifdef __KERNEL__
#include <linux/fs.h>
IFX_int32_t ifx_mps_open (struct inode *inode, struct file *file_p);
IFX_int32_t ifx_mps_close (struct inode *inode, struct file *filp);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
IFX_int32_t ifx_mps_ioctl (struct inode *inode, struct file *file_p,
#else
long ifx_mps_ioctl (struct file *filp,
#endif
                           IFX_uint32_t nCmd, unsigned long arg);
IFX_int32_t ifx_mps_register_data_callback (mps_devices type, IFX_uint32_t dir,
                                            IFX_void_t (*callback) (mps_devices
                                                                    type));
IFX_int32_t ifx_mps_unregister_data_callback (mps_devices type,
                                              IFX_uint32_t dir);
IFX_int32_t ifx_mps_register_event_callback (mps_devices type,
                                             MbxEventRegs_s * mask,
                                             IFX_void_t (*callback)
                                             (MbxEventRegs_s * events));
IFX_int32_t ifx_mps_unregister_event_callback (mps_devices type);
IFX_int32_t ifx_mps_event_activation (mps_devices type, MbxEventRegs_s * act);
IFX_int32_t ifx_mps_register_event_mbx_callback (IFX_uint32_t pDev,
                                                 IFX_void_t (*callback)
                                                 (IFX_uint32_t pDev,
                                                  mps_event_msg * msg));
IFX_int32_t ifx_mps_unregister_event_mbx_callback (void);
IFX_int32_t ifx_mps_read_mailbox (mps_devices type, mps_message * rw);
IFX_int32_t ifx_mps_write_mailbox (mps_devices type, mps_message * rw);
IFX_void_t ifx_mps_bufman_register (IFX_void_t *
                                    (*malloc) (IFX_size_t size,
                                               IFX_int32_t priority),
                                    IFX_void_t (*free) (const IFX_void_t * ptr),
                                    IFX_uint32_t fb_seg_size_byte,
                                    IFX_uint32_t treshold);

IFX_void_t ifx_mps_bufman_register_new(
   IFX_void_t *(*fb_ext_malloc) (IFX_size_t size, IFX_int32_t priority),
   IFX_void_t (*fb_ext_free) (const IFX_void_t * ptr),
   IFX_void_t (*fb_ext_freeall)(void),
   IFX_uint32_t fb_num_of_elem,
   IFX_uint32_t fb_seg_size_byte,
   IFX_uint32_t fb_treshold);

IFX_void_t ifx_mps_bufman_unregister(void);

IFX_void_t ifx_mps_dd_mbx_int_enable (void);
IFX_void_t ifx_mps_dd_mbx_int_disable (void);

#if defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)
   extern IFX_void_t ifx_mps_cache_inv (IFX_ulong_t addr, IFX_uint32_t len);
#else /* defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED) */
   #define ifx_mps_cache_inv(addr,len)
#endif /* defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED) */

#endif  /*__KERNEL__*/

#endif /* _DRV_MPS_VMMC_H */
