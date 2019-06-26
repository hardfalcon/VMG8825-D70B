/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

/**
   \file ifx_udp_redirect.c
   Implements the driver that transports voice data directly between the TAPI
   KPI interface and the Linux kernel network stack.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include <linux/kernel.h>
#ifdef MODULE
   #include <linux/module.h>
#endif
#include <linux/version.h>
#include <asm/byteorder.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/udp.h>
#include <net/inet_common.h>
#include <linux/file.h>

#include "ifx_types.h"                    /* ifx type definitions */

#include "ifxos_interrupt.h"

#include <srtp.h>

#include "drv_tapi_kpi_io.h"
#include "drv_tapi_qos_ll_interface.h"

#include "linux/udp_redirect.h"           /* patched into linux kernel */

#ifdef LINUX
#undef KPI_DBG_UDP_SENDMSG
#endif /* LINUX */

#include "drv_version.h"

/* ============================= */
/* Configuration Definitions     */
/* ============================= */

/* Maximum number of channels that can be handled at the same time.
   Please make sure that IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP matches this. */
#define MAX_REDIRECT_CHANNELS       16

/* Default TOS value put into UDP packets sent from this driver.
   This value can be overwritten with parameter TOS when loading the module. */
#define KPI2UDP_DEFAULT_TOS         0xB8

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* get the channel number without the group number in the upper 4 bits */
#define KPI_CHANNEL_GET(channel)  (channel & 0x0FFF)

/* Return code definition */
#define NO_ERROR                 0x0000
#define NO_CALLBACK              0x0001
#define CHANNEL_NO_ERR           0x0002
#define WRONG_PKT                0x0003
#define CALL_MK_SESSION_ERR      0x0004
#define NO_BUFFER                0x0005

#ifndef _MKSTR_1
#define _MKSTR_1(x)     #x
#define _MKSTR(x)       _MKSTR_1(x)
#endif

/** driver version string */
#define DRV_KPI2UDP_VER_STR      _MKSTR(MAJORSTEP)   "."   \
                                 _MKSTR(MINORSTEP)   "."   \
                                 _MKSTR(VERSIONSTEP) "."   \
                                 _MKSTR(VERS_TYPE)

/** what compatible driver version */
#define DRV_KPI2UDP_WHAT_STR \
        "@(#)Lantiq KPI2UDP driver, version " DRV_KPI2UDP_VER_STR


/* ============================= */
/* Local type definitions        */
/* ============================= */
/* entry in the redirect table */
typedef struct
{
  IFX_boolean_t   in_use;
  IFX_boolean_t   ext_sock;
  struct sock    *sk;
  struct socket  *sock;
  IFX_uint16_t   family;
  union
  {
      struct sockaddr_in dst_addr;
      struct sockaddr_in6 dst_addr6;
  };
  int             last_sock_ret;
  struct srtp_ctx_t  *srtp_session;
  IFX_uint8_t        localKey[64];
  IFX_uint8_t        remoteKey[64];

} IFX_KPI2UDP_REDIRECT_TABLE_ENTRY_t;

/* redirect table structure */
typedef struct
{
   /*the number of slots in the table below */
   IFX_int16_t                         channel_num;
   IFX_int16_t                         srtp_supported;
   /* table with redirect entries */
   IFX_KPI2UDP_REDIRECT_TABLE_ENTRY_t  channels[MAX_REDIRECT_CHANNELS];
} IFX_KPI2UDP_REDIRECT_TABLE_t;

/* ============================= */
/* Local variable definition     */
/* ============================= */
/* UDP redirect table */
static IFX_KPI2UDP_REDIRECT_TABLE_t  redtab = {0};
/* Struct containing function pointers that are registered with TAPI. */
static IFX_TAPI_DRV_CTX_QOS_t  gQosCtx;
/* TOS value put into UDP packets sent from this driver */
static IFX_uint8_t  TOS = KPI2UDP_DEFAULT_TOS;
/* Flag for buffer status, trace error only once */
static IFX_uint32_t nUDP2KPI_BufferPoolFailureCnt = 0;
/* Statistics */
static IFX_uint32_t nTxErrorCnt = 0;

/* ============================= */
/* Global variable definition    */
/* ============================= */
/** what string support, driver version string */
const IFX_char_t DRV_KPI2UDP_WHATVERSION[] = DRV_KPI2UDP_WHAT_STR;

/* ============================= */
/* Local function declaration    */
/* ============================= */
static void ifx_kpi2udp_tasklet(unsigned long foo);
DECLARE_TASKLET(tl_kpi_egress, ifx_kpi2udp_tasklet, 0L);
static IFX_boolean_t ifx_kpi2udp_sk_blocking(IFX_void_t);

