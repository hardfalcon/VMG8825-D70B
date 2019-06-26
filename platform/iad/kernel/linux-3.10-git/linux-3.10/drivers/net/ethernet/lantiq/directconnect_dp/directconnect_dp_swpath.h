#ifndef _DIRECTCONNECT_DP_SWPATH_H_
#define _DIRECTCONNECT_DP_SWPATH_H_

extern int32_t
dp_alloc_sw_port(struct module *owner, struct net_device *dev, int32_t dev_port, int32_t port_id, uint32_t flags);
extern int32_t
dp_dealloc_sw_port(struct module *owner, struct net_device *dev, int32_t dev_port, int32_t port_id, uint32_t flags);

#endif
