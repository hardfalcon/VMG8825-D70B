/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2015 Daniel Schwierzeck <daniel.schwierzeck@gmail.com>
 */

#include <linux/of_platform.h>

#include <lantiq_soc.h>

static struct clk *ahb_clk;

static int ahb_probe(struct platform_device *pdev)
{
	ahb_clk = clk_get_sys(NULL, "ahb");
	if (IS_ERR(ahb_clk)) {
		dev_err(&pdev->dev, "failed to enable AHB bus\n");
		return PTR_ERR(ahb_clk);
	}
	clk_prepare_enable(ahb_clk);

	return 0;
}

static const struct of_device_id ahb_match[] = {
	{ .compatible = "lantiq,ahb-xway" },
	{},
};

static struct platform_driver ahb_driver = {
	.probe = ahb_probe,
	.driver = {
		.name = "ahb-xway",
		.owner = THIS_MODULE,
		.of_match_table = ahb_match,
	},
};

int __init ahb_init(void)
{
	return platform_driver_register(&ahb_driver);
}

arch_initcall(ahb_init);
