#ifndef __IFX_UDP_REDIRECT_H__
#define __IFX_UDP_REDIRECT_H__
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file ifx_udp_redirect.h
   Implements
   This file contains definitions of structures and function prototypes for
   the driver that connects between KPI and linux kernel stack.
 */

#ifdef __KERNEL__

/* #define MAX_CHANNEL     8 */

/** UDP filter table status definition */
#define FILTER_NO_ERROR        0x00
#define FILTER_NULL            0x01
#define FILTER_NO_CHANNEL      0x02
#define FILTER_NO_SRCPORT      0x04
#define FILTER_NO_SRCIP        0x08
#define FILTER_NO_DSTPORT      0x10
#define FILTER_NO_DSTIP        0x20
#define FILTER_NO_CALLBACK     0x40

/** Return code definition */
#define NO_ERROR               0x0000
#define CHANNEL_NUM_ERR        0x0001
#define REG_CALLBACK_ERR       0x0002
#define NO_CALLBACK            0x0003
#define CHANNEL_NO_ERR         0x0004
#define WRONG_PKT              0x0005
#define CALL_MK_SESSION_ERR    0x0006
#define CALL_DEL_SESSION_ERR   0x0007
#define CALLBACK_ERR           0x0008


typedef int (*v_callback_ingress)(int chanDev,
                                  void* data,
                                  size_t len);

extern int (*reg_ingress)(v_callback_ingress callback,
                       int channel_num);

extern int (*vtou_redirectrtp)(int channel,
                            void* buff,
                            size_t len);

#endif /* __KERNEL__ */
#endif /* __IFX_UDP_REDIRECT_H__ */

