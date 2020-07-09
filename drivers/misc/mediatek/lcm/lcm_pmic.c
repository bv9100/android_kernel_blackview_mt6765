/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/regulator/consumer.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/hct_include/hct_project_all_config.h>

#if defined(CONFIG_RT5081_PMU_DSV) || defined(CONFIG_MT6370_PMU_DSV)
static struct regulator *disp_bias_pos;
static struct regulator *disp_bias_neg;
static int regulator_inited;

int display_bias_regulator_init(void)
{
	int ret = 0;

	if (regulator_inited)
		return ret;

	/* please only get regulator once in a driver */
	disp_bias_pos = regulator_get(NULL, "dsv_pos");
	if (IS_ERR(disp_bias_pos)) { /* handle return value */
		ret = PTR_ERR(disp_bias_pos);
		pr_info("get dsv_pos fail, error: %d\n", ret);
		return ret;
	}

	disp_bias_neg = regulator_get(NULL, "dsv_neg");
	if (IS_ERR(disp_bias_neg)) { /* handle return value */
		ret = PTR_ERR(disp_bias_neg);
		pr_info("get dsv_neg fail, error: %d\n", ret);
		return ret;
	}

	regulator_inited = 1;
	return ret; /* must be 0 */

}
EXPORT_SYMBOL(display_bias_regulator_init);

