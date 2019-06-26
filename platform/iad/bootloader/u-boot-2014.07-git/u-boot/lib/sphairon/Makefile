#
# Copyright (C) 2011-2015 Sphairon GmbH (a ZyXEL company)
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += util.o
obj-y += flash.o flashlayout.o
obj-y += etl.o
obj-y += boot.o image.o
obj-y += controlfile.o
obj-y += init.o
obj-$(CONFIG_SPHAIRON_ZYCLIP) += zyclip.o

obj-y += cmd_sas_upgrade.o
obj-$(CONFIG_SPHAIRON_ZYCLIP) += cmd_sas_zyclip.o

ifneq ($(CONFIG_SYS_BOOT_RAM),y)
obj-y += cmd_sas_boot.o
obj-y += cmd_sas_etl.o
obj-y += cmd_sas_controlfile.o
endif
