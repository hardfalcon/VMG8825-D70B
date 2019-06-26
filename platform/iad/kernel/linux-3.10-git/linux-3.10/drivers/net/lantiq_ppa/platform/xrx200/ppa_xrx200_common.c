#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <net/ppa_stack_al.h>

static struct ltq_mei_atm_showtime_info showtime_info;

int ppa_callback_set(e_ltq_mei_cb_type type, void *func)
{
	int ret = 0;

	if (!func)
		return -1;

	switch (type) {
	case LTQ_MEI_SHOWTIME_CHECK:
		showtime_info.check_ptr = func;
		break;
	case LTQ_MEI_SHOWTIME_ENTER:
		showtime_info.enter_ptr = func;
		break;
	case LTQ_MEI_SHOWTIME_EXIT:
		showtime_info.exit_ptr = func;
		break;
	case LTQ_MEI_TC_REQUEST:
		showtime_info.req_tc_ptr = func;
		break;
	case LTQ_MEI_TC_RESET:
		showtime_info.tc_reset_ptr = func;
		break;
	case LTQ_MEI_ERB_ADDR_GET:
		showtime_info.erb_addr_ptr = func;
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}
EXPORT_SYMBOL(ppa_callback_set);

void *ppa_callback_get(e_ltq_mei_cb_type type)
{
	switch (type) {
	case LTQ_MEI_SHOWTIME_CHECK:
		return showtime_info.check_ptr;
	case LTQ_MEI_SHOWTIME_ENTER:
		return showtime_info.enter_ptr;
	case LTQ_MEI_SHOWTIME_EXIT:
		return showtime_info.exit_ptr;
	case LTQ_MEI_TC_REQUEST:
		return showtime_info.req_tc_ptr;
	case LTQ_MEI_TC_RESET:
		return showtime_info.tc_reset_ptr;
	case LTQ_MEI_ERB_ADDR_GET:
		return showtime_info.erb_addr_ptr;
	default:
		return NULL;
	}
}
EXPORT_SYMBOL(ppa_callback_get);
