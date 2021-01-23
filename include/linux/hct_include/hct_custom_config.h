#ifndef __HCT_CUSTOM_CONFIG_H__
#define __HCT_CUSTOM_CONFIG_H__

/*
      ====================================THIS IS custom CONFIG ==========================================
*/
#define __HCT_SD_CARD_HOTPLUG_SUPPORT__  HCT_YES
#define __HCT_SIM_HOTPLUG_SUPPORT__  	 HCT_YES

#define __HCT_PA22A00001_S2_PS_THRELD_HIGH__	1000
#define __HCT_PA22A00001_S2_PS_THRELD_LOW__	900

#define __HCT_TYPEC_ACCDET_MIC_SUPPORT__	HCT_YES
#define __HCT_TP_GESTURE_SUPPORT__   HCT_YES

#define __HCT_HALL_SUPPORT__ HCT_NO

#define __HCT_SUB_FLASHLIGHT_SUPPORT__   HCT_NO

#define __HCT_VIBRATOR_WHEN_BOOT_UP__ HCT_YES

/*############### tpd button key releated config #####################*/
   /*this macro define tp button key y value, if we defined, we use this value,
     if we no define this value , we use default value  2000 */
#define TPD_BUTTON_Y_HIGHT        2000
/****HCT DUAL CAMERA START****/
#define __HCT_DUAL_CAMERA_USEDBY_YUV_MODE__  HCT_YES

//#define __HCT_DUAL_SUB_CAMERA__  HCT_YES

#define IMX298_OTP
#define S5K3P3SP_OTP

////////////////GT9XX _TP CONFIG ///////////////////////////////////

#define __HCT_CONFIG_HCT_GTP_HALL_CFG__   HCT_NO
#define __HCT_GTP_DRIVER_SEND_CFG__  HCT_NO

#if __HCT_GTP_DRIVER_SEND_CFG__
#define __HCT_CTP_CFG_GROUP0__ {\
}

#endif

#if __HCT_CONFIG_HCT_GTP_HALL_CFG__

#define __HCT_CTP_CFG_GROUP1_HALLCLOSE__{\
}

#endif

/*############### led releated config #####################*/
/***mode type defined in hct_common_dts_config.h****/
#define __HCT_RED_LED_MODE__                   __HCT_MT65XX_LED_MODE_NONE__ 
#define __HCT_GREEN_LED_MODE__                 __HCT_MT65XX_LED_MODE_NONE__
#define __HCT_BLUE_LED_MODE__                  __HCT_MT65XX_LED_MODE_NONE__
#define __HCT_JOGBALL_BACKLIGHT_LED_MODE__     __HCT_MT65XX_LED_MODE_NONE__
#define __HCT_KEYBOARD_BACKLIGHT_LED_MODE__    __HCT_MT65XX_LED_MODE_NONE__
#define __HCT_BUTTON_BACKLIGHT_LED_MODE__      __HCT_MT65XX_LED_MODE_NONE__
#define __HCT_LCD_BACKLIGHT_LED_MODE__         __HCT_MT65XX_LED_MODE_CUST_BLS_PWM__

#define __HCT_BLED_CHANNEL_VALUE__	0x3
/*###############battery power config #####################*/
/*****
Note: this value can be  4400000, 4350000, 4200000
******/
#define __HCT_BATTERY_VOLTAGE__   4400000

#define __HCT_Q_MAX_POS_50__	5965
#define __HCT_Q_MAX_POS_25__	5959
#define __HCT_Q_MAX_POS_0__	5782
#define __HCT_Q_MAX_NEG_10__	5782

#define __HCT_Q_MAX_POS_50_H_CURRENT__	5943
#define __HCT_Q_MAX_POS_25_H_CURRENT__	5927
#define __HCT_Q_MAX_POS_0_H_CURRENT__	5395
#define __HCT_Q_MAX_NEG_10_H_CURRENT__	5395

#define __HCT_AC_CHARGER_CURRENT__  2050000	//2A
#define __HCT_LK_SET_AICR__		1000		//lk:1A
#define __HCT_CHARGING_IEOC_CURRENT__ 	350000

#define __HCT_BAT_LOW_TEMP_PROTECT_ENABLE__
#define __HCT_MAX_CHARGE_TEMPERATURE__ 58
#define __HCT_MAX_CHARGE_TEMPERATURE_PLUS_X_DEGREE__ 55
#define __HCT_MIN_CHARGE_TEMPERATURE__ -10
#define __HCT_MIN_CHARGE_TEMPERATURE_PLUS_X_DEGREE__ -10


#define __HCT_V_CC2TOPOFF_THRES__	3400
#define __HCT_RECHARGING_VOLTAGE__  	4270

#define __HCT_V_0PERCENT_TRACKING__	3380

#endif
