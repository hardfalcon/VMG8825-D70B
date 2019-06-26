/*
 * Copyright (C) 2010 Lantiq Deutschland GmbH
 * Copyright (C) 2011-2013 Daniel Schwierzeck, daniel.schwierzeck@gmail.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __VRX200_SOC_H__
#define __VRX200_SOC_H__

#define LTQ_ASC0_BASE			0x1E100400
#define LTQ_SPI_BASE			0x1E100800
#define LTQ_GPIO_BASE			0x1E100B00
#define LTQ_SSIO_BASE			0x1E100BB0
#define LTQ_ASC1_BASE			0x1E100C00
#define LTQ_DMA_BASE			0x1E104100

#define LTQ_EBU_BASE			0x1E105300
#define LTQ_EBU_REGION0_BASE		0x10000000
#define LTQ_EBU_REGION1_BASE		0x14000000
#define LTQ_EBU_NAND_BASE		(LTQ_EBU_BASE + 0xB0)

#define LTQ_SWITCH_BASE			0x1E108000
#define LTQ_SWITCH_CORE_BASE		LTQ_SWITCH_BASE
#define LTQ_SWITCH_TOP_PDI_BASE		LTQ_SWITCH_CORE_BASE
#define LTQ_SWITCH_BM_PDI_BASE		(LTQ_SWITCH_CORE_BASE + 4 * 0x40)
#define LTQ_SWITCH_MAC_PDI_0_BASE	(LTQ_SWITCH_CORE_BASE + 4 * 0x900)
#define LTQ_SWITCH_MAC_PDI_X_BASE(x)	(LTQ_SWITCH_MAC_PDI_0_BASE + x * 0x30)
#define LTQ_SWITCH_TOPLEVEL_BASE	(LTQ_SWITCH_BASE + 4 * 0xC40)
#define LTQ_SWITCH_MDIO_PDI_BASE	(LTQ_SWITCH_TOPLEVEL_BASE)
#define LTQ_SWITCH_MII_PDI_BASE		(LTQ_SWITCH_TOPLEVEL_BASE + 4 * 0x36)
#define LTQ_SWITCH_PMAC_PDI_BASE	(LTQ_SWITCH_TOPLEVEL_BASE + 4 * 0x82)

#define LTQ_BOOTROM_BASE		0x1F000000
#define LTQ_PMU_BASE			0x1F102000
#define LTQ_CGU_BASE			0x1F103000
#define LTQ_DCDC_BASE			0x1F106A00
#define LTQ_MPS_BASE			0x1F107000
#define LTQ_CHIPID_BASE			(LTQ_MPS_BASE + 0x340)
#define LTQ_RCU_BASE			0x1F203000

#define LTQ_MC_GLOBAL_BASE		0x1F400000
#define LTQ_MC_DDR_BASE			0x1F401000
#define LTQ_MC_DDR_CCR_OFFSET(x)	(x * 0x10)

#endif /* __VRX200_SOC_H__ */
