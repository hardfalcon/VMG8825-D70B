/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/netdevice.h>
//#include <linux/rculist.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <net/directconnect_dp_dcmode_api.h>
#include <net/directconnect_dp_debug.h>
#include "directconnect_dp_device.h"
#include "directconnect_dp_dcmode_wrapper.h"

#define DC_DP_PROC "dc_dp"
#define DC_DP_PROC_DBG "dbg"
#define DC_DP_PROC_DEV "dev"
#define DC_DP_PROC_DCMODE "dcmode"
#ifdef FEATURE_DCDP_EXTENDER_SUPPORT
#define DC_DP_PROC_FAKEARP "fakearp"
#define DC_DP_PROC_DISCONN_IF "disconn_if"
#endif

/*
 * ####################################
 *             Declaration
 * ####################################
 */

static int proc_read_dc_dp_dbg_seq_open(struct inode *, struct file *);
static ssize_t proc_write_dc_dp_dbg(struct file *, const char __user *, size_t , loff_t *);
#ifdef FEATURE_DCDP_EXTENDER_SUPPORT
static ssize_t proc_write_dc_dp_fakearp(struct file *, const char __user *, size_t , loff_t *);
static ssize_t proc_write_dc_dp_disconn_if(struct file *, const char __user *, size_t , loff_t *);
#endif
static int proc_read_dc_dp_dev_show(struct seq_file *seq, void *v);
static int proc_read_dc_dp_dev_open(struct inode *, struct file *);
#if 0
static ssize_t proc_write_dc_dp_dev(struct file *, const char __user *, size_t , loff_t *);
#endif
static int proc_read_dc_dp_dcmode_show(struct seq_file *seq, void *v);
static int proc_read_dc_dp_dcmode_open(struct inode *, struct file *);

#define DC_DP_F_ENUM_OR_STRING(name,value, short_name) name = value

/*Note: per bit one variable */
#define DC_DP_F_FLAG_LIST  \
    DC_DP_F_ENUM_OR_STRING(DC_DP_F_DEREGISTER, 0x00000001, "De-Register"), \
    DC_DP_F_ENUM_OR_STRING(DC_DP_F_FASTPATH,   0x00000010, "FASTPATH"), \
    DC_DP_F_ENUM_OR_STRING(DC_DP_F_DIRECTPATH,   0x00000020, "LITEPATH"),\
    DC_DP_F_ENUM_OR_STRING(DC_DP_F_SWPATH,      0x00000040, "SWPATH"),\
    DC_DP_F_ENUM_OR_STRING(DC_DP_F_QOS,       0x00001000, "QoS")

/*
 * ####################################
 *           Global Variable
 * ####################################
 */

#undef DC_DP_F_ENUM_OR_STRING
#define DC_DP_F_ENUM_OR_STRING(name,value, short_name) short_name
static int8_t *g_dc_dp_port_type_str[] = {
    DC_DP_F_FLAG_LIST
};
#undef DC_DP_F_ENUM_OR_STRING

#undef DC_DP_F_ENUM_OR_STRING
#define DC_DP_F_ENUM_OR_STRING(name,value, short_name) value
static uint32_t g_dc_dp_port_flag[] = {
    DC_DP_F_FLAG_LIST
};
#undef DC_DP_F_ENUM_OR_STRING

static int8_t *g_dc_dp_port_status_str[] = {
    "PORT_FREE",
    "PORT_RESERVED",
    "PORT_ALLOCATED",
    "PORT_DEV_REGISTERED",
    "PORT_SUBIF_REGISTERED",
    "Invalid"
};

static int8_t *g_dc_dp_dbg_flag_str[] = {
    "dbg",            /*DC_DP_DBG_FLAG_DBG */

    "rx",            /*DC_DP_DBG_FLAG_DUMP_RX */
    "rx_data",        /*DC_DP_DBG_FLAG_DUMP_RX_DATA*/
    "rx_desc",        /*DC_DP_DBG_FLAG_DUMP_RX_DESCRIPTOR */

    "tx",            /*DC_DP_DBG_FLAG_DUMP_TX */
    "tx_data",        /*DC_DP_DBG_FLAG_DUMP_TX_DATA */
    "tx_desc",        /*DC_DP_DBG_FLAG_DUMP_TX_DESCRIPTOR */
#ifdef FEATURE_DCDP_EXTENDER_SUPPORT
    "fake_arp",       /*DC_DP_DBG_FLAG_FAKE_ARP*/
    "disconn_if",     /*DC_DP_DBG_FLAG_DISCONN_IF*/
#endif
    /*the last one*/
    "err"
};

