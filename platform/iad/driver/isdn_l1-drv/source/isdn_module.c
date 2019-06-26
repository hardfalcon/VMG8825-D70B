/*
 * (C) Copyright 2005-2017 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/sched.h>
#include <linux/slab.h>

#if defined(CONFIG_OF)
#include <linux/of.h>
#else
#include <ifx_devices.h>
#endif

#include "isdn_device.h"
#include "isdn_isacsx.h"
#include "isdn_l1.h"
#include "pcm_clk_gen.h"

#define MODULE_NAME			"isdnl1"

static bool isdnl1_skip_sres;
static bool isdnl1_swap_lts_te;
static bool isdnl1_swap_lts_ltt;
static bool isdnl1_pcm_clk_slave;

static struct workqueue_struct *isdnl1_wq;
static int isdnl1_dev_major;
static struct class *isdnl1_class;
static atomic_t isdnl1_irq_user = ATOMIC_INIT(0);
static int isdnl1_irq;

static ssize_t isdnl1_l1state_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	const struct isdnl1 *isdnl1 = dev_get_drvdata(dev->parent);
	const struct FsmInst *fi = &isdnl1->l1_fsm;

	return sprintf(buf, "%s\n", fi->fsm->strState[fi->state]);
}

static ssize_t isdnl1_l1active_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	const struct isdnl1 *isdnl1 = dev_get_drvdata(dev->parent);

	return sprintf(buf, "%d\n", test_bit(ISDNL1_ACTIVATED, &isdnl1->flags));
}

static ssize_t isdnl1_tr_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	const struct isdnl1 *isdnl1 = dev_get_drvdata(dev->parent);

	if (test_bit(ISDNL1_MODE_TE, &isdnl1->flags))
		return sprintf(buf, "TE\n");
	if (test_bit(ISDNL1_MODE_LTT, &isdnl1->flags))
		return sprintf(buf, "LT-T\n");
	if (test_bit(ISDNL1_MODE_NT, &isdnl1->flags))
		return sprintf(buf, "NT\n");
	if (test_bit(ISDNL1_MODE_LTS, &isdnl1->flags))
		return sprintf(buf, "LT-S\n");

	return -EINVAL;
}

static ssize_t isdnl1_line_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	const struct isdnl1 *isdnl1 = dev_get_drvdata(dev->parent);
	bool state = isacsx_has_power_src1(isdnl1);

	return sprintf(buf, "%d\n", state);
}

static struct device_attribute isdnl1_dev_attrs[] = {
	__ATTR(l1_state, S_IRUGO, isdnl1_l1state_show, NULL),
	__ATTR(l1_active, S_IRUGO, isdnl1_l1active_show, NULL),
	__ATTR(tr_mode, S_IRUGO, isdnl1_tr_mode_show, NULL),
	__ATTR(line, S_IRUGO, isdnl1_line_show, NULL),
	__ATTR_NULL,
};

static int isdnl1_open(struct inode *inode, struct file *file)
{
	struct cdev *cdev = inode->i_cdev;
	struct isdnl1 *isdnl1 =
		container_of(cdev, struct isdnl1, cdev);

	file->private_data = isdnl1;
	atomic_inc(&isdnl1->use_cnt);

	return nonseekable_open(inode, file);
}

static int isdnl1_release(struct inode *inode, struct file *file)
{
	struct isdnl1 *isdnl1 = file->private_data;

	if (atomic_dec_and_test(&isdnl1->use_cnt))
		file->private_data = NULL;

	return 0;
}

static ssize_t isdnl1_read(struct file *file, char __user *buf,
	size_t len, loff_t *off)
{
	struct isdnl1 *isdnl1 = file->private_data;
	unsigned int copied;
	int err;

	if (mutex_lock_interruptible(&isdnl1->mtx))
		return -ERESTARTSYS;

	while (kfifo_is_empty(&isdnl1->rx_msg)) {
		mutex_unlock(&isdnl1->mtx);

		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		if (wait_event_interruptible(isdnl1->rx_wait,
			kfifo_is_empty(&isdnl1->rx_msg)))
			return -ERESTARTSYS;

		if (mutex_lock_interruptible(&isdnl1->mtx))
			return -ERESTARTSYS;
	}

	err = kfifo_to_user(&isdnl1->rx_msg, buf, len, &copied);

	mutex_unlock(&isdnl1->mtx);

	return err ? err : copied;
}

static ssize_t isdnl1_write(struct file *file, const char __user *buf,
	size_t len, loff_t *off)
{
	struct isdnl1 *isdnl1 = file->private_data;
	unsigned int copied;
	int err;

	if (len != sizeof(L1MSG))
		return -EMSGSIZE;

	if (mutex_lock_interruptible(&isdnl1->mtx))
		return -ERESTARTSYS;

	if (!test_bit(ISDNL1_ACTIVATED, &isdnl1->flags)) {
		mutex_unlock(&isdnl1->mtx);
		dev_err(&isdnl1->sdev->dev, "L1 is not activated\n");
		return -ESHUTDOWN;
	}

	while (kfifo_is_full(&isdnl1->tx_msg)) {
		mutex_unlock(&isdnl1->mtx);

		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		if (wait_event_interruptible(isdnl1->tx_wait,
			kfifo_is_full(&isdnl1->tx_msg)))
			return -ERESTARTSYS;

		if (mutex_lock_interruptible(&isdnl1->mtx))
			return -ERESTARTSYS;
	}

	err = kfifo_from_user(&isdnl1->tx_msg, buf, len, &copied);

	if (!test_and_set_bit(ISDNL1_TX_BUSY, &isdnl1->flags))
		isacsx_send_tx_frame(isdnl1);

	mutex_unlock(&isdnl1->mtx);

	return err ? err : copied;
}

static long isdnl1_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct isdnl1 *isdnl1 = file->private_data;
	int err;

	mutex_lock(&isdnl1->mtx);

	switch (cmd) {
	case IOCTL_L1DEV_GET_INFO:
		err = isacsx_ioctl_get_info(isdnl1, arg);
		break;
	case IOCTL_L1DEV_SET_CONFIG:
		err = isacsx_ioctl_set_config(isdnl1, arg);
		break;
	case IOCTL_L1DEV_GET_CONFIG:
		err = isacsx_ioctl_get_config(isdnl1, arg);
		break;
	case IOCTL_L1DEV_SET_L1STATE:
		err = isacsx_ioctl_set_l1state(isdnl1, arg);
		break;
	case IOCTL_L1DEV_GET_L1STATE:
		err = isacsx_ioctl_get_l1state(isdnl1, arg);
		break;
	case IOCTL_L1DEV_SET_HWTEST:
		err = isacsx_ioctl_set_hwtest(isdnl1, arg);
		break;
	case IOCTL_L1DEV_GET_STATUS:
		err = isacsx_ioctl_get_status(isdnl1, arg);
		break;
	case IOCTL_L1DEV_SET_LEDSTATE:
		err = isacsx_ioctl_set_ledstatus(isdnl1, arg);
		break;
	case IOCTL_L1DEV_READ_REG:
		err = isacsx_ioctl_read_reg(isdnl1, arg);
		break;
	case IOCTL_L1DEV_WRITE_REG:
		err = isacsx_ioctl_write_reg(isdnl1, arg);
		break;
	case IOCTL_L1DEV_ENABLE:
		err = isacsx_ioctl_enable(isdnl1);
		break;
	case IOCTL_L1DEV_DISABLE:
		err = isacsx_ioctl_disable(isdnl1);
		break;
	case IOCTL_L1DEV_SET_BUSMODE:
		err = isacsx_ioctl_set_busmode(isdnl1, arg);
		break;
	default:
		err = -EINVAL;
		break;
	}

	mutex_unlock(&isdnl1->mtx);

	return err;
}

static unsigned int isdnl1_poll(struct file *file, poll_table *pt)
{
	unsigned int mask = 0;
	struct isdnl1 *isdnl1 = file->private_data;

	poll_wait(file, &isdnl1->rx_wait, pt);
	poll_wait(file, &isdnl1->tx_wait, pt);

	if (!kfifo_is_empty(&isdnl1->rx_msg))
		mask |= POLLIN | POLLRDNORM;

	if (!kfifo_is_full(&isdnl1->tx_msg))
		mask |= POLLOUT | POLLWRNORM;

	return mask;
}

static const struct file_operations isdnl1_fops = {
	.open = isdnl1_open,
	.release = isdnl1_release,
	.read = isdnl1_read,
	.write = isdnl1_write,
	.unlocked_ioctl = isdnl1_ioctl,
	.poll = isdnl1_poll,
};

static int isdnl1_cdev_init(void)
{
	dev_t dev;
	int err;

	err = alloc_chrdev_region(&dev, 0, MAX_LAYER1_DEVICES, MODULE_NAME);
	if (err)
		return err;

	isdnl1_dev_major = MAJOR(dev);
	return 0;
}

static void isdnl1_cdev_exit(void)
{
	unregister_chrdev_region(MKDEV(isdnl1_dev_major, 0), MAX_LAYER1_DEVICES);
}

static int isdnl1_cdev_add(struct isdnl1 *isdnl1)
{
	struct device *dev;
	int err;

	cdev_init(&isdnl1->cdev, &isdnl1_fops);
	err = cdev_add(&isdnl1->cdev,
		MKDEV(isdnl1_dev_major, isdnl1->cdev_minor), 1);
	if (err)
		return err;

	dev = device_create(isdnl1_class, &isdnl1->sdev->dev,
		MKDEV(isdnl1_dev_major, isdnl1->cdev_minor), isdnl1,
		"%s%d", MODULE_NAME, isdnl1->cdev_minor);
	if (IS_ERR(dev)) {
		cdev_del(&isdnl1->cdev);
		return PTR_ERR(dev);
	}

	return 0;
}

static void isdnl1_cdev_del(struct isdnl1 *isdnl1)
{
	device_destroy(isdnl1_class,
		MKDEV(isdnl1_dev_major, isdnl1->cdev_minor));
	cdev_del(&isdnl1->cdev);
}

static int isdnl1_irq_dispatch(struct device *dev, void *data)
{
	struct isdnl1 *isdnl1 = dev_get_drvdata(dev->parent);

	isacsx_irq_dispatch(isdnl1);
	return 0;
}

static void isdnl1_irq_worker(struct work_struct *work)
{
	class_for_each_device(isdnl1_class, NULL, NULL, isdnl1_irq_dispatch);
	enable_irq(isdnl1_irq);
}
static DECLARE_WORK(isdnl1_irq_work, isdnl1_irq_worker);

static irqreturn_t isdnl1_irq_handler(int irq, void *data)
{
	disable_irq_nosync(irq);
	queue_work(isdnl1_wq, &isdnl1_irq_work);

	return IRQ_HANDLED;
}

static void isdnl1_ph_event(struct isdnl1 *isdnl1, enum ph_event ev)
{
	L1MSG msg;

	memset(&msg, 0, sizeof(msg));

	switch (ev) {
	case PH_ACT_IND:
		dev_info(&isdnl1->sdev->dev, "L1: PH_ACTIVATE\n");
		msg.dwStatus = PH_ACTIVATE | L1_INDICATION;
		break;
	case PH_DEACT_IND:
		dev_info(&isdnl1->sdev->dev, "L1: PH_DEACTIVATE\n");
		msg.dwStatus = PH_DEACTIVATE | L1_INDICATION;
		break;
	case PH_DATA_IND:
		msg.dwStatus = PH_DATA | L1_INDICATION;
		break;
	case PH_DATA_CNF:
		msg.dwStatus = PH_DATA | L1_CONFIRM;
		break;
	default:
		return;
	}

	if (0 == atomic_read(&isdnl1->use_cnt))
		return;

	kfifo_put(&isdnl1->rx_msg, msg);
	wake_up_interruptible(&isdnl1->rx_wait);
}

static void isdnl1_l1_cmd(struct work_struct *work)
{
	struct isdnl1 *isdnl1 = container_of(work, struct isdnl1, l1_cmd_work);
	unsigned int len, cmd;

	len = kfifo_get(&isdnl1->l1_cmd_queue, &cmd);
	while (len) {
		isacsx_hwcmd(isdnl1, cmd);
		len = kfifo_get(&isdnl1->l1_cmd_queue, &cmd);
	}
}

void l1_cmd(struct isdnl1 *isdnl1, unsigned int cmd)
{
	isacsx_hwcmd(isdnl1, cmd);
}

void l1_cmd_queued(struct isdnl1 *isdnl1, unsigned int cmd)
{
	kfifo_put(&isdnl1->l1_cmd_queue, cmd);
	queue_work(isdnl1_wq, &isdnl1->l1_cmd_work);
}

#if defined(CONFIG_OF)
static int isdnl1_parse_dt(const struct device *dev, struct isdnl1 *isdnl1)
{
	const struct device_node *np = dev->of_node;
	u32 val;

	if (of_property_read_bool(np, "lantiq,mode-lts")) {
		if (isdnl1_swap_lts_te) {
			dev_info(dev, "force LT-S -> TE mode swap\n");
			set_bit(ISDNL1_MODE_TE, &isdnl1->flags);
			set_bit(ISDNL1_NO_PWR_SRC1, &isdnl1->flags);
		} else if (isdnl1_swap_lts_ltt) {
			dev_info(dev, "force LT-S -> LT-T mode swap\n");
			set_bit(ISDNL1_MODE_LTT, &isdnl1->flags);
			set_bit(ISDNL1_NO_PWR_SRC1, &isdnl1->flags);
		} else {
			dev_info(dev, "LTS mode\n");
			set_bit(ISDNL1_MODE_LTS, &isdnl1->flags);
		}
	}

	if (of_property_read_bool(np, "lantiq,mode-te")) {
		dev_info(dev, "TE mode\n");
		set_bit(ISDNL1_MODE_TE, &isdnl1->flags);
	}

	if (of_property_read_bool(np, "lantiq,mode-ltt")) {
		dev_info(dev, "LTT mode\n");
		set_bit(ISDNL1_MODE_LTT, &isdnl1->flags);
	}

	if (!test_bit(ISDNL1_MODE_LTS, &isdnl1->flags) &&
	    !test_bit(ISDNL1_MODE_TE, &isdnl1->flags) &&
	    !test_bit(ISDNL1_MODE_LTT, &isdnl1->flags)) {
		dev_err(dev, "Missing property: lantiq,mode-[lts|ltt|te]\n");
		return -EINVAL;
	}

	if (of_property_read_bool(np, "lantiq,dps"))
		set_bit(ISDNL1_DPS, &isdnl1->flags);

	if (of_property_read_u32(np, "lantiq,iom2-channel", &val)) {
		dev_err(dev, "Missing property: lantiq,iom2-channel\n");
		return -EINVAL;
	}
	isdnl1->iom2_channel = val;

	if (of_property_read_u32(np, "lantiq,pwr-src-pin", &val))
		set_bit(ISDNL1_NO_PWR_SRC1, &isdnl1->flags);
	else
		isdnl1->pwr_src_pin = val;

	return 0;
}
#else
static int isdnl1_parse_platdata(const struct device *dev, struct isdnl1 *isdnl1)
{
	struct peb3086_platform_data *pdata = dev_get_platdata(dev);

	if (!pdata) {
		dev_err(dev, "Missing platform data\n");
		return -EINVAL;
	}

	switch (pdata->mode) {
	case MODE_LTS:
		if (isdnl1_swap_lts_te) {
			dev_info(dev, "force LT-S -> TE mode swap\n");
			set_bit(ISDNL1_MODE_TE, &isdnl1->flags);
			set_bit(ISDNL1_NO_PWR_SRC1, &isdnl1->flags);
		} else if (isdnl1_swap_lts_ltt) {
			dev_info(dev, "force LT-S -> LT-T mode swap\n");
			set_bit(ISDNL1_MODE_LTT, &isdnl1->flags);
			set_bit(ISDNL1_NO_PWR_SRC1, &isdnl1->flags);
		} else {
			dev_info(dev, "LTS mode\n");
			set_bit(ISDNL1_MODE_LTS, &isdnl1->flags);
		}
		break;
	case MODE_TE:
		dev_info(dev, "TE mode\n");
		set_bit(ISDNL1_MODE_TE, &isdnl1->flags);
		break;
	case MODE_LTT:
		dev_info(dev, "LTT mode\n");
		set_bit(ISDNL1_MODE_LTT, &isdnl1->flags);
		break;
	default:
		dev_err(dev, "Invalid mode\n");
		return -EINVAL;
	}

	if (pdata->dps)
		set_bit(ISDNL1_DPS, &isdnl1->flags);

	isdnl1->iom2_channel = pdata->iom2_chan;

	if (pdata->pwr_src_pin)
		isdnl1->pwr_src_pin = pdata->pwr_src_pin;
	else
		set_bit(ISDNL1_NO_PWR_SRC1, &isdnl1->flags);

	return 0;
}
#endif

static int isdnl1_irq_init(struct isdnl1 *isdnl1)
{
	if (atomic_inc_return(&isdnl1_irq_user) > 1)
		return 0;

	isdnl1_irq = isdnl1->sdev->irq;
	return request_irq(isdnl1_irq, isdnl1_irq_handler,
		IRQF_TRIGGER_LOW, "isdnl1", NULL);
}

static void isdnl1_irq_exit(void)
{
	if (atomic_dec_return(&isdnl1_irq_user) == 0)
		free_irq(isdnl1_irq, NULL);
}

static int isdnl1_probe(struct spi_device *sdev)
{
	struct isdnl1 *isdnl1;
	int err;

	isdnl1 = kzalloc(sizeof(*isdnl1), GFP_KERNEL);
	if (!isdnl1)
		return -ENOMEM;

	isdnl1->sdev = sdev;
	spi_set_drvdata(sdev, isdnl1);

	atomic_set(&isdnl1->use_cnt, 0);
	mutex_init(&isdnl1->mtx);
	INIT_WORK(&isdnl1->l1_cmd_work, isdnl1_l1_cmd);
	INIT_KFIFO(isdnl1->tx_msg);
	INIT_KFIFO(isdnl1->rx_msg);
	INIT_KFIFO(isdnl1->l1_cmd_queue);
	init_waitqueue_head(&isdnl1->tx_wait);
	init_waitqueue_head(&isdnl1->rx_wait);

	isdnl1->cdev_minor = sdev->chip_select;
	isdnl1->ph_event_cb = isdnl1_ph_event;

#if defined(CONFIG_OF)
	err = isdnl1_parse_dt(&sdev->dev, isdnl1);
#else
	err = isdnl1_parse_platdata(&sdev->dev, isdnl1);
#endif
	if (err)
		goto err_isdnl1;

	dev_info(&sdev->dev, "IOM-2 channel %u DPS=%d\n",
		isdnl1->iom2_channel, test_bit(ISDNL1_DPS, &isdnl1->flags));

	if (isdnl1_skip_sres) {
		dev_info(&sdev->dev, "Skip SRES mode\n");
		set_bit(ISDNL1_SKIP_SRES, &isdnl1->flags);
	}


	err = isdnl1_irq_init(isdnl1);
	if (err)
		goto err_isdnl1;

	err = isdnl1_cdev_add(isdnl1);
	if (err)
		goto err_irq;

	err = l1_setup(&isdnl1->l1_fsm, &isdnl1->l1_timer, isdnl1);
	if (err)
		goto err_cdev;

	isdnl1->notifier.notifier_call = isacsx_pcm_clk_cb;
	err = pcm_clk_register_cb(&isdnl1->notifier);
	if (err && err != -ENODEV)
		goto err_cdev;

	isacsx_hwinit(isdnl1);

	return 0;

err_cdev:
	isdnl1_cdev_del(isdnl1);
err_irq:
	isdnl1_irq_exit();
err_isdnl1:
	kfree(isdnl1);
	return err;
}

static int isdnl1_remove(struct spi_device *sdev)
{
	struct isdnl1 *isdnl1 = spi_get_drvdata(sdev);

	pcm_clk_unregister_cb(&isdnl1->notifier);
	l1_event(isdnl1, MPH_DEACT_REQ);

	flush_work(&isdnl1->l1_cmd_work);
	isacsx_ioctl_disable(isdnl1);
	isdnl1_irq_exit();
	isdnl1_cdev_del(isdnl1);
	kfree(isdnl1);

	return 0;
}

static const struct spi_device_id isdnl1_ids[] = {
	{ "peb3086", CHIP_PEB3086 },
	{ "psb21150", CHIP_PSB21150 },
	{ }
};
MODULE_DEVICE_TABLE(spi, isdnl1_ids);

static struct spi_driver isdnl1_driver = {
	.probe = isdnl1_probe,
	.remove = isdnl1_remove,
	.id_table = isdnl1_ids,
	.driver = {
		.name = "isdnl1",
		.owner = THIS_MODULE,
	},
};

static int __init isdnl1_init(void)
{
	int ret;

	ret = l1_init();
	if (ret)
		return ret;

	isdnl1_class = class_create(THIS_MODULE, MODULE_NAME);
	if (IS_ERR(isdnl1_class)) {
		ret = PTR_ERR(isdnl1_class);
		goto err_isdn_l1;
	}
	isdnl1_class->dev_attrs = isdnl1_dev_attrs;

	ret = pcm_clk_gen_init(isdnl1_pcm_clk_slave);
	if (ret)
		goto err_class;

	ret = isdnl1_cdev_init();
	if (ret)
		goto err_pcm;

	isdnl1_wq = create_singlethread_workqueue("isdnl1_wq");
	if (!isdnl1_wq) {
		ret = -ENOMEM;
		goto err_dev_fs;
	}

	ret = spi_register_driver(&isdnl1_driver);
	if (ret)
		goto err_workqueue;

	atomic_inc(&isdnl1_irq_user);

	return 0;

err_workqueue:
	destroy_workqueue(isdnl1_wq);
err_dev_fs:
	isdnl1_cdev_exit();
err_pcm:
	pcm_clk_gen_exit();
err_class:
	class_destroy(isdnl1_class);
err_isdn_l1:
	l1_exit();

	return ret;
}

static void __exit isdnl1_exit(void)
{
	spi_unregister_driver(&isdnl1_driver);

	flush_work(&isdnl1_irq_work);
	isdnl1_irq_exit();

	destroy_workqueue(isdnl1_wq);
	isdnl1_cdev_exit();
	pcm_clk_gen_exit();
	class_destroy(isdnl1_class);
	l1_exit();
}

module_init(isdnl1_init);
module_exit(isdnl1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@sphairon.com>");
MODULE_DESCRIPTION("Sphairon ISDN L1 driver");

module_param_named(skip_sres, isdnl1_skip_sres, bool, 0444);
MODULE_PARM_DESC(skip_sres, "Skip soft reset of chip");
module_param_named(swap_lts_te, isdnl1_swap_lts_te, bool, 0444);
MODULE_PARM_DESC(swap_lts_te, "Swap device with LT-S mode to TE mode");
module_param_named(swap_lts_ltt, isdnl1_swap_lts_ltt, bool, 0444);
MODULE_PARM_DESC(swap_lts_ltt, "Swap device with LT-S mode to LT-T mode");
module_param_named(pcm_clk_slave, isdnl1_pcm_clk_slave, bool, 0444);
MODULE_PARM_DESC(pcm_clk_slave, "Enable PCM clock detection and control for PCM clock slave devices");