static IFX_int16_t ifx_kpi2udp_FindFreeChannel(IFX_void_t);
static IFX_int16_t ifx_kpi2udp_FindChannelNo(struct sock* sk);
static int ifx_kpi2udp_toUDP(IFX_TAPI_KPI_CH_t channel, void *data, size_t len);
static int ifx_kpi2udp_fromUDP(struct sock* sk,struct sk_buff *skb);

/* ============================= */
/* Global functions declaration  */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/**
   Tasklet to handle all KPI data and send it on a UDP socket

   \param  foo       currently unused
   \return void
*/
static void ifx_kpi2udp_tasklet(unsigned long foo)
{
   void                     *pData;
   IFX_uint32_t              data_length;
   IFX_int32_t               ret;
   IFX_TAPI_KPI_CH_t         k_ch;
   IFX_uint8_t               k_more;

   /* Important Note / Concept

      As tasklet we must ensure that udp_sendmsg() on the respective
      socket will be non-blocking. If this is not the case Linux 2.6 would
      show a "BUG: scheduling while atomic".

      Different concepts have been considered - the most efficient and
      less kernel invasive is to check the lock of the active sockets
      before taking any data from the KPI egress fifo.

      If one egress fifo's lock is taken, we exit the tasklet and wait
      to be scheduled again. No packet is dropped this way. Note that
      the KPI egress fifo might overflow (which will be visible from
      TAPI statistics).

      Alternative concepts which have been withdraws:
       - use ip_send() instead of udp_sendmsg()
         not available in Linux 2.4 and 2.6 kernel versions
       - use ip_append_data() instead of udp_sendmsg()
         requires to implement the UDP header handling
         not further analyzed
       - implement a non-blocking copy of udp_sendmsg(), udp_sendmsg_nb()
         as locks seem(!) to be taken only in udp_sendmsg() itself and
         lower layer functions seem to rely on the locks taken above,
         it might be feasible to create a copy of udp_sendmsg which would
         use spin locks instead of semaphores to protect the socket.
         Even in this case we would not be sure about any other context
         which might have accuired the normal sk_lock from a process ctx.
         We'd need the same kind of check as implemeted below - but with
         a much higher effort...
         Also it would require an extended kernel patch...
   */

   do
   {
      if (!ifx_kpi2udp_sk_blocking())
      {

         /* read data from the KPI group,
            global irq lock inside while accessing the fifo */
         ret = IFX_TAPI_KPI_ReadData( IFX_TAPI_KPI_UDP, &k_ch,
                                      &pData, &data_length, &k_more );

         /* softirqs might be marked for execution by the interrupt handler
            while another instance is currently executed. As we handle
            all available packets from the fifo, the subsequently scheduled
            tasklet might not find a packet...
            Tests have shown that this is a rare situation in highly loaded
            systems and don't influence the overall system performance.
            Alternative implementations would be more complex and consume
            more cycles for each packet... */
         if (ret >= 0)
         {
            k_ch = KPI_CHANNEL_GET(k_ch);

            /* write the data towards UDP stack - count errors */
            if (ifx_kpi2udp_toUDP(k_ch, pData, data_length) != NO_ERROR)
            {
               nTxErrorCnt++;
            }
            /* release the buffer now that we have sent it,
               global irq lock inside while accessing the bufferpool */
            IFX_TAPI_VoiceBufferPut(pData);
         }
      }
      else
      {
#ifdef KPI_DBG_UDP_SENDMSG
         pr_debug("defer egress handling\n");
#endif /* KPI_DBG_UDP_SENDMSG */
         /* exit tasklet/while loop */
         break;
      }
   } while (k_more);
}


/**
   Check if any active connection might block in the socket...
   \param void
   \return  IFX_TRUE if any sk is blocking, otherwise IFX_FALSE
*/
static IFX_boolean_t ifx_kpi2udp_sk_blocking(IFX_void_t)
{
   IFX_int16_t i;

   for (i=0; i<redtab.channel_num; i++)
   {
      if (redtab.channels[i].in_use == IFX_TRUE)
      {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
         if (redtab.channels[i].sk->sk_lock.owner != 0)
#else
         if (redtab.channels[i].sk->sk_lock.owned)
#endif
            return IFX_TRUE;
      }
   }
   return IFX_FALSE;
}


/**
   Find a free channel and return it's index

   \return
      - index of free channel found
      - -1 if no free channel found
*/
static IFX_int16_t ifx_kpi2udp_FindFreeChannel(IFX_void_t)
{
   IFX_int16_t  i;

   /* Find a free slot in the redirect table. The channel number is identical
      to the slot number. */
   for(i=0; i<redtab.channel_num; i++)
      if(redtab.channels[i].in_use == IFX_FALSE)
         return i;

   return -1;
}


