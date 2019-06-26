/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Copyright (C) 2012-2014 Daniel Schwierzeck <daniel.schwierzeck@sphairon.com>
 */

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/reset.h>
#include <linux/reboot.h>

#include <lantiq_soc.h>

#include "xway_switch.h"
#include "xway_switch_regs.h"

struct xway_gphy_hwcfg {
	bool check_soc_rev;
	size_t fw_align;
	const char *fw_name_template;
};

struct xway_gphy {
	struct device *dev;
	const struct xway_gphy_hwcfg *hwcfg;
	struct clk *gphy_clk_gate;
	struct reset_control *gphy_hreset;
	struct reset_control *gphy_sreset;
	unsigned int gphy_addr_base;
	void *fw_addr;
	size_t fw_size;
	dma_addr_t dev_addr;
	unsigned int phy_id;
	const char *phy_mode;
	struct notifier_block reboot_nb;
};

static int xway_gphy_reboot_notify(struct notifier_block *nb, unsigned long ev,
					void *data)
{
	struct xway_gphy *xway_gphy =
		container_of(nb, struct xway_gphy, reboot_nb);

	dev_dbg(xway_gphy->dev, "assert reset before reboot\n");
	reset_control_assert(xway_gphy->gphy_hreset);

	return NOTIFY_DONE;
}

static int xway_gphy_request_firmware(struct xway_gphy *xway_gphy)
{
	const struct xway_gphy_hwcfg *hwcfg = xway_gphy->hwcfg;
	const struct firmware *fw;
	char fw_name[64];
	void *fw_addr;
	int err;

	if (hwcfg->check_soc_rev)
		snprintf(fw_name, sizeof(fw_name), hwcfg->fw_name_template,
			xway_gphy->phy_mode, ltq_get_soc_rev());
	else
		snprintf(fw_name, sizeof(fw_name), hwcfg->fw_name_template,
			xway_gphy->phy_mode);

	err = request_firmware(&fw, fw_name, xway_gphy->dev);
	if (err)
		return err;

	xway_gphy->fw_addr = devm_kzalloc(xway_gphy->dev, fw->size + hwcfg->fw_align,
				GFP_KERNEL);
	if (!xway_gphy->fw_addr) {
		dev_err(xway_gphy->dev, "failed to alloc fw memory\n");
		release_firmware(fw);
		return -ENOMEM;
	}

	xway_gphy->fw_size = fw->size;
	fw_addr = PTR_ALIGN(xway_gphy->fw_addr, hwcfg->fw_align);
	memcpy(fw_addr, fw->data, fw->size);
	xway_gphy->dev_addr = dma_map_single(xway_gphy->dev, fw_addr, fw->size,
				DMA_TO_DEVICE);

	release_firmware(fw);

	return 0;
}

