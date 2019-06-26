#include <linux/module.h>
#include <net/directconnect_dp_dcmode_api.h>
#if IS_ENABLED(CONFIG_LTQ_PPA_API)
  #include <net/ppa_api.h>
  #if defined(CONFIG_LTQ_PPA_XRX330) && CONFIG_LTQ_PPA_XRX330
    #include <switch-api/lantiq_ethsw_api.h>
  #endif /* defined(CONFIG_LTQ_PPA_XRX330) && CONFIG_LTQ_PPA_XRX330 */
#endif /* #if IS_ENABLED(CONFIG_LTQ_PPA_API) */

#include "directconnect_dp_device.h"
#include "directconnect_dp_litepath.h"

/* Defines */
#if defined(CONFIG_LTQ_PPA_XRX330) && CONFIG_LTQ_PPA_XRX330
  #define SWITCH_DEV_L                "/dev/switch_api/0"
  #if defined(CONFIG_LTQ_ETHSW_API)
    #define SWITCH_DEV_HANDLE_TYPE    LTQ_ETHSW_API_HANDLE
    #define SWITCH_DEV_KOPEN           ltq_ethsw_api_kopen
    #define SWITCH_DEV_KIOCTL         ltq_ethsw_api_kioctl
    #define SWITCH_DEV_KCLOSE          ltq_ethsw_api_kclose
  #elif defined(CONFIG_IFX_ETHSW_API)
    #define SWITCH_DEV_HANDLE_TYPE    IFX_ETHSW_HANDLE
    #define SWITCH_DEV_KOPEN           ifx_ethsw_kopen
    #define SWITCH_DEV_KIOCTL         ifx_ethsw_kioctl
    #define SWITCH_DEV_KCLOSE          ifx_ethsw_kclose
  #else
    #error "Unsupported switch device handle type!!!"
  #endif
#endif

static int32_t
dc_dp_litepath_rx(struct net_device *rxif, struct net_device *txif, struct sk_buff *skb, int32_t len)
{
    /* NOTE : Tx/Rx subif is unknown here */
    return dc_dp_rx(rxif, txif, NULL, skb, len, DC_DP_F_RX_LITEPATH);
}

int32_t
dc_dp_litepath_register_dev(struct dc_dp_priv_dev_info *dev_ctx,
                       struct module *owner, uint32_t port_id,
                       struct net_device *dev, struct dc_dp_cb *datapathcb,
                       struct dc_dp_dev *devspec, uint32_t flags)
{
    int32_t ret = -1;

#if IS_ENABLED(CONFIG_LTQ_PPA_API)
    PPA_SUBIF lp_dp_subif = {0};
    PPA_DIRECTPATH_CB directpath_cb = {0};

    if (!dev)
        goto out;

    lp_dp_subif.port_id = -1;
    directpath_cb.stop_tx_fn = datapathcb->stop_fn;
    directpath_cb.start_tx_fn = datapathcb->restart_fn;
    directpath_cb.rx_fn = dc_dp_litepath_rx;

    if (ppa_hook_directpath_ex_register_dev_fn) {
        // FIXME : necessary flags?
        DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "port_id=%d\n", port_id);
        ret = ppa_hook_directpath_ex_register_dev_fn(&lp_dp_subif, dev, &directpath_cb, (PPA_F_DIRECTPATH_ETH_IF | PPA_F_DIRECTPATH_REGISTER));
        DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "lp_dp_subif.port_id=%d, lp_dp_subif.subif = %d\n", lp_dp_subif.port_id, lp_dp_subif.subif);
        if (ret == PPA_SUCCESS) {
            dev_ctx->litepath_port = lp_dp_subif.port_id;
            dev_ctx->litepath_used = 1;

            devspec->dc_accel_used = DC_DP_ACCEL_PARTIAL_OFFLOAD;
        }
    }

out:
#endif

    return ret;
}

int32_t
dc_dp_litepath_deregister_dev(struct dc_dp_priv_dev_info *dev_ctx,
                       struct module *owner, uint32_t port_id,
                       struct net_device *dev, struct dc_dp_cb *datapathcb,
                       struct dc_dp_dev *devspec, uint32_t flags)
{
    int32_t ret = -1;

#if IS_ENABLED(CONFIG_LTQ_PPA_API)
    PPA_SUBIF lp_dp_subif = {0};

    if (!dev)
        goto out;

    lp_dp_subif.port_id = dev_ctx->litepath_port;
    lp_dp_subif.subif = -1;

