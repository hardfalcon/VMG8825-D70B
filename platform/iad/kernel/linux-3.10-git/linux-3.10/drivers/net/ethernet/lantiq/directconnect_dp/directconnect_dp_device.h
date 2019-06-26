#ifndef _DIRECTCONNECT_DP_DEVICE_H_
#define _DIRECTCONNECT_DP_DEVICE_H_

#define DC_DP_MAX_SUBIF_PER_DEV 32 /* Maximum 32 Subif support */
//#define DC_DP_MAX_SOC_CLASS	16
#define DC_DP_DEV_CLASS_MASK    0x7

#define DC_DP_F_QOS_DEV_REG     0x01
#define DC_DP_F_QOS_DEV_DREG    0x02

#define DC_DP_SUBIFID_OFFSET    8
#define DC_DP_SUBIFID_MASK      0x1F

#define DC_DP_MAX_HW_DEVICE	16 /*< Applicable for DC fastpath device */
#define DC_DP_MAX_SW_DEVICE	4  /*< Applicable for DC litepath/swpath device */
#define DC_DP_HW_PORT_RANGE_START  0
#define DC_DP_HW_PORT_RANGE_END  ((DC_DP_HW_PORT_RANGE_START + DC_DP_MAX_HW_DEVICE) - 1)
#define DC_DP_SW_PORT_RANGE_START  16
#define DC_DP_SW_PORT_RANGE_END  ((DC_DP_SW_PORT_RANGE_START + DC_DP_MAX_SW_DEVICE) - 1)

#define DC_DP_MAX_DEV_NUM	(DC_DP_MAX_HW_DEVICE + DC_DP_MAX_SW_DEVICE)
typedef enum dc_dp_device_type {
    DC_DP_DEV_TYPE_HW = 0,
    DC_DP_DEV_TYPE_SW = 1,
    DC_DP_DEV_TYPE_MAX,
} dc_dp_device_type_t;

/*! Sub interface detail information */
typedef struct dc_dp_priv_subif_info {
	int32_t flags;
#define DC_DP_SUBIF_FREE       0x0
#define DC_DP_SUBIF_REGISTERED 0x1
	uint32_t subif:15;
	struct net_device  *netif;   /*! pointer to net_device*/
	uint32_t litepath_used;
	uint32_t litepath_subif;
	char device_name[IFNAMSIZ];  /*! devide name, like wlan0, */
} dc_dp_priv_subif_info_t;

typedef struct dc_dp_priv_dev_info {
	int32_t flags;	/*! port flags */
#define DC_DP_DEV_FREE             0x0
#define DC_DP_DEV_PORT_RESERVED    0x1
#define DC_DP_DEV_PORT_ALLOCATED   0x2
#define DC_DP_DEV_REGISTERED       0x3
#define DC_DP_DEV_SUBIF_REGISTERED 0x4
	int32_t alloc_flags;       /* the flags saved when calling dc_dp_alloc_port */
	struct module *owner;
	struct net_device *dev;
	uint32_t dev_port;
	int32_t port_id;
	int32_t start_port_id; /* Multiport reference start port id */
	struct dc_dp_cb cb;         /*! Callback Pointer to DIRECTPATH_CB */
	uint32_t litepath_used;
	uint32_t litepath_port;
	uint32_t num_subif;
	dc_dp_priv_subif_info_t subif_info[DC_DP_MAX_SUBIF_PER_DEV];

        enum dc_dp_accel_type dc_accel_used;
        enum dc_dp_ring_type dc_tx_ring_used;
        enum dc_dp_ring_type dc_rx_ring_used;

	/* WMM map information */
	uint8_t class2prio[DC_DP_MAX_SOC_CLASS];
	uint8_t prio2qos[DC_DP_MAX_DEV_CLASS];

	struct dc_dp_dcmode *dcmode;

	/* SoC specific private data */
	void *priv;
} dc_dp_priv_dev_info_t;

/* Extern variables */
extern int32_t
dc_dp_alloc_device(int32_t port_id, struct net_device *dev, struct dc_dp_priv_dev_info **pp_devinfo,
                   int32_t s_status, int32_t t_status, uint32_t flags);
extern void
dc_dp_free_device(int32_t port_id, struct net_device *dev, int32_t t_status, uint32_t flags);
int32_t
dc_dp_alloc_subif(struct dc_dp_priv_dev_info *p_devinfo, struct net_device *dev,
                  struct dp_subif *subif, struct dc_dp_priv_subif_info **pp_subifinfo);
void
dc_dp_free_subif(struct dc_dp_priv_subif_info *p_subifinfo);
extern int32_t
dc_dp_get_device_by_module_port(struct module *owner, int32_t port_id, struct dc_dp_priv_dev_info **pp_devinfo);
extern int32_t
dc_dp_get_device_by_port(int32_t port_id, struct dc_dp_priv_dev_info **pp_devinfo);
extern int32_t
dc_dp_get_device_by_subif_netif(struct net_device *netif, struct dc_dp_priv_dev_info **pp_devinfo);
extern int32_t
dc_dp_get_device_by_subif_ifname(char *ifname, struct dc_dp_priv_dev_info **pp_devinfo);
struct dc_dp_priv_dev_info *
dc_dp_get_hw_device_head(void);
struct dc_dp_priv_dev_info *
dc_dp_get_sw_device_head(void);
int32_t
dc_dp_device_init(void);
void
dc_dp_device_exit(void);

extern int32_t
is_hw_port(int32_t port_id);
extern int32_t
is_sw_port(int32_t port_id);

/* Extern functions */
#if IS_ENABLED(CONFIG_LTQ_PPA_API)
extern int ppa_br_fdb_delete(struct net_device *dev, const unsigned char *addr);
#endif /* #if IS_ENABLED(CONFIG_LTQ_PPA_API) */

static inline unsigned int dc_dp_get_subifidx(uint32_t subif)
{
    return (subif >> DC_DP_SUBIFID_OFFSET) & DC_DP_SUBIFID_MASK;
}

#endif /* _DIRECTCONNECT_DP_DEVICE_H_ */
