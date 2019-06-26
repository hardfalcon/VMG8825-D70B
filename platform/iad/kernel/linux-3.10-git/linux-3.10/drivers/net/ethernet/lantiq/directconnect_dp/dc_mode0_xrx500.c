/* Includes */
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/interrupt.h>

#include <xway/switch-api/lantiq_gsw_api.h>
#include <lantiq_dmax.h>
#include <net/lantiq_cbm.h>
#include <net/lantiq_cbm_api.h>
#include <linux/ltq_hwmcpy.h>
#if IS_ENABLED(CONFIG_LTQ_PPA_API)
#include <net/ppa_api.h>
#endif /* #if IS_ENABLED(CONFIG_LTQ_PPA_API) */

//#include <net/directconnect_dp_api.h>
#include <net/directconnect_dp_api.h>
#include <net/directconnect_dp_dcmode_api.h>
#include <net/directconnect_dp_debug.h>

/* Defines */
#define DEV2SOC_FRAG_EXCEPTION_HANDLING /* Dev2SoC Fragmentation exception handling */
#define XRX500_DCMODE0_BRIDGE_FLOW_LEARNING    1
#define XRX500_DCMODE0_MIB_WORKAROUND    1
#define XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND 1

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SWITCH_DEV "/dev/switch_api/1"

#define XRX500_DCMODE0_MAX_PORT          16
#define XRX500_DCMODE0_MAX_SUBIF_PER_DEV 16
#define XRX500_DCMODE0_SUBIFID_OFFSET    8
#define XRX500_DCMODE0_SUBIFID_MASK      0xF
#define XRX500_DCMODE0_GET_SUBIFIDX(subif_id) \
    ((subif_id >> XRX500_DCMODE0_SUBIFID_OFFSET) & XRX500_DCMODE0_SUBIFID_MASK)

#define DC_DP_LOCK    spin_lock_bh
#define DC_DP_UNLOCK  spin_unlock_bh

#if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE
    #define XRX500_DCMODE0_MAX_DEV_NUM         4
    #define XRX500_DCMODE0_MAX_DEV_PORT_NUM    6
#else /* #if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE */
    #define XRX500_DCMODE0_MAX_DEV_NUM         1
    #define XRX500_DCMODE0_MAX_DEV_PORT_NUM    2
#endif /* #else */
#define XRX500_DCMODE0_UMT_PERIOD_DEFAULT     200 /* in micro second (GRX350/550) */

#define DC_DP_MAX_SOC_CLASS        16

#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING

#define DMA1TX_DMA_BURST_SIZE       64
#define DMA1TX_SIZE_ALIGNMENT_MASK  0x003F /* DMA1TX DMA size alignemnt size is 64 bytes */
#define OWN(desc_p)                 (desc_p->status.field.own)
#define SOP(desc_p)                 (desc_p->status.field.sop)
#define EOP(desc_p)                 (desc_p->status.field.eop)
#define FRAG_READY(desc_p)          (desc_p->status.field.dic)
#define BYTE_OFFSET(desc_p)         (desc_p->status.field.byte_offset)
#define DATA_LEN(desc_p)            (desc_p->status.field.data_len)
#define DATA_POINTER(desc_p)        (desc_p->data_pointer)

#define CHAN_SOC2DEV_P(ctx) (&((ctx)->shared_info->ch[DCMODE_CHAN_SOC2DEV]))
#define CHAN_SOC2DEV_RET_P(ctx) (&((ctx)->shared_info->ch[DCMODE_CHAN_SOC2DEV_RET]))
#define CHAN_DEV2SOC_P(ctx) (&((ctx)->shared_info->ch[DCMODE_CHAN_DEV2SOC]))
#define CHAN_DEV2SOC_RET_P(ctx) (&((ctx)->shared_info->ch[DCMODE_CHAN_DEV2SOC_RET]))
#define CHAN_DEV2SOC_EX_P(ctx) (&((ctx)->shared_info->ch[DCMODE_CHAN_DEV2SOC_EXCEPT]))

typedef enum dcmode_chan_id {
    DCMODE_CHAN_SOC2DEV = 0,
    DCMODE_CHAN_SOC2DEV_RET,
    DCMODE_CHAN_DEV2SOC,
    DCMODE_CHAN_DEV2SOC_RET,
    DCMODE_CHAN_DEV2SOC_EXCEPT,
    DCMODE_CHAN_MAX,
} dcmode_chan_id_t;

typedef struct __attribute__ ((__packed__)) __attribute__ ((aligned(16)))
{
    uint32_t dev2soc_ring_idx;
} dev2soc_frag_except_bd_t;

typedef struct __attribute__ ((__packed__))
{
    volatile uint32_t wp;
    volatile uint32_t rp;

    uint32_t mask;
    uint32_t size;
    uint32_t quota;
} dcmode_ring_track_t;

typedef struct
{
    void *       phys; /* Ring physical base address */
    void *       virt; /* Ring virtual base address */
    unsigned int size; /* Ring size (=4Bytes) */
    unsigned int desc_dwsz; /* Ring size (=4Bytes) */

    dcmode_ring_track_t trck;  // Ring pointers and metadata tracker
} dcmode_ring_t;

typedef struct
{
    void *       phys; /* Counter physical address */
    void *       virt; /* Counter virtual address */
    unsigned int size; /* Counter size (=4Bytes) */
    unsigned int rw_endian; /* Big or little endian */
    unsigned int rw_mode; /* Cumulative or Incremental mode */
} dcmode_cntr_t;

typedef struct
{
    dcmode_chan_id_t  id  ;  // Channel ID

    /* Ring */
    dcmode_ring_t     ring; /* Ring */

    /* Counter */
    dcmode_cntr_t     write_cntr;
    dcmode_cntr_t     read_cntr;
} dcmode_chan_t;
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */

struct xrx500_dcmode0_dev_shared_info
{
#define XRX500_DCMODE0_DEV_STATUS_FREE    0x0
#define XRX500_DCMODE0_DEV_STATUS_USED    0x1
    int32_t status;
    int32_t ref_count;
    int32_t start_port_id; /* Multiport reference port id */
    int32_t cbm_pid;
    int32_t umt_id;
    int32_t umt_period;
    uint32_t dma_ctrlid;
    uint32_t dma_cid;
    uint32_t dma_ch;
    int32_t num_bufpools;
    struct dc_dp_buf_pool *buflist;
#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
    dcmode_chan_t    ch[DCMODE_CHAN_MAX]; // Array of Rings. Typically 4
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */
    int32_t num_subif;
};

struct xrx500_dcmode0_dev_info
{
#define XRX500_DCMODE0_DEV_STATUS_FREE    0x0
#define XRX500_DCMODE0_DEV_STATUS_USED    0x1
    int32_t status;
    int32_t port_id;
    uint32_t alloc_flags;
    int32_t num_subif;

    /* shared info */
    struct xrx500_dcmode0_dev_shared_info *shared_info;

    /* mib */
    struct dc_dp_dev_mib subif_mib[XRX500_DCMODE0_MAX_SUBIF_PER_DEV];
};
static struct xrx500_dcmode0_dev_shared_info g_dcmode0_dev_shinfo[XRX500_DCMODE0_MAX_DEV_NUM];
static struct xrx500_dcmode0_dev_info g_dcmode0_dev[XRX500_DCMODE0_MAX_DEV_PORT_NUM];
static struct xrx500_dcmode0_dev_info *g_dcmode0_dev_p[XRX500_DCMODE0_MAX_PORT] = {NULL} ;
spinlock_t g_dcmode0_dev_lock;
static int32_t g_dcmode0_init_ok = 0;

/* Function prototypes */
/* Local */
static int32_t
xrx500_dcmode0_rx_cb(struct net_device *rxif, struct net_device *txif,
                struct sk_buff *skb, int32_t len);
static int32_t
xrx500_dcmode0_get_netif_subifid_cb(struct net_device *netif,
                                    struct sk_buff *skb, void *subif_data,
                                    uint8_t dst_mac[MAX_ETH_ALEN],
                                    dp_subif_t *subif, uint32_t flags);

static inline int32_t
xrx500_setup_pmac_port(int32_t port_id, int32_t dma_cid, int32_t start_port_id, uint32_t flags);
static inline int32_t
xrx500_alloc_ring_buffers(uint32_t num_bufs_req, int32_t *num_bufpools, struct dc_dp_buf_pool **buflist);
static inline void
xrx500_free_ring_buffers(int32_t num_bufpools, struct dc_dp_buf_pool **buflist);
static inline int32_t
xrx500_setup_ring_resources(struct xrx500_dcmode0_dev_info *dev_info, int32_t port_id, struct dc_dp_res *res, uint32_t flags);
static inline void
xrx500_cleanup_ring_resources(struct xrx500_dcmode0_dev_info *dev_info, int32_t port_id, struct dc_dp_res *res, uint32_t flags);
static void
xrx500_flush_cbm_queue(int32_t port_id);
static void
xrx500_restore_tmu_queuemap(int32_t port_id);
static inline int32_t
xrx500_setup_umt_port(struct xrx500_dcmode0_dev_info *dev_ctx, int32_t port_id, struct dc_dp_res *res, uint32_t flags);
static inline void
xrx500_cleanup_umt_port(int32_t port_id, uint32_t umt_id);
static inline uint8_t
_dc_dp_get_class2devqos(uint8_t *class2prio, uint8_t *prio2devqos, uint8_t class);

#if 0
#if defined(CONFIG_DIRECTCONNECT_DP_DBG) && CONFIG_DIRECTCONNECT_DP_DBG
static void _dc_dp_dump_raw_data(char *buf, int len, char *prefix_str);
static void _dc_dp_dump_rx_pmac(struct pmac_rx_hdr *pmac);
#endif /* #if defined(CONFIG_DIRECTCONNECT_DP_DBG) && CONFIG_DIRECTCONNECT_DP_DBG */
#endif /* #if 0 */

#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
/* For 2.4.x kernels the readl/writel functions defined in asm/io.h
 * receive address as unsigned long that causes compilation warnings.
 */
#define _MMB_OPS_ADDR(a) ((uint32)(a))
#elif defined (CONFIG_MIPS)
/* For MIPSes all the readx/writex functions are defined
 * in asm/io.h via __BUILD_MEMORY_SINGLE with no const qualifier that causes
 * compilation warnings.
 */
#define _MMB_OPS_ADDR(a) (volatile void __iomem *)(a)
#else
#define _MMB_OPS_ADDR(a) (a)
#endif /* #else */

static inline void
dcmode_raw_writel(uint32_t val, volatile void __iomem *addr)
{
#if 0
    writel(le32_to_cpu(val), _MMB_OPS_ADDR(addr));
#else /* #if 0 */
    writel(val, _MMB_OPS_ADDR(addr));
#endif /* #else */
}

static inline uint32_t
dcmode_raw_readl(const volatile void __iomem *addr)
{
    uint32_t val = readl(_MMB_OPS_ADDR(addr));
#if 0
    return cpu_to_le32(val);
#else /* #if 0 */
    return val;
#endif /* #else */
}

static inline uint32_t
dcmode_read_cntr(dcmode_chan_t *ch)
{
#if 0
    if ( (ch->read_cntr.rw_endian & DC_DP_F_DCCNTR_MODE_BIG_ENDIAN) )
        //return (be32_to_cpu(*(uint32_t *)ch->read_cntr.virt));
        return (be32_to_cpu(*(uint32_t *)ch->read_cntr.phys));
    else
        //return (le32_to_cpu(*(uint32_t *)ch->read_cntr.virt));
        return (le32_to_cpu(*(uint32_t *)ch->read_cntr.phys));
#else /* #if 0 */
    return dcmode_raw_readl(ch->read_cntr.virt);
#endif /* #else */
}

static inline void
dcmode_write_cntr(dcmode_chan_t *ch, uint32_t write_val)
{
#if 0
    if ( (ch->write_cntr.rw_endian & DC_DP_F_DCCNTR_MODE_BIG_ENDIAN) )
        *(uint32_t *)ch->write_cntr.virt = cpu_to_be32(write_val);
    else
        *(uint32_t *)ch->write_cntr.virt = cpu_to_le32(write_val);
#else /* #if 0 */
    return dcmode_raw_writel(write_val, ch->write_cntr.virt);
#endif /* #else */
}
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */

/*
 * ========================================================================
 * Local Interface API
 * ========================================================================
 */