static int xway_gphy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct xway_gphy *xway_gphy;
	const struct xway_gphy_hwcfg *hwcfg;
	const struct of_device_id *match;
	const __be32 *offset;
	int err;

	match = of_match_device(dev->driver->of_match_table, dev);
	if (!match) {
		dev_err(dev, "no device match\n");
		return -EINVAL;
	}
	hwcfg = match->data;

	xway_gphy = devm_kzalloc(dev, sizeof(*xway_gphy), GFP_KERNEL);
	if (!xway_gphy)
		return -ENOMEM;

	xway_gphy->dev = dev;
	xway_gphy->hwcfg = hwcfg;

	err = of_property_read_string(np, "lantiq,phy-mode",
		&xway_gphy->phy_mode);
	if (err) {
		dev_err(dev, "no phy-mode property found\n");
		return -EINVAL;
	}

	offset = of_get_address(np, 0, NULL, NULL);
	if (!offset) {
		dev_err(dev, "failed to get GPHY address reg offset\n");
		return -ENOENT;
	}
	xway_gphy->gphy_addr_base = __be32_to_cpu(*offset);

	xway_gphy->gphy_clk_gate = devm_clk_get(dev, NULL);
	if (IS_ERR(xway_gphy->gphy_clk_gate)) {
		err = PTR_ERR(xway_gphy->gphy_clk_gate);
		switch (err) {
		case -ENOENT:
			xway_gphy->gphy_clk_gate = NULL;
			break;
		default:
			dev_err(dev, "failed to get gate clock\n");
			return PTR_ERR(xway_gphy->gphy_clk_gate);
		}
	}

	xway_gphy->gphy_hreset = devm_reset_control_get(dev, "hard");
	if (IS_ERR(xway_gphy->gphy_hreset)) {
		dev_err(dev, "failed to get hard reset control\n");
		return PTR_ERR(xway_gphy->gphy_hreset);
	}

	xway_gphy->gphy_sreset = devm_reset_control_get(dev, "soft");
	if (IS_ERR(xway_gphy->gphy_sreset)) {
		err = PTR_ERR(xway_gphy->gphy_sreset);
		switch (err) {
		case -EINVAL:
			xway_gphy->gphy_sreset = NULL;
			break;
		default:
			dev_err(dev, "failed to get soft reset control\n");
			return PTR_ERR(xway_gphy->gphy_sreset);
		}
	}

	if (xway_gphy->gphy_clk_gate) {
		err = clk_prepare_enable(xway_gphy->gphy_clk_gate);
		if (err)
			return err;
	}

	err = xway_gphy_request_firmware(xway_gphy);
	if (err)
		goto err_clk_disable;

	reset_control_assert(xway_gphy->gphy_hreset);
	if (xway_gphy->gphy_sreset)
		reset_control_deassert(xway_gphy->gphy_sreset);
	ltq_rcu_w32(xway_gphy->dev_addr, xway_gphy->gphy_addr_base);
	reset_control_deassert(xway_gphy->gphy_hreset);

	xway_gphy->reboot_nb.notifier_call = xway_gphy_reboot_notify;
	xway_gphy->reboot_nb.priority = -1;
	err = register_reboot_notifier(&xway_gphy->reboot_nb);
	if (err)
		dev_warn(dev, "failed to register reboot notifier\n");

	platform_set_drvdata(pdev, xway_gphy);

	return 0;

err_clk_disable:
	if (xway_gphy->gphy_clk_gate)
		clk_disable_unprepare(xway_gphy->gphy_clk_gate);

	return err;
}

static int xway_gphy_remove(struct platform_device *pdev)
{
	struct xway_gphy *xway_gphy = platform_get_drvdata(pdev);

	unregister_reboot_notifier(&xway_gphy->reboot_nb);

	reset_control_assert(xway_gphy->gphy_hreset);
	ltq_rcu_w32(0, xway_gphy->gphy_addr_base);

	if (xway_gphy->gphy_clk_gate)
		clk_disable_unprepare(xway_gphy->gphy_clk_gate);

	dma_unmap_single(xway_gphy->dev, xway_gphy->dev_addr,
		xway_gphy->fw_size, DMA_TO_DEVICE);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct xway_gphy_hwcfg gphy_xrx200 = {
	.check_soc_rev = 1,
	.fw_align = 16 * 1024,
	.fw_name_template = "lantiq/xrx200/phy%s_a%ux.bin",
};

static const struct xway_gphy_hwcfg gphy_xrx330 = {
	.check_soc_rev = 0,
	.fw_align = 16 * 1024,
	.fw_name_template = "lantiq/xrx330/phy%s_a21.bin",
};

static const struct of_device_id xway_gphy_match[] = {
	{ .compatible = "lantiq,xrx200-gphy", .data = &gphy_xrx200 },
	{ .compatible = "lantiq,xrx330-gphy", .data = &gphy_xrx330 },
	{},
};
MODULE_DEVICE_TABLE(of, xway_gphy_match);;

MODULE_FIRMWARE("lantiq/xrx200/phy11g_a1x.bin");
MODULE_FIRMWARE("lantiq/xrx200/phy11g_a2x.bin");
MODULE_FIRMWARE("lantiq/xrx200/phy22f_a1x.bin");
MODULE_FIRMWARE("lantiq/xrx200/phy22f_a2x.bin");
MODULE_FIRMWARE("lantiq/xrx330/phy11g_a21.bin");
MODULE_FIRMWARE("lantiq/xrx330/phy22f_a21.bin");

static struct platform_driver xway_gphy_driver = {
	.probe = xway_gphy_probe,
	.remove = xway_gphy_remove,
	.driver = {
		.name = "xway-rcu-gphy",
		.owner = THIS_MODULE,
		.of_match_table = xway_gphy_match,
	},
};

int xway_gphy_init(void)
{
	return platform_driver_register(&xway_gphy_driver);
}

void xway_gphy_exit(void)
{
	platform_driver_unregister(&xway_gphy_driver);
}
