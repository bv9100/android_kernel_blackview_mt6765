/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */
#include <linux/kernel.h>
#include "cam_cal_list.h"
#include "eeprom_i2c_common_driver.h"
#include "eeprom_i2c_custom_driver.h"
#include "kd_imgsensor.h"
#include <linux/hct_include/hct_project_all_config.h>
#include "imx298_otp/imx298_otp_cam_cal.h"
#include "s5k3p3sp_otp/s5k3p3sp_otp_cam_cal.h"
#include "s5k3p9_otp/s5k3p9_otp_cam_cal.h"
#include "s5k3p9sxmain_otp/s5k3p9sxmain_otp_cam_cal.h"
#include "s5k3p9sxsub_otp/s5k3p9sxsub_otp_cam_cal.h"

struct stCAM_CAL_LIST_STRUCT g_camCalList[] = {

#if defined(IMX298_OTP)
	{IMX298_SENSOR_ID, 0xA0, imx298_selective_read_region},
#endif
#if defined(IMX298NEW_OTP)
	{IMX298NEW_SENSOR_ID, 0xA0, imx298_selective_read_region},
#endif
#if defined(S5K3P3SP_OTP)
	{S5K3P3SP_SENSOR_ID, 0xA0, s5k3p3sp_selective_read_region},
#endif
#if defined(S5K3P9_OTP)
	{S5K3P9_SENSOR_ID, 0xA0, s5k3p9_selective_read_region},
#endif
#if defined(S5K3P9SXMAIN_OTP)
	{S5K3P9SXMAIN_SENSOR_ID, 0xA2, s5k3p9sxmain_selective_read_region},
#endif
#if defined(S5K3P9SXSUB_OTP)
	{S5K3P9SUB_SENSOR_ID, 0xA2, s5k3p9sxsub_selective_read_region},
#endif
#if 0
	/*Below is commom sensor */
	{IMX230_SENSOR_ID, 0xA0, Common_read_region},
	{S5K2T7SP_SENSOR_ID, 0xA4, Common_read_region},
	{IMX338_SENSOR_ID, 0xA0, Common_read_region},
	{S5K4E6_SENSOR_ID, 0xA8, Common_read_region},
	{IMX386_SENSOR_ID, 0xA0, Common_read_region},
	{S5K3M3_SENSOR_ID, 0xA0, Common_read_region},
	{S5K2L7_SENSOR_ID, 0xA0, Common_read_region},
	{IMX398_SENSOR_ID, 0xA0, Common_read_region},
	{IMX318_SENSOR_ID, 0xA0, Common_read_region},
	{OV8858_SENSOR_ID, 0xA8, Common_read_region},
	{IMX386_MONO_SENSOR_ID, 0xA0, Common_read_region},
	/*B+B*/
	{S5K2P7_SENSOR_ID, 0xA0, Common_read_region},
	{OV8856_SENSOR_ID, 0xA0, Common_read_region},
	/*61*/
	{IMX499_SENSOR_ID, 0xA0, Common_read_region},
	{S5K3L8_SENSOR_ID, 0xA0, Common_read_region},
	{S5K5E8YX_SENSOR_ID, 0xA2, Common_read_region},
	/*99*/
	{IMX258_SENSOR_ID, 0xA0, Common_read_region},
	{IMX258_MONO_SENSOR_ID, 0xA0, Common_read_region},
	/*97*/
	{OV23850_SENSOR_ID, 0xA0, Common_read_region},
	{OV23850_SENSOR_ID, 0xA8, Common_read_region},
	{S5K3M2_SENSOR_ID, 0xA0, Common_read_region},
	/*55*/
	{S5K2P8_SENSOR_ID, 0xA2, Common_read_region},
	{S5K2P8_SENSOR_ID, 0xA0, Common_read_region},
	{OV8858_SENSOR_ID, 0xA2, Common_read_region},
	/* Others */
	{S5K2X8_SENSOR_ID, 0xA0, Common_read_region},
	{IMX377_SENSOR_ID, 0xA0, Common_read_region},
	{IMX214_SENSOR_ID, 0xA0, Common_read_region},
	{IMX214_MONO_SENSOR_ID, 0xA0, Common_read_region},
	{IMX486_SENSOR_ID, 0xA8, Common_read_region},
	{OV12A10_SENSOR_ID, 0xA8, Common_read_region},
	{OV13855_SENSOR_ID, 0xA0, Common_read_region},
	{S5K3L8_SENSOR_ID, 0xA0, Common_read_region},
	{HI556_SENSOR_ID, 0x51, Common_read_region},
	{S5K5E8YX_SENSOR_ID, 0x5a, Common_read_region},
	{S5K5E8YXREAR2_SENSOR_ID, 0x5a, Common_read_region},
	/*  ADD before this line */
#endif
	{0, 0, 0}       /*end of list */
};

unsigned int cam_cal_get_sensor_list(
	struct stCAM_CAL_LIST_STRUCT **ppCamcalList)
{
	if (ppCamcalList == NULL)
		return 1;

	*ppCamcalList = &g_camCalList[0];
	return 0;
}