/**
   Find the channel number from the socket given

   \param  sk           Pointer to a "struct sock".

   \return
      - index of channel found
      - -1 if no channel found
*/
static IFX_int16_t ifx_kpi2udp_FindChannelNo(struct sock* sk)
{
   IFX_int16_t  i;

   for (i=0; i < redtab.channel_num; i++)
      if (redtab.channels[i].sk == sk)
         return i;

  return -1;
}


/**
   Function that sends packets on a UDP socket. (KPI to UDP direction)

   \param  channel      Channel number.
   \param  data         Pointer to data to be sent.
   \param  len          Length of the data.

   \return
      - CHANNEL_NO_ERR
*/
static int ifx_kpi2udp_toUDP(IFX_TAPI_KPI_CH_t channel, void *data, size_t len)
{
   int ret = NO_ERROR;
   struct sock *sk = NULL;
   struct msghdr msg;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   struct iovec iov;
#else
   struct kvec iov;
#endif

   if(len == 0)
      return CHANNEL_NO_ERR;
   /* Make sure the KPI channel_no is not out of range. */
   if(channel >= redtab.channel_num)
      return CHANNEL_NO_ERR;

   /* Send rtp with redtab.channel[channel]->sk */
   if(redtab.channels[channel].in_use == IFX_TRUE)
   {
      if(NULL != redtab.channels[channel].srtp_session)
      {
          int ret;
          //pr_err("rtp size: %d\n", len);
          if(0 != (ret = srtp_protect(redtab.channels[channel].srtp_session, data, &len)))
          {
              //pr_err("srtp_protect failed: %d\n", ret);
          }
      }
      sk = redtab.channels[channel].sk;

      memset(&msg,0x00, sizeof(msg));
      memset(&iov,0x00, sizeof(iov));

      iov.iov_base             = (void *)data;
      iov.iov_len              = len;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      msg.msg_iov              = &iov;
      msg.msg_iovlen           = 1;
#endif
      msg.msg_control          = NULL;
      msg.msg_controllen       = 0;

      if (redtab.channels[channel].ext_sock != IFX_TRUE)
      {
         if (redtab.channels[channel].family == AF_INET)
         {
            msg.msg_name             = &redtab.channels[channel].dst_addr;
            msg.msg_namelen          = sizeof(redtab.channels[channel].dst_addr);

            pr_debug("channel%d udp send to: %pIS\n",channel,
                   &redtab.channels[channel].dst_addr);
         }
         else
         {
            msg.msg_name             = &redtab.channels[channel].dst_addr6;
            msg.msg_namelen          = sizeof(redtab.channels[channel].dst_addr6);

            pr_debug("channel%d udp send to: %pIS\n",channel,
                   &redtab.channels[channel].dst_addr6);
         }
      }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      sk->allocation           = GFP_ATOMIC;

      ret = udp_sendmsg(sk, &msg, len);
#else
      sk->sk_allocation        = GFP_ATOMIC;
      msg.msg_flags            = MSG_DONTWAIT | MSG_NOSIGNAL;

      ret = kernel_sendmsg(redtab.channels[channel].sock, &msg, &iov, 1, len);
#endif
      if ((ret < 0) && (ret != redtab.channels[channel].last_sock_ret))
      {
         IFX_TAPI_EVENT_t tapiEvent;

         memset(&tapiEvent, 0, sizeof(tapiEvent));
         tapiEvent.id = IFX_TAPI_EVENT_KPI_SOCKET_FAILURE;
         tapiEvent.data.kpi_socket.error_code = ret;
         tapiEvent.data.kpi_socket.kpi_channel = (IFX_TAPI_KPI_UDP | channel);
         IFX_TAPI_KPI_ReportEvent ((IFX_TAPI_KPI_UDP | channel), &tapiEvent);

         pr_err("KPI->UDP ..._sendmsg() failed"
                " ret=%d\n", ret);
      }
      /* remember the last return code from the ..._sendmsg() */
      redtab.channels[channel].last_sock_ret = ret;
   }
   else
   {
      ret = CHANNEL_NO_ERR;
   }

   return ret;
}


