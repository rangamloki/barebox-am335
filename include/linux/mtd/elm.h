/*
 * BCH Error Location Module
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

#ifndef __ELM_H
#define __ELM_H

enum bch_ecc {
	BCH4_ECC = 0,
	BCH8_ECC,
	BCH16_ECC,
};

#define ELM_SYSCONFIG                0x010
#define ELM_SYSSTATUS                0x014
#define ELM_IRQSTATUS                0x018
#define ELM_IRQENABLE                0x01c
#define ELM_LOCATION_CONFIG          0x020
#define ELM_PAGE_CTRL                0x080
#define ELM_SYNDROME_FRAGMENT_0      0x400
#define ELM_SYNDROME_FRAGMENT_1      0x404
#define ELM_SYNDROME_FRAGMENT_2      0x408
#define ELM_SYNDROME_FRAGMENT_3      0x40c
#define ELM_SYNDROME_FRAGMENT_4      0x410
#define ELM_SYNDROME_FRAGMENT_5      0x414
#define ELM_SYNDROME_FRAGMENT_6      0x418
#define ELM_LOCATION_STATUS          0x800
#define ELM_ERROR_LOCATION_0         0x880
#define ELM_ERROR_LOCATION_1         0x884
#define ELM_ERROR_LOCATION_2         0x888
#define ELM_ERROR_LOCATION_3         0x88c
#define ELM_ERROR_LOCATION_4         0x890
#define ELM_ERROR_LOCATION_5         0x894
#define ELM_ERROR_LOCATION_6         0x898
#define ELM_ERROR_LOCATION_7         0x89c
#define ELM_ERROR_LOCATION_8         0x8a0
#define ELM_ERROR_LOCATION_9         0x8a4
#define ELM_ERROR_LOCATION_10        0x8a8
#define ELM_ERROR_LOCATION_11        0x8ac
#define ELM_ERROR_LOCATION_12        0x8b0
#define ELM_ERROR_LOCATION_13        0x8b4
#define ELM_ERROR_LOCATION_14        0x8b8
#define ELM_ERROR_LOCATION_15        0x8bc

/* ELM System Configuration Register */
#define ELM_SYSCONFIG_SOFTRESET      (1 << 1)
#define ELM_SYSCONFIG_SIDLE_MASK     (3 << 3)
#define ELM_SYSCONFIG_SMART_IDLE     (2 << 3)

/* ELM System Status Register */
#define ELM_SYSSTATUS_RESETDONE      (1 << 0)

/* ELM Location Configuration Register */
#define ECC_SIZE_MASK                (0x7ff << 16)
#define ECC_BCH_LEVEL_MASK           (0x3 << 0)
#define ECC_BCH4_LEVEL               (0x0 << 0)
#define ECC_BCH8_LEVEL               (0x1 << 0)
#define ECC_BCH16_LEVEL              (0x2 << 0)

/* ELM syndrome */
#define ELM_SYNDROME_VALID           (1 << 16)

/* ELM_LOCATION_STATUS Register */
#define ECC_CORRECTABLE_MASK         (1 << 8)
#define ECC_NB_ERRORS_MASK           (0x1f << 0)

/*  ELM_ERROR_LOCATION_0-15 Registers */
#define ECC_ERROR_LOCATION_MASK      (0x1fff << 0)
/* ELM support 8 error syndrome process */
#define ERROR_VECTOR_MAX                8
#define OOB_SECTOR_SIZE                 16

#define BCH_MAX_ECC_BYTES_PER_SECTOR    (OOB_SECTOR_SIZE * ERROR_VECTOR_MAX)

#define BCH8_ECC_OOB_BYTES              13
#define BCH4_ECC_OOB_BYTES		7
/* RBL requires 14 byte even though BCH8 uses only 13 byte */
#define BCH8_SIZE                       (BCH8_ECC_OOB_BYTES + 1)
#define BCH4_SIZE                       7

#define BCH8_SYNDROME_SIZE              4       /* 13 bytes of ecc */
#define BCH4_SYNDROME_SIZE              2       /* 7 bytes of ecc */

int elm_decode_bch_error(struct device_d *dev, char *ecc_calc,
						unsigned int *err_loc);
struct device_d *elm_request(enum bch_ecc bch_type);
#endif /* __ELM_H */
