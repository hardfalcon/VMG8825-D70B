/*
 * Copyright (C) 2015-2019 Sphairon GmbH (a ZyXEL company)
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
#include <asm/lantiq/reset.h>
#include <asm/arch/gphy.h>
#include <sas/controlfile.h>
#include <sas/etl.h>

#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_SYS_BOOT_RAM)
#define do_gpio_init	1
#else
#define do_gpio_init	0
#endif

static void gpio_init(void)
{
	/* GPIO button board reset (low-active) */
	gpio_direction_input(17);
	/* GPIO button WPS enable (low-active) */
	gpio_direction_input(15);
	/* GPIO button DC enable (low-active) */
	gpio_direction_input(1);
	/* GPIO button POWER (hi-active) */
	gpio_direction_input(9);

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
	return 0;
}

static const struct ltq_eth_port_config eth_port_config[] = {
	/* GMAC0: unused */
	{ 0, 0x0, LTQ_ETH_PORT_NONE, PHY_INTERFACE_MODE_NONE },
	/* GMAC1: internal GPHY0 with 1000 firmware for LAN1 */
	{ 1, 0x1, LTQ_ETH_PORT_PHY, PHY_INTERFACE_MODE_GMII },
	/* GMAC2: internal GPHY1 with 1000 firmware for LAN3 */
	{ 2, 0x2, LTQ_ETH_PORT_PHY, PHY_INTERFACE_MODE_GMII },
	/* GMAC3: internal GPHY2 with 1000 firmware for M2M */
	{ 3, 0x3, LTQ_ETH_PORT_PHY, PHY_INTERFACE_MODE_GMII },
	/* GMAC4: internal GPHY3 with 1000 firmware for LAN2 */
	{ 4, 0x4, LTQ_ETH_PORT_PHY, PHY_INTERFACE_MODE_GMII },
	/* GMAC5: RTL8211FS 10/100/1000 RJ45/Fiber port */
	{ 5, 0x5, LTQ_ETH_PORT_PHY, PHY_INTERFACE_MODE_RGMII },
};

static const struct ltq_eth_board_config eth_board_config = {
	.ports = eth_port_config,
	.num_ports = ARRAY_SIZE(eth_port_config),
};

int board_eth_init(bd_t * bis)
{
	const enum ltq_gphy_clk clk = LTQ_GPHY_CLK_25MHZ_PLL0;
	void *fw_addr;

	/* Assert GPHY reset for at least 72ms */
	ltq_reset_once(LTQ_RESET_HARD, 100000);

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
	return 0 == gpio_get_value(17);
}

void sas_cf_led_action(enum sas_cf_state state)
{
	switch (state) {
	case CF_STARTED:
		/* LED Power green on */
		gpio_direction_output(5, 1);
		break;
	case CF_FINISHED:
		/* LED Power green off */
		gpio_direction_output(5, 0);
		break;
	case CF_FAILED:
		/* LED Power red on */
		gpio_direction_output(10, 1);
		/* LED Power green off */
		gpio_direction_output(5, 0);
		break;
	}
}

#if defined(CONFIG_SPHAIRON_ZYCLIP)
int sas_board_early_init_r(void)
{
	sas_etl_set_integer(SAS_ETL_BASE_PLATFORM, 520);

#if defined(CONFIG_SPHAIRON_FLASHLAYOUT)
	sas_etl_set_string(SAS_ETL_FLASH_LAYOUT, CONFIG_SPHAIRON_FLASHLAYOUT);
#endif

	/* on known device, set predefined ETL_TRIV_NAME */
	sas_etl_set_value(SAS_ETL_TRIV_NAME, "FMG3542-D10A");

	return 0;
}
#endif