/**
   Callback function that copies packets from UDP to KPI.

   \param  sk           socket
   \param  skb          socket buffer

   \return
      - NO_ERROR
      - WRONG_PKT
      - NO_BUFFER
      - NO_CALLBACK
*/
static int ifx_kpi2udp_fromUDP(struct sock* sk, struct sk_buff *skb)
{
   int ret;
   int offset = sizeof(struct udphdr);
   int n;
   void* buff = NULL;
   int data_length;

   pr_debug("ifx_kpi2udp_fromUDP\n");
   /* dbg("sa:%x port:%x\n", sk->saddr, sk->sport); */

   data_length = skb->len-offset;
   /* __sock_put(sk); */
   if(data_length < 0)
   {
      return WRONG_PKT;
   }

   /* Get a buffer from TAPI for writing data to the KPI */
   buff = IFX_TAPI_VoiceBufferGetWithOwnerId (IFX_TAPI_BUFFER_OWNER_KPI2UDP);
   if (buff == IFX_NULL)
   {
      /* Error retrieving buffer. */
      if (nUDP2KPI_BufferPoolFailureCnt % 5000 == 0)
      {
         pr_err("UDP->KPI getting a data buffer failed, "
                "%d repetitions\n", nUDP2KPI_BufferPoolFailureCnt);
      }
      nUDP2KPI_BufferPoolFailureCnt++;
      return NO_BUFFER;
   }

   /* Copy data into TAPI buffer. */
   memcpy(buff, (void*)(skb->data+offset), data_length);

   if((n=ifx_kpi2udp_FindChannelNo(sk)) > -1)
   {
      if(NULL != redtab.channels[n].srtp_session)
      {
           if(0 != srtp_unprotect(redtab.channels[n].srtp_session, buff, &data_length))
           {
                //pr_err("srtp_unprotect failed\n");
           }
      }
      ret = IFX_TAPI_KPI_WriteData((IFX_TAPI_KPI_UDP | n), buff, data_length);
      if (ret != data_length)
      {
         IFX_TAPI_VoiceBufferPut(buff);
         return NO_BUFFER;
      }
      return NO_ERROR;
   }
   else
   {
      IFX_TAPI_VoiceBufferPut(buff);
      return NO_CALLBACK;
   }
}


