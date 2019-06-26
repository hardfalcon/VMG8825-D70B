obj-y += vrx218_common.o
ifneq ($(CONFIG_LTQ_PPA_E1),)
obj-$(CONFIG_LTQ_PPA_DATAPATH)	+= vrx318_tc.o
vrx318_tc-objs += vrx218_dfe.o
vrx318_tc-objs += vrx218_ppe_ptm_init.o
vrx318_tc-objs += vrx218_ptm_datapath.o
vrx318_tc-objs += vrx218_ptm_main.o
vrx318_tc-objs += vrx218_ptm_qos.o
vrx318_tc-objs += vrx218_ppe_atm_init.o
vrx318_tc-objs += vrx218_atm_datapath.o
vrx318_tc-objs += vrx218_atm_main.o
endif
