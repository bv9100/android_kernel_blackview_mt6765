/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
 /* MediaTek Inc. (C) 2010. All rights reserved.
  *
  * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
  * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
  * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
  * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
  * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
  * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
  * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
  * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
  * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
  * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
  * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
  * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
  * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
  * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
  * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
  * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
  * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
  * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
  *
  * The following software/firmware and/or related documentation ("MediaTek Software")
  * have been modified by MediaTek Inc. All revisions are subject to any receiver's
  * applicable license agreements with MediaTek Inc.
  */

  /* drivers/hwmon/mt6516/amit/stk3x3x.c - stk3x3x ALS/PS driver
   *
   * Author: MingHsien Hsieh <minghsien.hsieh@mediatek.com>
   *
   * This software is licensed under the terms of the GNU General Public
   * License version 2, as published by the Free Software Foundation, and
   * may be copied, distributed, and modified under those terms.
   *
   * This program is distributed in the hope that it will be useful,
   * but WITHOUT ANY WARRANTY; without even the implied warranty of
   * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   * GNU General Public License for more details.
   *
   */
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/wakelock.h>
#include <asm/io.h>
#include <linux/module.h>
//#include <linux/hwmsen_helper.h>
//#include "cust_eint.h"
#include <hwmsensor.h>
//#include <linux/sensors_io.h>
//#include <linux/hwmsen_dev.h>
//#include <stk_cust_alsps.h>
#include "cust_alsps.h"
#include "alsps.h"

#define stk3x3x_DEV_ALS_NAME     "stk3x3x_als"

struct i2c_client *stk3x3x_als_i2c_client = NULL;
static const struct i2c_device_id stk3x3x_als_i2c_id[] = { {stk3x3x_DEV_ALS_NAME, 0}, {} };
static int stk3x3x_als_init_flag = -1;	// 0<==>OK -1 <==> fail
static int stk3x3x_local_als_init(void);
static int stk3x3x_local_als_uninit(void);
static int stk3x3x_als_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int stk3x3x_als_i2c_remove(struct i2c_client *client);
static int stk3x3x_als_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);

#ifdef CONFIG_OF
static const struct of_device_id als_of_match[] =
{
	{.compatible = "mediatek,stk3x3x_als"},
	{},
};
#endif

static struct i2c_driver stk3x3x_als_i2c_driver =
{
	.probe = stk3x3x_als_i2c_probe,
	.remove = stk3x3x_als_i2c_remove,
	.detect = stk3x3x_als_i2c_detect,
	.id_table = stk3x3x_als_i2c_id,
	.driver = {
		.name = stk3x3x_DEV_ALS_NAME,
#ifdef CONFIG_OF
		.of_match_table = als_of_match,
#endif
	},
};

static int stk3x3x_als_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{
	stk3x3x_als_init_flag = 0;
	stk3x3x_als_i2c_client = client;

	return 0;
}

static int stk3x3x_als_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static int stk3x3x_als_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	strcpy(info->type, stk3x3x_DEV_ALS_NAME);
	return 0;
}

static int stk3x3x_local_als_init(void)
{
	if (i2c_add_driver(&stk3x3x_als_i2c_driver))
	{
		printk("add driver error\n");
		return -1;
	}

	if (-1 == stk3x3x_als_init_flag)
	{
		printk("stk3x3x_als_local_init fail with stk3x3x_als_init_flag=%d\n", stk3x3x_als_init_flag);
		return -1;
	}

	return 0;
}

static int  stk3x3x_local_als_uninit(void)
{
	i2c_del_driver(&stk3x3x_als_i2c_driver);
	stk3x3x_als_i2c_client = NULL;
	return 0;
}

static struct alsps_init_info stk3x3x_als_init_info =
{
	.name = "stk3x3x_als",
	.init = stk3x3x_local_als_init,
	.uninit = stk3x3x_local_als_uninit,
};

static int __init stk3x3x_als_init(void)
{
	printk("----stk3x3x_als_init In----\n");
	alsps_driver_add(&stk3x3x_als_init_info);// hwmsen_alsps_add(&stk3x3x_init_info);
	printk("----stk3x3x_als_init Out----\n");
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit stk3x3x_als_exit(void)
{
	printk("----stk3x3x_als_exit done----\n");
}

/*----------------------------------------------------------------------------*/
module_init(stk3x3x_als_init);
module_exit(stk3x3x_als_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("Sensortek");
MODULE_DESCRIPTION("Create by micheal for SensorTek stk3x3x light sensor probe only");
MODULE_LICENSE("GPL");