static uint32_t g_dc_dp_dbg_flag_list[] = {
    DC_DP_DBG_FLAG_DBG,

    DC_DP_DBG_FLAG_DUMP_RX,
    DC_DP_DBG_FLAG_DUMP_RX_DATA,
    DC_DP_DBG_FLAG_DUMP_RX_DESCRIPTOR,

    DC_DP_DBG_FLAG_DUMP_TX,
    DC_DP_DBG_FLAG_DUMP_TX_DATA,
    DC_DP_DBG_FLAG_DUMP_TX_DESCRIPTOR,

#ifdef FEATURE_DCDP_EXTENDER_SUPPORT
    DC_DP_DBG_FLAG_FAKE_ARP,
    DC_DP_DBG_DISCONN_IF,
#endif

    /*The last one*/
    DC_DP_DBG_FLAG_ERR
};

static inline int32_t _dc_dp_get_port_type_str_size(void)
{
    return ARRAY_SIZE(g_dc_dp_port_type_str);
}

static inline int32_t _dc_dp_get_dbg_flag_str_size(void)
{
    return ARRAY_SIZE(g_dc_dp_dbg_flag_str);
}

static inline int32_t _dc_dp_get_port_status_str_size(void)
{
    return ARRAY_SIZE(g_dc_dp_port_status_str);
}

static struct proc_dir_entry *g_dc_dp_proc = NULL;

static struct file_operations g_dc_dp_dbg_proc_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dc_dp_dbg_seq_open,
    .read       = seq_read,
    .write      = proc_write_dc_dp_dbg,
    .llseek     = seq_lseek,
    .release    = seq_release,
};

static struct file_operations g_dc_dp_dev_proc_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dc_dp_dev_open,
    .read       = seq_read,
#if 0
    .write      = proc_write_dc_dp_dev,
#endif
    .llseek     = seq_lseek,
    .release    = single_release,
};

static struct file_operations g_dc_dp_dcmode_proc_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dc_dp_dcmode_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

#ifdef FEATURE_DCDP_EXTENDER_SUPPORT
static struct file_operations g_dc_dp_fakearp_proc_fops = {
    .owner      = THIS_MODULE,
    .write      = proc_write_dc_dp_fakearp,
};

static struct file_operations g_dc_dp_disconn_if_proc_fops = {
    .owner      = THIS_MODULE,
    .write      = proc_write_dc_dp_disconn_if,
};
#endif
/*
 * ####################################
 *           Extern Variable
 * ####################################
 */

/*
 * ####################################
 *            Extern Function
 * ####################################
 */

/*
 * ####################################
 *            Local Function
 * ####################################
 */
static void proc_write_dc_dp_dbg_usage(void)
{
	int i;
    pr_info("usage:\n");
	pr_info("    echo <mask> > /proc/%s/%s\n", DC_DP_PROC, DC_DP_PROC_DBG);
	for (i = 0; i < _dc_dp_get_dbg_flag_str_size(); i++)
		pr_info("	%s = 0x%08x\n", g_dc_dp_dbg_flag_str[i], g_dc_dp_dbg_flag_list[i]);
}

#ifdef FEATURE_DCDP_EXTENDER_SUPPORT
static void proc_write_dc_dp_fakearp_usage(void)
{
    pr_info("usage:\n");
    pr_info("    echo help / type=<fake_arp_type> mac=<client_mac> dev=<dev_name>/proc/%s/%s\n", DC_DP_PROC, DC_DP_PROC_FAKEARP);
    pr_info("        type       - fake arp type (1 - type 1, 2 - type 2)\n");
    pr_info("        client mac - mac address to be used as source mac in the fake arp\n");
    pr_info("        dev        - net_device name on which to simulate the fake arp was received on\n");
}

static void proc_write_dc_dp_disconn_if_usage(void)
{
    pr_info("usage:\n");
    pr_info("    echo help / flags=<flags> mac=<client_mac> dev=<dev_name>/proc/%s/%s\n", DC_DP_PROC, DC_DP_PROC_DISCONN_IF);
    pr_info("        flags      - 0/1/2\n");
    pr_info("        client mac - mac address of client to disconnect from acceleration\n");
    pr_info("        dev        - net_device name to which the client is connected\n");
}
#endif

