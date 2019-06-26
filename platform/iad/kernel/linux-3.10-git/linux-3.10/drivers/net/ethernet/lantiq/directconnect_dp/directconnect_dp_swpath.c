#include <linux/module.h>
#include <linux/netdevice.h>
#include <net/directconnect_dp_dcmode_api.h>
#include "directconnect_dp_device.h"

static uint32_t sw_port_map = 0x0;

int32_t
dp_alloc_sw_port(struct module *owner, struct net_device *dev, int32_t dev_port, int32_t port_id, uint32_t flags)
{
    int32_t port_idx = 1;
    int32_t sw_port_id = 0;

    while ( (sw_port_map & port_idx) ) {
        port_idx <<= 1;
        sw_port_id++;
    }

    if (sw_port_id >= DC_DP_MAX_SW_DEVICE)
        return -1;

    sw_port_map |= port_idx;
    return (sw_port_id + DC_DP_SW_PORT_RANGE_START);
}

int32_t
dp_dealloc_sw_port(struct module *owner, struct net_device *dev, int32_t dev_port, int32_t port_id, uint32_t flags)
{
    int32_t port_idx = 1;
    int32_t sw_port_id;

    if (!is_sw_port(port_id))
        return -1;

    sw_port_id = port_id - DC_DP_SW_PORT_RANGE_START;
    port_idx <<= sw_port_id;

    if ( !(sw_port_map & port_idx) )
        return -1;

    sw_port_map &= ~port_idx;

    return 0;
}

/* TODO : Miscleneous API to be used by any SW DC mode */
