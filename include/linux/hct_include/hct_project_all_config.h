
#ifndef __HCT_PROJECT_ALL_CONFIG_H__
#define __HCT_PROJECT_ALL_CONFIG_H__

#include "hct_common_config.h"
#include "hct_board_config.h"
#include "hct_custom_config.h"



//Jay: when you add new macro for project , you need defined default macro valut here, so you need not defined in code logic
//Next is Begin :::::::::

// Note: default: sd card detection is low level
#ifndef __HCT_SD_CARD_DET_LEVEL__
#define __HCT_SD_CARD_DET_LEVEL__ HCT_NO
#endif

// Note: default: c2k config is off
/*
    when you switch c2k on, you need config __HCT_C2K_DTS_SUPPORT__ HCT_YES
*/
#ifndef __HCT_C2K_DTS_SUPPORT__
#define __HCT_C2K_DTS_SUPPORT__  HCT_NO
#endif

#ifndef __HCT_T637_DK_C2_OPPO_MODULE_SUPPORT__
#define __HCT_T637_DK_C2_OPPO_MODULE_SUPPORT__  HCT_NO
#endif


#ifndef __HCT_USB_MTK_OTG_SUPPORT__
#define __HCT_USB_MTK_OTG_SUPPORT__ HCT_NO
#endif


#ifndef  __HCT_DISCRETE_MEMERY_SUPPORT__
#define  __HCT_DISCRETE_MEMERY_SUPPORT__  HCT_NO
#endif


#ifndef __HCT_DUAL_IRIS_SUPPORT__
#define __HCT_DUAL_IRIS_SUPPORT__  HCT_NO
#endif

#ifndef __HCT_DUALCAMERA_MIPI_SWITCH_SUPPORT__
#define __HCT_DUALCAMERA_MIPI_SWITCH_SUPPORT__  HCT_NO
#endif

#ifndef __HCT_DUAL_CAMERA_USEDBY_YUV_MODE__
#define __HCT_DUAL_CAMERA_USEDBY_YUV_MODE__  HCT_NO
#endif

#ifndef __HCT_CAMERA_I2C_IS_SAME__
#define __HCT_CAMERA_I2C_IS_SAME__  HCT_NO
#endif

#ifndef __HCT_DUAL_SUB_CAMERA__
#define __HCT_DUAL_SUB_CAMERA__  HCT_NO
#endif

#ifndef __HCT_MAIN_FAKE_I2C_USE_SUB__
#define __HCT_MAIN_FAKE_I2C_USE_SUB__  HCT_NO
#endif

#ifndef __HCT_SUB_FAKE_I2C_USE_MAIN__
#define __HCT_SUB_FAKE_I2C_USE_MAIN__  HCT_NO
#endif

#ifndef __HCT_CAM_OTP_PDAF_STATUS__
#define __HCT_CAM_OTP_PDAF_STATUS__  HCT_NO
#endif

#ifndef __HCT_TYPEC_ACCDET_MIC_SUPPORT__
#define __HCT_TYPEC_ACCDET_MIC_SUPPORT__  HCT_NO
#endif


#ifndef __HCT_TRUE_DUAL_FRONT_CAMER__
#define __HCT_TRUE_DUAL_FRONT_CAMER__	HCT_NO
#endif



#ifndef __HCT_CONFIG_HCT_GTP_HALL_CFG__
#define __HCT_CONFIG_HCT_GTP_HALL_CFG__   HCT_NO
#endif

#ifndef __HCT_GTP_DRIVER_SEND_CFG__
#define __HCT_GTP_DRIVER_SEND_CFG__  HCT_NO
#endif

#ifndef __HCT_TA_9V_SUPPORT__
#define __HCT_TA_9V_SUPPORT__ HCT_YES
#endif

#ifndef __HCT_TA_12V_SUPPORT__
#define __HCT_TA_12V_SUPPORT__ HCT_YES
#endif

#ifndef __HCT_TO_OPTIMIZE_THE_EFFECT_OF_POP__
#define __HCT_TO_OPTIMIZE_THE_EFFECT_OF_POP__  HCT_NO
#endif

//hct-drv add for tp_GT9XX_V26 2.8v electricity
#ifndef __HCT_GTP_VIO28_SUPPORT__
#define __HCT_GTP_VIO28_SUPPORT__  HCT_NO
#endif


#ifndef __HCT_SD_VMCH_POWER_DEFAULT_ON__
#define __HCT_SD_VMCH_POWER_DEFAULT_ON__	HCT_NO
#endif

//add by hct-jerry for tp warp x y for GT1X_V14
#ifndef __HCT_CONFIG_HCT_WARP_TP_XY_OFF__
#define __HCT_CONFIG_HCT_WARP_TP_XY_OFF__	HCT_NO
#endif

#ifndef __HCT_NIGHT_VISION_SUPPORT__
#define __HCT_NIGHT_VISION_SUPPORT__		HCT_NO
#endif

#ifndef __HCT_BACK_CLAMP_SUPPORT__
#define __HCT_BACK_CLAMP_SUPPORT__		HCT_NO
#endif
///////////////////
/////  next pls do not modify
////////////////////
#ifndef PINMUX_GPIO1__FUNC_GPIO1
	#if defined(CONFIG_MACH_MT6739) || defined(MACH_TYPE_MT6739)
	#include "mt6739-pinfunc.h"
	#endif

	#if defined(CONFIG_MACH_MT6763)
	#include "mt6763-pinfunc.h"
	#endif

	#if defined(CONFIG_MACH_MT6771)
	#include "mt6771-pinfunc.h"
	#endif

	#if defined(CONFIG_MACH_MT6765)
	#include "mt6765-pinfunc.h"
	#endif

#endif

#include "hct_board_dts_config.h"


#ifndef __HCT_GPIO_LCM_POWER_ENN_PINMUX__
#define __HCT_GPIO_LCM_POWER_ENN_PINMUX__ PUNMUX_GPIO_NONE_FUNC_NONE
#endif

#ifndef __HCT_GPIO_LCM_POWER_ENP_PINMUX__
#define __HCT_GPIO_LCM_POWER_ENP_PINMUX__ PUNMUX_GPIO_NONE_FUNC_NONE
#endif

#ifndef __HCT_GPIO_LCM_POWER_DM_PINMUX__
#define __HCT_GPIO_LCM_POWER_DM_PINMUX__ PUNMUX_GPIO_NONE_FUNC_NONE
#endif

#ifndef __HCT_GPIO_LCM_POWER_DP_PINMUX__
#define __HCT_GPIO_LCM_POWER_DP_PINMUX__ PUNMUX_GPIO_NONE_FUNC_NONE
#endif

#ifndef __HCT_GPIO_CTP_RST_PIN__
#define __HCT_GPIO_CTP_RST_PIN__ PUNMUX_GPIO_NONE_FUNC_NONE
#endif

#endif