#if 0
static void proc_write_dc_dp_dev_usage(void)
{
    pr_info("usage:\n");
    //pr_info("    echo show dev <idx> > /proc/%s/%s\n", DC_DP_PROC, DC_DP_PROC_DEV);
    pr_info("    echo set dev <idx> umt_period <us> > /proc/%s/%s\n", DC_DP_PROC, DC_DP_PROC_DEV);
    pr_info("    echo set dev <idx> umt <0|1> > /proc/%s/%s\n", DC_DP_PROC, DC_DP_PROC_DEV);
    pr_info("    echo set dev <idx> dma <0|1> > /proc/%s/%s\n", DC_DP_PROC, DC_DP_PROC_DEV);
}
#endif

static int proc_read_dc_dp_dbg(struct seq_file *seq, void *v)
{
    int i;

    seq_printf(seq, "dbg_flag=0x%08x\n", dc_dp_get_dbg_flag());
    seq_printf(seq, "Supported Flags =%d\n",
           _dc_dp_get_dbg_flag_str_size());
    seq_printf(seq, "Enabled Flags(0x%0x):", dc_dp_get_dbg_flag());

    for (i = 0; i < _dc_dp_get_dbg_flag_str_size(); i++)
        if ((dc_dp_get_dbg_flag() & g_dc_dp_dbg_flag_list[i]) == g_dc_dp_dbg_flag_list[i])
            seq_printf(seq, "%s ", g_dc_dp_dbg_flag_str[i]);

    seq_printf(seq, "\nrun echo help > /proc/dc_dp/dbg to get help\n");

    return 0;
}

static int proc_read_dc_dp_dbg_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dc_dp_dbg, NULL);
}

static ssize_t proc_write_dc_dp_dbg(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    int len;
    char str[40];
    unsigned long mask;

    len = min(count, (size_t)(sizeof(str) - 1));
    len -= copy_from_user(str, buf, len);
	str[len] = 0;

	if (!strncmp(str, "help", strlen("help")))
		goto help;

    if (kstrtoul(str, 0, &mask) < 0) {
		pr_err("invalid input (%s)\n", str);
		goto help;
    }

	dc_dp_set_dbg_flag(mask);
    return len;

help:
    proc_write_dc_dp_dbg_usage();
    return count;
}

#ifdef FEATURE_DCDP_EXTENDER_SUPPORT
extern int32_t dc_dp_fake_arp(uint8_t mac[ETH_ALEN], uint32_t flags);
static ssize_t proc_write_dc_dp_fakearp(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    int i, ret, values[ETH_ALEN];
    bool mac_set = false, type_set = false;
    unsigned long type; /* fake arp type */
    char *p, *args, *buffer = vzalloc(count);
    uint8_t mac[ETH_ALEN];

    if (!buffer) {
        DC_DP_ERROR("vzalloc failure\n");
        return -ENOMEM;
    }

	if (copy_from_user(buffer, buf, count)) {
        DC_DP_ERROR("copy_from_user failure\n");
        ret = -EFAULT;
        goto out;
    }

    args = buffer;
    while ((p = strsep(&args, " ")) != NULL) {
        if (!*p) continue;
        if (strncmp(p, "help", 4) == 0)
            goto out;
        if (strncmp(p, "type=", 5) == 0) {
            if (kstrtoul(p+5, 0, &type) < 0) {
                DC_DP_ERROR("invalid type (%s)\n", p);
                goto out;
            }
            if (type > 2 || type < 1) {
                DC_DP_ERROR("invalid input (type=%lu)\n", type);
                goto out;
            }
            type_set = true;
            continue;
        }
        if (strncmp(p, "mac=", 4) == 0) {
            if (ETH_ALEN != sscanf(p+4, "%x:%x:%x:%x:%x:%x",
                           &values[0], &values[1], &values[2],
                           &values[3], &values[4], &values[5])) {
                DC_DP_ERROR("invalid mac address %s\n", p+4);
                goto out;
            }
            /* convert to uint8_t */
            for (i = 0; i < ETH_ALEN; i++)
                mac[i] = (uint8_t) values[i];
            mac_set = true;
            continue;
        }
    }

	if (type_set == false) {
        DC_DP_ERROR("type parameter (type=) not given\n");
        goto out;
    }

    if (mac_set == false) {
        DC_DP_ERROR("mac parameter (mac=) not given\n");
        goto out;
    }

    DC_DP_DEBUG(DC_DP_DBG_FLAG_FAKE_ARP,"triggering fake arp type %lu from client mac %pM\n", type, mac);
    if (dc_dp_fake_arp(mac, type) != DC_DP_SUCCESS)
        DC_DP_ERROR("fake arp trigger failed\n");

    vfree(buffer);
    return count;

out:
    proc_write_dc_dp_fakearp_usage();
    vfree(buffer);
    return ret;
}