/**
   Function that creates a UDP socket and attaches it to the given KPI channel.

   \param  channel      Channel number.
   \param  saddr        Source (local) IP address.
   \param  sport        Source (local) port number.
   \param  daddr        Destination (remote) IP address.
   \param  dport        Destination (remote) port number.

   \return
      - CALL_MK_SESSION_ERR
      - NO_ERROR
*/
static IFX_int32_t ifx_kpi2udp_make_session(IFX_TAPI_KPI_CH_t channel,
                                            IFX_uint16_t family,
                                            IFX_void_t*  saddr,
                                            IFX_uint16_t sport,
                                            IFX_void_t*  daddr,
                                            IFX_uint16_t dport,

                                            IFX_boolean_t do_srtp,
                                            IFX_uint32_t ssrc,
                                            IFX_TAPI_ALGS_ENCR_t eSRTP,
                                            IFX_TAPI_ALGS_AUTH_t eSRTP_Auth,
                                            IFX_uint32_t nSRTP_AuthFieldLength,
                                            const IFX_uint8_t * localKey,
                                            const IFX_uint8_t * localSalt,
                                            const IFX_uint8_t * remoteKey,
                                            const IFX_uint8_t * remoteSalt)
{
   struct socket      *sock      = IFX_NULL;
   struct sock        *sk        = IFX_NULL;
   struct sockaddr_in  lo_addr;
   struct sockaddr_in6 lo_addr6;
   IFXOS_INTSTAT       lock;
   int ret;

   IFX_uint32_t saddrv4;
   IFX_uint32_t daddrv4;
   IFX_uint8_t* saddrv6 = NULL;
   IFX_uint8_t* daddrv6 = NULL;
   
   if (family == AF_INET)
   {
        saddrv4 = *(IFX_uint32_t*)saddr;
        daddrv4 = *(IFX_uint32_t*)daddr;
   }
   else if (family == AF_INET6)
   {
       saddrv6 = (IFX_uint8_t*)saddr;
       daddrv6 = (IFX_uint8_t*)daddr;
   }
   else
   {
     pr_err("Invalid family!\n");
     return CALL_MK_SESSION_ERR;
   }

   /* Make sure the KPI channel_no is not out of range. */
   if(channel >= redtab.channel_num)
      return CHANNEL_NO_ERR;

   sk = redtab.channels[channel].sk;


   if(sk == NULL)
   {
      /* Create socket */
      if (family == AF_INET)
      {
        if(sock_create(PF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock) < 0)
        {
           pr_err("Create socket error!\n");
           return CALL_MK_SESSION_ERR;
        }
      }
      else
      {
        if (sock_create(PF_INET6, SOCK_DGRAM, IPPROTO_UDP, &sock) < 0)
        {
           pr_err("Create socket error!\n");
           return CALL_MK_SESSION_ERR;
        }
      }
      pr_err("Created socket!\n");

      if (family == AF_INET)
      {
        memset((char *)&lo_addr, 0x00, sizeof(struct sockaddr_in));
        lo_addr.sin_family = AF_INET;
        lo_addr.sin_addr.s_addr = saddrv4;
        lo_addr.sin_port = sport;
        /* Bind the sock to sport/saddr */
        if(sock->ops->bind(sock,
                           (struct sockaddr*)&lo_addr,
                           sizeof(lo_addr)    ) < 0)
        {
           pr_err("Bind socket error!\n");
           sock_release(sock);
           return CALL_MK_SESSION_ERR;
        }
      }
      else
      {
        memset(&lo_addr6, 0x00, sizeof(lo_addr6));
        lo_addr6.sin6_family = AF_INET6;
        memcpy(lo_addr6.sin6_addr.s6_addr, saddrv6, sizeof(lo_addr6.sin6_addr.s6_addr));
        lo_addr6.sin6_port = sport;
        /* Bind the sock to sport/saddr */
        if(sock->ops->bind(sock,
                           (struct sockaddr*)&lo_addr6,
                           sizeof(lo_addr6)    ) < 0)
        {
           pr_err("Bind socket error!\n");
           sock_release(sock);
           return CALL_MK_SESSION_ERR;
        }
      }

      sk = sock->sk;
      if(sk == NULL){
         pr_err("Error! sk is NULL!\n");
         sock_release(sock);
         return CALL_MK_SESSION_ERR;
      }
   }
   else
   {
      /* Socket exists. Verify that the source port is the same. */
#if   (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      if (sk->num != htons(sport))
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
      if (((struct inet_sock *)sk)->num != htons(sport))
#else
      if (((struct inet_sock *)sk)->inet_num != htons(sport))
#endif
      {
         return CALL_MK_SESSION_ERR;
      }
      sock = redtab.channels[channel].sock;
   }

   if (do_srtp)
   {
       if (redtab.channels[channel].srtp_session != NULL)
       {
           srtp_dealloc(redtab.channels[channel].srtp_session);
           redtab.channels[channel].srtp_session = NULL;
       }
       if(0 != srtp_create(&redtab.channels[channel].srtp_session, NULL))
       {
            pr_err("SRTP will not be used for call\n");
       }
       else
       {
         srtp_policy_t policy_in;
         srtp_policy_t policy_out;
         memset(&policy_in, 0, sizeof(policy_in));
         memset(&policy_out, 0, sizeof(policy_out));

         memcpy(redtab.channels[channel].localKey, localKey, IFX_TAPI_SRTP_MKI_KEY_MAX);
         append_salt_to_key(redtab.channels[channel].localKey, IFX_TAPI_SRTP_MKI_KEY_MAX, (uint8_t *)localSalt, IFX_TAPI_SRTP_MKI_SALT_MAX);
         memcpy(redtab.channels[channel].remoteKey, remoteKey, IFX_TAPI_SRTP_MKI_KEY_MAX);
         append_salt_to_key(redtab.channels[channel].remoteKey, IFX_TAPI_SRTP_MKI_KEY_MAX, (uint8_t *)remoteSalt, IFX_TAPI_SRTP_MKI_SALT_MAX);


         if (eSRTP == IFX_TAPI_ENCR_AES_CTR)
         {
             if (eSRTP_Auth == IFX_TAPI_AUTH_HMAC_SHA1)
             {
                 if (nSRTP_AuthFieldLength == 10)
                 {
                     crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy_in.rtp);
                     crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy_in.rtcp);
                     crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy_out.rtp);
                     crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy_out.rtcp);
                 }
                 else if (nSRTP_AuthFieldLength == 4)
                 {
                     crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy_in.rtp);
                     crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy_in.rtcp);
                     crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy_out.rtp);
                     crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy_out.rtcp);
                 }
                 else
                 {
                     pr_err("unsupported crypto suite, len1: %d\n", nSRTP_AuthFieldLength);
                     return CALL_MK_SESSION_ERR;
                 }
             }
             else if (eSRTP_Auth == IFX_TAPI_AUTH_NONE)
             {
                 crypto_policy_set_aes_cm_128_null_auth(&policy_in.rtp);
                 crypto_policy_set_aes_cm_128_null_auth(&policy_in.rtcp);
                 crypto_policy_set_aes_cm_128_null_auth(&policy_out.rtp);
                 crypto_policy_set_aes_cm_128_null_auth(&policy_out.rtcp);
             }
             else
             {
                 pr_err("unsupported crypto suite, auth: %d\n", eSRTP_Auth);
                 return CALL_MK_SESSION_ERR;
             }
         }
         else if (eSRTP == IFX_TAPI_ENCR_NONE)
         {
             if (nSRTP_AuthFieldLength == 10)
             {
                 crypto_policy_set_null_cipher_hmac_sha1_80(&policy_in.rtp);
                 crypto_policy_set_null_cipher_hmac_sha1_80(&policy_in.rtcp);
                 crypto_policy_set_null_cipher_hmac_sha1_80(&policy_out.rtp);
                 crypto_policy_set_null_cipher_hmac_sha1_80(&policy_out.rtcp);
             }
             else if (nSRTP_AuthFieldLength == 4)
             {
                 crypto_policy_set_null_cipher_hmac_sha1_32(&policy_in.rtp);
                 crypto_policy_set_null_cipher_hmac_sha1_32(&policy_in.rtcp);
                 crypto_policy_set_null_cipher_hmac_sha1_32(&policy_out.rtp);
                 crypto_policy_set_null_cipher_hmac_sha1_32(&policy_out.rtcp);
             }
             else
             {
                 pr_err("unsupported crypto suite, len2: %d\n", nSRTP_AuthFieldLength);
                 return CALL_MK_SESSION_ERR;
             }

         }
         else
         {
             pr_err("unsupported crypto suite, sncr: %d\n", eSRTP);
             return CALL_MK_SESSION_ERR;
         }
         policy_out.ssrc.type = ssrc_specific;
         policy_out.ssrc.value = ssrc;
         policy_in.ssrc.type = ssrc_any_inbound;

         policy_in.key = redtab.channels[channel].remoteKey;
         policy_out.key = redtab.channels[channel].localKey;
         if (0 != (ret = srtp_add_stream(redtab.channels[channel].srtp_session, &policy_in)))
         {
             pr_err("Error initializing policy incomming stream: %d\n", ret);
             pr_err("session: %p\n", redtab.channels[channel].srtp_session);
             pr_err("key: %p\n", policy_in.key);
         }
         if (0 != (ret = srtp_add_stream(redtab.channels[channel].srtp_session, &policy_out)))
         {
             pr_err("Error initializing policy for outgoing stream: %d\n", ret);
         }
       }
   }
   else
   {
       if (redtab.channels[channel].srtp_session != NULL)
       {
           srtp_dealloc(redtab.channels[channel].srtp_session);
           redtab.channels[channel].srtp_session = NULL;
       }
   }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   /* set the TOS field as a flag for later netfilter and
      further external processing */
   sk->protinfo.af_inet.tos = TOS;
   /* mark the packets with the UDP_REDIRECT _MAGIC (so they will be handled by
      proprietary code in the IP stack) */
   sk->user_data            = UDP_REDIRECT_MAGIC;
   /* increase the processing priority of the socket to the maximum */
   sk->priority             = 6;
