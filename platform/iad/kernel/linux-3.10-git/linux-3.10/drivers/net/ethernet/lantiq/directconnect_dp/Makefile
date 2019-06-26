obj-$(CONFIG_DIRECTCONNECT_DP_API) += directconnect_datapath.o
directconnect_datapath-y += directconnect_dp_api.o
directconnect_datapath-y += directconnect_dp_dcmode_api.o
directconnect_datapath-y += directconnect_dp_dcmode_wrapper.o
directconnect_datapath-y += directconnect_dp_debug.o
directconnect_datapath-y += directconnect_dp_device.o
directconnect_datapath-y += directconnect_dp_proc.o
directconnect_datapath-y += directconnect_dp_litepath.o
directconnect_datapath-y += directconnect_dp_swpath.o

ifeq ($(CONFIG_SW_DCMODE1_BUILTIN),y)
directconnect_datapath-y += dc_mode1_sw.o
else
obj-m += dc_mode1-sw.o
dc_mode1-sw-objs += dc_mode1_sw.o
endif

obj-$(CONFIG_HW_DCMODE0_XRX500) += dc_mode0_xrx500.o
obj-$(CONFIG_HW_DCMODE1_XRX330) += dc_mode1_xrx330.o

ccflags-$(CONFIG_SW_DCMODE1_BUILTIN) += -DSW_DCMODE1_BUILTIN
ccflags-$(CONFIG_LTQ_DATAPATH) += -DHAVE_DATAPATH_API