static struct xrx500_dcmode0_dev_shared_info *
xrx500_dcmode0_alloc_dev_shared_ctx(int32_t port_id, int32_t start_port_id, uint32_t alloc_flags)
{
    int32_t dev_shinfo_idx;
    struct xrx500_dcmode0_dev_shared_info *dev_shinfo_ctx = NULL;
    struct xrx500_dcmode0_dev_info *ref_dev_ctx = NULL;

    /* Multiport 2nd device? */
    if ( ((alloc_flags & DC_DP_F_MULTI_PORT) && (alloc_flags && DC_DP_F_SHARED_RES)) &&
         (NULL != (ref_dev_ctx = g_dcmode0_dev_p[start_port_id])) ) {
        dev_shinfo_ctx = ref_dev_ctx->shared_info;
        if (NULL == dev_shinfo_ctx)
            goto err_out;

        dev_shinfo_ctx->ref_count++;
    } else {
        /* Find a free device shinfo index */
        for (dev_shinfo_idx = 0; dev_shinfo_idx < XRX500_DCMODE0_MAX_DEV_NUM; dev_shinfo_idx++) {
            if (g_dcmode0_dev_shinfo[dev_shinfo_idx].status != XRX500_DCMODE0_DEV_STATUS_USED) {
                break;
            }
        }

        if (dev_shinfo_idx >= XRX500_DCMODE0_MAX_DEV_NUM) {
            DC_DP_ERROR("failed to allocate port as it reaches maximum directconnect device limit - %d!!!\n", XRX500_DCMODE0_MAX_DEV_NUM);
            goto err_out;
        }
        dev_shinfo_ctx = &g_dcmode0_dev_shinfo[dev_shinfo_idx];

        memset(dev_shinfo_ctx, 0, sizeof(struct xrx500_dcmode0_dev_shared_info));
        dev_shinfo_ctx->status = XRX500_DCMODE0_DEV_STATUS_USED;
        dev_shinfo_ctx->ref_count++;

        /* Multiport 2nd radio? */
        if ( (alloc_flags & DC_DP_F_MULTI_PORT) && (alloc_flags && DC_DP_F_SHARED_RES) )
            dev_shinfo_ctx->start_port_id = start_port_id;
        else
            dev_shinfo_ctx->start_port_id = port_id;
    }

err_out:
    return dev_shinfo_ctx;
}

static inline void
xrx500_dcmode0_free_dev_shared_ctx(struct xrx500_dcmode0_dev_shared_info *dev_shinfo_ctx)
{
    if (NULL != dev_shinfo_ctx) {
        dev_shinfo_ctx->ref_count--;
        if (0 == dev_shinfo_ctx->ref_count)
            memset(dev_shinfo_ctx, 0, sizeof(struct xrx500_dcmode0_dev_shared_info));
    }
}

static struct xrx500_dcmode0_dev_info *
xrx500_dcmode0_alloc_dev_ctx(int32_t port_id, int32_t start_port_id, uint32_t alloc_flags)
{
    int32_t dev_idx;
    struct xrx500_dcmode0_dev_info *dev_ctx = NULL;
    struct xrx500_dcmode0_dev_shared_info *dev_shinfo_ctx = NULL;

    /* Find a free device index */
    for (dev_idx = 0; dev_idx < XRX500_DCMODE0_MAX_DEV_PORT_NUM; dev_idx++) {
        if (g_dcmode0_dev[dev_idx].status != XRX500_DCMODE0_DEV_STATUS_USED) {
            break;
        }
    }

    if (dev_idx >= XRX500_DCMODE0_MAX_DEV_PORT_NUM) {
        DC_DP_ERROR("failed to allocate port as it reaches maximum directconnect device limit - %d!!!\n", XRX500_DCMODE0_MAX_DEV_PORT_NUM);
        goto out;
    }

    /* Allocate device shared context */
    dev_shinfo_ctx = xrx500_dcmode0_alloc_dev_shared_ctx(port_id, start_port_id, alloc_flags);
    if (NULL == dev_shinfo_ctx)
        goto out;

    dev_ctx = &g_dcmode0_dev[dev_idx];

    /* Reset DC Mode0 device structure */
    memset(dev_ctx, 0, sizeof(struct xrx500_dcmode0_dev_info));
    dev_ctx->status = XRX500_DCMODE0_DEV_STATUS_USED;
    dev_ctx->port_id = port_id;
    dev_ctx->alloc_flags = alloc_flags;
    dev_ctx->shared_info = dev_shinfo_ctx;

    g_dcmode0_dev_p[port_id] = dev_ctx;

out:
    return dev_ctx;
}

static inline void
xrx500_dcmode0_free_dev_ctx(struct xrx500_dcmode0_dev_info *dev_ctx)
{
    if (NULL != dev_ctx) {
        /* Free device shared context */
        xrx500_dcmode0_free_dev_shared_ctx(dev_ctx->shared_info);

        g_dcmode0_dev_p[dev_ctx->port_id] = NULL;
        memset(dev_ctx, 0, sizeof(struct xrx500_dcmode0_dev_info));
    }
}

static inline int32_t
xrx500_setup_pmac_port(int32_t port_id, int32_t dma_cid, int32_t start_port_id, uint32_t flags)
{
    u8 i = 0, j = 0;
    GSW_API_HANDLE gswr;
    GSW_PMAC_Eg_Cfg_t egCfg;
    GSW_PMAC_Ig_Cfg_t igCfg;
    GSW_register_t regCfg;

    memset((void *)&egCfg, 0x00, sizeof(egCfg));
    memset((void *)&igCfg, 0x00, sizeof(igCfg));

    /* Do the GSW-R configuration */
    gswr = gsw_api_kopen(SWITCH_DEV);
    if (gswr == 0) {
        DC_DP_ERROR("Open SWAPI device FAILED!!!\n");
        return -EIO;
    }

    /* FIXME : setup FCS/PMAC setting based on device request */

    /* GSWIP-R PMAC Egress Configuration Table */
    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "PMAC_EG_CFG_SET for GSW-R.\n");
    for (i = 0; i <= 15; i++) {
        for (j = 0; j <= 3; j++) {
            egCfg.nRxDmaChanId  = 0;
            egCfg.bPmacEna      = 0;
            egCfg.bFcsEna       = 0;
            egCfg.bRemL2Hdr     = 0;
            egCfg.numBytesRem   = 0;
            egCfg.nResDW1       = 0;
            egCfg.nRes1DW0      = 0;
            egCfg.nRes2DW0      = 0;
            egCfg.nDestPortId   = port_id;
            egCfg.nTrafficClass = i;
            egCfg.bMpe1Flag     = 0;
            egCfg.bMpe2Flag     = 0;
            egCfg.bEncFlag      = 0;
            egCfg.bDecFlag      = 0;
            egCfg.nFlowIDMsb    = j;
            egCfg.bTCEnable        = 1;

            gsw_api_kioctl(gswr, GSW_PMAC_EG_CFG_SET, (unsigned int)&egCfg);
        }
    }

    /* GSWIP-R PMAC Ingress Configuration Table */
    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "PMAC_IG_CFG_SET for GSW-R.\n");
    igCfg.nTxDmaChanId  = dma_cid;
    igCfg.bPmacPresent  = 0;
    igCfg.bSpIdDefault  = 1;
    igCfg.bSubIdDefault = 0;
    igCfg.bClassDefault = 0;
    igCfg.bClassEna     = 0;
    igCfg.bErrPktsDisc  = 1;

    igCfg.bPmapDefault  = 1;
    igCfg.bPmapEna      = 1;

    igCfg.defPmacHdr[0] = 0;
    igCfg.defPmacHdr[1] = 0;
    igCfg.defPmacHdr[2] = (start_port_id << 4);
    igCfg.defPmacHdr[3] = 0x80;
    igCfg.defPmacHdr[4] = 0;
    igCfg.defPmacHdr[5] = 0;
    igCfg.defPmacHdr[6] = 0xFF;
    igCfg.defPmacHdr[7] = 0xFF;

    gsw_api_kioctl(gswr, GSW_PMAC_IG_CFG_SET, (unsigned int)&igCfg);

    /* FIMXE : setup based on DSL or not */
        /* Allow traffic from one VAP to any VAP */

        /* PCE_PCTRL_3 */
        memset((void *)&regCfg, 0x00, sizeof(regCfg));
        regCfg.nRegAddr = 0x483 + (10 * port_id);
        gsw_api_kioctl(gswr, GSW_REGISTER_GET, (unsigned int)&regCfg);
        regCfg.nData |= 0x4000;
        gsw_api_kioctl(gswr, GSW_REGISTER_SET, (unsigned int)&regCfg);

        /* PCE_IGPTRM */
        memset((void *)&regCfg, 0x00, sizeof(regCfg));
        regCfg.nRegAddr = 0x544 + (16 * port_id);
        gsw_api_kioctl(gswr, GSW_REGISTER_GET, (unsigned int)&regCfg);
        regCfg.nData |= 0xFFFF;
        gsw_api_kioctl(gswr, GSW_REGISTER_SET, (unsigned int)&regCfg);

    gsw_api_kclose(gswr);

    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "GSW PMAC Init Done.\n");
    return 0;
}

static inline int32_t
xrx500_alloc_ring_buffers(uint32_t num_bufs_req, int32_t *num_bufpools, struct dc_dp_buf_pool **buflist)
{
    int32_t i;
    uint32_t order;
    uint32_t max_buf_pool_sz;
    uint32_t max_buf_pool_num;
    uint32_t max_bufs_req_sz;
    struct dc_dp_buf_pool *buflist_base = NULL;
    size_t buflist_sz;
    size_t num_buflist_entries;
    uint32_t num_buf_req_rem;
    uint32_t tmp_num_bufs_req;
    uint32_t tmp_buf_pool_sz;
    uint8_t *buf_addr_base = NULL;

    if (num_bufs_req <= 0) {
        return -1;
    }

    max_buf_pool_sz = 0x400000; //4MB : (2 ^ (MAX_ORDER - 1)) * PAGE_SIZE;
    max_buf_pool_num = 2048; //(max_buf_pool_sz / DC_DP_PKT_BUF_SIZE_DEFAULT);
    max_bufs_req_sz = num_bufs_req * DC_DP_PKT_BUF_SIZE_DEFAULT;

    num_buflist_entries = (num_bufs_req + (max_buf_pool_num - 1)) / max_buf_pool_num;

    buflist_sz = (num_buflist_entries * sizeof(struct dc_dp_buf_pool)); /* virt buflist size */

    /* Allocate Tx buffers */
    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Allocating %d DMA1-Tx buffer lists.\n", num_buflist_entries);
    buflist_base = (struct dc_dp_buf_pool *) kmalloc(buflist_sz, GFP_KERNEL);
    if (!buflist_base) {
        DC_DP_ERROR("failed to allocate %d buffer lists!!!\n", num_buflist_entries);
        return -ENOMEM;
    }
    memset((void *)buflist_base, 0, buflist_sz);

    num_buf_req_rem = num_bufs_req;
    for (i = 0; i < num_buflist_entries; i++) {
        tmp_num_bufs_req = MIN(num_buf_req_rem, max_buf_pool_num);
        tmp_buf_pool_sz = tmp_num_bufs_req * DC_DP_PKT_BUF_SIZE_DEFAULT;
        order = get_order(tmp_buf_pool_sz);

        DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "allocating pool %d of size %d KB.\n",
                    (i + 1), (tmp_buf_pool_sz >> 10));
        buf_addr_base = (uint8_t *)__get_free_pages(GFP_KERNEL, order);
        if (!buf_addr_base) {
            DC_DP_ERROR("failed to allocate pool %d of size %d bytes!!!\n",
                            (i + 1), tmp_buf_pool_sz);
            goto err_out_free_buf;
        }

        /* Buffer pool */
        buflist_base[i].pool = (void *)buf_addr_base;
        buflist_base[i].phys_pool = (void *)virt_to_phys(buf_addr_base);
        buflist_base[i].size = tmp_buf_pool_sz;

        num_buf_req_rem -= tmp_num_bufs_req;
    }

    /* Return */
    *num_bufpools = num_buflist_entries;
    *buflist = buflist_base;

    return 0;

err_out_free_buf:
    xrx500_free_ring_buffers(num_buflist_entries, &buflist_base);

    return -ENOMEM;
}

static inline void
xrx500_free_ring_buffers(int32_t num_bufpools, struct dc_dp_buf_pool **buflist_base)
{
    int32_t i;
    uint32_t order;
    struct dc_dp_buf_pool *buflist;

    if (NULL == buflist_base)
        return;

    buflist = *buflist_base;

    /* De-allocate Tx buffer pool */
    if (buflist) {
        for (i = 0; i < num_bufpools; i++) {
            if (buflist[i].pool && buflist[i].size) {
                DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "De-allocating pool %d (0x%p) of size %d bytes.\n",
                            (i + 1), buflist[i].pool, buflist[i].size);
                order = get_order(buflist[i].size);
                free_pages((unsigned long)buflist[i].pool, order);
                buflist[i].pool = NULL;
                buflist[i].size = 0;
            }
        }

        DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "De-allocating %d buffer lists.\n", num_bufpools);
        kfree(buflist);
        *buflist_base = NULL;
    }
}