#else
   /* set the TOS field as a flag for later netfilter and
      further external processing */
   ((struct inet_sock *)sk)->tos = TOS;
   /* mark the packets with the UDP_REDIRECT _MAGIC (so they will be handled by
      proprietary code in the IP stack) */
   sk->sk_user_data         = UDP_REDIRECT_MAGIC;
   /* increase the processing priority of the socket to the maximum */
   sk->sk_priority          = 6;
#endif

   IFXOS_LOCKINT(lock);
   /* add this session to the redirect table */
   redtab.channels[channel].family = family;
   if (family == AF_INET)
   {
     memset(&redtab.channels[channel].dst_addr, 0x00,
            sizeof(redtab.channels[channel].dst_addr));
     redtab.channels[channel].dst_addr.sin_family      = AF_INET;
     redtab.channels[channel].dst_addr.sin_port        = dport;
     redtab.channels[channel].dst_addr.sin_addr.s_addr = daddrv4;
   }
   else
   {
     if (inet6_sk(sk) != NULL)
       inet6_sk(sk)->tclass = TOS;
     memset(&redtab.channels[channel].dst_addr6, 0x00,
            sizeof(redtab.channels[channel].dst_addr6));
     redtab.channels[channel].dst_addr6.sin6_family      = AF_INET6;
     redtab.channels[channel].dst_addr6.sin6_port        = dport;
     memcpy(redtab.channels[channel].dst_addr6.sin6_addr.s6_addr, daddrv6, sizeof(redtab.channels[channel].dst_addr6.sin6_addr.s6_addr));
   }
   redtab.channels[channel].sk = sk;
   redtab.channels[channel].sock = sock;
   redtab.channels[channel].ext_sock = IFX_FALSE;
   redtab.channels[channel].in_use = IFX_TRUE;
   IFXOS_UNLOCKINT(lock);

   if (family == AF_INET)
   {
     pr_debug("Session established!"
         "ch:%d  da:%pISpc  sa:%pISpc\n",
         channel, &redtab.channels[channel].dst_addr, &lo_addr);
   }
   else
   {
     pr_debug("Session established!"
         "ch:%d  da:%pISpc  sa:%pISpc\n",
         channel, &redtab.channels[channel].dst_addr6, &lo_addr6);
   }

   return NO_ERROR;
}


