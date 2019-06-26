#ifndef _DIRECTCONNECT_DP_LITEPATH_H_
#define _DIRECTCONNECT_DP_LITEPATH_H_

static inline int32_t
is_litepath_dev_registered(struct dc_dp_priv_dev_info *dev_ctx, int32_t port_id, struct net_device *dev)
{
    return (dev_ctx->litepath_used);
}

static inline int32_t
is_litepath_subif_registered(struct dc_dp_priv_dev_info *dev_ctx, struct dp_subif *subif)
{
    int32_t subif_idx = ((subif->subif >> DC_DP_SUBIFID_OFFSET) & DC_DP_SUBIFID_MASK);

    if (subif_idx >= DC_DP_MAX_SUBIF_PER_DEV)
        return 0;

    return (dev_ctx->subif_info[subif_idx].litepath_used);
}

extern int32_t
dc_dp_litepath_register_dev(struct dc_dp_priv_dev_info *dev_ctx,
                       struct module *owner, uint32_t port_id,
                       struct net_device *dev, struct dc_dp_cb *datapathcb,
                       struct dc_dp_dev *devspec, uint32_t flags);


extern int32_t
dc_dp_litepath_deregister_dev(struct dc_dp_priv_dev_info *dev_ctx,
                       struct module *owner, uint32_t port_id,
                       struct net_device *dev, struct dc_dp_cb *datapathcb,
                       struct dc_dp_dev *devspec, uint32_t flags);


extern int32_t
dc_dp_litepath_register_subif(struct dc_dp_priv_dev_info *dev_ctx,
                         struct module *owner, struct net_device *dev,
                         const uint8_t*subif_name, struct dp_subif *subif_id, uint32_t flags);

extern int32_t
dc_dp_litepath_deregister_subif(struct dc_dp_priv_dev_info *dev_ctx,
                         struct module *owner, struct net_device *dev,
                         const uint8_t *subif_name, struct dp_subif *subif_id, uint32_t flags);


extern int32_t
dc_dp_litepath_xmit(struct dc_dp_priv_dev_info *dev_ctx, struct net_device *rx_if,
                    struct dp_subif *rx_subif, struct dp_subif *tx_subif, struct sk_buff *skb, int32_t len, uint32_t flags);


extern struct sk_buff *
dc_dp_litepath_alloc_skb(struct dc_dp_priv_dev_info *dev_ctx, uint32_t len, struct dp_subif *subif, uint32_t flags);

extern int32_t
dc_dp_litepath_free_skb(struct dc_dp_priv_dev_info *dev_ctx, struct dp_subif *subif, struct sk_buff *skb);

#if defined(CONFIG_LTQ_PPA_XRX330) && CONFIG_LTQ_PPA_XRX330
extern int32_t
dc_dp_litepath_disconn_if(struct dc_dp_priv_dev_info *dev_ctx, struct net_device *netif, struct dp_subif *subif_id,
                          uint8_t mac_addr[MAX_ETH_ALEN], uint32_t flags);
#endif /* defined(CONFIG_LTQ_PPA_XRX330) && CONFIG_LTQ_PPA_XRX330 */

#endif /* #ifndef _DIRECTCONNECT_DP_LITEPATH_H_ */