    if (ppa_hook_directpath_ex_register_dev_fn) {
        ret = ppa_hook_directpath_ex_register_dev_fn(&lp_dp_subif, dev, NULL, 0);
        if (ret == PPA_SUCCESS) {
            dev_ctx->litepath_port = 0;
            dev_ctx->litepath_used = 0;
        }
    }

out:
#endif

    return ret;
}

int32_t
dc_dp_litepath_register_subif(struct dc_dp_priv_dev_info *dev_ctx,
                         struct module *owner, struct net_device *dev,
                         const uint8_t *subif_name, struct dp_subif *subif_id, uint32_t flags)
{
    int32_t ret = -1;

    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "\n");

#if IS_ENABLED(CONFIG_LTQ_PPA_API)
    if (is_sw_port(subif_id->port_id) && dev_ctx->litepath_used) {
        PPA_SUBIF lp_dp_subif = {0};
        PPA_DIRECTPATH_CB directpath_cb = {0};
        int32_t subif_idx;

        if (!dev)
            goto out;

        subif_idx = ((subif_id->subif >> DC_DP_SUBIFID_OFFSET) & DC_DP_SUBIFID_MASK);
        lp_dp_subif.port_id = dev_ctx->litepath_port;
        lp_dp_subif.subif = -1;

        directpath_cb.stop_tx_fn = dev_ctx->cb.stop_fn;
        directpath_cb.start_tx_fn = dev_ctx->cb.restart_fn;
        directpath_cb.rx_fn = dc_dp_litepath_rx;

        /* FIXME : Litepath only mode, subif_id->subif may not be set. So, need to allocate a new subif? */
        if (ppa_hook_directpath_ex_register_dev_fn) {
            DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "subif_id->port_id=%d, subif_id->subif = %d\n", subif_id->port_id, subif_id->subif);
            ret = ppa_hook_directpath_ex_register_dev_fn(&lp_dp_subif, dev, &directpath_cb, (PPA_F_DIRECTPATH_ETH_IF | PPA_F_DIRECTPATH_REGISTER));
            DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "lp_dp_subif.port_id=%d, lp_dp_subif.subif = %d\n", lp_dp_subif.port_id, lp_dp_subif.subif);
            if (ret == PPA_SUCCESS) {
                dev_ctx->subif_info[subif_idx].litepath_subif = lp_dp_subif.subif;
                dev_ctx->subif_info[subif_idx].litepath_used = 1;
            }
        }
    }

out:
#endif

    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "returnd status = %d\n", ret);
    return ret;
}

int32_t
dc_dp_litepath_deregister_subif(struct dc_dp_priv_dev_info *dev_ctx,
                         struct module *owner, struct net_device *dev,
                         const uint8_t *subif_name, struct dp_subif *subif_id, uint32_t flags)
{
    int32_t ret = -1;
    int32_t subif_idx;

    if (!dev)
        goto out;

    subif_idx = ((subif_id->subif >> DC_DP_SUBIFID_OFFSET) & DC_DP_SUBIFID_MASK);

#if IS_ENABLED(CONFIG_LTQ_PPA_API)
    if (is_sw_port(subif_id->port_id) && dev_ctx->subif_info[subif_idx].litepath_used) {
        PPA_SUBIF lp_dp_subif = {0};

        lp_dp_subif.port_id = dev_ctx->litepath_port;
        lp_dp_subif.subif = dev_ctx->subif_info[subif_idx].litepath_subif;

        if (ppa_hook_directpath_ex_register_dev_fn) {
            ret = ppa_hook_directpath_ex_register_dev_fn(&lp_dp_subif, dev, NULL, 0);
            if (ret == PPA_SUCCESS) {
                dev_ctx->subif_info[subif_idx].litepath_subif = lp_dp_subif.subif;
                dev_ctx->subif_info[subif_idx].litepath_used = 1;
            }
        }
    }
#endif

out:
    return ret;
}

int32_t
dc_dp_litepath_xmit(struct dc_dp_priv_dev_info *dev_ctx, struct net_device *rx_if,
                    struct dp_subif *rx_subif, struct dp_subif *tx_subif, struct sk_buff *skb, int32_t len, uint32_t flags)
{
    int32_t ret = -1;

#if IS_ENABLED(CONFIG_LTQ_PPA_API)
    int32_t tx_subif_idx;

    tx_subif_idx = ((tx_subif->subif >> DC_DP_SUBIFID_OFFSET) & DC_DP_SUBIFID_MASK);

