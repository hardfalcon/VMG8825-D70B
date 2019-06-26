/* Includes */
#include <linux/module.h>

static uint32_t g_dc_dp_dbg_flag = 0x1;

uint32_t
dc_dp_get_dbg_flag(void)
{
    return g_dc_dp_dbg_flag;
}
EXPORT_SYMBOL(dc_dp_get_dbg_flag);

uint32_t
dc_dp_set_dbg_flag(uint32_t dbg_flag)
{
    g_dc_dp_dbg_flag = dbg_flag;
    return 0;
}
EXPORT_SYMBOL(dc_dp_set_dbg_flag);