#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
static int32_t
xrx500_prepare_channel(dcmode_chan_t *ch, dcmode_chan_id_t ch_id, struct dc_dp_res *res)
{
    ch->id = ch_id;

    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Preparing channel %d.\n", ch_id);
    if (DCMODE_CHAN_DEV2SOC == ch_id) {
        /* Dev2SoC ring */
        ch->ring.phys = res->rings.dev2soc.phys_base;
        ch->ring.virt = res->rings.dev2soc.base;
        ch->ring.size = res->rings.dev2soc.size;
        ch->ring.desc_dwsz = res->rings.dev2soc.desc_dwsz;
        ch->ring.trck.rp = 0;
        ch->ring.trck.wp = 0;
        ch->ring.trck.size = res->rings.dev2soc.size;
        ch->ring.trck.mask = (ch->ring.trck.size - 1); /* Ring size is multiple of 2??? */

        res->rings.dev2soc.ring = (void *)ch;

        /* No counters required */

    } else if (DCMODE_CHAN_DEV2SOC_EXCEPT == ch_id) {
        /* Dev2SoC_Except ring */
        res->rings.dev2soc_except.desc_dwsz = 1;

        ch->ring.phys = res->rings.dev2soc_except.phys_base;
        ch->ring.virt = res->rings.dev2soc_except.base;
        ch->ring.size = res->rings.dev2soc_except.size;
        ch->ring.desc_dwsz = res->rings.dev2soc_except.desc_dwsz;
        ch->ring.trck.rp = 0;
        ch->ring.trck.wp = 0;
        ch->ring.trck.size = res->rings.dev2soc_except.size;
        ch->ring.trck.mask = (ch->ring.trck.size - 1); /* Ring size is multiple of 2??? */
        ch->ring.trck.quota = 32;

        /* Initialize ring */
        memset(ch->ring.virt, 0, (res->rings.dev2soc_except.size * sizeof(dev2soc_frag_except_bd_t)));
        res->rings.dev2soc_except.ring = (void *)ch;

        /* Dev2SoC_Except counters */
        if (res->dccntr[0].dev2soc_except_deq_base) {
            ch->write_cntr.phys = res->dccntr[0].dev2soc_except_deq_phys_base;
            ch->write_cntr.virt = res->dccntr[0].dev2soc_except_deq_base;
            ch->write_cntr.size = res->dccntr[0].dev2soc_except_deq_dccntr_len;
            if ( (res->dccntr[0].soc_write_dccntr_mode & DC_DP_F_DCCNTR_MODE_BIG_ENDIAN) )
                ch->write_cntr.rw_endian = DC_DP_F_DCCNTR_MODE_BIG_ENDIAN;
            else
                ch->write_cntr.rw_endian = DC_DP_F_DCCNTR_MODE_LITTLE_ENDIAN;

            if ( (res->dccntr[0].soc_write_dccntr_mode & DC_DP_F_DCCNTR_MODE_INCREMENTAL) )
                ch->write_cntr.rw_mode = DC_DP_F_DCCNTR_MODE_INCREMENTAL;
            else
                ch->write_cntr.rw_mode = DC_DP_F_DCCNTR_MODE_CUMULATIVE;
        }

        if (res->dccntr[0].dev2soc_except_enq_base) {
            ch->read_cntr.phys = res->dccntr[0].dev2soc_except_enq_phys_base;
            ch->read_cntr.virt = res->dccntr[0].dev2soc_except_enq_base;
            ch->read_cntr.size = res->dccntr[0].dev2soc_except_enq_dccntr_len;
            ch->read_cntr.rw_endian = ch->write_cntr.rw_endian;
            ch->read_cntr.rw_mode = ch->write_cntr.rw_mode;

            /* Initialize counters */
            if (0 != dcmode_read_cntr(ch))
                dcmode_write_cntr(ch, dcmode_read_cntr(ch));
        }
    }
    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Prepared channel %d.\n", ch_id);

    return 0;
}

static void
xrx500_cleanup_channel(dcmode_chan_t *ch, dcmode_chan_id_t ch_id, struct dc_dp_res *res)
{
    memset(ch, 0, sizeof(dcmode_chan_t));
}
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */

static inline int32_t
xrx500_setup_ring_resources(struct xrx500_dcmode0_dev_info *dev_ctx, int32_t port_id, struct dc_dp_res *res, uint32_t flags)
{
    int32_t ret;
    int32_t num_bufpools = 0;
    struct dc_dp_buf_pool *buflist = NULL;
    int32_t i;
    int32_t j;
    int32_t buflist_idx;
    cbm_dq_port_res_t cbm_res = {0};

    /* Allocate DMA1-TX DMA channel */
    ret = ltq_request_dma(dev_ctx->shared_info->dma_ch, "dma1tx dc");
    if (ret) {
        DC_DP_ERROR("failed to allocate DMA1-TX DMA channel 0x%x!!!\n", dev_ctx->shared_info->dma_ch);
        goto err_out;
    }

    /* Set DMA1-TX DMA channel descriptors */
    ret = ltq_dma_chan_desc_alloc(dev_ctx->shared_info->dma_ch, res->rings.dev2soc.size);
    if (ret) {
        DC_DP_ERROR("failed to allocate %d descriptors for DMA1-TX DMA channel 0x%x!!!\n",
                        res->rings.dev2soc.size, dev_ctx->shared_info->dma_ch);
        goto err_out_free_dma;
    }

    /* Update returned 'resource' structure */
    res->rings.dev2soc.phys_base = (void *)ltq_dma_chan_get_desc_phys_base(dev_ctx->shared_info->dma_ch);
    res->rings.dev2soc.base = (void *)CKSEG1ADDR((uint32_t)res->rings.dev2soc.phys_base);
    /* res->rings.dev2soc.size unchanged */
    res->rings.dev2soc.desc_dwsz = 4;

    /* dev2soc_ret same as dev2soc */
    res->rings.dev2soc_ret.phys_base = res->rings.dev2soc.phys_base;
    res->rings.dev2soc_ret.base = res->rings.dev2soc.base;
    res->rings.dev2soc_ret.size = res->rings.dev2soc.size;
    res->rings.dev2soc_ret.desc_dwsz = res->rings.dev2soc.desc_dwsz;

    /* CBM Resources */
    ret = cbm_dequeue_port_resources_get(port_id, &cbm_res, 0);
    if (ret != CBM_OK) {
        DC_DP_ERROR("failed to get CBM dequeue port resources for the port %d!!!", port_id);
        goto err_out_free_dma;
    }

    res->rings.soc2dev_ret.base = (void *)cbm_res.cbm_buf_free_base;
    res->rings.soc2dev_ret.phys_base = (void *)RPHYSADDR((uint32_t)cbm_res.cbm_buf_free_base);
    res->rings.soc2dev_ret.size = 32; // FIXME : cbm_res.num_free_entries;
    res->rings.soc2dev_ret.desc_dwsz = 4;
    if (cbm_res.num_deq_ports) {
        res->rings.soc2dev.base = (void *)cbm_res.deq_info[0].cbm_dq_port_base;
        res->rings.soc2dev.phys_base = (void *)RPHYSADDR((uint32_t)cbm_res.deq_info[0].cbm_dq_port_base);
        res->rings.soc2dev.size = cbm_res.deq_info[0].num_desc;
        res->rings.soc2dev.desc_dwsz = 4;

        kfree(cbm_res.deq_info);
    }

#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
    /* Allocate dev2soc exception ring for fragmentation handling */
    if ((res->rings.dev2soc_except.size > 0) && (NULL == res->rings.dev2soc_except.base)) {
        /* FIXME : allocate it from DDR, if it is not provided by peripheral */
    }
    res->rings.dev2soc_except.desc_dwsz = 1;
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */

    /* FIMXE : Tx buffer allocation */

    /* Allocate Rx buffers */
    ret = xrx500_alloc_ring_buffers(res->num_bufs_req, &num_bufpools, &buflist);
    if (ret) {
        DC_DP_ERROR("failed to register dev as tx buffer allocation failure!!!\n");
        goto err_out;
    }
    dev_ctx->shared_info->num_bufpools = num_bufpools;
    dev_ctx->shared_info->buflist = buflist;

    res->num_bufpools = res->num_bufs_req;
    res->buflist = (struct dc_dp_buf_pool *) kmalloc((res->num_bufpools * sizeof(struct dc_dp_buf_pool)), GFP_KERNEL);
    if (!res->buflist) {
        DC_DP_ERROR("failed to allocate %d buffer lists!!!\n", res->num_bufs_req);
        ret = DC_DP_FAILURE;
        goto err_out_free_buf;
    }

    buflist_idx = 0;
    for (i = 0; i < num_bufpools; i++) {
        for (j = 0; j < buflist[i].size; j += DC_DP_PKT_BUF_SIZE_DEFAULT) {
            res->buflist[buflist_idx].pool = buflist[i].pool + j;
            res->buflist[buflist_idx].phys_pool = buflist[i].phys_pool + j;
            res->buflist[buflist_idx].size = MIN(buflist[i].size, DC_DP_PKT_BUF_SIZE_DEFAULT);
            buflist_idx++;
        }
    }

#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
    /* Dev2SoC ring and counter */
    xrx500_prepare_channel(CHAN_DEV2SOC_P(dev_ctx), DCMODE_CHAN_DEV2SOC, res);
    /* Dev2SoC Exception ring and counter */
    xrx500_prepare_channel(CHAN_DEV2SOC_EX_P(dev_ctx), DCMODE_CHAN_DEV2SOC_EXCEPT, res);
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */

    return ret;

err_out_free_buf:
    xrx500_free_ring_buffers(num_bufpools, &buflist);

err_out_free_dma:
    ltq_free_dma(dev_ctx->shared_info->dma_ch);

err_out:
    return ret;
}

static inline void
xrx500_cleanup_ring_resources(struct xrx500_dcmode0_dev_info *dev_ctx, int32_t port_id, struct dc_dp_res *res, uint32_t flags)
{
#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
    /* If allocated, Free dev2soc exception ring for fragmentation handling */
    if ((res->rings.dev2soc_except.size > 0) && (NULL == res->rings.dev2soc_except.base)) {
        /* FIXME */
    }
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */

    /* Reset the DMA1-TX DMA channel */
    ltq_dma_chan_reset(dev_ctx->shared_info->dma_ch);

    /* De-allocate Tx buffer pool */
    xrx500_free_ring_buffers(dev_ctx->shared_info->num_bufpools, &dev_ctx->shared_info->buflist);

    /* Free the DMA1-TX DMA channel descriptors */
    if (ltq_dma_chan_desc_free(dev_ctx->shared_info->dma_ch)) {
        DC_DP_ERROR("failed to free descriptors for DMA1-TX DMA channel 0x%x!!!\n", dev_ctx->shared_info->dma_ch);
    }

    /* Free DMA1-TX DMA channel */
    if (ltq_free_dma(dev_ctx->shared_info->dma_ch)) {
        DC_DP_ERROR("failed to free DMA1-TX DMA channel 0x%x!!!\n", dev_ctx->shared_info->dma_ch);
    }

#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
    /* Dev2SoC ring and counter */
    xrx500_cleanup_channel(CHAN_DEV2SOC_P(dev_ctx), DCMODE_CHAN_DEV2SOC, res);
    /* Dev2SoC Exception ring and counter */
    xrx500_cleanup_channel(CHAN_DEV2SOC_EX_P(dev_ctx), DCMODE_CHAN_DEV2SOC_EXCEPT, res);
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */
}

static void
xrx500_flush_cbm_queue(int32_t port_id)
{
    int32_t ret;
    uint32_t num_tmu_ports = 0;
    cbm_tmu_res_t *cbm_tmu_res = NULL;
    uint32_t tmu_port = 0;
    int32_t cbm_port_id = -1;

    ret = cbm_dp_port_resources_get((uint32_t *)&port_id, &num_tmu_ports, &cbm_tmu_res, 0);
    if (ret < 0)
        return;

    if (NULL == cbm_tmu_res)
        return;

    tmu_port = cbm_tmu_res[0].tmu_port;
    cbm_port_id = cbm_tmu_res[0].cbm_deq_port;
    kfree(cbm_tmu_res);

    /* Reset the Dequeue port current desc index and buffer return desc index */
    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Resetting CBM port - port_id=%d, cbm_port_id=%d.\n", port_id, cbm_port_id);
    cbm_port_quick_reset(cbm_port_id, CBM_PORT_F_DEQUEUE_PORT);

    /* Flush the CBM/TMU port */
    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Flushing the TMU queues - port_id=%d, tmu_port=%d.\n", port_id, tmu_port);
    cbm_dp_q_enable(port_id, -1, tmu_port, -1 , 200000, 0, (CBM_Q_F_DISABLE | CBM_Q_F_FLUSH | CBM_Q_F_FORCE_FLUSH));

    /* Reset the Dequeue port current desc index and buffer return desc index */
    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Resetting CBM port - port_id=%d, cbm_port_id=%d.\n", port_id, cbm_port_id);
    cbm_port_quick_reset(cbm_port_id, CBM_PORT_F_DEQUEUE_PORT);
}

static void
xrx500_restore_tmu_queuemap(int32_t port_id)
{
    int32_t ret;
    uint32_t num_tmu_ports = 0;
    cbm_tmu_res_t *cbm_tmu_res = NULL;
    uint32_t tmu_port = 0;
    int32_t cbm_port_id = -1;

    ret = cbm_dp_port_resources_get((uint32_t *)&port_id, &num_tmu_ports, &cbm_tmu_res, 0);
    if (ret < 0)
        return;

    if (NULL == cbm_tmu_res)
        return;

    tmu_port = cbm_tmu_res[0].tmu_port;
    cbm_port_id = cbm_tmu_res[0].cbm_deq_port;
    kfree(cbm_tmu_res);

    /* Restore the CBM/TMU port */
    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Restoring Qmap - port_id=%d, tmu_port=%d.\n", port_id, tmu_port);
    cbm_dp_q_enable(port_id, -1, tmu_port, -1 , 200000, 0, CBM_Q_F_RESTORE_ONLY);
}

