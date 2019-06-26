/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2012 John Crispin <blogic@openwrt.org>
 *
 */

#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of_platform.h>
#include <linux/mutex.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/of_gpio.h>
#include <linux/clk.h>
#include <linux/err.h>

#include <lantiq_soc.h>

/*
 * The Serial To Parallel (STP) is found on MIPS based Lantiq socs. It is a
 * peripheral controller used to drive external shift register cascades. At most
 * 3 groups of 8 bits can be driven. The hardware is able to allow the DSL modem
 * to drive the 2 LSBs of the cascade automatically.
 */

/* control register 0 */
#define XWAY_STP_CON0		0x00
/* control register 1 */
#define XWAY_STP_CON1		0x04
/* data register 0 */
#define XWAY_STP_CPU0		0x08
/* data register 1 */
#define XWAY_STP_CPU1		0x0C
/* access register */
#define XWAY_STP_AR		0x10

/* software or hardware update select bit */
#define XWAY_STP_CON_SWU	BIT(31)

/* automatic update rates */
#define XWAY_STP_2HZ		0
#define XWAY_STP_4HZ		1
#define XWAY_STP_8HZ		2
#define XWAY_STP_10HZ		3
#define XWAY_STP_SPEED_SHIFT	23
#define XWAY_STP_SPEED_MASK	0x1f

/* clock source for automatic update */
#define XWAY_STP_UPD_FPI	BIT(31)
#define XWAY_STP_UPD_MASK	(BIT(31) | BIT(30))

#define XWAY_STP_FPID_VALUE	BIT(23)|BIT(24)
#define XWAY_STP_FPID_MASK	(BIT(23) | BIT(24)| BIT(25)| BIT(26)| BIT(27))

#define XWAY_STP_FPIS_VALUE	BIT(21)
#define XWAY_STP_FPIS_MASK	(BIT(20) | BIT(21))
/* let the adsl core drive the 2 LSBs */
#define XWAY_STP_ADSL_SHIFT	24
#define XWAY_STP_ADSL_MASK	0x3

/* 2 groups of 3 bits can be driven by the phys */
#define XWAY_STP_PHY_MASK	0x7
#define XWAY_STP_PHY1_SHIFT	27
#define XWAY_STP_WIFI_MASK	0x3f
#define XWAY_STP_WIFI_SHIFT	9

/* STP has 3 groups of 8 bits */
#define XWAY_STP_GROUP0		BIT(0)
#define XWAY_STP_GROUP1		BIT(1)
#define XWAY_STP_GROUP2		BIT(2)
#define XWAY_STP_GROUP_MASK	(0x7)

/* Edge configuration bits */
#define XWAY_STP_FALLING	BIT(26)
#define XWAY_STP_EDGE_MASK	BIT(26)

#define xway_stp_r32(m, reg)		__raw_readl(m + reg)
#define xway_stp_w32(m, val, reg)	__raw_writel(val, m + reg)
#define xway_stp_w32_mask(m, clear, set, reg) \
		ltq_w32((ltq_r32(m + reg) & ~(clear)) | (set), \
		m + reg)

struct xway_stp_hwcfg {
	unsigned int num_phys;
	unsigned int phy2_shift;
	unsigned int phy3_shift;
	unsigned int phy4_shift;
	unsigned int has_wifi;
	unsigned int wifi_shift;
};

struct xway_stp {
	struct gpio_chip gc;
	void __iomem *virt;
	const struct xway_stp_hwcfg *hwcfg;
	u32 edge;	/* rising or falling edge triggered shift register */
	u32 shadow;	/* shadow the shift registers state */
	u32 reserved;	/* mask out the hw driven bits in gpio_request */
	u8 groups;	/* we can drive 1-3 groups of 8bit each */
	u8 dsl;		/* the 2 LSBs can be driven by the dsl core */
	u8 phy1;	/* 3 bits can be driven by phy1 */
	u8 phy2;	/* 3 bits can be driven by phy2 */
	u8 phy3;	/* 3 bits can be driven by phy3 */
	u8 phy4;	/* 3 bits can be driven by phy4 */
	u8 wifi;	/* 6 bits can be driven by on-chip wifi */
	u8 speed;	/* shift clock */
};

/**
 * xway_stp_set() - gpio_chip->set - set gpios.
 * @gc:     Pointer to gpio_chip device structure.
 * @gpio:   GPIO signal number.
 * @val:    Value to be written to specified signal.
 *
 * Set the shadow value and call ltq_ebu_apply.
 */
static void xway_stp_set(struct gpio_chip *gc, unsigned gpio, int val)
{
	struct xway_stp *chip =
		container_of(gc, struct xway_stp, gc);

	if (val)
		chip->shadow |= BIT(gpio);
	else
		chip->shadow &= ~BIT(gpio);
	xway_stp_w32(chip->virt, chip->shadow, XWAY_STP_CPU0);

	if (!chip->reserved)
		xway_stp_w32_mask(chip->virt, 0, XWAY_STP_CON_SWU, XWAY_STP_CON0);
}

