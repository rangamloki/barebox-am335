/*
 * Error Location Module
 *
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <common.h>
#include <driver.h>
#include <errno.h>
#include <io.h>
#include <init.h>
#include <linux/list.h>
#include <linux/bitops.h>
#include <linux/mtd/elm.h>

struct elm_info {
	struct device_d *dev;
	void __iomem *elm_base;
	struct list_head list;
	enum bch_ecc bch_type;
};

static LIST_HEAD(elm_devices);

void elm_reset(struct elm_info *info)
{
	u32 reg;

	reg = readl(info->elm_base + ELM_SYSCONFIG);
	reg |= ELM_SYSCONFIG_SOFTRESET;
	writel(reg, info->elm_base + ELM_SYSCONFIG);

	while (!(readl(info->elm_base + ELM_SYSSTATUS)
					& ELM_SYSSTATUS_RESETDONE))
		;

	reg = readl(info->elm_base + ELM_SYSCONFIG);
	reg &= ~ELM_SYSCONFIG_SIDLE_MASK;
	reg |= ELM_SYSCONFIG_SMART_IDLE;
	writel(reg, info->elm_base + ELM_SYSCONFIG);
}

void elm_config(struct elm_info *info)
{
	u32 reg;
	u32 buffer_size = 0x7ff;

	reg = (info->bch_type & ECC_BCH_LEVEL_MASK) | (buffer_size << 16);
	writel(reg, info->elm_base + ELM_LOCATION_CONFIG);

	/* config in Continuous mode */
	writel(0x0, info->elm_base + ELM_PAGE_CTRL);
}

void elm_load_syndrome(struct elm_info *info, char *syndrome)
{
	int reg;
	int i;

	for (i = 0; i < 4; i++) {
		reg = syndrome[0] | syndrome[1] << 8 |
			syndrome[2] << 16 | syndrome[3] << 24;
		writel(reg, info->elm_base + ELM_SYNDROME_FRAGMENT_0 + i * 4);
		syndrome += 4;
	}
}

void elm_start_processing(struct elm_info *info)
{
	u32 reg;

	reg = readl(info->elm_base + ELM_SYNDROME_FRAGMENT_6);
	reg |= ELM_SYNDROME_VALID;
	writel(reg, info->elm_base + ELM_SYNDROME_FRAGMENT_6);
}

void rotate_ecc_bytes(u8 *src, u8 *dst)
{
	int i;

	for (i = 0; i < BCH8_ECC_OOB_BYTES; i++)
		dst[BCH8_ECC_OOB_BYTES - 1 - i] = src[i];
}

int elm_decode_bch_error(struct device_d *dev, char *ecc_calc,
							unsigned int *err_loc)
{
	u8 ecc_data[28] = {0};
	u32 reg;
	int i, err_no;
	struct elm_info *info = dev->priv;

	rotate_ecc_bytes(ecc_calc, ecc_data);
	elm_load_syndrome(info, ecc_data);
	elm_start_processing(info);

	while ((readl(info->elm_base + ELM_IRQSTATUS) & (0x1 << 0)) != 0x1)
		;
	writel((0x1 << 0), info->elm_base + ELM_IRQSTATUS);

	reg = readl(info->elm_base + ELM_LOCATION_STATUS);
	if (reg & ECC_CORRECTABLE_MASK) {
		err_no = reg & ECC_NB_ERRORS_MASK;

		for (i = 0; i < err_no; i++) {
			reg = readl(info->elm_base +
					ELM_ERROR_LOCATION_0 + i * 4);
			err_loc[i] = reg;
		}
		return err_no;
	}

	return -EINVAL;
}
EXPORT_SYMBOL(elm_decode_bch_error);

struct device_d *elm_request(enum bch_ecc bch_type)
{
	struct elm_info *info;

	list_for_each_entry(info, &elm_devices, list) {
		if (info && info->dev) {
			info->bch_type = bch_type;
			elm_config(info);
			return info->dev;
		}
	}

	return NULL;
}
EXPORT_SYMBOL(elm_request);
static int elm_probe(struct device_d *pdev)
{
	struct resource *res = NULL;
	struct elm_info *info;

	info = xzalloc(sizeof(*info));

	res =  dev_request_mem_region(pdev, 0);

	info->dev = pdev;
	info->dev->priv = info;
	info->elm_base = res;

	INIT_LIST_HEAD(&info->list);
	list_add(&info->list, &elm_devices);

	elm_reset(info);
	elm_config(info);

	return 0;
}

static struct driver_d elm_driver = {
	.name   = "elm",
	.probe  = elm_probe,
};

static int elm_init(void)
{
	return platform_driver_register(&elm_driver);
}

device_initcall(elm_init);
