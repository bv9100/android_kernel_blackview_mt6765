#ifndef __HCT_BOARD_CONFIG_H__
#define __HCT_BOARD_CONFIG_H__


#undef HCT_YES
#define HCT_YES 1
#undef HCT_NO
#define HCT_NO 0

/*audio releated */
/* phone mic mode



*/
#define __HCT_PHONE_MIC_MODE__   2
 
 /*phone use exp audio pa*/
#define __HCT_USING_EXTAMP_HP__  HCT_YES

/**************************ACCDET MIC Related*********/
#define __HCT_ACCDET_MIC_MODE__  1
/**###########################audio gpio define##################***/

#if  __HCT_USING_EXTAMP_HP__
    #define __HCT_EXTAMP_HP_MODE__    2
    #define __HCT_EXTAMP_GPIO_NUM__    159
#endif

/****POWER RELEATED****/
#define __HCT_CAR_TUNE_VALUE__     100
#define __HCT_5G_WIFI_SUPPORT__   HCT_YES
#define __HCT_SHUTDOWN_SYSTEM_VOLTAGE__  3300
#define __HCT_MT6370_AICL_VTH_MAX__  4400000
/********************sensor old calibraion Interface*************************/
#define __HCT_MTK_OLD_FACTORY_CALIBRATION__ HCT_YES
/*############### USB OTG releated config--START #####################*/

#define __HCT_USB_MTK_OTG_SUPPORT__  HCT_YES
/*############### USB OTG releated config--END #####################*/

/***********************TP***********************/
#define __HCT_CONFIG_HCT_WARP_TP_XY_OFF__   HCT_YES

// #define __HCT_DUAL_CAMERA_USEDBY_YUV_MODE__ HCT_YES
// #define __MAIN_FAKE_I2C_SAMEAS_MAIN__ HCT_YES

/***********************RGB***********************/
#define __HCT_RGB_MAX_BRIGHTNESS__	1

#endif