extern int32_t dc_dp_disconn_if(struct net_device *netif, struct dp_subif *subif_id, uint8_t mac_addr[MAX_ETH_ALEN], uint32_t flags);
static ssize_t proc_write_dc_dp_disconn_if(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    int i, ret, values[ETH_ALEN];
    bool mac_set = false;
    unsigned long flags = 0;
    char *p, *args, *buffer = vzalloc(count);
    uint8_t mac[ETH_ALEN];
    struct net_device *dev = NULL;

    if (!buffer) {
        DC_DP_ERROR("vzalloc failure\n");
        return -ENOMEM;
    }

    if (copy_from_user(buffer, buf, count)) {
        DC_DP_ERROR("copy_from_user failure\n");
        ret = -EFAULT;
        goto out;
    }

    args = buffer;
    while ((p = strsep(&args, " ")) != NULL) {
        if (!*p) continue;
        if (strncmp(p, "help", 4) == 0)
            goto out;
        if (strncmp(p, "flags=", 6) == 0) {
            if (kstrtoul(p+6, 0, &flags) < 0) {
                DC_DP_ERROR("invalid type (%s)\n", p);
                goto out;
            }
            if (flags > 2) {
                DC_DP_ERROR("invalid input (flags=%lu)\n", flags);
                goto out;
            }
            continue;
        }
        if (strncmp(p, "mac=", 4) == 0) {
            if (ETH_ALEN != sscanf(p+4, "%x:%x:%x:%x:%x:%x",
                           &values[0], &values[1], &values[2],
                           &values[3], &values[4], &values[5])) {
                DC_DP_ERROR("invalid mac address %s\n", p+4);
                goto out;
            }
            /* convert to uint8_t */
            for (i = 0; i < ETH_ALEN; i++)
                mac[i] = (uint8_t) values[i];
            mac_set = true;
            continue;
        }
        if (strncmp(p, "dev=", 4) == 0) {
            dev = dev_get_by_name(&init_net, p+4);
            if (dev == NULL) {
                DC_DP_ERROR("dev_get_by_name(%s) failed\n", p+4);
                goto out;
            }
            continue;
        }
    }

    if (dev == NULL) {
        DC_DP_ERROR("dev parameter (dev=) not given\n");
        goto out;
    }

    DC_DP_DEBUG(DC_DP_DBG_DISCONN_IF,"calling dc_dp_disconn_if(dev=%s, mac=%pM, flags=%lu)\n", dev->name, mac, flags);
    if (dc_dp_disconn_if(dev, NULL, mac_set ? mac : NULL, flags) != DC_DP_SUCCESS)
        DC_DP_ERROR("dc_dp_disconn_if failed\n");

    vfree(buffer);
    return count;

out:
    proc_write_dc_dp_disconn_if_usage();
    vfree(buffer);
    if (dev)
        dev_put(dev);
    return ret;
}
#endif /* #ifdef FEATURE_DCDP_EXTENDER_SUPPORT */

