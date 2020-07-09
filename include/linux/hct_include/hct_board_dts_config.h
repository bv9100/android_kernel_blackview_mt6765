
#include "hct_common_config.h"
#include "mt6765-pinfunc.h"
/*************** lcm fhd/amoled *********************/
//reset
#define __HCT_GPIO_LCM_RST_PINMUX__   			PINMUX_GPIO45__FUNC_GPIO45
#define __HCT_GPIO_LCM_RST_PIN_NUM__   			45
// +1.8v
//#define __HCT_GPIO_LCM_POWER_DM_PINMUX__   	PINMUX_GPIO177__FUNC_GPIO177
#define __HCT_GPIO_LCM_POWER_ENN_PINMUX__	PINMUX_GPIO177__FUNC_GPIO177
#define __HCT_GPIO_LCM_POWER_DM_PIN__   	177
//  2.8V
#define __HCT_GPIO_LCM_POWER_DP_PINMUX__   	PINMUX_GPIO176__FUNC_GPIO176
#define __HCT_GPIO_LCM_POWER_DP_PIN__   	176

/**************************SD CARD hotplug Related*********/
#if __HCT_SD_CARD_HOTPLUG_SUPPORT__
#define __HCT_MSDC_CD_EINT_NUM__  		1
#define __HCT_MSDC_CD_GPIO_PIN_NUM__  		1
#define __HCT_MSDC_CD_POLARITY_HIGH__  		HCT_NO
#endif

/**************************Custom Key****************/

#define __HCT_CUSTOMKEY1EINT_EN_PIN_NUM__    90
#define __HCT_CUSTOMKEY1_EINT_PIN__           PINMUX_GPIO90__FUNC_GPIO90
#define __HCT_CUSTOMKEY1_EINT_GPIO_NUM__      90

/**************************  Hall  Related*********/
#if __HCT_HALL_SUPPORT__
#define __HCT_HALL_EINT_PIN_NUM__           2
#define __HCT_KPD_SLIDE_EINT_PIN__          PINMUX_GPIO2__FUNC_GPIO2
#define __HCT_HALL_EINT_GPIO_NUM__          2
#endif

/**************************Camera Related*********/

#define __HCT_GPIO_CAMERA_CMRST1_PINMUX__ PINMUX_GPIO102__FUNC_GPIO102
#define __HCT_GPIO_CAMERA_CMPDN1_PINMUX__ PINMUX_GPIO98__FUNC_GPIO98
#define __HCT_GPIO_CAMERA_LDO_EN_PINMUX__ PINMUX_GPIO170__FUNC_GPIO170

//-----------main camera
#define __HCT_GPIO_CAMERA_CMRST_PINMUX__ PINMUX_GPIO101__FUNC_GPIO101
#define __HCT_GPIO_CAMERA_CMPDN_PINMUX__ PINMUX_GPIO97__FUNC_GPIO97
#define __HCT_GPIO_CAMERA_VCAMD_PINMUX__ PUNMUX_GPIO_NONE_FUNC_NONE
#define __HCT_GPIO_CAMERA_VCAMA_PINMUX__ PUNMUX_GPIO_NONE_FUNC_NONE
//-----------sub camera
//#define __HCT_GPIO_CAMERA_SUB_CMRST1_PINMUX__ PINMUX_GPIO102__FUNC_GPIO102
//#define __HCT_GPIO_CAMERA_SUB_CMPDN1_PINMUX__ PINMUX_GPIO98__FUNC_GPIO98
#define __HCT_GPIO_CAMERA_SUB_VCAMD1_PINMUX__ PUNMUX_GPIO_NONE_FUNC_NONE
#define __HCT_GPIO_CAMERA_SUB_VCAMA1_PINMUX__ PUNMUX_GPIO_NONE_FUNC_NONE
//-----------main2 camera
#define __HCT_GPIO_CAMERA_MAIN2_CMRST_PINMUX__ PINMUX_GPIO87__FUNC_GPIO87
#define __HCT_GPIO_CAMERA_MAIN2_CMPDN_PINMUX__ PINMUX_GPIO160__FUNC_GPIO160
#define __HCT_GPIO_CAMERA_MAIN2_VCAMD_PINMUX__ PUNMUX_GPIO_NONE_FUNC_NONE
#define __HCT_GPIO_CAMERA_MAIN2_VCAMA_PINMUX__ PUNMUX_GPIO_NONE_FUNC_NONE
//-----------sub2 camera
//#define __HCT_GPIO_CAMERA_SUB2_CMRST1_PINMUX__ PINMUX_GPIO18__FUNC_GPIO18
//#define __HCT_GPIO_CAMERA_SUB2_CMPDN1_PINMUX__ PINMUX_GPIO19__FUNC_GPIO19

