/*
 * Copyright (C) 2017 Daniel Schwierzeck <daniel.schwierzeck@gmail.com>
 *
 * This program is free software; you can distribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/delay.h>
#include <linux/phy/phy.h>
#include <lantiq_soc.h>

/* Transmitter HS Pre-Emphasis Enable */
#define RCU_CFG1_TX_PEE		BIT(0)
/* Disconnect Threshold */
#define RCU_CFG1_DIS_THR_MASK	0x00038000
#define RCU_CFG1_DIS_THR_SHIFT	15

struct ltq_rcu_usb2_bits {
	u8 hostmode;
	u8 slave_endianness;
	u8 host_endianness;
	bool have_ana_cfg;
};

struct ltq_rcu_usb2_priv {
	struct device *dev;
	struct clk *phy_gate_clk;
	struct phy *phy;
	struct reset_control *phy_reset;
	struct reset_control *core_reset;
	const struct ltq_rcu_usb2_bits *reg_bits;
	unsigned int phy_reg_offset;
	unsigned int ana_cfg1_reg_offset;
};

static const struct ltq_rcu_usb2_bits xrx200_rcu_usb2_reg_bits = {
	.hostmode = 11,
	.slave_endianness = 9,
	.host_endianness = 10,
	.have_ana_cfg = true,
};

static const struct ltq_rcu_usb2_bits xrx330_rcu_usb2_reg_bits = {
	.hostmode = 11,
	.slave_endianness = 9,
	.host_endianness = 10,
	.have_ana_cfg = true,
};

static int ltq_rcu_usb2_phy_init(struct phy *phy)
{
	struct ltq_rcu_usb2_priv *priv = phy_get_drvdata(phy);

	if (priv->reg_bits->have_ana_cfg) {
		ltq_rcu_w32_mask(0, RCU_CFG1_TX_PEE, priv->ana_cfg1_reg_offset);
		ltq_rcu_w32_mask(RCU_CFG1_DIS_THR_MASK,
			7 << RCU_CFG1_DIS_THR_SHIFT, priv->ana_cfg1_reg_offset);
	}

	ltq_rcu_w32_mask(BIT(priv->reg_bits->hostmode), 0, priv->phy_reg_offset);

	ltq_rcu_w32_mask(BIT(priv->reg_bits->slave_endianness), 0,
		priv->phy_reg_offset);
	ltq_rcu_w32_mask(0, BIT(priv->reg_bits->host_endianness),
		priv->phy_reg_offset);

	if (priv->core_reset) {
		reset_control_assert(priv->core_reset);
		msleep(50);
		reset_control_deassert(priv->core_reset);
		msleep(50);
	}

	return 0;
}

static int ltq_rcu_usb2_phy_power_on(struct phy *phy)
{
	struct ltq_rcu_usb2_priv *priv = phy_get_drvdata(phy);
	int err;

	err = clk_prepare_enable(priv->phy_gate_clk);
	if (err)
		return err;

	msleep(50);
	reset_control_deassert(priv->phy_reset);

	return 0;
}

static int ltq_rcu_usb2_phy_power_off(struct phy *phy)
{
	struct ltq_rcu_usb2_priv *priv = phy_get_drvdata(phy);

	reset_control_assert(priv->phy_reset);
	clk_disable_unprepare(priv->phy_gate_clk);

	return 0;
}

static const struct phy_ops ltq_rcu_usb2_phy_ops = {
	.init		= ltq_rcu_usb2_phy_init,
	.power_on	= ltq_rcu_usb2_phy_power_on,
	.power_off	= ltq_rcu_usb2_phy_power_off,
	.owner		= THIS_MODULE,
};

static int ltq_rcu_usb2_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct ltq_rcu_usb2_priv *priv;
	struct phy_provider *provider;
	const struct of_device_id *match;
	const __be32 *offset;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = dev;

	match = of_match_device(dev->driver->of_match_table, dev);
	if (!match)
		return -EINVAL;
	priv->reg_bits = match->data;

	priv->phy_reset = devm_reset_control_get(dev, "phy");
	if (IS_ERR(priv->phy_reset))
		return PTR_ERR(priv->phy_reset);

	priv->core_reset = devm_reset_control_get(dev, "ctl");
	if (IS_ERR(priv->core_reset)) {
		int ret = PTR_ERR(priv->core_reset);
		switch (ret) {
		case -ENODEV:
		case -EINVAL:
			priv->core_reset = NULL;
			break;
		case -EPROBE_DEFER:
			return ret;
		default:
			dev_err(dev, "error getting global reset control %d\n", ret);
			return ret;
		}
	}

	priv->phy_gate_clk = devm_clk_get(dev, "phy");
	if (IS_ERR(priv->phy_gate_clk)) {
		dev_err(dev, "Unable to get USB phy gate clk\n");
		return PTR_ERR(priv->phy_gate_clk);
	}

	offset = of_get_address(node, 0, NULL, NULL);
	if (!offset) {
		dev_err(dev, "Failed to get RCU PHY reg offset\n");
		return -ENOENT;
	}
	priv->phy_reg_offset = __be32_to_cpu(*offset);

	if (priv->reg_bits->have_ana_cfg) {
		offset = of_get_address(dev->of_node, 1, NULL, NULL);
		if (!offset) {
			dev_err(dev, "Failed to get RCU ANA CFG1 reg offset\n");
			return -ENOENT;
		}
		priv->ana_cfg1_reg_offset = __be32_to_cpu(*offset);
	}

	priv->phy = devm_phy_create(dev, node, &ltq_rcu_usb2_phy_ops);
	if (IS_ERR(priv->phy))
		return PTR_ERR(priv->phy);
	phy_set_drvdata(priv->phy, priv);

	provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
	if (IS_ERR(provider))
		return PTR_ERR(provider);

	reset_control_assert(priv->phy_reset);

	platform_set_drvdata(pdev, priv);
	return 0;
}

static const struct of_device_id ltq_rcu_usb2_phy_of_match[] = {
	{
		.compatible = "lantiq,xrx200-usb2-phy",
		.data = &xrx200_rcu_usb2_reg_bits,
	}, {
		.compatible = "lantiq,xrx330-usb2-phy",
		.data = &xrx330_rcu_usb2_reg_bits,
	},
	{},
};
MODULE_DEVICE_TABLE(of, ltq_rcu_usb2_phy_of_match);

static struct platform_driver ltq_rcu_usb2_phy_driver = {
	.probe		= ltq_rcu_usb2_phy_probe,
	.driver		= {
		.name	= "lantiq-rcu-usb2-phy",
		.owner	= THIS_MODULE,
		.of_match_table = ltq_rcu_usb2_phy_of_match,
	},
};
module_platform_driver(ltq_rcu_usb2_phy_driver);

MODULE_DESCRIPTION("Lantiq USB 2.0 phy controller");
MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@gmail.com>");
MODULE_LICENSE("GPL");