static int xway_stp_get(struct gpio_chip *gc, unsigned gpio)
{
	struct xway_stp *chip =
		container_of(gc, struct xway_stp, gc);

	return chip->shadow & BIT(gpio);
}

/**
 * xway_stp_dir_out() - gpio_chip->dir_out - set gpio direction.
 * @gc:     Pointer to gpio_chip device structure.
 * @gpio:   GPIO signal number.
 * @val:    Value to be written to specified signal.
 *
 * Same as xway_stp_set, always returns 0.
 */
static int xway_stp_dir_out(struct gpio_chip *gc, unsigned gpio, int val)
{
	xway_stp_set(gc, gpio, val);

	return 0;
}

/**
 * xway_stp_request() - gpio_chip->request
 * @gc:     Pointer to gpio_chip device structure.
 * @gpio:   GPIO signal number.
 *
 * We mask out the HW driven pins
 */
static int xway_stp_request(struct gpio_chip *gc, unsigned gpio)
{
	struct xway_stp *chip =
		container_of(gc, struct xway_stp, gc);

	if (chip->reserved & BIT(gpio)) {
		dev_err(gc->dev, "GPIO %d is driven by hardware\n", gpio);
		return -ENODEV;
	}

	return 0;
}

/**
 * xway_stp_hw_init() - Configure the STP unit and enable the clock gate
 * @virt: pointer to the remapped register range
 */
static int xway_stp_hw_init(struct xway_stp *chip)
{
	const struct xway_stp_hwcfg *hwcfg = chip->hwcfg;

	/* sane defaults */
	xway_stp_w32(chip->virt, 0, XWAY_STP_AR);
	xway_stp_w32(chip->virt, chip->shadow, XWAY_STP_CPU0);
	xway_stp_w32(chip->virt, 0, XWAY_STP_CPU1);
	xway_stp_w32(chip->virt, XWAY_STP_CON_SWU, XWAY_STP_CON0);
	xway_stp_w32(chip->virt, 0, XWAY_STP_CON1);

	/* apply edge trigger settings for the shift register */
	xway_stp_w32_mask(chip->virt, XWAY_STP_EDGE_MASK,
				chip->edge, XWAY_STP_CON0);

	/* apply led group settings */
	xway_stp_w32_mask(chip->virt, XWAY_STP_GROUP_MASK,
				chip->groups, XWAY_STP_CON1);

	/* tell the hardware which pins are controlled by the dsl modem */
	xway_stp_w32_mask(chip->virt,
			XWAY_STP_ADSL_MASK << XWAY_STP_ADSL_SHIFT,
			chip->dsl << XWAY_STP_ADSL_SHIFT,
			XWAY_STP_CON0);

	/* tell the hardware which pins are controlled by the phys */
	if (hwcfg->num_phys) {
		xway_stp_w32_mask(chip->virt,
				XWAY_STP_PHY_MASK << XWAY_STP_PHY1_SHIFT,
				chip->phy1 << XWAY_STP_PHY1_SHIFT,
				XWAY_STP_CON0);
		xway_stp_w32_mask(chip->virt,
				XWAY_STP_PHY_MASK << hwcfg->phy2_shift,
				chip->phy2 << hwcfg->phy2_shift,
				XWAY_STP_CON1);
	}
	if (hwcfg->num_phys > 2) {
		xway_stp_w32_mask(chip->virt,
				XWAY_STP_PHY_MASK << hwcfg->phy3_shift,
				chip->phy3 << hwcfg->phy3_shift,
				XWAY_STP_CON1);
	}
	if (hwcfg->num_phys > 3) {
		xway_stp_w32_mask(chip->virt,
				XWAY_STP_PHY_MASK << hwcfg->phy4_shift,
				chip->phy4 << hwcfg->phy4_shift,
				XWAY_STP_CON1);
	}

	/* mask out the hw driven bits in gpio_request */
	chip->reserved = (chip->phy4 << 17) | (chip->phy3 << 8) |
			(chip->phy2 << 5) | (chip->phy1 << 2) | chip->dsl;

	/*
	 * if we have pins that are driven by hw, we need to tell the stp what
	 * clock to use as a timer.
	 */
	if (chip->reserved) {
		xway_stp_w32_mask(chip->virt, XWAY_STP_UPD_MASK,
			XWAY_STP_UPD_FPI, XWAY_STP_CON1);
		xway_stp_w32_mask(chip->virt,
			XWAY_STP_SPEED_MASK << XWAY_STP_SPEED_SHIFT,
			chip->speed << XWAY_STP_SPEED_SHIFT, XWAY_STP_CON1);
	}

	return 0;
}

static const struct xway_stp_hwcfg hwcfg_xway = {
	.num_phys = 0,
};

static const struct xway_stp_hwcfg hwcfg_xr9 = {
	.num_phys = 2,
	.phy2_shift = 15,
};

static const struct xway_stp_hwcfg hwcfg_xr10 = {
	.num_phys = 4,
	.phy2_shift = 3,
	.phy3_shift = 6,
	.phy4_shift = 15,
	.has_wifi = 1,
};