#if 0
#if defined(CONFIG_DIRECTCONNECT_DP_DBG) && CONFIG_DIRECTCONNECT_DP_DBG
static void
_dc_dp_dump_rx_pmac(struct pmac_rx_hdr *pmac)
{
    int i;
    unsigned char *p = (char *)pmac;

    if (!pmac) {
        pr_err("dump_rx_pmac pmac NULL ??\n");
        return ;
    }

    pr_info("PMAC at 0x%p: ", p);
    for (i = 0; i < 8; i++)
        pr_info("0x%02x ", p[i]);
    pr_info("\n");

    /*byte 0 */
    pr_info("  byte 0:res=%d ver_done=%d ip_offset=%d\n", pmac->res1,
           pmac->ver_done, pmac->ip_offset);
    /*byte 1 */
    pr_info("  byte 1:tcp_h_offset=%d tcp_type=%d\n", pmac->tcp_h_offset,
           pmac->tcp_type);
    /*byte 2 */
    pr_info("  byte 2:ppid=%d class=%d\n", pmac->sppid, pmac->class);
    /*byte 3 */
    pr_info("  byte 3:res=%d pkt_type=%d\n", pmac->res2, pmac->pkt_type);
    /*byte 4 */
    pr_info("  byte 4:res=%d redirect=%d res2=%d src_sub_inf_id=%d\n",
           pmac->res3, pmac->redirect, pmac->res4, pmac->src_sub_inf_id);
    /*byte 5 */
    pr_info("  byte 5:src_sub_inf_id2=%d\n", pmac->src_sub_inf_id2);
    /*byte 6 */
    pr_info("  byte 6:port_map=%d\n", pmac->port_map);
    /*byte 7 */
    pr_info("  byte 7:port_map2=%d\n", pmac->port_map2);
}
#endif /* #if defined(CONFIG_DIRECTCONNECT_DP_DBG) && CONFIG_DIRECTCONNECT_DP_DBG */
#endif /* #if 0 */

#define DC_DP_DEV_CLASS_MASK    0x7
static inline uint8_t
_dc_dp_get_class2devqos(uint8_t class2prio[], uint8_t prio2devqos[], uint8_t class)
{
    uint8_t devqos;
    uint8_t prio;

    class = (class & 0x0F);
    prio = class2prio[class];

    prio = (prio & DC_DP_DEV_CLASS_MASK);
    devqos = prio2devqos[prio];

    return devqos;
}

static inline int32_t
xrx500_setup_umt_port(struct xrx500_dcmode0_dev_info *dev_ctx, int32_t port_id, struct dc_dp_res *res, uint32_t flags)
{
    int32_t ret;
    uint32_t cbm_pid = 0;
    uint32_t dma_ctrlid = 0;
    uint32_t dma_cid = 0;
    uint32_t umt_id = 0;

    /* Get the CBM logical pid */
    ret = cbm_get_wlan_umt_pid(port_id, &cbm_pid);
    if (ret < 0) {
        DC_DP_ERROR("failed to acquire CBM pid for port_id=%d!!!\n", port_id);
        goto err_out;
    }

    /* Allocate UMT port */
#if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE
    ret = ltq_umt_request(port_id, cbm_pid, &dma_ctrlid, &dma_cid, &umt_id);
    if (ret < 0) {
        DC_DP_ERROR("failed to allocate umt port for port_id=%d, cbm_pid=%d!!!\n", port_id, cbm_pid);
        goto err_out;
    }

    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Configuring UMT HW.\n");
    ret = ltq_umt_set_mode(umt_id, port_id, UMT_SELFCNT_MODE, UMT_MSG0_MSG1,
                           (uint32_t)res->dccntr[0].dev2soc_ret_enq_phys_base,
                           XRX500_DCMODE0_UMT_PERIOD_DEFAULT, UMT_DISABLE);
    if (ret) {
        DC_DP_ERROR("failed to configure UMT transfer!!!\n");
        goto err_out_release_umt;
    }
#else /* #if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE */
    *dma_ctrlid = DMA1TX;
    *dma_cid = DMA_CHANNEL_7;
    *umt_id = 0;

    ret = ltq_umt_set_mode(UMT_SELFCNT_MODE, UMT_MSG0_MSG1,
                           (uint32_t)resources->dccntr[0].dev2soc_ret_enq_phys_base,
                           XRX500_DCMODE0_UMT_PERIOD_DEFAULT, UMT_DISABLE);
    if (ret) {
        DC_DP_ERROR("failed to configure UMT transfer!!!\n");
        goto err_out_release_umt;
    }
#endif /* #else */

    /* Initialize */
    dev_ctx->shared_info->cbm_pid = cbm_pid;
    dev_ctx->shared_info->umt_id = umt_id;
    dev_ctx->shared_info->dma_ctrlid = dma_ctrlid;
    dev_ctx->shared_info->dma_cid = dma_cid;
    dev_ctx->shared_info->dma_ch = _DMA_C(dma_ctrlid, DMA1TX_PORT, dma_cid);
    dev_ctx->shared_info->umt_period = XRX500_DCMODE0_UMT_PERIOD_DEFAULT;

    return ret;

err_out_release_umt:
    xrx500_cleanup_umt_port(port_id, umt_id);

err_out:
    return ret;
}

static inline void
xrx500_cleanup_umt_port(int32_t port_id, uint32_t umt_id)
{
    int32_t ret = 0;

#if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE
    /* Delay for ongoing UMT transfer */
    udelay(XRX500_DCMODE0_UMT_PERIOD_DEFAULT * 2);

    /* Release UMT port */
    ret = ltq_umt_release(umt_id, port_id);
    if (ret < 0) {
        DC_DP_ERROR("failed to release umt_id=%d for port_id=%d!!!\n", umt_id, port_id);
        goto err_out;
    }
#endif /* #if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE */

err_out:
    return;
}

#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
static inline int32_t
dcmode_set_ring_start_index(dcmode_chan_t *ch, uint32_t start_idx)
{
    if (start_idx >= ch->ring.trck.size) {
        DC_DP_ERROR("start_idx=%#x >= ch->ring.trck.size=%#x!!!\n", start_idx, ch->ring.trck.size);
        return -1;
    }

    ch->ring.trck.rp = start_idx;
    ch->ring.trck.wp = start_idx;

    return 0;
}

static inline void *
dcmode_get_bdp_from_ring(dcmode_chan_t *ch)
{
    return ((void *)((uint32_t *)ch->ring.virt + (ch->ring.trck.rp * ch->ring.desc_dwsz)));
}

static inline void dcmode_ring_inc_read_index(dcmode_chan_t *ch, uint32_t idx)
{
    ch->ring.trck.rp = ((ch->ring.trck.rp + idx) & ch->ring.trck.mask);
}

static inline void dcmode_ring_inc_write_index(dcmode_chan_t *ch, uint32_t idx)
{
    ch->ring.trck.wp = ((ch->ring.trck.wp + idx) & ch->ring.trck.mask);
}

static int32_t
handle_dev2soc_frag_exception(dcmode_chan_t *ch, uint32_t start_idx)
{
     int32_t ret;
     struct dma_rx_desc *desc_bd_p;
     struct dma_rx_desc *first_desc_bd_p = NULL;
     int32_t desc_byte_offset;
     int32_t desc_data_len;
     int32_t desc_aligned_data_len = 0;
     int32_t desc_unaligned_data_len = 0;
     uint32_t desc_unaligned_data_vpos = 0;
     int32_t desc_total_data_len = 0;
     int32_t desc_total_len = 0;
     int32_t frag_ready = 1;
     uint32_t src_data_pointer = 0;
     uint32_t data_vpointer = 0;
     uint32_t dest_data_pointer = 0;

     ret = dcmode_set_ring_start_index(ch, start_idx);
     if (ret < 0)
         return ret;

     /* LOOP on frag_ready */
     do {
         desc_bd_p = (struct dma_rx_desc *)dcmode_get_bdp_from_ring(ch);

         if (!FRAG_READY(desc_bd_p)) {
             DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "FRAG_READY bit is not set!!! DW0=0x%08x DW1=0x%08x DW2=0x%08x DW3=0x%08x.\n",
                    desc_bd_p->dw0.all, desc_bd_p->dw1.all, desc_bd_p->data_pointer, desc_bd_p->status.all);
             return -1;
         }

         desc_byte_offset = BYTE_OFFSET(desc_bd_p);
         desc_data_len = DATA_LEN(desc_bd_p);
         desc_total_len = (desc_byte_offset + desc_data_len);

         /* Handling for intermediate fragment of a packet */
         if ( (0 == SOP(desc_bd_p)) && (0 == EOP(desc_bd_p)) ) {
             /* Byte_Offset MUST be ZERO and Data_length MUST be multiple of DMA_BURST_SIZE(=64Byte) */

//             DC_DP_DEBUG(DC_DP_DBG_FLAG_DUMP_RX_DESCRIPTOR, "DW0=0x%08x DW1=0x%08x DW2=0x%08x DW3=0x%08x.\n",
//                         desc_bd_p->dw0.all, desc_bd_p->dw1.all, desc_bd_p->data_pointer, desc_bd_p->status.all);

             if ( (0 != desc_byte_offset) || (0 != desc_unaligned_data_len) ) {
                 data_vpointer = (uint32_t)phys_to_virt(DATA_POINTER(desc_bd_p));
                 (void)dma_map_single(NULL, (void *)data_vpointer, (desc_data_len + desc_unaligned_data_len), DMA_FROM_DEVICE);

                 dest_data_pointer = (data_vpointer + desc_unaligned_data_len);
                 src_data_pointer = (data_vpointer + desc_byte_offset);
                 if (dest_data_pointer != src_data_pointer)
                     memmove((void *)dest_data_pointer, (void *)src_data_pointer, DATA_LEN(desc_bd_p));

                 memcpy((void *)(data_vpointer), (void *)(desc_unaligned_data_vpos), desc_unaligned_data_len);

                 desc_total_data_len = (desc_data_len + desc_unaligned_data_len);
                 desc_unaligned_data_len = (desc_total_data_len & DMA1TX_SIZE_ALIGNMENT_MASK);
                 desc_aligned_data_len = (desc_total_data_len - desc_unaligned_data_len);
                 if (desc_aligned_data_len) {
                     desc_unaligned_data_vpos = (data_vpointer + desc_aligned_data_len);
                     DATA_LEN(desc_bd_p) = desc_aligned_data_len;
                 } else {
                     /* FIXME : try to get data from next desc */
                     DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "DATA_LEN<64!!! DW0=0x%08x DW1=0x%08x DW2=0x%08x DW3=0x%08x.\n",
                            desc_bd_p->dw0.all, desc_bd_p->dw1.all, desc_bd_p->data_pointer, desc_bd_p->status.all);
                 }

                 (void)dma_map_single(NULL, (void *)data_vpointer, desc_aligned_data_len, DMA_BIDIRECTIONAL);
                 BYTE_OFFSET(desc_bd_p) = 0;
             }

             FRAG_READY(desc_bd_p) = 0;
             OWN(desc_bd_p) = 1;

//             DC_DP_DEBUG(DC_DP_DBG_FLAG_DUMP_RX_DESCRIPTOR, "DW0=0x%08x DW1=0x%08x DW2=0x%08x DW3=0x%08x.\n",
//                         desc_bd_p->dw0.all, desc_bd_p->dw1.all, desc_bd_p->data_pointer, desc_bd_p->status.all);

         /* Handling for 1st fragment of a packet */
         } else if ( (1 == SOP(desc_bd_p)) && (0 == EOP(desc_bd_p)) ) {
             /* (Byte_Offset+Data_length) MUST be multiple of DMA_BURST_SIZE(=64Bytes) */

//             DC_DP_DEBUG(DC_DP_DBG_FLAG_DUMP_RX_DESCRIPTOR, "DW0=0x%08x DW1=0x%08x DW2=0x%08x DW3=0x%08x.\n",
//                         desc_bd_p->dw0.all, desc_bd_p->dw1.all, desc_bd_p->data_pointer, desc_bd_p->status.all);

             first_desc_bd_p = desc_bd_p; // Save first desc of the packet, to set OWN bit as the last action
             desc_unaligned_data_len = (desc_total_len & DMA1TX_SIZE_ALIGNMENT_MASK);
             if ( 0 != desc_unaligned_data_len ) {
                 desc_aligned_data_len = (desc_data_len - desc_unaligned_data_len);

                 if ( 0 != desc_aligned_data_len ) {
                     data_vpointer = (uint32_t)phys_to_virt(DATA_POINTER(desc_bd_p));
                     desc_unaligned_data_vpos = (data_vpointer + desc_byte_offset + desc_aligned_data_len);
                     (void)dma_map_single(NULL, (void *)desc_unaligned_data_vpos, desc_unaligned_data_len, DMA_FROM_DEVICE);
                     DATA_LEN(desc_bd_p) = desc_aligned_data_len;
                 } else {
                     /* FIXME : try to get data from next desc */
                     DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "DATA_LEN<64!!! DW0=0x%08x DW1=0x%08x DW2=0x%08x DW3=0x%08x.\n",
                            desc_bd_p->dw0.all, desc_bd_p->dw1.all, desc_bd_p->data_pointer, desc_bd_p->status.all);
                 }
             }

             FRAG_READY(desc_bd_p) = 0;