/**
   Function that associates an UDP socket with an given KPI channel.

   \param  channel      Channel number.
   \param  fd           File descriptor of an open and connected socket.

   \return
      - CALL_MK_SESSION_ERR
      - NO_ERROR
*/
static IFX_int32_t ifx_kpi2udp_make_fd_session(IFX_TAPI_KPI_CH_t channel,
                                               int fd)
{
   struct socket      *sock      = IFX_NULL;
   struct sock        *sk        = IFX_NULL;
   int                 err       = -1;
   IFXOS_INTSTAT       lock;

   /* Make sure the KPI channel_no is not out of range. */
   if(channel >= redtab.channel_num)
      return CHANNEL_NO_ERR;
   /* The file descriptor must be a positive number. */
   if (fd < 0)
   {
      pr_err("fd parameter is negative (%d)\n", fd);
      return CALL_MK_SESSION_ERR;
   }

   sk = redtab.channels[channel].sk;

   if(sk != NULL)
   {
      /* Socket exists. */
      pr_err("KPI channel already has a fd\n");
      return CALL_MK_SESSION_ERR;
   }

   sock = sockfd_lookup(fd, &err);
   if (sock == NULL)
   {
      pr_err("Failed to find sock for fd %d (err=%i)\n", fd, err);
      return CALL_MK_SESSION_ERR;
   }

   sockfd_put(sock);

   /*
   Use this to find the remote udp port set by connect() :
   rtp_sock->ops->getname(rtp_sock, (struct sockaddr *)&uaddr, &sockaddr_len, 1);
   */

   sk = sock->sk;
   if(sk == NULL){
      pr_err("Error! sk is NULL!\n");
      return CALL_MK_SESSION_ERR;
   }

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   /* mark the packets with the UDP_REDIRECT _MAGIC (so they will be handled by
      proprietary code in the IP stack) */
   sk->user_data            = UDP_REDIRECT_MAGIC;
   /* increase the processing priority of the socket to the maximum */
   sk->priority             = 6;
#else
   /* mark the packets with the UDP_REDIRECT _MAGIC (so they will be handled by
      proprietary code in the IP stack) */
   sk->sk_user_data         = UDP_REDIRECT_MAGIC;
   /* increase the processing priority of the socket to the maximum */
   sk->sk_priority          = 6;
#endif

   IFXOS_LOCKINT(lock);
   /* add this session to the redirect table */
   memset(&redtab.channels[channel].dst_addr6, 0x00,
          sizeof(redtab.channels[channel].dst_addr6));
   redtab.channels[channel].sk = sk;
   redtab.channels[channel].sock = sock;
   redtab.channels[channel].ext_sock = IFX_TRUE;
   redtab.channels[channel].in_use = IFX_TRUE;
   IFXOS_UNLOCKINT(lock);

   pr_debug("Session established via external fd! ch:%d\n",
       channel);

   return NO_ERROR;
}


