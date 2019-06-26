#ifndef _DIRECTCONNECT_DP_DEBUG_H_
#define _DIRECTCONNECT_DP_DEBUG_H_

extern uint32_t
dc_dp_get_dbg_flag(void);

uint32_t
dc_dp_set_dbg_flag(uint32_t dbg_flag);

#define DC_DP_DBG_FLAG_DBG                     0x1
#define DC_DP_DBG_FLAG_DUMP_RX_DATA            0x10
#define DC_DP_DBG_FLAG_DUMP_RX_DESCRIPTOR      0x20
#define DC_DP_DBG_FLAG_DUMP_RX  (DC_DP_DBG_FLAG_DUMP_RX_DATA |\
                  DC_DP_DBG_FLAG_DUMP_RX_DESCRIPTOR)
#define DC_DP_DBG_FLAG_DUMP_TX_DATA            0x100
#define DC_DP_DBG_FLAG_DUMP_TX_DESCRIPTOR      0x200
#define DC_DP_DBG_FLAG_DUMP_TX  (DC_DP_DBG_FLAG_DUMP_TX_DATA |\
                  DC_DP_DBG_FLAG_DUMP_TX_DESCRIPTOR)
#define DC_DP_DBG_FLAG_FAKE_ARP                0x400
#define DC_DP_DBG_DISCONN_IF                   0x800
#define DC_DP_DBG_FLAG_ERR                     0x10000000

#define DC_DP_ERROR(fmt, args...) pr_err("DC DP [%s:%d] " fmt, __func__, __LINE__, ##args)

#if defined(CONFIG_DIRECTCONNECT_DP_DBG) && CONFIG_DIRECTCONNECT_DP_DBG
    #define DC_DP_DEBUG(flags, fmt, args...) \
        do { \
            if ((dc_dp_get_dbg_flag() & flags)) { \
                pr_info("DC DP [%s:%d] " fmt, __func__, __LINE__, ##args); \
            } \
        } while (0)
#else /* #if defined(CONFIG_DIRECTCONNECT_DP_DBG) && CONFIG_DIRECTCONNECT_DP_DBG */
    #define DC_DP_DEBUG(level, fmt, args...)
#endif /* #else */

#endif /* _DIRECTCONNECT_DP_DEBUG_H_ */