/* zhouhongbin add start 2017.08.19 */
#define __HCT_GPIO_CAMERA_MAIN_FAKE_CMRST_PINMUX__ PINMUX_GPIO87__FUNC_GPIO87
#define __HCT_GPIO_CAMERA_MAIN_FAKE_CMPDN_PINMUX__ PINMUX_GPIO160__FUNC_GPIO160
/* add end */
/* zhouhongbin add start 2017.09.14 */
//#define __HCT_GPIO_CAMERA_SUB_FAKE_CMRST_PINMUX__ PINMUX_GPIO16__FUNC_GPIO16
//#define __HCT_GPIO_CAMERA_SUB_FAKE_CMPDN_PINMUX__ PINMUX_GPIO15__FUNC_GPIO15
/* add end */

/**************************Audio EXMP Related*********/
#define __HCT_GPIO_EXTAMP_EN_PIN__     PUNMUX_GPIO_NONE_FUNC_NONE
#define __HCT_GPIO_EXTAMP2_EN_PIN__    PUNMUX_GPIO_NONE_FUNC_NONE



/**************************CTP Related****************/
#define __HCT_CTP_EINT_ENT_PINNUX__         PINMUX_GPIO0__FUNC_GPIO0

#define __HCT_CTP_RESET_PINNUX__          PINMUX_GPIO174__FUNC_GPIO174
#define __HCT_GPIO_CTP_RST_PIN__		174

#define __HCT_CTP_FT_ALL_SENSOR_ADDR__     0x38

#ifdef CONFIG_TOUCHSCREEN_MTK_MSG28XX
#if CONFIG_TOUCHSCREEN_MTK_MSG28XX
	#define	__HCT_CTP_MSG28XX_SENSOR_ADDR__		0x26
#endif
#endif

#ifndef __HCT_TOUCHSCREEN_SYNAPTICS_ADDR__ 
	#define __HCT_TOUCHSCREEN_SYNAPTICS_ADDR__ 0x20
#endif

#ifndef __HCT_TOUCHSCREEN_NT36672_ADDR__ 
	#define __HCT_TOUCHSCREEN_NT36672_ADDR__ 0x62
#endif

#ifndef __HCT_TOUCHSCREEN_MELFAS_ADDR__
	#define __HCT_TOUCHSCREEN_MELFAS_ADDR__ 0x48
#endif

/**************************Accdet Related****************/
#define __HCT_ACCDET_EINT_PIN__     	PINMUX_GPIO9__FUNC_GPIO9

#define __HCT_TYPEC_ACCDET_MIC_SWITCH_GPIO_NUM__     88
/**************************Gsensor Related****************/
/* Gsensor releated*/
/*Step1: define this macro*/
/*Step2: need define dws int tab with the right bus num*/
#define __HCT_GSENSOR_I2C_BUS_NUM__       	1

#ifdef CONFIG_MTK_BMI160_ACC
#define __HCT__BMI160_ACC_SENSOR_ADDR__			0x68
#define __HCT__BMI160_ACC_SENSOR_DIRECTION__		0
#define __HCT__BMI160_ACC_SENSOR_BATCH_SUPPORT__ 	0
#endif


