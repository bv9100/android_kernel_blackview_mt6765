#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/init.h>
#include <linux/types.h>
#include <mt-plat/mtk_boot_common.h>

#ifdef CONFIG_MTK_SMI_EXT
#include "mmdvfs_mgr.h"
#endif

#ifdef CONFIG_OF
/* device tree */
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#endif

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#ifdef CONFIG_MTK_CCU
#include "ccu_inc.h"
#endif

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"
#include "kd_imgsensor_errcode.h"

#include "imgsensor_cfg_table.h"
#include "imgsensor_sensor_list.h"
#include "imgsensor_hw.h"
#include "imgsensor_i2c.h"
#include "imgsensor_proc.h"
#include "imgsensor_clk.h"
#include "imgsensor.h"
#include <linux/hct_include/hct_project_all_config.h>

#ifndef __HCT_FAKE_CHANGE_I2C__
#define __HCT_FAKE_CHANGE_I2C_FLAG__ HCT_NO
#else
#define __HCT_FAKE_CHANGE_I2C_FLAG__ HCT_YES
#endif
#if  (__HCT_DUAL_CAMERA_USEDBY_YUV_MODE__ || __HCT_DUAL_SUB_CAMERA__)
static MINT8 j = 0;
#endif

#if  __HCT_DUAL_CAMERA_USEDBY_YUV_MODE__

#ifdef SP0A38_MIPI_YUV
extern SENSOR_FUNCTION_STRUCT_1 SensorFuncSP0A38YUV_1;
static SENSOR_FUNCTION_STRUCT_1 *g_pSensorFunc_1[2] = {&SensorFuncSP0A38YUV_1, &SensorFuncSP0A38YUV_1};
#else
#ifdef GC2145_MIPI_YUV
extern SENSOR_FUNCTION_STRUCT_1 SensorFuncGC2145YUV_1;
static SENSOR_FUNCTION_STRUCT_1 *g_pSensorFunc_1[2] = {&SensorFuncGC2145YUV_1, &SensorFuncGC2145YUV_1};
#else
extern SENSOR_FUNCTION_STRUCT_1	SensorFuncGC0310YUV_1;
static SENSOR_FUNCTION_STRUCT_1 *g_pSensorFunc_1[2] = {&SensorFuncGC0310YUV_1, &SensorFuncGC0310YUV_1};
#endif
#endif

#endif

#if  __HCT_DUAL_SUB_CAMERA__

#if  __HCT_DUAL_CAMERA_USEDBY_YUV_MODE__
#else

#ifdef SP0A38_MIPI_YUV
extern SENSOR_FUNCTION_STRUCT_1 SensorFuncSP0A38YUV_1;
static SENSOR_FUNCTION_STRUCT_1 *g_pSensorFunc_1[2] = {&SensorFuncSP0A38YUV_1, &SensorFuncSP0A38YUV_1};
#else
extern SENSOR_FUNCTION_STRUCT_1	SensorFuncGC0310YUV_1;
static SENSOR_FUNCTION_STRUCT_1 *g_pSensorFunc_1[2] = {&SensorFuncGC0310YUV_1, &SensorFuncGC0310YUV_1};
#endif

#endif
#endif


#if  (__HCT_DUAL_CAMERA_USEDBY_YUV_MODE__ || __HCT_DUAL_SUB_CAMERA__)

#if (__HCT_SUB_FAKE_I2C_USE_MAIN__	||  __HCT_FAKE_CHANGE_I2C_FLAG__)//	hct-drv zhb 2018.1.29
#if __HCT_SUB_FAKE_I2C_USE_MAIN__	//	hct-drv zhb 2018.1.29
extern char sub_fake_i2c_flag;
#endif
#ifdef __HCT_FAKE_CHANGE_I2C__
extern int hct_i2cflag;
struct IMGSENSOR_SENSOR *psensor_mainfake = NULL;
#endif
static DEFINE_MUTEX(gimgsensor_mutex);
extern struct IMGSENSOR_SENSOR *imgsensor_sensor_get_inst(enum IMGSENSOR_SENSOR_IDX idx);

