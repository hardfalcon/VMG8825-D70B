obj-$(CONFIG_LTQ_PPA_DATAPATH)	+= datapath_eth.o
datapath_eth-objs += ltqmips_datapath.o
datapath_eth-objs += ltqmips_hal.o
datapath_eth-objs += ltqmips_ppe_drv_stub.o
ifneq ($(CONFIG_DIRECTCONNECT_DP_API),)
datapath_eth-objs += dcdp/ltqmips_datapath_api.o
datapath_eth-objs += dcdp/ltqmips_datapath_proc_api.o
datapath_eth-objs += dcdp/ltqmips_datapath_proc.o
endif