/**************************Gyro sensor Related****************/
#define __HCT_GYRO_I2C_BUS_NUM__       1
#define __HCT__BMG160_NEW_SENSOR_ADDR__     0x68
#define __HCT__BMG160_NEW_SENSOR_DIRECTION__		0


#ifdef CONFIG_MTK_BMI160_GYRO
#define __HCT__BMI160_GYRO_SENSOR_ADDR__     0x66
#define __HCT__BMI160_GYRO_SENSOR_DIRECTION__		0
#endif

#define __HCT_GYRO_EINT_PINMUX__     12
/**************************ALSPS Related****************/
/*Step1: define this macro*/
/*Step2: need define dws int tab with the right bus num*/
#define __HCT_ALSPS_I2C_BUS_NUM__               1
#define __HCT_STK3X1X_SENSOR_ADDR__			0x48
#define __HCT_HCT_STK3X1X_PS_THRELD_HIGH__		1700
#define __HCT_HCT_STK3X1X_PS_THRELD_LOW__		1500


#ifdef CONFIG_MTK_STK3X3X
#define __HCT_STK3X3X_SENSOR_ADDR__			0x47
#define __HCT_STK3X3X_SENSOR_ALS_ADDR__			0x57
#define __HCT_HCT_STK3X3X_PS_THRELD_HIGH__		1700
#define __HCT_HCT_STK3X3X_PS_THRELD_LOW__		1500
#endif
#ifdef CONFIG_MTK_STK3338V
#define __HCT_STK3X3X_SENSOR_ADDR__			0x67
#define __HCT_HCT_STK3X3X_PS_THRELD_HIGH__		1700
#define __HCT_HCT_STK3X3X_PS_THRELD_LOW__		1500
#endif
#define __HCT_PA22A00001_S2_SENSOR_ADDR__	0x1E

#define __HCT_ALSPS_EINT_PINMUX__                  PINMUX_GPIO6__FUNC_GPIO6
#define __HCT_ALSPS_EINT_PIN_NUM__              6
/**************************msensor sensor Related****************/
#define __HCT_MSENSOR_I2C_BUS_NUM__       	1

#define __HCT__BMM156_NEW_SENSOR_ADDR__			0x12
#define __HCT__BMM156_NEW_SENSOR_DIRECTION__		1
#define __HCT__BMM156_NEW_SENSOR_BATCH_SUPPORT__ 	0

#define __HCT_QMC7983_SENSOR_ADDR__			0x2c
#define __HCT_QMC7983_SENSOR_DIRECTION__		0

//add by micheal for mmc3680x start 20190116
#define __HCT_MMC3680X_SENSOR_ADDR__			0x30
#define __HCT_MMC3680X_SENSOR_DIRECTION__		0
//add by micheal for mmc3680x end 20190116

/**************************GPS LNA Related****************/
#define __HCT_GPS_LNA_EN_PINMUX__     			PINMUX_GPIO110__FUNC_GPIO110

/**************************USB Related****************/
#define __HCT_MT6370_USB_EINT_PIN_NUM__			11
#define __HCT_MT6370_TYPEC_PD_PIN_NUM__			8
/**************************USB OTG Related****************/

/**************************FINGERPRINT SUPPORT****************/
#ifdef CONFIG_HCT_FINGERPRINT_SUPPORT
#define __HCT_FINGERPRINT_EINT_EN_PIN_NUM__	3
#define __HCT_FINGERPRINT_EINT_PIN__	PINMUX_GPIO3__FUNC_GPIO3
#define __HCT_FINGERPRINT_POWER_PIN__	PUNMUX_GPIO_NONE_FUNC_NONE
#define __HCT_FINGERPRINT_RESET_PIN__	PINMUX_GPIO175__FUNC_GPIO175
#endif


/**************************NFC Related****************/
#define __HCT_NFC_RST_PIN_NUM__	       172
#define __HCT_NFC_EINT_PIN_NUM__       10


//////////*****************customise end******************//////////
#include "hct_custom_config.h"