//             DC_DP_DEBUG(DC_DP_DBG_FLAG_DUMP_RX_DESCRIPTOR, "DW0=0x%08x DW1=0x%08x DW2=0x%08x DW3=0x%08x.\n",
//                         desc_bd_p->dw0.all, desc_bd_p->dw1.all, desc_bd_p->data_pointer, desc_bd_p->status.all);

         /* Handling for last fragment of a packet */
         } else if ( (0 == SOP(desc_bd_p)) && (1 == EOP(desc_bd_p)) ) {
             /* Byte_Offset MUST be ZERO */

//             DC_DP_DEBUG(DC_DP_DBG_FLAG_DUMP_RX_DESCRIPTOR, "DW0=0x%08x DW1=0x%08x DW2=0x%08x DW3=0x%08x.\n",
//                         desc_bd_p->dw0.all, desc_bd_p->dw1.all, desc_bd_p->data_pointer, desc_bd_p->status.all);

             if ( (0 != desc_byte_offset) || (0 != desc_unaligned_data_len) ) {

                 data_vpointer = (uint32_t)phys_to_virt(DATA_POINTER(desc_bd_p));
                 (void)dma_map_single(NULL, (void *)data_vpointer, (desc_data_len + desc_unaligned_data_len), DMA_FROM_DEVICE);

                 dest_data_pointer = (data_vpointer + desc_unaligned_data_len);
                 src_data_pointer = (data_vpointer + desc_byte_offset);
                 if (dest_data_pointer != src_data_pointer)
                     memmove((void *)dest_data_pointer, (void *)src_data_pointer, DATA_LEN(desc_bd_p));

                 memcpy((void *)(data_vpointer), (void *)(desc_unaligned_data_vpos), desc_unaligned_data_len);

                 desc_total_data_len = (desc_data_len + desc_unaligned_data_len);

                 BYTE_OFFSET(desc_bd_p) = 0;
                 DATA_LEN(desc_bd_p) = desc_total_data_len;
                 (void)dma_map_single(NULL, (void *)data_vpointer, desc_total_data_len, DMA_BIDIRECTIONAL);
             }

             FRAG_READY(desc_bd_p) = 0;
             OWN(desc_bd_p) = 1;

             wmb();

             /* FIXME : Set the OWN bit of 1st fragment */
             if (first_desc_bd_p)
                 OWN(first_desc_bd_p) = 1;

//             DC_DP_DEBUG(DC_DP_DBG_FLAG_DUMP_RX_DESCRIPTOR, "DW0=0x%08x DW1=0x%08x DW2=0x%08x DW3=0x%08x.\n",
//                         desc_bd_p->dw0.all, desc_bd_p->dw1.all, desc_bd_p->data_pointer, desc_bd_p->status.all);

             frag_ready = 0;
         } else {

         }

         dcmode_ring_inc_read_index(ch, 1);
     } while ( 1 == frag_ready );

     return 0;
}
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */

static inline void
get_soc_capability(uint32_t *cap)
{
    /* Linux offload capability */
    *cap = DC_DP_F_HOST_CAP_SG | DC_DP_F_HOST_CAP_HW_CSUM | DC_DP_F_HOST_CAP_RXCSUM;
#ifdef CONFIG_LTQ_TOE_DRIVER
    *cap |= DC_DP_F_HOST_CAP_TSO;
#endif /* #ifdef CONFIG_LTQ_TOE_DRIVER */
#ifdef CONFIG_LTQ_PPA_LRO
    *cap |= DC_DP_F_HOST_CAP_LRO;
#endif /* #ifdef CONFIG_LTQ_PPA_LRO */

    /* FCS capability */
    *cap |= DC_DP_F_HOST_CAP_TX_FCS;
    *cap |= DC_DP_F_HOST_CAP_RX_FCS;
    *cap |= DC_DP_F_HOST_CAP_TX_WO_FCS;
    *cap |= DC_DP_F_HOST_CAP_RX_WO_FCS;

    /* PMAC capability */
    *cap |= DC_DP_F_HOST_CAP_TX_PMAC;
    *cap |= DC_DP_F_HOST_CAP_RX_PMAC;
    *cap |= DC_DP_F_HOST_CAP_RX_WO_PMAC;

    /* QoS */
    *cap |= DC_DP_F_HOST_CAP_HW_QOS | DC_DP_F_HOST_CAP_HW_QOS_WAN;
    *cap |= DC_DP_F_HOST_CAP_DEVQOS;

    /* Rx fragmentation workaround */
    *cap |= DC_DP_F_HOST_CAP_RX_FRAG_HANDLING_RESTRICTED;

    *cap |= DC_DP_F_HOST_CAP_CPU_BIG_ENDIAN;
}

/*
 * ========================================================================
 * DirectConnect Driver Interface API (SoC specific)
 * ========================================================================
 */
static int32_t
xrx500_dcmode0_get_host_capability(struct dc_dp_host_cap *cap, uint32_t flags)
{
    int32_t ret = -1;

    if (cap) {
        cap->fastpath.support = 1;
        cap->fastpath.hw_dcmode = DC_DP_MODE_TYPE_0;

        cap->fastpath.hw_cmode.soc2dev_write = DC_DP_F_DCCNTR_MODE_INCREMENTAL;
        cap->fastpath.hw_cmode.soc2dev_write |= DC_DP_F_DCCNTR_MODE_LITTLE_ENDIAN;

        cap->fastpath.hw_cmode.dev2soc_write = DC_DP_F_DCCNTR_MODE_CUMULATIVE;
#ifdef CONFIG_LITTLE_ENDIAN
        cap->fastpath.hw_cmode.dev2soc_write |= DC_DP_F_DCCNTR_MODE_LITTLE_ENDIAN;
#else /* #ifdef CONFIG_LITTLE_ENDIAN */
        cap->fastpath.hw_cmode.dev2soc_write |= DC_DP_F_DCCNTR_MODE_BIG_ENDIAN;
#endif /* #else */

        get_soc_capability(&cap->fastpath.hw_cap);

        ret = 0;
    }

    return ret;
}

static int32_t
xrx500_dcmode0_register_dev_ex(void *ctx,
                            struct module *owner, uint32_t port_id,
                            struct net_device *dev, struct dc_dp_cb *datapathcb,
                            struct dc_dp_res *resources, struct dc_dp_dev *devspec,
                            int32_t start_port_id, uint32_t alloc_flags, uint32_t flags)
{
    int32_t ret;
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;
    dp_cb_t dp_cb = {0};

    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "dev_ctx=%p, owner=%p, port_id=%u, dev=%p, datapathcb=%p, \
                    resources=%p, dev_spec=%p, flags=0x%08X\n",
                    dev_ctx, owner, port_id, dev, datapathcb, resources, devspec, flags);

    /* De-register */
    if (flags & DC_DP_F_DEREGISTER) {

        DC_DP_LOCK(&g_dcmode0_dev_lock);

        if ( !((NULL != dev_ctx) && (NULL != dev_ctx->shared_info)) ) {
            ret = DC_DP_FAILURE;
            goto err_unlock_out;
        }

        /* De-register DC Mode0 device from DC Common layer */
        dc_dp_register_dcmode_device(owner, port_id, dev, dev_ctx, DC_DP_DCMODE_DEV_DEREGISTER);

        /* De-register device from DP Lib */
#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
        /* Skip, iff sub-sequent Multiport device? */
        if ( !((dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) && (dev_ctx->alloc_flags & DC_DP_F_SHARED_RES)) )
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */
            dp_register_dev(owner, port_id, &dp_cb, DP_F_DEREGISTER);

        /* For the last device */
        if ( (1 == dev_ctx->shared_info->ref_count) ) {
            /* Cleanup ring resources */
            DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "De-configuring DMA1-Tx channel 0x%x.\n",
                        dev_ctx->shared_info->dma_ch);
            xrx500_cleanup_ring_resources(dev_ctx, dev_ctx->shared_info->start_port_id, resources, flags);

            /* Cleanup UMT resources */
            xrx500_cleanup_umt_port(dev_ctx->shared_info->start_port_id, dev_ctx->shared_info->umt_id);
        }

        /* Free DC Mode0 device context */
        xrx500_dcmode0_free_dev_ctx(dev_ctx);

        DC_DP_UNLOCK(&g_dcmode0_dev_lock);

        DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Success, returned %d.\n", DC_DP_SUCCESS);
        return DC_DP_SUCCESS;
    }

    /* Validate input arguments */
    if (resources->num_dccntr != 1 && !resources->dccntr) {
        DC_DP_ERROR("failed to register device for the port_id %d!!!\n", port_id);
        ret = DC_DP_FAILURE;
        goto err_out;
    }

    DC_DP_LOCK(&g_dcmode0_dev_lock);

    if (NULL != g_dcmode0_dev_p[port_id]) {
        ret = DC_DP_FAILURE;
        goto err_unlock_out;
    }

    dev_ctx = xrx500_dcmode0_alloc_dev_ctx(port_id, start_port_id, alloc_flags);
    if (NULL == dev_ctx) {
        ret = DC_DP_FAILURE;
        goto err_unlock_out;
    }

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* Skip, iff sub-sequent Multiport device? */
    if ( !((alloc_flags & DC_DP_F_MULTI_PORT) && (alloc_flags & DC_DP_F_SHARED_RES)) ) {
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */
        /* Datapath Library callback registration */
        dp_cb.rx_fn = xrx500_dcmode0_rx_cb;
        dp_cb.stop_fn = datapathcb->stop_fn;
        dp_cb.restart_fn = datapathcb->restart_fn;
#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
        if ( (alloc_flags & DC_DP_F_MULTI_PORT) )
            dp_cb.get_subifid_fn = xrx500_dcmode0_get_netif_subifid_cb;
        else
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */
            dp_cb.get_subifid_fn = dc_dp_get_netif_subifid; // Exported from DC common layer
        dp_cb.reset_mib_fn = datapathcb->reset_mib_fn;
        dp_cb.get_mib_fn = datapathcb->get_mib_fn;

        ret = dp_register_dev(owner, port_id, &dp_cb, 0);
        if (ret != DP_SUCCESS) {
            DC_DP_ERROR("failed to register dev to Datapath Library/Core!!!\n");
            goto err_out_free_dev;
        }
#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    }
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    /* For the first device */
    if ( (1 == dev_ctx->shared_info->ref_count) ) {
        /* Setup UMT resources */
        ret = xrx500_setup_umt_port(dev_ctx, dev_ctx->shared_info->start_port_id, resources, flags);
        if (ret != DP_SUCCESS) {
            DC_DP_ERROR("failed to setup UMT resources!!!\n");
            goto err_out_dereg_dev;
        }

        /* Setup Port resources */
        ret = xrx500_setup_pmac_port(port_id, dev_ctx->shared_info->dma_cid, dev_ctx->shared_info->start_port_id, flags);
        if (ret != DP_SUCCESS) {
            DC_DP_ERROR("failed to setup UMT resources!!!\n");
            goto err_out_rel_umt;
        }

        /* Setup ring resources */
        DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Configuring DMA1-Tx channel 0x%x.\n",
                    dev_ctx->shared_info->dma_ch);
        ret = xrx500_setup_ring_resources(dev_ctx, dev_ctx->shared_info->start_port_id, resources, flags);
        if (ret) {
            DC_DP_ERROR("failed to configure DMA1-TX DMA channel 0x%x!!!\n",
                            dev_ctx->shared_info->dma_ch);
            goto err_out_res_pmac;
        }
    }

    /* FIXME : prepare returned resources */

    devspec->dc_accel_used = DC_DP_ACCEL_FULL_OFFLOAD;
    devspec->dc_tx_ring_used = DC_DP_RING_HW_MODE0;
    devspec->dc_rx_ring_used = DC_DP_RING_HW_MODE0;
    get_soc_capability(&devspec->dc_cap);

    /* Register DC Mode0 device to DC common layer */
    ret = dc_dp_register_dcmode_device(owner, port_id, dev, dev_ctx, 0);
    if (ret) {
        DC_DP_ERROR("failed to register device to DC common layer!!!\n");
        goto err_out_res_pmac;
    }

    DC_DP_UNLOCK(&g_dcmode0_dev_lock);

    return ret;

err_out_res_pmac:
err_out_rel_umt:
    xrx500_cleanup_umt_port(dev_ctx->shared_info->start_port_id, dev_ctx->shared_info->umt_id);

err_out_dereg_dev:
#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* Skip, iff sub-sequent Multiport device? */
    if ( !((alloc_flags & DC_DP_F_MULTI_PORT) && (alloc_flags & DC_DP_F_SHARED_RES)) )
#endif
        dp_register_dev(owner, port_id, &dp_cb, DP_F_DEREGISTER);

err_out_free_dev:
    xrx500_dcmode0_free_dev_ctx(dev_ctx);

err_unlock_out:
    DC_DP_UNLOCK(&g_dcmode0_dev_lock);

err_out:
    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Failure, returned %d.\n", ret);
    return ret;
}