static int proc_read_dc_dp_dev_show(struct seq_file *seq, void *v)
{
    int32_t dev_idx = 0;
    int i;
    struct dc_dp_priv_dev_info *p_hw_dev_info = dc_dp_get_hw_device_head();
    struct dc_dp_priv_dev_info *p_sw_dev_info = dc_dp_get_sw_device_head();
    struct dc_dp_priv_dev_info *dev_info = NULL;

    for (dev_idx = 0; dev_idx < DC_DP_MAX_HW_DEVICE + DC_DP_MAX_SW_DEVICE; dev_idx++) {
        if (dev_idx < DC_DP_MAX_HW_DEVICE)
            dev_info = &p_hw_dev_info[dev_idx];
        else
            dev_info = &p_sw_dev_info[dev_idx - DC_DP_MAX_HW_DEVICE];

        if (dev_info->flags == DC_DP_DEV_FREE) {
            seq_printf(seq, "%d: Not registered\n", dev_idx);
            continue;
        }

        seq_printf(seq,
               "%d: module=0x%08x(name:%8s) dev_port=%02d dp_port=%02d\n",
               dev_idx, (uint32_t)dev_info->owner, dev_info->owner->name,
               dev_info->dev_port, dev_info->port_id);
        seq_printf(seq,  "    status:            %s\n",
            g_dc_dp_port_status_str[dev_info->flags]);

        seq_printf(seq, "    allocate_flags:    ");
        for (i = 0; i < _dc_dp_get_port_type_str_size(); i++) {
            if (dev_info->alloc_flags & g_dc_dp_port_flag[i])
                seq_printf(seq,  "%s ", g_dc_dp_port_type_str[i]);
        }
        seq_printf(seq, "\n");

        seq_printf(seq, "    cb->rx_fn:         0x%0x\n",
            (uint32_t) dev_info->cb.rx_fn);
        seq_printf(seq, "    cb->stop_fn:       0x%0x\n",
            (uint32_t) dev_info->cb.stop_fn);
        seq_printf(seq, "    cb->restart_fn:    0x%0x\n",
            (uint32_t) dev_info->cb.restart_fn);
        seq_printf(seq, "    cb->get_subif_fn:  0x%0x\n",
            (uint32_t) dev_info->cb.get_subif_fn);
        seq_printf(seq, "    cb->get_desc_info_fn:  0x%0x\n",
            (uint32_t) dev_info->cb.get_desc_info_fn);
        seq_printf(seq, "    cb->reset_mib_fn:  0x%0x\n",
            (uint32_t) dev_info->cb.reset_mib_fn);
        seq_printf(seq, "    cb->get_mib_fn:  0x%0x\n",
            (uint32_t) dev_info->cb.reset_mib_fn);
        seq_printf(seq, "    cb->recovery_fn:  0x%0x\n",
            (uint32_t) dev_info->cb.recovery_fn);

        seq_printf(seq, "    class2prio:        ");
        for (i = 0; i < DC_DP_MAX_SOC_CLASS; i++) {
            seq_printf(seq, "[%d->%d],", i, dev_info->class2prio[i]);
        }
        seq_printf(seq, "\n");
        seq_printf(seq, "    prio2devqos:          ");
        for (i = 0; i < DC_DP_MAX_DEV_CLASS; i++) {
            seq_printf(seq, "[%d->%d],", i, dev_info->prio2qos[i]);
        }
        seq_printf(seq, "\n");

        dc_dp_dcmode_wrapper_dump_proc(dev_info, seq);

        seq_printf(seq, "    num_subif:         %02d\n", dev_info->num_subif);
        for (i = 0; i < DC_DP_MAX_SUBIF_PER_DEV; i++) {
            if (dev_info->subif_info[i].flags) {
                seq_printf(seq,
                       "      [%02d]: subif=0x%04x(vap=%d) netif=0x%0x(name=%s), device_name=%s\n",
                       i, dev_info->subif_info[i].subif,
                       (dev_info->subif_info[i].subif >> DC_DP_SUBIFID_OFFSET) & DC_DP_SUBIFID_MASK,
                       (uint32_t)dev_info->subif_info[i].netif,
                       dev_info->subif_info[i].netif ? dev_info->subif_info[i].netif->name : "NULL",
                       dev_info->subif_info[i].device_name);
            }
        }
    }

    return 0;
}

static int proc_read_dc_dp_dev_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dc_dp_dev_show, NULL);
}

#if 0
static __always_inline char *parse_token(char **str, char *delim)
{
    *str = strim(*str);
    return strsep(str, delim);
}

static uint32_t btoi(char *str)
{
    unsigned int sum = 0;
    signed len = 0, i = 0;

    len = strlen(str);
    len = len - 1;
    while (len >= 0) {
        if (*(str + len) == '1')
            sum = (sum | (1 << i));
        i++;
        len--;
    }
    return sum;
}