int display_bias_enable(void)
{
	int ret = 0;
	int retval = 0;

	display_bias_regulator_init();

	/* set voltage with min & max*/
	#ifdef __HCT_KL_LCM_REGULATOR_VOLTAGE_5_5V__
	ret = regulator_set_voltage(disp_bias_pos, 5500000, 5500000);
	#else
	ret = regulator_set_voltage(disp_bias_pos, 5400000, 5400000);
	#endif
	if (ret < 0)
		pr_info("set voltage disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;
	#ifdef __HCT_KL_LCM_REGULATOR_VOLTAGE_5_5V__
	ret = regulator_set_voltage(disp_bias_neg, 5500000, 5500000);
	#else
	ret = regulator_set_voltage(disp_bias_neg, 5400000, 5400000);
	#endif
	if (ret < 0)
		pr_info("set voltage disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

#if 0
	/* get voltage */
	ret = mtk_regulator_get_voltage(&disp_bias_pos);
	if (ret < 0)
		pr_info("get voltage disp_bias_pos fail\n");
	pr_debug("pos voltage = %d\n", ret);

	ret = mtk_regulator_get_voltage(&disp_bias_neg);
	if (ret < 0)
		pr_info("get voltage disp_bias_neg fail\n");
	pr_debug("neg voltage = %d\n", ret);
#endif
	/* enable regulator */
	ret = regulator_enable(disp_bias_pos);
	if (ret < 0)
		pr_info("enable regulator disp_bias_pos fail, ret = %d\n",
			ret);
	retval |= ret;

	ret = regulator_enable(disp_bias_neg);
	if (ret < 0)
		pr_info("enable regulator disp_bias_neg fail, ret = %d\n",
			ret);
	retval |= ret;

	return retval;
}
EXPORT_SYMBOL(display_bias_enable);

int display_bias_disable(void)
{
	int ret = 0;
	int retval = 0;

	display_bias_regulator_init();

	ret = regulator_disable(disp_bias_neg);
	if (ret < 0)
		pr_info("disable regulator disp_bias_neg fail, ret = %d\n",
			ret);
	retval |= ret;

	ret = regulator_disable(disp_bias_pos);
	if (ret < 0)
		pr_info("disable regulator disp_bias_pos fail, ret = %d\n",
			ret);
	retval |= ret;

	return retval;
}
EXPORT_SYMBOL(display_bias_disable);
int PMU_db_pos_neg_setting_delay_hct(int ms, int vol)
{
	int ret = 0;
	int retval = 0;
	vol = vol * 100000;		//convert
	display_bias_regulator_init();
	ret = regulator_set_voltage(disp_bias_pos, vol, vol);
	if (ret < 0)
		pr_err("set voltage disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;
	ret = regulator_set_voltage(disp_bias_neg, vol, vol);
	if (ret < 0)
		pr_err("set voltage disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;
	ret = regulator_enable(disp_bias_pos);
	if (ret < 0)
		pr_err("enable regulator disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;
	if (ms > 0)
		mdelay(ms);
	ret = regulator_enable(disp_bias_neg);
	if (ret < 0)
		pr_err("enable regulator disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;
	return retval;
}
EXPORT_SYMBOL(PMU_db_pos_neg_setting_delay_hct);
int PMU_db_pos_neg_setting_delay(int ms)
{
	int ret = 0;
	int retval = 0;
	display_bias_regulator_init();
	#ifdef __HCT_KL_LCM_REGULATOR_VOLTAGE_5_5V__
	ret = regulator_set_voltage(disp_bias_pos, 5500000, 5500000);
	#else
	ret = regulator_set_voltage(disp_bias_pos, 5400000, 5400000);
	#endif
	if (ret < 0)
		pr_err("set voltage disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;
	#ifdef __HCT_KL_LCM_REGULATOR_VOLTAGE_5_5V__
	ret = regulator_set_voltage(disp_bias_neg, 5500000, 5500000);
	#else
	ret = regulator_set_voltage(disp_bias_neg, 5400000, 5400000);
	#endif
	if (ret < 0)
		pr_err("set voltage disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;
#if 0
	ret = mtk_regulator_get_voltage(&disp_bias_pos);
	if (ret < 0)
		pr_err("get voltage disp_bias_pos fail\n");
	pr_debug("pos voltage = %d\n", ret);
	ret = mtk_regulator_get_voltage(&disp_bias_neg);
	if (ret < 0)
		pr_err("get voltage disp_bias_neg fail\n");
	pr_debug("neg voltage = %d\n", ret);
#endif
	ret = regulator_enable(disp_bias_pos);
	if (ret < 0)
		pr_err("enable regulator disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;
	if (ms > 0)
		mdelay(ms);
	ret = regulator_enable(disp_bias_neg);
	if (ret < 0)
		pr_err("enable regulator disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;
	return retval;
}
EXPORT_SYMBOL(PMU_db_pos_neg_setting_delay);
int PMU_db_pos_neg_disable_delay(int ms)
{
	int ret = 0;
	int retval = 0;
	display_bias_regulator_init();
	ret = regulator_disable(disp_bias_neg);
	if (ret < 0)
		pr_err("disable regulator disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;
	if (ms > 0)
		mdelay(ms);
	ret = regulator_disable(disp_bias_pos);
	if (ret < 0)
		pr_err("disable regulator disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;
	return retval;
}
EXPORT_SYMBOL(PMU_db_pos_neg_disable_delay);


//ctrl by ext bin, single pin ctrl enn + enp
int RT5081_db_pos_neg_setting(void)
{	
	int ret = 0;
	ret = display_bias_regulator_init();
	if(regulator_is_enabled(disp_bias_neg))
	{
		printk("%s: regulator is enabled,return\n",__func__);
		return 1;
	}
	display_bias_enable();
	return ret;
}

//ctrl by i2c
int RT5081_db_pos_neg_setting_by_i2c(int mdelay)
{	
	int ret = 0;

	ret = display_bias_regulator_init();
	if(regulator_is_enabled(disp_bias_neg))
	{
		printk("%s: regulator is enabled,return\n",__func__);
		return 1;
	}

	PMU_db_pos_neg_setting_delay(mdelay);
	return ret;
}

//ctrl by ext bin, single pin ctrl enn + enp
int RT5081_db_pos_neg_disable(void)
{
	int ret = 0;
	display_bias_disable();
	return ret;
}

//ctrl by i2c
int RT5081_db_pos_neg_disable_by_i2c(int ms)
{
	int ret = 0;
	ret = PMU_db_pos_neg_disable_delay(ms);
	return ret;
}

#else
int display_bias_regulator_init(void)
{
	return 0;
}
EXPORT_SYMBOL(display_bias_regulator_init);

int display_bias_enable(void)
{
	return 0;
}
EXPORT_SYMBOL(display_bias_enable);

int display_bias_disable(void)
{
	return 0;
}
EXPORT_SYMBOL(display_bias_disable);
#endif