static const struct of_device_id xway_stp_match[] = {
	{ .compatible = "lantiq,gpio-stp-xway", .data = &hwcfg_xway },
	{ .compatible = "lantiq,gpio-stp-xr9", .data = &hwcfg_xr9 },
	{ .compatible = "lantiq,gpio-stp-xr10", .data = &hwcfg_xr10 },
	{},
};
MODULE_DEVICE_TABLE(of, xway_stp_match);

static int xway_stp_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	const __be32 *shadow, *groups, *speed, *dsl, *phy, *wifi;
	const struct xway_stp_hwcfg *hwcfg;
	const struct of_device_id *match;
	struct xway_stp *chip;
	struct clk *clk;
	int ret = 0;

	if (!res) {
		dev_err(&pdev->dev, "failed to request STP resource\n");
		return -ENOENT;
	}

	match = of_match_device(xway_stp_match, &pdev->dev);
	if (!match) {
		dev_err(&pdev->dev, "no device match\n");
		return -EINVAL;
	}
	hwcfg = match->data;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->virt = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(chip->virt))
		return PTR_ERR(chip->virt);

	chip->hwcfg = hwcfg;
	chip->gc.dev = &pdev->dev;
	chip->gc.label = "stp-xway";
	chip->gc.direction_output = xway_stp_dir_out;
	chip->gc.set = xway_stp_set;
	chip->gc.get = xway_stp_get;
	chip->gc.request = xway_stp_request;
	chip->gc.base = -1;
	chip->gc.owner = THIS_MODULE;

	/* store the shadow value if one was passed by the devicetree */
	shadow = of_get_property(pdev->dev.of_node, "lantiq,shadow", NULL);
	if (shadow)
		chip->shadow = be32_to_cpu(*shadow);

	/* find out which gpio groups should be enabled */
	groups = of_get_property(pdev->dev.of_node, "lantiq,groups", NULL);
	if (groups)
		chip->groups = be32_to_cpu(*groups) & XWAY_STP_GROUP_MASK;
	else
		chip->groups = XWAY_STP_GROUP0;
	chip->gc.ngpio = fls(chip->groups) * 8;

	/* find out speed of shift clock (2, 4, 8 or 10 Hz) */
	speed = of_get_property(pdev->dev.of_node, "lantiq,speed", NULL);
	if (speed) {
		switch (be32_to_cpu(*speed)) {
		case 2:
			chip->speed = XWAY_STP_2HZ;
			break;
		case 4:
			chip->speed = XWAY_STP_4HZ;
			break;
		case 8:
			chip->speed = XWAY_STP_8HZ;
			break;
		case 10:
			chip->speed = XWAY_STP_10HZ;
			break;
		default:
			chip->speed = XWAY_STP_2HZ;
			break;
		}
	} else {
		chip->speed = XWAY_STP_2HZ;
	}

	/* find out which gpios are controlled by the dsl core */
	dsl = of_get_property(pdev->dev.of_node, "lantiq,dsl", NULL);
	if (dsl)
		chip->dsl = be32_to_cpu(*dsl) & XWAY_STP_ADSL_MASK;

	/* find out which gpios are controlled by the phys */
	if (hwcfg->num_phys) {
		phy = of_get_property(pdev->dev.of_node, "lantiq,phy1", NULL);
		if (phy)
			chip->phy1 = be32_to_cpu(*phy) & XWAY_STP_PHY_MASK;
		phy = of_get_property(pdev->dev.of_node, "lantiq,phy2", NULL);
		if (phy)
			chip->phy2 = be32_to_cpu(*phy) & XWAY_STP_PHY_MASK;
	}
	if (hwcfg->num_phys > 2) {
		phy = of_get_property(pdev->dev.of_node, "lantiq,phy3", NULL);
		if (phy)
			chip->phy3 = be32_to_cpu(*phy) & XWAY_STP_PHY_MASK;
	}
	if (hwcfg->num_phys > 3) {
		phy = of_get_property(pdev->dev.of_node, "lantiq,phy4", NULL);
		if (phy)
			chip->phy4 = be32_to_cpu(*phy) & XWAY_STP_PHY_MASK;
	}
	if (hwcfg->has_wifi) {
		wifi = of_get_property(pdev->dev.of_node, "lantiq,wifi", NULL);
		if (wifi)
			chip->wifi = be32_to_cpu(*wifi) & XWAY_STP_WIFI_MASK;
	}

	/* check which edge trigger we should use, default to a falling edge */
	if (!of_find_property(pdev->dev.of_node, "lantiq,rising", NULL))
		chip->edge = XWAY_STP_FALLING;

	clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "Failed to get clock\n");
		return PTR_ERR(clk);
	}
	clk_enable(clk);

	ret = xway_stp_hw_init(chip);
	if (!ret)
		ret = gpiochip_add(&chip->gc);

	if (!ret)
		dev_info(&pdev->dev, "Init done\n");

	return ret;
}

static struct platform_driver xway_stp_driver = {
	.probe = xway_stp_probe,
	.driver = {
		.name = "gpio-stp-xway",
		.owner = THIS_MODULE,
		.of_match_table = xway_stp_match,
	},
};

int __init xway_stp_init(void)
{
	return platform_driver_register(&xway_stp_driver);
}

core_initcall_sync(xway_stp_init);
