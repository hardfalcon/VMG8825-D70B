/*
 * drv_srtp_main.c
 *
 * init and exit driver routines
 *
 * Rudolf Svanda
 * @copyright 2018 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: Zyxel
 */


#include <linux/module.h>	/* Needed by all modules */
#include <linux/init.h>
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include "srtp.h"

EXPORT_SYMBOL(srtp_init);
EXPORT_SYMBOL(srtp_create);
EXPORT_SYMBOL(srtp_add_stream);
EXPORT_SYMBOL(srtp_dealloc);
EXPORT_SYMBOL(srtp_protect);
EXPORT_SYMBOL(srtp_unprotect);
EXPORT_SYMBOL(crypto_policy_set_rtp_default);
EXPORT_SYMBOL(append_salt_to_key);
EXPORT_SYMBOL(srtp_profile_get_master_salt_length);
EXPORT_SYMBOL(srtp_profile_get_master_key_length);
EXPORT_SYMBOL(crypto_policy_set_aes_cm_128_null_auth);
EXPORT_SYMBOL(crypto_policy_set_null_cipher_hmac_sha1_32);
EXPORT_SYMBOL(crypto_policy_set_aes_cm_128_hmac_sha1_32);
EXPORT_SYMBOL(crypto_policy_set_null_cipher_hmac_sha1_80);

static int __init drv_init_module(void)
{
	printk(KERN_INFO "Loaded SRTP library\n");
	srtp_init();
	/* 
	 * A non 0 return means init_module failed; module can't be loaded. 
	 */
	return 0;
}

static void __exit drv_cleanup_module(void)
{
        srtp_shutdown();
	printk(KERN_INFO "Unloaded SRTP library.\n");
}

module_init(drv_init_module);
module_exit(drv_cleanup_module);

MODULE_AUTHOR           ("Rudolf Svanda");
MODULE_DESCRIPTION      ("drv_srtp");
MODULE_LICENSE          ("Dual BSD/GPL");