static void imgsensor_mutex_lock(struct IMGSENSOR_SENSOR_INST *psensor_inst)
{
#ifdef IMGSENSOR_LEGACY_COMPAT
	if (psensor_inst->status.arch) {
		mutex_lock(&psensor_inst->sensor_mutex);
	} else {
		mutex_lock(&gimgsensor_mutex);
		imgsensor_i2c_set_device(&psensor_inst->i2c_cfg);
	}
#else
	mutex_lock(&psensor_inst->sensor_mutex);
#endif
}

static void imgsensor_mutex_unlock(struct IMGSENSOR_SENSOR_INST *psensor_inst)
{
#ifdef IMGSENSOR_LEGACY_COMPAT
	if (psensor_inst->status.arch)
		mutex_unlock(&psensor_inst->sensor_mutex);
	else
		mutex_unlock(&gimgsensor_mutex);
#else
	mutex_lock(&psensor_inst->sensor_mutex);
#endif
}
#endif

static ssize_t yuvcamera_show_value(struct device_driver *ddri, char *buf)
{
	kal_uint32 sensor_reg_1 = 0;
#if __HCT_SUB_FAKE_I2C_USE_MAIN__	//	hct-drv zhb 2018.1.29
	struct IMGSENSOR_SENSOR *psensor_subfake = NULL;
	if(sub_fake_i2c_flag)
	{
		psensor_subfake = imgsensor_sensor_get_inst(IMGSENSOR_SENSOR_IDX_MAIN);
		if (psensor_subfake != NULL) {
			psensor_subfake->inst.sensor_idx = IMGSENSOR_SENSOR_IDX_MAIN;
		}
		else {
			printk("[%s] psensor_subfake is NULL.\n", __func__);
		}

		imgsensor_mutex_lock(&psensor_subfake->inst);

		sensor_reg_1=g_pSensorFunc_1[j]->Sensorreadshutter();

		imgsensor_mutex_unlock(&psensor_subfake->inst);
	}
	else
#endif
#ifdef __HCT_FAKE_CHANGE_I2C__
psensor_mainfake = imgsensor_sensor_get_inst(IMGSENSOR_SENSOR_IDX_MAIN);
		if (psensor_mainfake != NULL) {
			psensor_mainfake->inst.sensor_idx = IMGSENSOR_SENSOR_IDX_MAIN;
		}
		else {
			printk("[%s] psensor_mainfake is NULL.\n", __func__);
		}
				hct_i2cflag = 1;
imgsensor_mutex_lock(&psensor_mainfake->inst);
	sensor_reg_1=g_pSensorFunc_1[j]->Sensorreadshutter();
	
		imgsensor_mutex_unlock(&psensor_mainfake->inst);
				hct_i2cflag = 0;
#else
		sensor_reg_1=g_pSensorFunc_1[j]->Sensorreadshutter();
#endif
    printk("Read_Shutter %s start! %x\n", __func__,sensor_reg_1);
    return sprintf(buf, "%d\n", sensor_reg_1);
}

static DRIVER_ATTR(yuvcamera,  0664, yuvcamera_show_value, NULL);

static struct driver_attribute *yuvcamera_attr_list[] = {
    &driver_attr_yuvcamera,
};



int yuvcamera_create_attr(struct device_driver *driver)
{
    int idx, err = 0;
    int num = (int)(sizeof(yuvcamera_attr_list)/sizeof(yuvcamera_attr_list[0]));

    if (driver == NULL)
        return -EINVAL;

    for (idx = 0; idx < num; idx++) {
        if ((err = driver_create_file(driver, yuvcamera_attr_list[idx]))) {
            printk("driver_create_file (%s) = %d\n", yuvcamera_attr_list[idx]->attr.name, err);
            break;
        }
    }
    return err;
}

int yuvcamera_delete_attr(struct device_driver *driver)
{
    int idx , err = 0;
    int num = (int)(sizeof(yuvcamera_attr_list)/sizeof(yuvcamera_attr_list[0]));

    if (!driver)
        return -EINVAL;

    for (idx = 0; idx < num; idx++)
        driver_remove_file(driver, yuvcamera_attr_list[idx]);

    return err;
}

#endif