/**
   Function that deletes the one session on the given channel.

   Deleting a session means deactivating it and deleting the socket.

   \param  channel      Channel number.

   \return
      - CHANNEL_NO_ERR
      - NO_ERROR
*/
static IFX_int32_t ifx_kpi2udp_delete_session(IFX_TAPI_KPI_CH_t channel)
{
   IFX_int32_t    ret = NO_ERROR;
   struct sock   *vsk;
   struct socket *vsock;
   IFXOS_INTSTAT  lock;
   IFX_boolean_t  ext_sock;

   /* Make sure the KPI channel_no is not out of range. */
   if(channel >= redtab.channel_num)
      return CHANNEL_NO_ERR;

   /* Free struct socket */
   if(redtab.channels[channel].in_use == IFX_TRUE)
   {
      /* delete the open socket (session) on this channel */
      IFXOS_LOCKINT(lock);
      vsk=redtab.channels[channel].sk;
      vsock=redtab.channels[channel].sock;
      ext_sock = redtab.channels[channel].ext_sock;

      redtab.channels[channel].sk = NULL;
      redtab.channels[channel].sock = NULL;
      /* mark the channel as free */
      redtab.channels[channel].in_use = IFX_FALSE;

      if(NULL != redtab.channels[channel].srtp_session)
      {
          srtp_dealloc(redtab.channels[channel].srtp_session);
          redtab.channels[channel].srtp_session = NULL;
      }

      IFXOS_UNLOCKINT(lock);

#if   (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      if((ext_sock!=IFX_TRUE) && (vsock != NULL) &&
         (vsk != NULL) && (vsk->num > 0))
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
      if((ext_sock!=IFX_TRUE) && (vsock != NULL)&&
         (vsk != NULL) && (((struct inet_sock *)vsk)->num > 0))
#else
      if((ext_sock!=IFX_TRUE) && (vsock != NULL)&&
         (vsk != NULL) && (((struct inet_sock *)vsk)->inet_num > 0))
#endif
      {
         pr_debug("releasing vsock...%p, ops %p\n", vsock, vsock->ops);
         sock_release(vsock);
         /*printk("[KPI2UDP] sock_release!\n");*/
      }
   }
   else
      ret = CHANNEL_NO_ERR;

   pr_debug("Session close\n");
   return ret;
}


/**
   Function that stops the entire qos support. It deactivates and deletes all
   sessions on all channels.

   \return
      - NO_ERROR
*/
static IFX_int32_t ifx_kpi2udp_close_redirect(IFX_void_t)
{
   IFX_int16_t i;

   for(i=0; i<redtab.channel_num; i++)
   {
      ifx_kpi2udp_delete_session(i);
   }

   return NO_ERROR;
}


static int __init ifx_kpi2udp_DriverStart(void)
{
   IFX_uint8_t    i;

   /* Set the number of entries in the table */
   redtab.channel_num = MAX_REDIRECT_CHANNELS;

   /* Initialise the table holding the redirect information. */
   for(i=0; i<redtab.channel_num; i++)
   {
      redtab.channels[i].in_use = IFX_FALSE;
      redtab.channels[i].sk = NULL;
      redtab.channels[i].sock = NULL;
      redtab.channels[i].srtp_session = NULL;
   }

   /* Set the pointer in the kernel packet path that redirect packets */
   udp_do_redirect_fn = ifx_kpi2udp_fromUDP;

   /* Compile version number into the interface struct. */
   gQosCtx.InterfaceVersion = DRV_QOS_INTERFACE_VER_STR;
   /* Identify this driver */
   gQosCtx.drvName  = "kpi2udp";
   /* Register our configuration functions to TAPI. */
   gQosCtx.start    = ifx_kpi2udp_make_session;
   gQosCtx.start_fd = ifx_kpi2udp_make_fd_session;
   gQosCtx.stop     = ifx_kpi2udp_delete_session;
   gQosCtx.clean    = ifx_kpi2udp_close_redirect;
   gQosCtx.getFreeChannel = ifx_kpi2udp_FindFreeChannel;
   gQosCtx.pQosEgressTasklet = &tl_kpi_egress;
   IFX_TAPI_QOS_DrvRegister(&gQosCtx);

    if(0 == (redtab.srtp_supported = srtp_init()))
    {
        pr_alert("SRTP is not available\n");
    }

   pr_info("%s, (c) 2008-2011 Lantiq Deutschland GmbH\n",
          DRV_KPI2UDP_WHATVERSION + 4);

   return 0;
}

static void __exit ifx_kpi2udp_DriverStop(void)
{
   /* Set pointer in the kernel packet path to NULL so that it is
      no longer called. UDP to KPI direction. */
   udp_do_redirect_fn = NULL;

   /* remove KPI egress tasklet */
   tasklet_kill (&tl_kpi_egress);

   /* Registering IFX_NULL does an unregister of this driver. */
   IFX_TAPI_QOS_DrvRegister(IFX_NULL);

   /* Cleanup all data structures and resources allocated by this driver. */
   ifx_kpi2udp_close_redirect();

   pr_info("driver unloaded\n");

   return;
}


module_init(ifx_kpi2udp_DriverStart);
module_exit(ifx_kpi2udp_DriverStop);


MODULE_AUTHOR           ("Lantiq Deutschland GmbH - www.lantiq.com");
MODULE_DESCRIPTION      ("TAPI KPI2UDP driver");
MODULE_LICENSE          ("Dual BSD/GPL");

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
MODULE_PARM      (TOS, "b");
#else
module_param     (TOS, byte, 0);
#endif
MODULE_PARM_DESC (TOS, "Value put in the TOS field of all UDP packets sent");
