/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#include "precomp.h"
#include "gl_fw_log.h"
#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
#include "connv3_debug_utility.h"
#include "connsyslog/connv3_mcu_log.h"
#elif (CFG_SUPPORT_CONNINFRA == 1)
#include "connsys_debug_utility.h"
#endif

struct fw_log_wifi_interface fw_log_wifi_inf;

static int fw_log_wifi_open(struct inode *inode, struct file *file);
static int fw_log_wifi_release(struct inode *inode, struct file *file);
static ssize_t fw_log_wifi_read(struct file *filp, char __user *buf,
	size_t len, loff_t *off);
static unsigned int fw_log_wifi_poll(struct file *filp, poll_table *wait);
static long fw_log_wifi_unlocked_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg);
#ifdef CONFIG_COMPAT
static long fw_log_wifi_compat_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg);
#endif

const struct file_operations fw_log_wifi_fops = {
	.open = fw_log_wifi_open,
	.release = fw_log_wifi_release,
	.read = fw_log_wifi_read,
	.poll = fw_log_wifi_poll,
	.unlocked_ioctl = fw_log_wifi_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = fw_log_wifi_compat_ioctl,
#endif
};

void wifi_fwlog_event_func_register(void (*func)(int, int))
{
	struct fw_log_wifi_interface *prInf = &fw_log_wifi_inf;

	prInf->cb = func;
}

static int fw_log_wifi_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int fw_log_wifi_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t fw_log_wifi_read(struct file *filp, char __user *buf,
	size_t len, loff_t *off)
{
#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	return connv3_log_read_to_user(CONNV3_DEBUG_TYPE_WIFI, buf, len);
#elif (CFG_SUPPORT_CONNINFRA == 1)
	return connsys_log_read_to_user(CONN_DEBUG_TYPE_WIFI, buf, len);
#else
	return 0;
#endif
}

static unsigned int fw_log_wifi_poll(struct file *filp, poll_table *wait)
{
	struct fw_log_wifi_interface *prInf = &fw_log_wifi_inf;

	poll_wait(filp, &prInf->wq, wait);

#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	if (connv3_log_get_buf_size(CONNV3_DEBUG_TYPE_WIFI) > 0)
		return POLLIN | POLLRDNORM;
#elif (CFG_SUPPORT_CONNINFRA == 1)
	if (connsys_log_get_buf_size(CONN_DEBUG_TYPE_WIFI) > 0)
		return POLLIN | POLLRDNORM;
#endif

	return 0;
}

static long fw_log_wifi_unlocked_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	struct fw_log_wifi_interface *prInf = &fw_log_wifi_inf;
	int ret = 0;

	down(&prInf->ioctl_mtx);

	switch (cmd) {
	case WIFI_FW_LOG_IOCTL_ON_OFF:{
		int log_on_off = (int)arg;

		if (prInf->cb)
			prInf->cb(WIFI_FW_LOG_CMD_ON_OFF, log_on_off);
		break;
	}
	case WIFI_FW_LOG_IOCTL_SET_LEVEL:{
		int log_level = (int)arg;

		if (prInf->cb)
			prInf->cb(WIFI_FW_LOG_CMD_SET_LEVEL, log_level);

		break;
	}
	case WIFI_FW_LOG_IOCTL_GET_VERSION:{
		prInf->ver_length = 0;
		kalMemZero(prInf->ver_name, MANIFEST_BUFFER_SIZE);
		schedule_work(&prInf->getFwVerQ);
		flush_work(&prInf->getFwVerQ);

		if (copy_to_user((char *) arg, prInf->ver_name,
				 prInf->ver_length))
			ret = -EFAULT;

		DBGLOG(INIT, INFO, "ver_name=%s\n", prInf->ver_name);
		break;
	}
	default:
		ret = -EINVAL;
	}

	up(&prInf->ioctl_mtx);

	return ret;
}

#ifdef CONFIG_COMPAT
static long fw_log_wifi_compat_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	long ret = 0;

	if (!filp->f_op || !filp->f_op->unlocked_ioctl)
		return -ENOTTY;

	fw_log_wifi_unlocked_ioctl(filp, cmd, arg);

	return ret;
}
#endif

static void fw_log_wifi_inf_event_cb(void)
{
	struct fw_log_wifi_interface *prInf = &fw_log_wifi_inf;

	wake_up_interruptible(&prInf->wq);
}

static void fw_log_get_version_workQ(struct work_struct *work)
{
	struct fw_log_wifi_interface *prInf = &fw_log_wifi_inf;
	struct mt66xx_chip_info *prChipInfo;

	glGetChipInfo((void **)&prChipInfo);
	if (prChipInfo && prChipInfo->fw_dl_ops->getFwVerInfo)
		prChipInfo->fw_dl_ops->getFwVerInfo(prInf->ver_name,
			&prInf->ver_length, MANIFEST_BUFFER_SIZE);
}

