/*
 * Copyright (C) 2018 Daniel Schwierzeck, daniel.schwierzeck@gmail.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <phy_lantiq.h>
#include <asm/gpio.h>
#include <asm/lantiq/eth.h>
#include <asm/lantiq/chipid.h>
#include <asm/lantiq/cpu.h>
#include <asm/lantiq/mem.h>
#include <asm/arch/gphy.h>
#include <sas/controlfile.h>

#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_SYS_BOOT_RAM)
#define do_gpio_init	1
#else
#define do_gpio_init	0
#endif

static void gpio_init(void)
{
	/* #PCIE2_RST_DSL (low-active) */
	gpio_direction_output(0, 0);

	/* GPIO button board reset (low-active) */
	gpio_direction_input(3);

	/* WAN PHY interrupt */
	gpio_direction_input(1);

	/* LED Power green (low-active) */
	gpio_direction_output(4, 1);
	/* LED Power red (low-active) */
	gpio_direction_output(5, 0);
	/* LED TBD green */
	gpio_direction_output(6, 0);
	/* LED TBD red */
	gpio_direction_output(8, 0);
	/* LED DSL */
	gpio_direction_output(18, 0);
	/* LED SFP */
	gpio_direction_output(27, 0);
	/* LED GPHY0 LAN1 */
	gpio_direction_output(9, 0);
	/* LED GPHY1 LAN2 */
	gpio_direction_output(10, 0);
	/* LED GPHY2 LAN3 */
	gpio_direction_output(11, 0);
	/* LED GPHY3 LAN4 */
	gpio_direction_output(17, 0);

	/* #DGSP_RST */
	gpio_direction_input(16);

	/* SFP_PON_RS */
	gpio_direction_output(15, 0);
	/* SFP_TX_Disable */
	gpio_direction_output(26, 0);
	/* SFP_PRESENT */
	gpio_direction_input(34);
	/* SFP_I2C_DATA */
	gpio_direction_input(35);
	/* SFP_I2C_CLK */
	gpio_direction_output(36, 0);
	/* SFP_TX_FAULT */
	gpio_direction_input(58);
	/* SFP_RX_LOS */
	gpio_direction_input(61);

	/* NAND CS1 */
	gpio_set_altfunc(23, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND CLE */
	gpio_set_altfunc(24, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND ALE */
	gpio_set_altfunc(13, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND RDBY */
	gpio_set_altfunc(48, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_IN);
	/* NAND RE */
	gpio_set_altfunc(49, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND WE */
	gpio_set_altfunc(59, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND WP */
	gpio_set_altfunc(60, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND Data[1] */
	gpio_set_altfunc(50, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND Data[0] */
	gpio_set_altfunc(51, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND Data[2] */
	gpio_set_altfunc(52, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND Data[7] */
	gpio_set_altfunc(53, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND Data[6] */
	gpio_set_altfunc(54, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND Data[5] */
	gpio_set_altfunc(55, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND Data[4] */
	gpio_set_altfunc(56, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
	/* NAND Data[3] */
	gpio_set_altfunc(57, GPIO_ALTSEL_SET, GPIO_ALTSEL_CLR, GPIO_DIR_OUT);
}

int board_early_init_f(void)
{
	if (do_gpio_init) {
		ltq_gpio_init();
		gpio_init();
	}

	return 0;
}

int checkboard(void)
{
	puts("Board: " CONFIG_BOARD_NAME "\n");
	ltq_chip_print_info();

	return 0;
}

int misc_init_r(void)
{
#if 0
	return mc_tune_store_flash();
#else
	return 0;
#endif
}

static const struct ltq_eth_port_config eth_port_config[] = {
	/* GMAC0: xMII0 unused */
	{ 0, 0x0, LTQ_ETH_PORT_NONE, PHY_INTERFACE_MODE_NONE },
	/* GMAC1: internal GPHY0 with 1000 firmware for LAN port 3 */
	{ 1, 0x1, LTQ_ETH_PORT_PHY, PHY_INTERFACE_MODE_GMII },
	/* GMAC2: internal GPHY1 with 1000 firmware for LAN port 1 */
	{ 2, 0x2, LTQ_ETH_PORT_PHY, PHY_INTERFACE_MODE_GMII },
	/* GMAC3: internal GPHY2 with 1000 firmware for LAN port 4 */
	{ 3, 0x3, LTQ_ETH_PORT_PHY, PHY_INTERFACE_MODE_GMII },
	/* GMAC4: internal GPHY3 with 1000 firmware for LAN port 2 */
	{ 4, 0x4, LTQ_ETH_PORT_PHY, PHY_INTERFACE_MODE_GMII },
	/* GMAC5: Realtek RTL8211FS at xMII1, not useable in U-Boot */
	{ 5, 0x5, LTQ_ETH_PORT_NONE, PHY_INTERFACE_MODE_NONE },
};

static const struct ltq_eth_board_config eth_board_config = {
	.ports = eth_port_config,
	.num_ports = ARRAY_SIZE(eth_port_config),
};

int board_eth_init(bd_t * bis)
{
	const enum ltq_gphy_clk clk = LTQ_GPHY_CLK_25MHZ_PLL0;
	void *fw_addr;

	fw_addr = ltq_gphy_alloc();
	if (!fw_addr)
		return -1;

	ltq_gphy_phy11g_a21_load(fw_addr);

	ltq_cgu_gphy_clk_src(clk);

	ltq_rcu_gphy_boot(0, fw_addr);
	ltq_rcu_gphy_boot(1, fw_addr);
	ltq_rcu_gphy_boot(2, fw_addr);
	ltq_rcu_gphy_boot(3, fw_addr);

	return ltq_eth_initialize(&eth_board_config);
}

int sas_cf_check_board(void)
{
	/* check if reset button is pressed */
	return 0 == gpio_get_value(3);
}

void sas_cf_led_action(enum sas_cf_state state)
{
	switch (state) {
	case CF_STARTED:
		/* LED Power green on */
		gpio_direction_output(4, 0);
		break;
	case CF_FINISHED:
		/* LED Power green off */
		gpio_direction_output(4, 1);
		break;
	case CF_FAILED:
		/* LED Power green off */
		gpio_direction_output(4, 1);
		break;
	}
}