static int32_t
xrx500_dcmode0_register_subif(void *ctx,
                              struct module *owner, struct net_device *dev,
                              const uint8_t *subif_name, struct dp_subif *subif_id, uint32_t flags)
{
    int32_t ret = DC_DP_FAILURE;
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;
    struct dp_subif subif = {0};

    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "dev_ctx=%p, owner=%p, dev=%p, subif_id=%p, flags=0x%08X\n",
                    dev_ctx, owner, dev, subif_id, flags);

    memcpy(&subif, subif_id, sizeof(struct dp_subif));

    /* De-register */
    if (flags & DC_DP_F_DEREGISTER) {

        DC_DP_LOCK(&g_dcmode0_dev_lock);

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
        /* Multiport device? */
        if ( (dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) ) {
            /* Multiport 1st radio? */
            if ( !(dev_ctx->alloc_flags & DC_DP_F_SHARED_RES) )
                subif.subif |= 0x0800;

            /* Multiport sub-sequent radio? */
            else
                subif.port_id = dev_ctx->shared_info->start_port_id;
        }
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

        /* For the last subif */
        if (dev_ctx->shared_info->num_subif == 1) {
            /* Disable DMA1-TX DMA channel */
            if (ltq_dma_chan_off(dev_ctx->shared_info->dma_ch)) {
                DC_DP_ERROR("failed to close dma1-tx dma channel 0x%x!!!\n", dev_ctx->shared_info->dma_ch);
            }

            /* Disable UMT port */
            DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Disabling UMT.\n");
#if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE
            if (ltq_umt_enable(dev_ctx->shared_info->umt_id, dev_ctx->shared_info->start_port_id, UMT_DISABLE)) {
                DC_DP_ERROR("failed to disable umt_id=%d, port_id=%d!!!\n",
                                dev_ctx->shared_info->umt_id, dev_ctx->shared_info->start_port_id);
            }
#else /* #if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE */
            if (ltq_umt_enable(UMT_DISABLE)) {
                DC_DP_ERROR("failed to disable UMT!!!\n");
            }
#endif /* #else */
            /* Delay for ongoing UMT transfer */
            udelay(XRX500_DCMODE0_UMT_PERIOD_DEFAULT * 2);

            /* Disable and Flush CBM/TMU Queue(s) */
            xrx500_flush_cbm_queue(subif.port_id);
        }

        /* De-register subif from Datapath Library/Core */
        ret = dp_register_subif(owner, dev, (uint8_t *)subif_name, &subif, DP_F_DEREGISTER);
        if (ret != DP_SUCCESS) {
            DC_DP_ERROR("failed to de-register subif from Datapath Library/Core!!!\n");
            goto err_unlock_out;
        }

        /* For the last subif, Restore TMU queue map */
        if (dev_ctx->shared_info->num_subif == 1)
            xrx500_restore_tmu_queuemap(subif.port_id);

        /* Reset device mib structure */
        memset(&dev_ctx->subif_mib[XRX500_DCMODE0_GET_SUBIFIDX(subif_id->subif)], 0, sizeof(struct dc_dp_dev_mib));

        dev_ctx->num_subif--;
        dev_ctx->shared_info->num_subif--;

        DC_DP_UNLOCK(&g_dcmode0_dev_lock);

        DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Success, returned %d.\n", DC_DP_SUCCESS);
        return DC_DP_SUCCESS;
    }

    DC_DP_LOCK(&g_dcmode0_dev_lock);

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* Multiport device? */
    if ( (dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) ) {
        /* Single port should not have more than 8 Subif */
        if ( XRX500_DCMODE0_GET_SUBIFIDX(subif_id->subif) >= 8 ) {
            DC_DP_ERROR("failed to register subif, subif_id_num=0x%04x reaches maximum limit 8!!!\n", subif_id->subif);
            goto err_unlock_out;
        }

        /* Multiport 1st radio? */
        if ( !(dev_ctx->alloc_flags & DC_DP_F_SHARED_RES) )
            subif.subif |= 0x0800;

        /* Multiport sub-sequent radio? */
        else
            subif.port_id = dev_ctx->shared_info->start_port_id;
    }
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    /* For the first subif, enable DMA1-TX DMA channel */
    if (dev_ctx->shared_info->num_subif == 0) {
        /* Enable UMT hw */
        DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Enabling UMT HW.\n");
#if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE
        ret = ltq_umt_enable(dev_ctx->shared_info->umt_id, dev_ctx->shared_info->start_port_id, UMT_ENABLE);
#else /* #if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE */
        ret = ltq_umt_enable(UMT_ENABLE);
#endif /* #else */
        if (ret) {
            DC_DP_ERROR("failed to enable umt_id=%d, port_id=%d!!!\n",
                            dev_ctx->shared_info->umt_id, dev_ctx->shared_info->start_port_id);
            goto err_unlock_out;
        }

        /* Enable DMA1-TX DMA channel */
        ret = ltq_dma_chan_on(dev_ctx->shared_info->dma_ch);
        if (ret) {
            DC_DP_ERROR("failed to open dma1-tx dma channel=0x%x!!!\n", dev_ctx->shared_info->dma_ch);
            goto err_out_disable_umt;
        }
    }

    /* Register subif to Datapath Library/Core */
    ret = dp_register_subif(owner, dev, (uint8_t *)subif_name, &subif, 0);
    if (ret != DP_SUCCESS) {
        DC_DP_ERROR("failed to register subif to Datapath Library/Core!!!\n");
        goto err_out_disable_dmach;
    }

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* Multiport 1st radio? */
    if ( (dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) && !(dev_ctx->alloc_flags & DC_DP_F_SHARED_RES) )
        subif_id->subif = (subif.subif & 0xF7FF);
    else
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */
        subif_id->subif = subif.subif;

    /* Reset device mib structure */
    memset(&dev_ctx->subif_mib[XRX500_DCMODE0_GET_SUBIFIDX(subif_id->subif)], 0, sizeof(struct dc_dp_dev_mib));

    dev_ctx->shared_info->num_subif++;
    dev_ctx->num_subif++;

    DC_DP_UNLOCK(&g_dcmode0_dev_lock);

    goto out;

err_out_disable_dmach:
    ltq_dma_chan_off(dev_ctx->shared_info->dma_ch);

err_out_disable_umt:
#if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE
    ltq_umt_enable(dev_ctx->shared_info->umt_id, dev_ctx->shared_info->start_port_id, UMT_DISABLE);
#else /* #if defined(CONFIG_LTQ_UMT_EXPAND_MODE) && CONFIG_LTQ_UMT_EXPAND_MODE */
    ltq_umt_enable(UMT_DISABLE);
#endif /* #else */

err_unlock_out:
    DC_DP_UNLOCK(&g_dcmode0_dev_lock);

out:
    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Returned %d.\n", ret);
    return ret;
}

static int32_t
xrx500_dcmode0_xmit(void *ctx, struct net_device *rx_if, struct dp_subif *rx_subif, struct dp_subif *tx_subif,
                    struct sk_buff *skb, int32_t len, uint32_t flags)
{
    int32_t ret;
    uint32_t dp_flags = 0;
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    struct dp_subif subif = {0};

    memcpy(&subif, tx_subif, sizeof(struct dp_subif));
    /* Multiport device? */
    if ( (dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) ) {
        /* Multiport 1st radio? */
        if ( !(dev_ctx->alloc_flags & DC_DP_F_SHARED_RES) ) {
            subif.subif |= 0x0800;
            dc_dp_set_subifid_pkt(subif.port_id, skb, subif.subif, 0);

        /* Multiport sub-sequent radio? */
        } else {
            subif.port_id = dev_ctx->shared_info->start_port_id;
            dc_dp_set_ep_pkt(skb, subif.port_id, 0);
        }
    }
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    if ( (skb->ip_summed == CHECKSUM_PARTIAL) )
        dp_flags = DP_TX_CAL_CHKSUM;

    /* Send it to Datapath library for transmit */
#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    ret = dp_xmit(skb->dev, &subif, skb, skb->len, dp_flags);
#else /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */
    ret = dp_xmit(skb->dev, tx_subif, skb, skb->len, dp_flags);
#endif /* #else */
    if ( (DP_SUCCESS == ret) ) {
        dev_ctx->subif_mib[XRX500_DCMODE0_GET_SUBIFIDX(tx_subif->subif)].tx_pkts ++;
        dev_ctx->subif_mib[XRX500_DCMODE0_GET_SUBIFIDX(tx_subif->subif)].tx_bytes += len;
    } else
        dev_ctx->subif_mib[XRX500_DCMODE0_GET_SUBIFIDX(tx_subif->subif)].tx_errs ++;

    return ret;
}

#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
static int32_t
xrx500_dcmode0_handle_ring_sw(void *ctx, struct module *owner, uint32_t port_id, struct net_device *dev,
                              struct dc_dp_ring *ring, uint32_t flags)
{
    int32_t ret = 0;
    uint32_t idx;
    uint32_t cntr_val;
    uint32_t dev2soc_ring_idx;
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;
    dev2soc_frag_except_bd_t *frag_ex_bd;
    uint32_t quota = 0;

    if (NULL == dev_ctx) {
        DC_DP_ERROR("ctx=0x%p, owner=%s, port_id=%d, dev=%s, ring=0x%p, flags=0x%08x.\n",
                    ctx, (owner ? owner->name : "NULL"), port_id, (dev ? dev->name : "NULL"), ring, flags);
        ret = -1;
        goto out;
    }

    if ( (flags & DC_DP_F_RX_FRAG_EXCEPT) ) {
//        DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "ctx=0x%p, owner=%s, port_id=%d, dev=%s, ring=0x%p, flags=0x%08x.\n",
//                    ctx, (owner ? owner->name : "NULL"), port_id, (dev ? dev->name : "NULL"), ring, flags);
        cntr_val = dcmode_read_cntr(CHAN_DEV2SOC_EX_P(dev_ctx));
        if (0 == cntr_val)
            goto out;

//        DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Read counter val from dev2soc_frag_except ring = 0x%x.\n", cntr_val);

        quota = MIN(CHAN_DEV2SOC_EX_P(dev_ctx)->ring.trck.quota, cntr_val);
        for (idx = 0; idx < quota; idx++) {
            frag_ex_bd = (dev2soc_frag_except_bd_t *)dcmode_get_bdp_from_ring(CHAN_DEV2SOC_EX_P(dev_ctx));
            dev2soc_ring_idx = frag_ex_bd->dev2soc_ring_idx;
//            DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Reference index of dev2soc ring = 0x%x.\n", dev2soc_ring_idx);
            ret = handle_dev2soc_frag_exception(CHAN_DEV2SOC_P(dev_ctx), dev2soc_ring_idx);
            if (0 != ret) {
                DC_DP_ERROR("Failed to fix DMA for dev2soc ring idx = 0x%x - ret=%d.\n", dev2soc_ring_idx, ret);
                break;
            }
            dcmode_ring_inc_read_index(CHAN_DEV2SOC_EX_P(dev_ctx), 1);
        }
        dcmode_ring_inc_write_index(CHAN_DEV2SOC_EX_P(dev_ctx), idx);
        dcmode_write_cntr(CHAN_DEV2SOC_EX_P(dev_ctx), idx);
    }

out:
    return ret;
}
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */

#if (defined(XRX500_DCMODE0_BRIDGE_FLOW_LEARNING) && XRX500_DCMODE0_BRIDGE_FLOW_LEARNING)
static int32_t
xrx500_dcmode0_add_session_shortcut_forward(void *ctx, struct dp_subif *subif, struct sk_buff *skb, uint32_t flags)
{
    int32_t ret = 0;

#if IS_ENABLED(CONFIG_LTQ_PPA_API) && defined(CONFIG_LTQ_BRIDGE_LEARNING)
    struct ethhdr *eth;

    if (!skb)
        return DC_DP_FAILURE;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* Skipping as subif argument is not used in PPA API */
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND  */

    /* FIXME : Enabled bridge flow learning globally for all netif (mainly appicable for WLAN netif) */
    if ( (flags & DC_DP_F_PREFORWARDING) ) {
        skb_reset_mac_header(skb);
        eth = eth_hdr(skb);
        if (unlikely(is_multicast_ether_addr(eth->h_dest)) ||
            unlikely(ether_addr_equal_64bits(eth->h_dest, skb->dev->dev_addr))) {
            /* Skipping, as no acceleration is possible */
            return 0;
        }

        skb->pkt_type = PACKET_OTHERHOST;
        skb->protocol = ntohs(eth->h_proto);
        skb_set_network_header(skb, ETH_HLEN);

        if ( NULL != ppa_hook_session_add_fn )
            ret = ppa_hook_session_add_fn(skb, NULL, (PPA_F_BRIDGED_SESSION | PPA_F_BEFORE_NAT_TRANSFORM));
    } else if ( (flags & DC_DP_F_POSTFORWARDING) ) {
        if ( NULL != ppa_hook_session_add_fn )
            ret = ppa_hook_session_add_fn(skb, NULL, PPA_F_BRIDGED_SESSION);
    }
#endif /* #if IS_ENABLED(CONFIG_LTQ_PPA_API) && defined(CONFIG_LTQ_BRIDGE_LEARNING) */

    return 0;
}
#endif /* #if (defined(XRX500_DCMODE0_BRIDGE_FLOW_LEARNING) && XRX500_DCMODE0_BRIDGE_FLOW_LEARNING) */