    if (dev_ctx->subif_info[tx_subif_idx].litepath_used) {
        PPA_SUBIF lp_dp_subif = {0};

        lp_dp_subif.port_id = dev_ctx->litepath_port;
        lp_dp_subif.subif = dev_ctx->subif_info[tx_subif_idx].litepath_subif;

        if (ppa_hook_directpath_ex_send_fn) {
            if (!rx_if) {
//                DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "QoS: lp_dp_subif.port_id=%d, lp_dp_subif.subif=%04x\n", lp_dp_subif.port_id, lp_dp_subif.subif);
                ret = ppa_hook_directpath_ex_send_fn(&lp_dp_subif, skb, skb->len, PPA_F_DIRECTPATH_XMIT_QOS);
                if (ret == PPA_EINVAL) {
                    ret = -2;
//                    DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "ret=%d\n", ret);
                }
            } else {
//                DC_DP_DEBUG(DC_DP_DBG_FLAG_DBG, "Litepath:lp_dp_subif.port_id=%d, lp_dp_subif.subif=%04x\n", lp_dp_subif.port_id, lp_dp_subif.subif);
                ret = ppa_hook_directpath_ex_send_fn(&lp_dp_subif, skb, skb->len, 0);
            }
        }
    }
#endif

    return ret;
}

struct sk_buff *
dc_dp_litepath_alloc_skb(struct dc_dp_priv_dev_info *dev_ctx, uint32_t len, struct dp_subif *subif, uint32_t flags)
{
    struct sk_buff *skb = NULL;

#if IS_ENABLED(CONFIG_LTQ_PPA_API)
    PPA_SUBIF lp_dp_subif = {0};
    int32_t subif_idx;

    subif_idx = ((subif->subif >> DC_DP_SUBIFID_OFFSET) & DC_DP_SUBIFID_MASK);

    lp_dp_subif.port_id = dev_ctx->litepath_port;
    lp_dp_subif.subif = dev_ctx->subif_info[subif_idx].litepath_subif;

    if (ppa_hook_directpath_alloc_skb_fn)
        skb = ppa_hook_directpath_alloc_skb_fn(&lp_dp_subif, len, 0);
#if 0 // FIXME : should be removed
        if ((int32_t)skb == -22)
            skb = NULL;
#endif
#endif

    return skb;
}

int32_t
dc_dp_litepath_free_skb(struct dc_dp_priv_dev_info *dev_ctx, struct dp_subif *subif, struct sk_buff *skb)
{
    int32_t ret = -1;

#if IS_ENABLED(CONFIG_LTQ_PPA_API)
    PPA_SUBIF lp_dp_subif = {0};
    int32_t subif_idx;

    subif_idx = ((subif->subif >> DC_DP_SUBIFID_OFFSET) & DC_DP_SUBIFID_MASK);

    lp_dp_subif.port_id = dev_ctx->litepath_port;
    lp_dp_subif.subif = dev_ctx->subif_info[subif_idx].litepath_subif;

    if (ppa_hook_directpath_recycle_skb_fn)
        ret = ppa_hook_directpath_recycle_skb_fn(&lp_dp_subif, skb, 0);
#endif

    return ret;
}

#if defined(CONFIG_LTQ_PPA_XRX330) && CONFIG_LTQ_PPA_XRX330
int32_t
dc_dp_litepath_disconn_if(struct dc_dp_priv_dev_info *dev_ctx, struct net_device *netif, struct dp_subif *subif_id,
                          uint8_t mac_addr[MAX_ETH_ALEN], uint32_t flags)
{
    int32_t ret = 0;
    SWITCH_DEV_HANDLE_TYPE handle;
    union ifx_sw_param x;

    /* Check for null/zero mac */
    if ( (NULL == mac_addr) || ( (NULL != mac_addr) && is_zero_ether_addr(mac_addr) ) ) {
        /* Nothing to do */
        goto out;
    }

    /* Legacy platform (Reconn workaround) : Delete the MAC address manually */
    memset(&x.MAC_tableRemove, 0x00, sizeof(x.MAC_tableRemove));

    handle = SWITCH_DEV_KOPEN(SWITCH_DEV_L);
    if (handle < 0) {
        ret = -1;
        goto out;
    }

    x.MAC_tableRemove.nFId = 0;
    memcpy(&x.MAC_tableRemove.nMAC[0], &mac_addr[0], IFX_MAC_ADDRESS_LENGTH);
    SWITCH_DEV_KIOCTL(handle, IFX_ETHSW_MAC_TABLE_ENTRY_REMOVE, (unsigned int)&x.MAC_tableRemove);
    SWITCH_DEV_KCLOSE(handle);

out:
    return ret;
}
#endif /* defined(CONFIG_LTQ_PPA_XRX330) && CONFIG_LTQ_PPA_XRX330 */