static int32_t dc_dp_atoi(uint8_t *str)
{
    uint32_t n = 0;
    int32_t i = 0;
    int32_t nega_sign = 0;

    if (!str)
        return 0;
    //dp_replace_ch(str, strlen(str), '.', 0);
    //dp_replace_ch(str, strlen(str), ' ', 0);
    //dp_replace_ch(str, strlen(str), '\r', 0);
    //dp_replace_ch(str, strlen(str), '\n', 0);
    if (str[0] == 0)
        return 0;

    if (str[0] == 'b' || str[0] == 'B') {    /*binary format */
        n = btoi(str + 1);
    } else if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X'))) {
        /*hex format */
        str += 2;

        while (str[i]) {
            n = n * 16;
            if (('0' <= str[i] && str[i] <= '9')) {
                n += str[i] - '0';
            } else if (('A' <= str[i] && str[i] <= 'F')) {
                n += str[i] - 'A' + 10;
                ;
            } else if (('a' <= str[i] && str[i] <= 'f')) {
                n += str[i] - 'a' + 10;
                ;
            } else
                DC_DP_ERROR("Wrong value:%u\n", str[i]);

            i++;
        }

    } else {
        if (str[i] == '-') {    /*negative sign */
            nega_sign = 1;
            i++;
        }
        while (str[i]) {
            n *= 10;
            n += str[i] - '0';
            i++;
        }
    }
    if (nega_sign)
        n = -(int)n;
    return n;
}

static ssize_t proc_write_dc_dp_dev(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    int len;
    char str[40] = {0};
    char *p, *param_name = NULL, *param_val = NULL;
    int32_t dev_idx;
    char *delim = " \t\n\v\f\r";
    int32_t value;

    len = min(count, (size_t)(sizeof(str) - 1));
    len -= copy_from_user(str, buf, len);

    p = str;

    if (!strncmp(p, "set", 3)) {
        /* set ... */
        parse_token(&p, delim); //Skip command 'set'
        if (!p) {
            goto help;
        }
        param_name = parse_token(&p, delim);
        if (!p) {
            goto help;
        }
        if (strncmp(param_name, "dev", 3)) {
            goto help;
        }
        param_val = parse_token(&p, delim);
        if (!p) {
            goto help;
        }
        dev_idx = dc_dp_atoi(param_val);
        if (dev_idx >= DC_DP_MAX_DEV_NUM) {
            goto help;
        }
        param_name = parse_token(&p, delim);
        if (!p) {
            goto help;
        }
        param_val = parse_token(&p, delim);
        value = dc_dp_atoi(param_val);

        if (!strncmp(param_name, "umt_period", strlen("umt_period"))) {
            g_priv_info[dev_idx].umt_period = value;
#if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE
            ltq_umt_set_period(g_priv_info[dev_idx].umt_id, g_priv_info[dev_idx].port_id, value);
#else /* #if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE */
            ltq_umt_set_period(value);
#endif /* #else */

        } else if (!strncmp(param_name, "umt", 3)) {
            if (value == 0) {
#if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE
                ltq_umt_enable(g_priv_info[dev_idx].umt_id, g_priv_info[dev_idx].port_id, 0);
#else /* #if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE */
                ltq_umt_enable(0);
#endif /* #else */

            } else {
#if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE
                ltq_umt_enable(g_priv_info[dev_idx].umt_id, g_priv_info[dev_idx].port_id, 1);
#else /* #if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE */
                ltq_umt_enable(1);
#endif /* #else */
            }

        } else if (!strncmp(param_name, "dma", 3)) {
            if (value == 0) {
                ltq_dma_chan_off(g_priv_info[dev_idx].dma_ch);
            } else {
                ltq_dma_chan_on(g_priv_info[dev_idx].dma_ch);
            }
        } else {
            goto help;
        }
    } else {
        goto help;
    }

    return len;

help:
    proc_write_dc_dp_dev_usage();
    return count;
}
#endif

