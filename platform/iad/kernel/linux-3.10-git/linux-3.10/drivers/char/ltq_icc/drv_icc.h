#ifndef _DRV_ICC_H
#define _DRV_ICC_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

****************************************************************************
   Module      : drv_icc.h
   Description : This file contains the defines, the structures declarations
                 the tables declarations and the global functions declarations.
*******************************************************************************/
#include <linux/fs.h>
#include "../ltq_mps2/drv_config.h"
#include "../ltq_mps2/drv_mps.h"
/* ============================= */
/* ICC Common defines            */
/* ============================= */
/*---------------------------------------------------------------------------*/
#define MAX_MMAP 64
#define MAX_DEPTH 16
#define MIN_THRESHOLD 4
#define MAX_THRESHOLD 12
#define ICC_MSG_FLOW_CONTROL 0x1



/*DebugInfo*/
#define DBG_LEVEL_OFF      4
#define DBG_LEVEL_HIGH     3
#define DBG_LEVEL_NORMAL   2
#define DBG_LEVEL_LOW      1

#define CREATE_TRACE_GROUP(name) unsigned int G_nTraceGroup##name = DBG_LEVEL_HIGH
#define DECLARE_TRACE_GROUP(name) extern unsigned int G_nTraceGroup##name
#define PRINTF printk
#define TRACE(name,level,message) do {if(level >= G_nTraceGroup##name) \
      { PRINTF message ; } } while(0)

/*---------------------------------------------------------------------------*/
/* Device connection structure                                               */
/*---------------------------------------------------------------------------*/
typedef struct{
 uint8_t src_client_id;
 uint8_t dst_client_id;
 uint8_t msg_id;
 uint8_t param_attr;
 uint32_t param[MAX_UPSTRM_DATAWORDS];
} icc_msg_t;

typedef struct{
	uint32_t address[MAX_UPSTRM_DATAWORDS];
  uint32_t length[MAX_UPSTRM_DATAWORDS];
  uint32_t offset[MAX_UPSTRM_DATAWORDS];
	uint32_t count;
}icc_commit_t;

/*Always keep your new clients within InvalidClient and MAX_CLIENT*/
typedef enum
{
   ICC_Client,/*Icc client Id*/
	 IA,/*Image authenticator*/
	 SR,/*System reset driver*/
	 IR,/*Ioctl redirector*/
	 PR,/*procfs redirector*/
   MAX_CLIENT=21/*MAX_CLIENT_ID*/
} icc_devices;/*enum for all the possible clients*/
/******************************************************************************
 * Exported functions
 ******************************************************************************/
/** magic number */
#define ICC_MAGIC 'S'
#define ICC_IOC_REG_CLIENT _IOW(ICC_MAGIC,1,uint32_t)
#define ICC_IOC_MEM_COMMIT _IOW(ICC_MAGIC,2,icc_commit_t)
#define ICC_IOC_MEM_INVALIDATE _IOW(ICC_MAGIC,3,icc_commit_t)

#define ICC_BOOTCORE_UP 0x1

#ifdef __KERNEL__

typedef enum
{
	ICC_INVALID=0x0,
	ICC_READ=0x1,
	ICC_WRITE=0x2,
	ICC_RW=0x3
}icc_wake_type;

/**
 * ICC Device Structure.
 */
typedef struct
{
   /* Wakeuplist for the select mechanism */
   wait_queue_head_t wakeuplist;
   void (*up_callback) (icc_wake_type);
   volatile int Installed;
}icc_dev;

int icc_open (struct inode *inode, struct file *file_p);
int icc_close (struct inode *inode, struct file *filp);
int icc_register_callback (icc_devices type,
																			 void (*callback) (icc_wake_type));
int icc_unregister_callback (icc_devices type);
int icc_read (icc_devices type, icc_msg_t * rw);
int icc_write (icc_devices type, icc_msg_t * rw);
#endif /*__KERNEL__*/
#ifdef __LIBRARY__
int icc_init(void);
int icc_raw_read (icc_msg_t * rw);
int icc_write (icc_devices type, icc_msg_t * rw);
#endif
#endif /* _DRV_ICC_H */
