obj-$(CONFIG_LANTIQ_VRX318) += vrx318.o
vrx318-objs += ltq_vrx318.o
obj-$(CONFIG_LANTIQ_ETH_FRAMEWORK) += lantiq_eth_framework.o
obj-$(CONFIG_LTQ_ETH_OAM) += ltq_eth_oam_handler.o
ifneq ($(CONFIG_LTQ_ETH_OAM),)
  obj-y += ltq_eth_oam_netif.o
endif
obj-$(CONFIG_LTQ_ETHSW_API) += switch-api/
ifeq ($(CONFIG_SOC_XWAY),)
obj-$(CONFIG_LTQ_DATAPATH) += datapath/
endif
obj-$(CONFIG_LTQ_TMU) += tmu/
obj-$(CONFIG_LTQ_CBM) += cbm/
obj-$(CONFIG_LTQ_ETH_XRX500) += ltq_eth_drv_xrx500.o
obj-$(CONFIG_LTQ_TOE_DRIVER) += ltq_toe_drv.o
obj-$(CONFIG_VRX318_DATAPATH) += vrx318/
obj-$(CONFIG_DIRECTCONNECT_DP_API) += directconnect_dp/

obj-$(CONFIG_LANTIQ_XWAY_SWITCH) += xway_switch.o
xway_switch-objs += xway_switch_main.o xway_gphy.o