extern struct dc_dp_dcmode **dc_dp_get_dcmode_head(void);
static int proc_read_dc_dp_dcmode_show(struct seq_file *seq, void *v)
{
    int32_t dcmode_idx;
    struct dc_dp_dcmode **dcmode = dc_dp_get_dcmode_head();

    for (dcmode_idx = 0; dcmode_idx < DC_DP_DCMODE_MAX; dcmode_idx++) {
        if (dcmode[dcmode_idx] == NULL) {
            seq_printf(seq, "%d: Not registered\n", dcmode_idx);
            continue;
        }
        seq_printf(seq, "%d:\n", dcmode_idx);
        seq_printf(seq, "    dcmode_cap:         0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_cap);

        seq_printf(seq, "    register_dev:         0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->register_dev);
        seq_printf(seq, "    register_subif:       0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->register_subif);
        seq_printf(seq, "    xmit:    0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->xmit);
        seq_printf(seq, "    handle_ring_sw:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->handle_ring_sw);
        seq_printf(seq, "    add_session_shortcut_forward:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->add_session_shortcut_forward);
        seq_printf(seq, "    disconn_if:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->disconn_if);
        seq_printf(seq, "    get_netif_stats:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->get_netif_stats);
        seq_printf(seq, "    clear_netif_stats:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->clear_netif_stats);
        seq_printf(seq, "    register_qos_class2prio_cb:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->register_qos_class2prio_cb);
        seq_printf(seq, "    map_class2devqos:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->map_class2devqos);
        seq_printf(seq, "    alloc_skb:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->alloc_skb);
        seq_printf(seq, "    free_skb:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->free_skb);
        seq_printf(seq, "    change_dev_status:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->change_dev_status);
        seq_printf(seq, "    get_wol_cfg:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->get_wol_cfg);
        seq_printf(seq, "    set_wol_cfg:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->set_wol_cfg);
        seq_printf(seq, "    set_wol_ctrl:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->set_wol_ctrl);
        seq_printf(seq, "    get_wol_ctrl_status:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->get_wol_ctrl_status);
        seq_printf(seq, "    dump_proc:  0x%0x\n",
            (uint32_t) dcmode[dcmode_idx]->dcmode_ops->dump_proc);
    }

    return 0;
}

static int proc_read_dc_dp_dcmode_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dc_dp_dcmode_show, NULL);
}

int32_t dc_dp_proc_init(void)
{
    struct proc_dir_entry *entry;

    g_dc_dp_proc = proc_mkdir(DC_DP_PROC, NULL);
    if (!g_dc_dp_proc)
        return -ENOMEM;

    entry = proc_create_data(DC_DP_PROC_DBG, 0, g_dc_dp_proc,
            &g_dc_dp_dbg_proc_fops, NULL);
    if (!entry)
        goto __dbg_proc_err;

    entry = proc_create_data(DC_DP_PROC_DEV, 0, g_dc_dp_proc,
            &g_dc_dp_dev_proc_fops, NULL);
    if (!entry)
        goto __dev_proc_err;

    entry = proc_create_data(DC_DP_PROC_DCMODE, 0, g_dc_dp_proc,
            &g_dc_dp_dcmode_proc_fops, NULL);
    if (!entry)
        goto __dcmode_proc_err;

#ifdef FEATURE_DCDP_EXTENDER_SUPPORT
    entry = proc_create_data(DC_DP_PROC_FAKEARP, 0, g_dc_dp_proc,
            &g_dc_dp_fakearp_proc_fops, NULL);
    if (!entry)
        goto __fakearp_proc_err;

    entry = proc_create_data(DC_DP_PROC_DISCONN_IF, 0, g_dc_dp_proc,
            &g_dc_dp_disconn_if_proc_fops, NULL);
    if (!entry)
        goto __disconn_if_proc_err;
#endif

    return 0;

#ifdef FEATURE_DCDP_EXTENDER_SUPPORT
__disconn_if_proc_err:
    remove_proc_entry(DC_DP_PROC_FAKEARP, g_dc_dp_proc);

__fakearp_proc_err:
    remove_proc_entry(DC_DP_PROC_DCMODE, g_dc_dp_proc);
#endif
__dcmode_proc_err:
    remove_proc_entry(DC_DP_PROC_DEV, g_dc_dp_proc);

__dev_proc_err:
    remove_proc_entry(DC_DP_PROC_DBG, g_dc_dp_proc);

__dbg_proc_err:
    proc_remove(g_dc_dp_proc);
    return -ENOMEM;
}

void dc_dp_proc_exit(void)
{
#ifdef FEATURE_DCDP_EXTENDER_SUPPORT
    remove_proc_entry(DC_DP_PROC_DISCONN_IF, g_dc_dp_proc);
    remove_proc_entry(DC_DP_PROC_FAKEARP, g_dc_dp_proc);
#endif
    remove_proc_entry(DC_DP_PROC_DCMODE, g_dc_dp_proc);
    remove_proc_entry(DC_DP_PROC_DEV, g_dc_dp_proc);
    remove_proc_entry(DC_DP_PROC_DBG, g_dc_dp_proc);
    proc_remove(g_dc_dp_proc);
    g_dc_dp_proc = NULL;
}