uint32_t fw_log_notify_rcv(enum ENUM_FW_LOG_CTRL_TYPE type,
	uint8_t *buffer,
	uint32_t size)
{
	uint32_t written = 0;
#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	enum connv3_log_type eType = CONNV3_LOG_TYPE_PRIMARY;
#endif

#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	switch (type) {
	case ENUM_FW_LOG_CTRL_TYPE_MCU:
		eType = CONNV3_LOG_TYPE_MCU;
		break;
	case ENUM_FW_LOG_CTRL_TYPE_WIFI:
		eType = CONNV3_LOG_TYPE_PRIMARY;
		break;
	default:
		DBGLOG(INIT, ERROR, "invalid type: %d\n",
			type);
		break;
	}
	written = connv3_log_handler(CONNV3_DEBUG_TYPE_WIFI, eType,
		buffer, size);
	DBGLOG(INIT, TRACE,
		"[%d] connv3_log_handler written=%d\n",
		type,
		written);
#endif

	return written;
}

int fw_log_wifi_inf_init(void)
{
	struct fw_log_wifi_interface *prInf = &fw_log_wifi_inf;
	int ret = 0;

	ret = alloc_chrdev_region(&prInf->devno, 0, 1,
		FW_LOG_WIFI_INF_NAME);
	if (ret) {
		DBGLOG(INIT, ERROR,
			"alloc_chrdev_region failed, ret: %d\n",
			ret);
		goto return_fn;
	}

	cdev_init(&prInf->cdev, &fw_log_wifi_fops);
	prInf->cdev.owner = THIS_MODULE;

	ret = cdev_add(&prInf->cdev, prInf->devno, 1);
	if (ret) {
		DBGLOG(INIT, ERROR,
			"cdev_add failed, ret: %d\n",
			ret);
		goto unregister_chrdev_region;
	}

	prInf->driver_class = class_create(THIS_MODULE,
		FW_LOG_WIFI_INF_NAME);
	if (IS_ERR(prInf->driver_class)) {
		DBGLOG(INIT, ERROR,
			"class_create failed, ret: %d\n",
			ret);
		ret = PTR_ERR(prInf->driver_class);
		goto cdev_del;
	}

	prInf->class_dev = device_create(prInf->driver_class,
		NULL, prInf->devno, NULL, FW_LOG_WIFI_INF_NAME);
	if (IS_ERR(prInf->class_dev)) {
		ret = PTR_ERR(prInf->class_dev);
		DBGLOG(INIT, ERROR,
			"class_device_create failed, ret: %d\n",
			ret);
		goto class_destroy;
	}

#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	ret = connv3_log_init(CONNV3_DEBUG_TYPE_WIFI,
		RING_BUFFER_SIZE_WF_FW,
		RING_BUFFER_SIZE_WF_MCU,
		fw_log_wifi_inf_event_cb);
	if (ret) {
		DBGLOG(INIT, ERROR,
			"connv3_log_init failed, ret: %d\n",
			ret);
		device_destroy(prInf->driver_class, prInf->devno);
		goto class_destroy;
	}
#elif (CFG_SUPPORT_CONNINFRA == 1)
	ret = connsys_log_init(CONN_DEBUG_TYPE_WIFI);
	if (ret) {
		DBGLOG(INIT, ERROR,
			"connsys_log_init failed, ret: %d\n",
			ret);
		device_destroy(prInf->driver_class, prInf->devno);
		goto class_destroy;
	}
	connsys_log_register_event_cb(CONN_DEBUG_TYPE_WIFI,
		fw_log_wifi_inf_event_cb);
#endif

	INIT_WORK(&prInf->getFwVerQ, fw_log_get_version_workQ);
	init_waitqueue_head(&prInf->wq);
	sema_init(&prInf->ioctl_mtx, 1);

	goto return_fn;

class_destroy:
	class_destroy(prInf->driver_class);
cdev_del:
	cdev_del(&prInf->cdev);
unregister_chrdev_region:
	unregister_chrdev_region(prInf->devno, 1);
return_fn:
	if (ret)
		DBGLOG(INIT, ERROR, "ret: %d\n",
			ret);

	return ret;
}

void fw_log_wifi_inf_deinit(void)
{
	struct fw_log_wifi_interface *prInf = &fw_log_wifi_inf;

#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	connv3_log_deinit(CONNV3_DEBUG_TYPE_WIFI);
#elif (CFG_SUPPORT_CONNINFRA == 1)
	connsys_log_deinit(CONN_DEBUG_TYPE_WIFI);
#endif
	device_destroy(prInf->driver_class, prInf->devno);
	class_destroy(prInf->driver_class);
	cdev_del(&prInf->cdev);
	unregister_chrdev_region(prInf->devno, 1);
}