static int32_t
xrx500_dcmode0_disconn_if(void *ctx, struct net_device *netif, struct dp_subif *subif_id,
                          uint8_t mac_addr[MAX_ETH_ALEN], uint32_t flags)
{
    int32_t ret = DC_DP_SUCCESS;

#if IS_ENABLED(CONFIG_LTQ_PPA_API)
#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;
    struct dp_subif subif = {0};

    memcpy(&subif, subif_id, sizeof(struct dp_subif));
    /* Multiport device? */
    if ( (dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) ) {
        /* Multiport 1st radio? */
        if ( !(dev_ctx->alloc_flags & DC_DP_F_SHARED_RES) )
            subif.subif |= 0x0800;

        /* Multiport sub-sequent radio? */
        else
            subif.port_id = dev_ctx->shared_info->start_port_id;
    }

    /* Remove all the sessions from PPA */
    if (ppa_hook_disconn_if_fn)
        ret = ppa_hook_disconn_if_fn(netif, &subif, mac_addr, flags);
#else /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    /* Remove all the sessions from PPA */
    if (ppa_hook_disconn_if_fn)
        ret = ppa_hook_disconn_if_fn(netif, subif_id, mac_addr, flags);
#endif /* #else */
#endif /* #if IS_ENABLED(CONFIG_LTQ_PPA_API) */

    return ret;
}

/*
 * ========================================================================
 * Callbacks Registered to Datapath Library/Core
 * ========================================================================
 */
static int32_t
xrx500_dcmode0_rx_cb(struct net_device *rxif, struct net_device *txif,
                     struct sk_buff *skb, int32_t len)
{
    int32_t ret;
    struct pmac_rx_hdr *pmac;
    struct dp_subif rx_subif = {0};
    int32_t subif_idx;
    struct xrx500_dcmode0_dev_info *dev_ctx = NULL;

    if (!skb) {
        DC_DP_ERROR("failed to receive as skb=%p!!!\n", skb);
        return -1;
    }

    if (!rxif) {
        DC_DP_ERROR("failed to receive as rxif=%p!!!\n", rxif);
        dev_kfree_skb_any(skb);
        return -1;
    }

    pmac = (struct pmac_rx_hdr *)(skb->data);
    rx_subif.port_id = pmac->sppid;
    rx_subif.subif = (pmac->src_sub_inf_id << 8);
    rx_subif.subif |= (pmac->src_sub_inf_id2 & 0xFF);

    dev_ctx = g_dcmode0_dev_p[rx_subif.port_id];
    if (NULL == dev_ctx) {
        DC_DP_ERROR("failed to receive as dev_ctx=%p for port_id=%d!!!\n", dev_ctx, rx_subif.port_id);
        dev_kfree_skb_any(skb);
        return -1;
    }

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* Multiport device? */
    if ( (dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) ) {
        /* Multiport 1st radio? */
        if ( XRX500_DCMODE0_GET_SUBIFIDX(rx_subif.subif) >= 8 )
            rx_subif.subif &= 0xF7FF;

        /* Multiport sub-sequent radio? */
        else {
            rx_subif.port_id += 1; /* FIXME */

            dev_ctx = g_dcmode0_dev_p[rx_subif.port_id];
            if (NULL == dev_ctx) {
                DC_DP_ERROR("failed to receive as dev_ctx=%p for port_id=%d!!!\n", dev_ctx, rx_subif.port_id);
                dev_kfree_skb_any(skb);
                return -1;
            }
        }
    }
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    len -= sizeof(struct pmac_rx_hdr);
    skb_pull(skb, sizeof(struct pmac_rx_hdr));

    if ( (rxif->features & NETIF_F_RXCSUM) )
        skb->ip_summed = CHECKSUM_UNNECESSARY;

    ret = dc_dp_rx(rxif, txif, &rx_subif, skb, skb->len, 0);
    if ( !ret ) {
        /* Update Rx mib */
        subif_idx = XRX500_DCMODE0_GET_SUBIFIDX(rx_subif.subif);
        dev_ctx->subif_mib[subif_idx].rx_fn_rxif_pkts ++;
        dev_ctx->subif_mib[subif_idx].rx_fn_rxif_bytes += len;
    }

    return ret;
}

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
static int32_t
xrx500_dcmode0_get_netif_subifid_cb(struct net_device *netif,
                                    struct sk_buff *skb, void *subif_data,
                                    uint8_t dst_mac[MAX_ETH_ALEN],
                                    dp_subif_t *subif, uint32_t flags) /*! get subifid */
{
    int32_t ret = 1;
    struct dp_subif subif_id = {0};

    if (!netif) {
        DC_DP_ERROR("failed to get subifid as netif=%p!!!\n", netif);
        goto out;
    }

    if (!subif) {
        DC_DP_ERROR("failed to get subifid as subif=%p!!!\n", subif);
        goto out;
    }

    memcpy(&subif_id, subif, sizeof(struct dp_subif));

    /* Multiport 1st radio? */
    if ( XRX500_DCMODE0_GET_SUBIFIDX(subif_id.subif) >= 8 )
        subif_id.subif &= 0xF7FF;

    /* Multiport sub-sequent radio? */
    else
        subif_id.port_id += 1; /* FIXME */

    ret = dc_dp_get_netif_subifid(netif, skb, subif_data, dst_mac, &subif_id, flags);
    if (ret)
        goto out;

    subif->subif &= ~0x00FF;
    subif->subif |= (subif_id.subif & 0x00FF);

out:
    return ret;
}
#endif /* XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

/*
 * ========================================================================
 * Misclleneous API
 * ========================================================================
 */

static int32_t
xrx500_dcmode0_register_qos_class2prio_cb(void *ctx, int32_t port_id, struct net_device *netif,
                                          int (*cb)(int32_t port_id, struct net_device *netif, uint8_t class2prio[]),
                                          int32_t flags)
{
    int32_t ret = DC_DP_SUCCESS;

#if IS_ENABLED(CONFIG_LTQ_PPA_API)
#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;

    /* Multiport 2nd radio? */
    if ( (dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) && (dev_ctx->alloc_flags & DC_DP_F_SHARED_RES) )
        return DC_DP_SUCCESS;
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    if (ppa_register_qos_class2prio_hook_fn)
        ret = ppa_register_qos_class2prio_hook_fn(port_id, netif, cb, flags);
#endif /* #if IS_ENABLED(CONFIG_LTQ_PPA_API) */

    return ret;
}

static int32_t
xrx500_dcmode0_map_class2devqos(void *ctx, int32_t port_id, struct net_device *netif,
                                uint8_t class2prio[], uint8_t prio2devqos[])
{
    uint8_t devqos;

    /* Configure the egress PMAC table to mark the WMM/TID in the descriptor DW1[7:4] */
    uint8_t i = 0, j = 0;
    GSW_PMAC_Eg_Cfg_t egcfg;
    GSW_API_HANDLE gswr;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;

    /* Multiport 2nd radio? */
    if ( (dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) && (dev_ctx->alloc_flags & DC_DP_F_SHARED_RES) )
        return DC_DP_SUCCESS;
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    /* Do the GSW-R configuration */
    gswr = gsw_api_kopen(SWITCH_DEV);
    if (gswr == 0) {
        DC_DP_ERROR("failed to open SWAPI device!!!\n");
        return -EIO;
    }

    /* GSWIP-R PMAC Egress Configuration Table */
    for (i = 0; i < DC_DP_MAX_SOC_CLASS; i++) {
        devqos = _dc_dp_get_class2devqos(class2prio, prio2devqos, i);

        for (j = 0; j <= 3; j++) {
            memset((void *)&egcfg, 0x00, sizeof(egcfg));
            egcfg.nDestPortId   = port_id;
            egcfg.nTrafficClass = i;
            egcfg.nFlowIDMsb    = j;
            gsw_api_kioctl(gswr, GSW_PMAC_EG_CFG_GET, (unsigned int)&egcfg);

            egcfg.nResDW1       = devqos;
            gsw_api_kioctl(gswr, GSW_PMAC_EG_CFG_SET, (unsigned int)&egcfg);
        }
    }

    gsw_api_kclose(gswr);

    return DC_DP_SUCCESS;
}

static int32_t
xrx500_dcmode0_get_netif_stats(void *ctx,
                               struct net_device *netif, struct dp_subif *subif_id,
                               struct rtnl_link_stats64 *if_stats, uint32_t flags)
{
    int32_t ret;
    uint32_t dp_flags = 0x0;
    int32_t subif_idx;
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    struct dp_subif subif = {0};

    memcpy(&subif, subif_id, sizeof(struct dp_subif));

    /* Multiport radio? */
    if ( dev_ctx && (dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) ) {
        /* Multiport 1st radio? */
        if ( !(dev_ctx->alloc_flags & DC_DP_F_SHARED_RES) )
            subif.subif |= 0xF8FF;

        /* Multiport sub-sequent radio? */
        else
            subif.port_id = dev_ctx->shared_info->start_port_id;
    }
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    if ( (flags & DC_DP_F_SUBIF_LOGICAL) )
        dp_flags = DP_F_STATS_SUBIF;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    ret = dp_get_netif_stats(netif, &subif, if_stats, dp_flags);
#else /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */
    ret = dp_get_netif_stats(netif, subif_id, if_stats, dp_flags);
#endif /* #else */

    if ( (NULL != dev_ctx) ) {
        if ( (flags & DC_DP_F_SUBIF_LOGICAL) ) {
            subif_idx = XRX500_DCMODE0_GET_SUBIFIDX(subif_id->subif);
#if (defined(XRX500_DCMODE0_MIB_WORKAROUND) && (XRX500_DCMODE0_MIB_WORKAROUND == 1))
            /* Subif specific? Only CPU counters, no accelerated counters (buggy?) */
            if_stats->rx_packets = dev_ctx->subif_mib[subif_idx].rx_fn_rxif_pkts;
            if_stats->tx_packets = dev_ctx->subif_mib[subif_idx].tx_pkts;
            if_stats->rx_bytes = dev_ctx->subif_mib[subif_idx].rx_fn_rxif_bytes;
            if_stats->tx_bytes = dev_ctx->subif_mib[subif_idx].tx_bytes;
            if_stats->tx_errors = dev_ctx->subif_mib[subif_idx].tx_errs;
#else /* #if (defined(XRX500_DCMODE0_MIB_WORKAROUND) && (XRX500_DCMODE0_MIB_WORKAROUND == 1)) */
            /* Subif specific? Add CPU counters, on top of accelerated counters */
            if_stats->rx_packets += dev_ctx->subif_mib[subif_idx].rx_fn_rxif_pkts;
            if_stats->tx_packets += dev_ctx->subif_mib[subif_idx].tx_pkts;
            if_stats->rx_bytes += dev_ctx->subif_mib[subif_idx].rx_fn_rxif_bytes;
            if_stats->tx_bytes += dev_ctx->subif_mib[subif_idx].tx_bytes;
            if_stats->tx_errors += dev_ctx->subif_mib[subif_idx].tx_errs;
#endif /* #else */
        }
#if (defined(XRX500_DCMODE0_MIB_WORKAROUND) && (XRX500_DCMODE0_MIB_WORKAROUND == 1))
        /* Port specific? no Tx counter, so adding all subif CPU counters */
        else {
            if_stats->tx_packets = 0;
            if_stats->tx_bytes = 0;
            if_stats->tx_errors = 0;
            for (subif_idx = 0; subif_idx < XRX500_DCMODE0_MAX_SUBIF_PER_DEV; subif_idx++) {
                if_stats->tx_packets += dev_ctx->subif_mib[subif_idx].tx_pkts;
                if_stats->tx_bytes += dev_ctx->subif_mib[subif_idx].tx_bytes;
                if_stats->tx_errors += dev_ctx->subif_mib[subif_idx].tx_errs;
            }
        }
#endif /* #if (defined(XRX500_DCMODE0_MIB_WORKAROUND) && (XRX500_DCMODE0_MIB_WORKAROUND == 1)) */
    }

    return ret;
}

static int32_t
xrx500_dcmode0_clear_netif_stats(void *ctx,
                                 struct net_device *netif, struct dp_subif *subif_id,
                                 uint32_t flags)
{
    uint32_t dp_flags = 0x0;
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    struct dp_subif subif = {0};

    memcpy(&subif, subif_id, sizeof(struct dp_subif));

    /* Multiport radio? */
    if ( dev_ctx && (dev_ctx->alloc_flags & DC_DP_F_MULTI_PORT) ) {
        /* multiport 1st radio? */
        if ( !(dev_ctx->alloc_flags & DC_DP_F_SHARED_RES) )
            subif.subif |= 0xF8FF;

        /* multiport sub-sequent radio? */
        else
            subif.port_id = dev_ctx->shared_info->start_port_id;
    }
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    if ( (flags & DC_DP_F_SUBIF_LOGICAL) ) {
        /* Clear local subif MIB statistics */
        if ( (NULL != dev_ctx) )
            memset(&dev_ctx->subif_mib[XRX500_DCMODE0_GET_SUBIFIDX(subif_id->subif)], 0, sizeof(struct dc_dp_dev_mib));

        dp_flags = DP_F_STATS_SUBIF;
    } else {
        /* Clear local port MIB statistics */
        if ( (NULL != dev_ctx) )
            memset(&dev_ctx->subif_mib[0], 0, (XRX500_DCMODE0_MAX_SUBIF_PER_DEV * sizeof(struct dc_dp_dev_mib)));
    }

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    return dp_clear_netif_stats(netif, &subif, dp_flags);
#else /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */
    return dp_clear_netif_stats(netif, subif_id, dp_flags);
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */
}

static int32_t
xrx500_dcmode0_change_dev_status(void *ctx, int32_t port_id, uint32_t flags)
{
    int32_t ret = 0;
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* FIXME : Skipping, as not used currently */
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

#if 0 // FIXME
    if (flags & DC_DP_F_ENQ_ENABLE) {
        ret = cbm_dp_enable(dev_ctx->owner, dp_port_id, 0, dev_ctx->alloc_flags);
    } else if (flags & DC_DP_F_ENQ_DISABLE) {
        ret = cbm_dp_enable(dev_ctx->owner, dp_port_id, CBM_PORT_F_DISABLE, dev_ctx->alloc_flags);
    }
#endif /* #if 0 */

    if (flags & DC_DP_F_DEQ_ENABLE) {
        /* Enable DMA1-TX DMA channel */
        ret = ltq_dma_chan_on(dev_ctx->shared_info->dma_ch);
    } else if (flags & DC_DP_F_DEQ_DISABLE) {
        /* Disable DMA1-TX DMA channel */
        ret = ltq_dma_chan_off(dev_ctx->shared_info->dma_ch);
    }

    return ret;
}

static int32_t
xrx500_dcmode0_get_wol_cfg(void *ctx, int32_t port_id, struct dc_dp_wol_cfg *cfg, uint32_t flags)
{
    int32_t ret;
    GSW_API_HANDLE gswr;
    GSW_WoL_Cfg_t wol_cfg;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* FIXME : Skipping, as not used currently */
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    /* Do the GSW-R configuration */
    gswr = gsw_api_kopen(SWITCH_DEV);
    if (gswr == 0) {
        DC_DP_ERROR("failed to open SWAPI device!!!\n");
        return -EIO;
    }

    memset(&wol_cfg, 0, sizeof(GSW_WoL_Cfg_t));
    ret = gsw_api_kioctl(gswr, GSW_WOL_CFG_GET, (unsigned int)&wol_cfg);
    memcpy(cfg->wol_mac, wol_cfg.nWolMAC, sizeof(wol_cfg.nWolMAC));
    memcpy(cfg->wol_passwd, wol_cfg.nWolPassword, sizeof(cfg->wol_passwd));
    cfg->wol_passwd_enable = wol_cfg.bWolPasswordEnable;

    gsw_api_kclose(gswr);

    return ret;
}

static int32_t
xrx500_dcmode0_set_wol_cfg(void *ctx, int32_t port_id, struct dc_dp_wol_cfg *cfg, uint32_t flags)
{
    int32_t ret;
    GSW_API_HANDLE gswr;
    GSW_WoL_Cfg_t wol_cfg;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* FIXME : Skipping, as not used currently */
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    /* Do the GSW-R configuration */
    gswr = gsw_api_kopen(SWITCH_DEV);
    if (gswr == 0) {
        DC_DP_ERROR("failed to open SWAPI device!!!\n");
        return -EIO;
    }

    memset(&wol_cfg, 0, sizeof(GSW_WoL_Cfg_t));
    memcpy(wol_cfg.nWolMAC, cfg->wol_mac, sizeof(wol_cfg.nWolMAC));
    memcpy(wol_cfg.nWolPassword, cfg->wol_passwd, sizeof(wol_cfg.nWolPassword));
    wol_cfg.bWolPasswordEnable = cfg->wol_passwd_enable;
    ret = gsw_api_kioctl(gswr, GSW_WOL_CFG_SET, (unsigned int)&wol_cfg);

    gsw_api_kclose(gswr);

    return ret;
}

static int32_t
xrx500_dcmode0_set_wol_ctrl(void *ctx, int32_t port_id, uint32_t enable)
{
    int32_t ret;
    GSW_API_HANDLE gswr;
    GSW_WoL_PortCfg_t wol_port_cfg;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* FIXME : Skipping, as not used currently */
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    /* Do the GSW-R configuration */
    gswr = gsw_api_kopen(SWITCH_DEV);
    if (gswr == 0) {
        DC_DP_ERROR("failed to open SWAPI device!!!\n");
        return -EIO;
    }

    memset(&wol_port_cfg, 0, sizeof(GSW_WoL_PortCfg_t));
    wol_port_cfg.nPortId = port_id;
    wol_port_cfg.bWakeOnLAN_Enable = enable;
    ret = gsw_api_kioctl(gswr, GSW_WOL_PORT_CFG_SET, (unsigned int)&wol_port_cfg);

    gsw_api_kclose(gswr);

    return ret;
}

static int32_t
xrx500_dcmode0_get_wol_ctrl_status(void *ctx, int32_t port_id)
{
    int32_t ret;
    GSW_API_HANDLE gswr;
    GSW_WoL_PortCfg_t wol_port_cfg;

#if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND
    /* FIXME : Skipping, as not used currently */
#endif /* #if XRX500_DCMODE0_MULTIPORT_8SUBIF_WORKAROUND */

    /* Do the GSW-R configuration */
    gswr = gsw_api_kopen(SWITCH_DEV);
    if (gswr == 0) {
        DC_DP_ERROR("failed to open SWAPI device!!!\n");
        return -EIO;
    }

    memset(&wol_port_cfg, 0, sizeof(GSW_WoL_PortCfg_t));
    wol_port_cfg.nPortId = port_id;
    ret = gsw_api_kioctl(gswr, GSW_WOL_PORT_CFG_GET, (unsigned int)&wol_port_cfg);
    if (ret == 0) {
        if (wol_port_cfg.bWakeOnLAN_Enable)
            ret = 1;
        else
            ret = 0;
    }

    gsw_api_kclose(gswr);

    return ret;
}

static void
xrx500_dcmode0_dump_proc(void *ctx, struct seq_file *seq)
{
    struct xrx500_dcmode0_dev_info *dev_ctx = (struct xrx500_dcmode0_dev_info *)ctx;
    int i;

    seq_printf(seq, "    cbm_pid:           %d\n",
           dev_ctx->shared_info->cbm_pid);

    seq_printf(seq, "    dma_ch:            %d\n",
           _DMA_CHANNEL(dev_ctx->shared_info->dma_ch));

    seq_printf(seq, "    num_bufpools:      %02d\n",
           dev_ctx->shared_info->num_bufpools);
    for (i = 0; i < dev_ctx->shared_info->num_bufpools; i++) {
        seq_printf(seq, "    buflist %d:\n", (i + 1));
        seq_printf(seq, "      virtual range:    0x%p-0x%p (%d KB)\n",
               dev_ctx->shared_info->buflist[i].pool,
            ((uint8_t *)dev_ctx->shared_info->buflist[i].pool + dev_ctx->shared_info->buflist[i].size),
            (dev_ctx->shared_info->buflist[i].size >> 10));
        seq_printf(seq, "      physical range:   0x%p-0x%p (%d KB)\n",
               dev_ctx->shared_info->buflist[i].phys_pool,
            ((uint8_t *)dev_ctx->shared_info->buflist[i].phys_pool + dev_ctx->shared_info->buflist[i].size),
            (dev_ctx->shared_info->buflist[i].size >> 10));
    }

#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
    for (i = 0; i < DCMODE_CHAN_MAX; i++) {
        if (DCMODE_CHAN_SOC2DEV == i)
            continue; //seq_printf(seq, "    Ring: soc2dev:\n");
        else if (DCMODE_CHAN_SOC2DEV_RET == i)
            continue; //seq_printf(seq, "    Ring: soc2dev_ret:\n");
        else if (DCMODE_CHAN_DEV2SOC == i)
            seq_printf(seq, "    Ring: dev2soc:\n");
        else if (DCMODE_CHAN_DEV2SOC_RET == i)
            continue; //seq_printf(seq, "    Ring: dev2soc_ret:\n");
        else if (DCMODE_CHAN_DEV2SOC_EXCEPT == i)
            seq_printf(seq, "    Ring: dev2soc_frag_except:\n");

        seq_printf(seq, "      Ring Base:           P:0x%p, V:0x%p\n",
               dev_ctx->shared_info->ch[i].ring.phys, dev_ctx->shared_info->ch[i].ring.virt);
        seq_printf(seq, "      Ring Size:           %d, each of size %d DW\n",
               dev_ctx->shared_info->ch[i].ring.size, dev_ctx->shared_info->ch[i].ring.desc_dwsz);
        seq_printf(seq, "      Ring Track:          rp=%#x wp=%#x\n",
               dev_ctx->shared_info->ch[i].ring.trck.rp, dev_ctx->shared_info->ch[i].ring.trck.wp);
        seq_printf(seq, "      Soc To Dev Write Counter:           P:0x%p, V:0x%p (Size %d Bytes)\n",
               dev_ctx->shared_info->ch[i].write_cntr.phys, dev_ctx->shared_info->ch[i].write_cntr.virt,
               dev_ctx->shared_info->ch[i].write_cntr.size);
        seq_printf(seq, "      Soc From Dev Read Counter:           P:0x%p, V:0x%p (Size %d Bytes)\n",
               dev_ctx->shared_info->ch[i].read_cntr.phys, dev_ctx->shared_info->ch[i].read_cntr.virt,
               dev_ctx->shared_info->ch[i].read_cntr.size);
    }
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */

    seq_printf(seq, "    umt_id:            %d\n",
           dev_ctx->shared_info->umt_id);
    seq_printf(seq, "    umt_period:        %d (in micro second)\n",
           dev_ctx->shared_info->umt_period);
}

static struct dc_dp_dcmode_ops xrx500_dcmode0_ops = {
    .get_host_capability = xrx500_dcmode0_get_host_capability,
    .register_dev = NULL,
    .register_dev_ex = xrx500_dcmode0_register_dev_ex,
    .register_subif = xrx500_dcmode0_register_subif,
    .xmit = xrx500_dcmode0_xmit,
#ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING
    .handle_ring_sw = xrx500_dcmode0_handle_ring_sw,
#endif /* #ifdef DEV2SOC_FRAG_EXCEPTION_HANDLING */
#if (defined(XRX500_DCMODE0_BRIDGE_FLOW_LEARNING) && XRX500_DCMODE0_BRIDGE_FLOW_LEARNING)
    .add_session_shortcut_forward = xrx500_dcmode0_add_session_shortcut_forward,
#else /* #if (defined(XRX500_DCMODE0_BRIDGE_FLOW_LEARNING) && XRX500_DCMODE0_BRIDGE_FLOW_LEARNING) */
    .add_session_shortcut_forward = NULL,
#endif /* #else */
    .disconn_if = xrx500_dcmode0_disconn_if,
    .get_netif_stats = xrx500_dcmode0_get_netif_stats,
    .clear_netif_stats = xrx500_dcmode0_clear_netif_stats,
    .register_qos_class2prio_cb = xrx500_dcmode0_register_qos_class2prio_cb,
    .map_class2devqos = xrx500_dcmode0_map_class2devqos,
    .alloc_skb = NULL,
    .free_skb = NULL,
    .change_dev_status = xrx500_dcmode0_change_dev_status,
    .get_wol_cfg = xrx500_dcmode0_get_wol_cfg,
    .set_wol_cfg = xrx500_dcmode0_set_wol_cfg,
    .get_wol_ctrl_status = xrx500_dcmode0_get_wol_ctrl_status,
    .set_wol_ctrl = xrx500_dcmode0_set_wol_ctrl,
    .dump_proc = xrx500_dcmode0_dump_proc
};

static struct dc_dp_dcmode xrx500_dcmode0 = {
    .dcmode_cap = DC_DP_F_DCMODE_HW | DC_DP_F_DCMODE_0,
    .dcmode_ops = &xrx500_dcmode0_ops
};

static __init int xrx500_dcmode0_init_module(void)
{
    int32_t ret = 0;

    if (!g_dcmode0_init_ok) {
        spin_lock_init(&g_dcmode0_dev_lock);

        memset(g_dcmode0_dev, 0, sizeof(g_dcmode0_dev));

        /* Register DCMODE0 */
        ret = dc_dp_register_dcmode(&xrx500_dcmode0, 0);

#if IS_ENABLED(CONFIG_LTQ_PPA_API)
        ppa_check_if_netif_fastpath_fn = dc_dp_check_if_netif_fastpath;
#endif /* #if IS_ENABLED(CONFIG_LTQ_PPA_API) */

        g_dcmode0_init_ok = 1;
    }

    return ret;
}

static __exit void xrx500_dcmode0_exit_module(void)
{
    if (g_dcmode0_init_ok) {
#if IS_ENABLED(CONFIG_LTQ_PPA_API)
        ppa_check_if_netif_fastpath_fn = NULL;
#endif /* #if IS_ENABLED(CONFIG_LTQ_PPA_API) */

        /* De-register DCMODE0 */
        dc_dp_register_dcmode(&xrx500_dcmode0, DC_DP_F_DCMODE_DEREGISTER);

        /* Reset private data structure */
        memset(g_dcmode0_dev, 0, sizeof(g_dcmode0_dev));
        g_dcmode0_init_ok = 0;
    }
}

module_init(xrx500_dcmode0_init_module);
module_exit(xrx500_dcmode0_exit_module);

MODULE_AUTHOR("Anath Bandhu Garai");
MODULE_DESCRIPTION("DC Mode0 support for XRX500");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_MODULE_VERSION);
