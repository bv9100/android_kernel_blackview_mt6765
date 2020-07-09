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

/* drivers/hwmon/mt6516/amit/DiscreteALSPS.c - DiscreteALSPS ALS/PS driver
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
#ifdef CONFIG_PM_WAKELOCKS
#else
#include <linux/wakelock.h> 
#endif
#include <asm/io.h>
#include <linux/module.h>

//#include <linux/hct_include/hct_project_all_config.h>

//#include <linux/hwmsen_helper.h>
//#include "cust_eint.h"
//#include <linux/hwmsensor.h>
//#include <linux/sensors_io.h>
//#include <linux/hwmsen_dev.h>
//#include <stk_cust_alsps.h>
#include "DiscreteALSPS.h"
#define DRIVER_VERSION          "3.5.2 20151230"
//#define STK_PS_POLLING_LOG
//#define STK_PS_DEBUG
#define STK_TUNE0
#define CALI_EVERY_TIME
#define STK_ALS_FIR
//#define STK_IRS
//#define STK_CHK_REG
//#define STK_GES

/////
#include "cust_alsps.h"  //wangxiqiang
//extern struct alsps_hw* get_cust_alsps_hw(void);
#include "alsps.h"


#if ((defined MT6573) || (defined MT6575) || (defined MT6577) || (defined MT6589)  || (defined MT6572))	
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);
#else
    //#include <mach/eint.h>
	// extern void mt_eint_mask(unsigned int eint_num);
	// extern void mt_eint_unmask(unsigned int eint_num);
	// extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
	// extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
	// extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
	// extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
	// extern void mt_eint_print_status(void);
#endif

/******************************************************************************
 * configuration
*******************************************************************************/

/*----------------------------------------------------------------------------*/
#define DiscreteALSPS_DEV_NAME     "DiscreteALSPS"
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[ALS/PS] "
/*
#define APS_FUN(f)               printk(KERN_ERR APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)    printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    printk(KERN_ERR APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_ERR fmt, ##args) 
*/
#define APS_FUN(f)               pr_err(APS_TAG"%s\n", __func__)
#define APS_ERR(fmt, args...)    pr_err(APS_TAG"%s %d : "fmt, __func__, __LINE__, ##args)
#define APS_LOG(fmt, args...)    pr_err(APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    pr_err(APS_TAG fmt, ##args)                
/******************************************************************************
 * extern functions
*******************************************************************************/
#ifdef MT6516
extern void MT6516_EINTIRQUnmask(unsigned int line);
extern void MT6516_EINTIRQMask(unsigned int line);
extern void MT6516_EINT_Set_Polarity(kal_uint8 eintno, kal_bool ACT_Polarity);
extern void MT6516_EINT_Set_HW_Debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 MT6516_EINT_Set_Sensitivity(kal_uint8 eintno, kal_bool sens);
extern void MT6516_EINT_Registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);
#endif
/*----------------------------------------------------------------------------*/
#define mt6516_I2C_DATA_PORT        ((base) + 0x0000)
#define mt6516_I2C_SLAVE_ADDR       ((base) + 0x0004)
#define mt6516_I2C_INTR_MASK        ((base) + 0x0008)
#define mt6516_I2C_INTR_STAT        ((base) + 0x000c)
#define mt6516_I2C_CONTROL          ((base) + 0x0010)
#define mt6516_I2C_TRANSFER_LEN     ((base) + 0x0014)
#define mt6516_I2C_TRANSAC_LEN      ((base) + 0x0018)
#define mt6516_I2C_DELAY_LEN        ((base) + 0x001c)
#define mt6516_I2C_TIMING           ((base) + 0x0020)
#define mt6516_I2C_START            ((base) + 0x0024)
#define mt6516_I2C_FIFO_STAT        ((base) + 0x0030)
#define mt6516_I2C_FIFO_THRESH      ((base) + 0x0034)
#define mt6516_I2C_FIFO_ADDR_CLR    ((base) + 0x0038)
#define mt6516_I2C_IO_CONFIG        ((base) + 0x0040)
#define mt6516_I2C_DEBUG            ((base) + 0x0044)
#define mt6516_I2C_HS               ((base) + 0x0048)
#define mt6516_I2C_DEBUGSTAT        ((base) + 0x0064)
#define mt6516_I2C_DEBUGCTRL        ((base) + 0x0068)
/*----------------------------------------------------------------------------*/

#ifdef STK_TUNE0
	#define STK_MAX_MIN_DIFF	200
#if 0//defined(__HCT_STK3X1X_PS_THRELD_LOW__) && defined(__HCT_STK3X1X_PS_THRELD_HIGH__) 
	#define STK_LT_N_CT	__HCT_STK3X1X_PS_THRELD_LOW__
	#define STK_HT_N_CT	__HCT_STK3X1X_PS_THRELD_HIGH__
#else
	#define STK_LT_N_CT	100
	#define STK_HT_N_CT	150
#endif
#endif /* #ifdef STK_TUNE0 */

#define STK_IRC_MAX_ALS_CODE		20000
#define STK_IRC_MIN_ALS_CODE		25
#define STK_IRC_MIN_IR_CODE		50
#define STK_IRC_ALS_DENOMI		2		
#define STK_IRC_ALS_NUMERA		5
#define STK_IRC_ALS_CORREC		748

#define STK_IRS_IT_REDUCE				2
#define STK_ALS_READ_IRS_IT_REDUCE	5
#define STK_ALS_THRESHOLD			30

#define STK3310SA_PID		0x17
#define STK3311SA_PID		0x1E
#define STK3311WV_PID	0x1D
#define MTK_AUTO_DETECT_ALSPS
/*----------------------------------------------------------------------------*/
static struct i2c_client *DiscreteALSPS_i2c_client = NULL;
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id DiscreteALSPS_i2c_id[] = {{DiscreteALSPS_DEV_NAME,0},{}};
//static struct regulator *DiscreteALSPS_regulator;

/*the adapter id & i2c address will be available in customization*/
//static unsigned short DiscreteALSPS_force[] = {0x00, 0x00, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const DiscreteALSPS_forces[] = { DiscreteALSPS_force, NULL };
//static struct i2c_client_address_data DiscreteALSPS_addr_data = { .forces = DiscreteALSPS_forces,};

/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int DiscreteALSPS_i2c_remove(struct i2c_client *client);
static int DiscreteALSPS_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
/*----------------------------------------------------------------------------*/
//static int DiscreteALSPS_i2c_suspend(struct i2c_client *client, pm_message_t msg);
//static int DiscreteALSPS_i2c_resume(struct i2c_client *client);
//static struct DiscreteALSPS_priv *g_DiscreteALSPS_ptr = NULL;

static int	DiscreteALSPS_init_flag = -1;	// 0<==>OK -1 <==> fail
static int  DiscreteALSPS_local_init(void);
static int  DiscreteALSPS_local_uninit(void);
//static struct sensor_init_info DiscreteALSPS_init_info = {
static struct alsps_init_info DiscreteALSPS_init_info = {
		.name = "DiscreteALSPS",
		.init = DiscreteALSPS_local_init,
		.uninit = DiscreteALSPS_local_uninit,
	
};

struct alsps_hw DiscreteALSPS_alsps_cust;
static struct alsps_hw *hw = &DiscreteALSPS_alsps_cust;
struct platform_device *alspsPltFmDev_DiscreteALSPS;
/* For alsp driver get cust info */
#if 0
static struct alsps_hw *get_cust_alsps(void)
{
	return &DiscreteALSPS_alsps_cust;
}
#endif
/*----------------------------------------------------------------------------*/
typedef enum {
    STK_TRC_ALS_DATA= 0x0001,
    STK_TRC_PS_DATA = 0x0002,
    STK_TRC_EINT    = 0x0004,
    STK_TRC_IOCTL   = 0x0008,
    STK_TRC_I2C     = 0x0010,
    STK_TRC_CVT_ALS = 0x0020,
    STK_TRC_CVT_PS  = 0x0040,
    STK_TRC_DEBUG   = 0x8000,
} STK_TRC;
/*----------------------------------------------------------------------------*/
typedef enum {
    STK_BIT_ALS    = 1,
    STK_BIT_PS     = 2,
} STK_BIT;
/*----------------------------------------------------------------------------*/
struct DiscreteALSPS_i2c_addr {    
/*define a series of i2c slave address*/
    u8  state;      	/* enable/disable state */
    u8  psctrl;     	/* PS control */
    u8  alsctrl;    	/* ALS control */
    u8  ledctrl;   		/* LED control */
    u8  intmode;    	/* INT mode */
    u8  wait;     		/* wait time */
    u8  thdh1_ps;   	/* PS INT threshold high 1 */
	u8	thdh2_ps;		/* PS INT threshold high 2 */
    u8  thdl1_ps;   	/* PS INT threshold low 1 */
	u8  thdl2_ps;   	/* PS INT threshold low 2 */
    u8  thdh1_als;   	/* ALS INT threshold high 1 */
	u8	thdh2_als;		/* ALS INT threshold high 2 */
    u8  thdl1_als;   	/* ALS INT threshold low 1 */
	u8  thdl2_als;   	/* ALS INT threshold low 2 */	
	u8  flag;			/* int flag */
	u8  data1_ps;		/* ps data1 */
	u8  data2_ps;		/* ps data2 */
	u8  data1_als;		/* als data1 */
	u8  data2_als;		/* als data2 */
	u8  data1_offset;	/* offset data1 */
	u8  data2_offset;	/* offset data2 */
	u8  data1_ir;		/* ir data1 */
	u8  data2_ir;		/* ir data2 */
	u8  soft_reset;		/* software reset */
};
/*----------------------------------------------------------------------------*/
#ifdef STK_ALS_FIR
	#define STK_FIR_LEN	8
	#define MAX_FIR_LEN 32
struct data_filter {
    u16 raw[MAX_FIR_LEN];
    int sum;
    int num;
    int idx;
};
#endif

#ifdef STK_GES
union stk_ges_operation{
	uint8_t ops[4];
	struct {
		uint8_t rw_len_retry;
		uint8_t reg;
		uint8_t reg_value_retry_crit;
		uint8_t sleep_10ns;
	}action;
};

union stk_ges_operation stk_ges_op[10] =
{
	{.ops={0xc1, 0x24, 0, 0}},
	{.ops={0, 0, 0, 0}},	
	{.ops={0, 0, 0, 0}},	
	{.ops={0, 0, 0, 0}},	
	{.ops={0, 0, 0, 0}},	
	{.ops={0, 0, 0, 0}},	
	{.ops={0, 0, 0, 0}},	
	{.ops={0, 0, 0, 0}},	
	{.ops={0, 0, 0, 0}},	
	{.ops={0, 0, 0, 0}}
};
#endif

struct DiscreteALSPS_priv {
    struct alsps_hw  *hw;
    struct i2c_client *client;
   // struct delayed_work  eint_work;

    /*i2c address group*/
    struct DiscreteALSPS_i2c_addr  addr;
    
    /*misc*/
    atomic_t    trace;
    atomic_t    i2c_retry;
    atomic_t    als_suspend;
    atomic_t    als_debounce;   /*debounce time after enabling als*/
    atomic_t    als_deb_on;     /*indicates if the debounce is on*/
    atomic_t    als_deb_end;    /*the jiffies representing the end of debounce*/
    atomic_t    ps_mask;        /*mask ps: always return far away*/
    atomic_t    ps_debounce;    /*debounce time after enabling ps*/
    atomic_t    ps_deb_on;      /*indicates if the debounce is on*/
    atomic_t    ps_deb_end;     /*the jiffies representing the end of debounce*/
    atomic_t    ps_suspend;
	atomic_t    init_done;
	//struct      device_node *irq_node;
	int         irq;

    /*data*/
    u16         als;
    u16         ps;
    u8          _align;
    u16         als_level_num;
    u16         als_value_num;
    u32         als_level[C_CUST_ALS_LEVEL-1];
    u32         als_value[C_CUST_ALS_LEVEL];

	atomic_t	state_val;
	atomic_t 	psctrl_val;
	atomic_t 	alsctrl_val;
	u8 			wait_val;
	u8		 	ledctrl_val;
	u8		 	int_val;
	
    atomic_t    ps_high_thd_val;     /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_low_thd_val;     /*the cmd value can't be read, stored in ram*/
    ulong       enable;         /*enable mask*/
    ulong       pending_intr;   /*pending interrupt*/
	atomic_t	recv_reg;
    /*early suspend*/    
	bool first_boot;
#ifdef STK_TUNE0
	uint16_t psa;
	uint16_t psi;	
	uint16_t psi_set;	
	uint16_t stk_max_min_diff;
	uint16_t stk_lt_n_ct;
	uint16_t stk_ht_n_ct;
	
#ifdef CALI_EVERY_TIME
	uint16_t ps_high_thd_boot;
	uint16_t ps_low_thd_boot;
#endif  
	uint16_t boot_cali;
	//struct hrtimer ps_tune0_timer;	
	//struct workqueue_struct *stk_ps_tune0_wq;
    //struct work_struct stk_ps_tune0_work;
	//ktime_t ps_tune0_delay;	
	bool tune_zero_init_proc;
	uint32_t ps_stat_data[3];
	int data_count;	
#endif	
#ifdef STK_ALS_FIR
	struct data_filter      fir;
	atomic_t                firlength;		
#endif
	uint16_t ir_code;
	uint16_t als_correct_factor;	
	u16 als_last;
#ifdef STK_GES		
	struct input_dev *ges_input_dev;
	int ges_enabled;
	bool re_enable_ges;	
	int re_enable_ges2;	
	atomic_t gesture2;	
#endif	
	bool re_enable_ps;
	bool re_enable_als;
	u16 ps_cali;		
	
	uint8_t pid;
	uint8_t	p_wv_r_bd_with_co;
	uint16_t p_wv_r_bd_ratio;
	uint32_t als_code_last;	
};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_OF
static const struct of_device_id alsps_of_match[] = {
	{.compatible = "mediatek,DiscreteALSPS"},
	{},
};
#endif

static struct i2c_driver DiscreteALSPS_i2c_driver = {	
	.probe      = DiscreteALSPS_i2c_probe,
	.remove     = DiscreteALSPS_i2c_remove,
	.detect     = DiscreteALSPS_i2c_detect,
//	.suspend    = DiscreteALSPS_i2c_suspend,
//	.resume     = DiscreteALSPS_i2c_resume,
	.id_table   = DiscreteALSPS_i2c_id,
	.driver = {
		.name           = DiscreteALSPS_DEV_NAME,
#ifdef CONFIG_OF
		.of_match_table = alsps_of_match,
#endif
	},
};

static struct DiscreteALSPS_priv *DiscreteALSPS_obj = NULL;
//static int DiscreteALSPS_get_ps_value(struct DiscreteALSPS_priv *obj, u16 ps);
//static int DiscreteALSPS_get_ps_value_only(struct DiscreteALSPS_priv *obj, u16 ps);
//static int DiscreteALSPS_get_als_value(struct DiscreteALSPS_priv *obj, u16 als);
//static int DiscreteALSPS_read_als(struct i2c_client *client, u16 *data);
//static int DiscreteALSPS_read_ps(struct i2c_client *client, u16 *data);
//static int DiscreteALSPS_set_als_int_thd(struct i2c_client *client, u16 als_data_reg);
//static int32_t DiscreteALSPS_get_ir_value(struct DiscreteALSPS_priv *obj, int32_t als_it_reduce);
#ifdef STK_TUNE0
//static int stk_ps_tune_zero_func_fae(struct DiscreteALSPS_priv *obj);
#endif
#ifdef STK_CHK_REG	
//static int DiscreteALSPS_validate_n_handle(struct i2c_client *client);
#endif
//static int DiscreteALSPS_init_client(struct i2c_client *client);
#ifdef CONFIG_PM_WAKELOCKS
#else
static struct wake_lock ps_lock;
#endif
#ifdef STK_GES		
static uint32_t DiscreteALSPS_get_ges_value(struct DiscreteALSPS_priv *obj, unsigned int *ges0, unsigned int *ges1, unsigned int *ges2);
static int32_t DiscreteALSPS_enable_ges(struct i2c_client *client, int enable, int mode);
#endif
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_get_addr(struct alsps_hw *hw, struct DiscreteALSPS_i2c_addr *addr)
{
	if(!hw || !addr)
	{
		return -EFAULT;
	}
	addr->state   = STK_STATE_REG; 
	addr->psctrl   = STK_PSCTRL_REG;         
	addr->alsctrl  = STK_ALSCTRL_REG;
	addr->ledctrl  = STK_LEDCTRL_REG;
	addr->intmode    = STK_INT_REG;
	addr->wait    = STK_WAIT_REG;
	addr->thdh1_ps    = STK_THDH1_PS_REG;
	addr->thdh2_ps    = STK_THDH2_PS_REG;
	addr->thdl1_ps = STK_THDL1_PS_REG;
	addr->thdl2_ps = STK_THDL2_PS_REG;
	addr->thdh1_als    = STK_THDH1_ALS_REG;
	addr->thdh2_als    = STK_THDH2_ALS_REG;
	addr->thdl1_als = STK_THDL1_ALS_REG ;
	addr->thdl2_als = STK_THDL2_ALS_REG;
	addr->flag = STK_FLAG_REG;	
	addr->data1_ps = STK_DATA1_PS_REG;
	addr->data2_ps = STK_DATA2_PS_REG;
	addr->data1_als = STK_DATA1_ALS_REG;	
	addr->data2_als = STK_DATA2_ALS_REG;	
	addr->data1_offset = STK_DATA1_OFFSET_REG;
	addr->data2_offset = STK_DATA2_OFFSET_REG;
	addr->data1_ir = STK_DATA1_IR_REG;	
	addr->data2_ir = STK_DATA2_IR_REG;		
	addr->soft_reset = STK_SW_RESET_REG;	
	
	return 0;
}
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_hwmsen_read_block(struct i2c_client *client, u8 addr, u8 *data, u8 len)
{
	u8 beg = addr; 
	struct i2c_msg msgs[2] = 
	{
		{
			.addr = client->addr,	 
			.flags = 0,
			.len = 1,				 
			.buf= &beg
		},
		{
			.addr = client->addr,	 
			.flags = I2C_M_RD,
			.len = len, 			 
			.buf = data,
		}
	};
	int err;

	if (!client)
		return -EINVAL;
	else if (len > C_I2C_FIFO_SIZE) 
	{		 
		APS_LOG(" length %d exceeds %d\n", len, C_I2C_FIFO_SIZE);
		return -EINVAL;
	}

	err = i2c_transfer(client->adapter, msgs, sizeof(msgs)/sizeof(msgs[0]));
	if (err != 2) 
	{
		APS_LOG("i2c_transfer error: (%d %p %d) %d\n", addr, data, len, err);
		err = -EIO;
	}
	else 
	{
		err = 0;/*no error*/
	}
	return err;
}
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_get_timing(void)
{
	return 200;
/*
	u32 base = I2C2_BASE; 
	return (__raw_readw(mt6516_I2C_HS) << 16) | (__raw_readw(mt6516_I2C_TIMING));
*/
}

/*----------------------------------------------------------------------------*/
int DiscreteALSPS_master_recv(struct i2c_client *client, u16 addr, u8 *buf ,int count)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	int ret = 0, retry = 0;
	int trc = atomic_read(&obj->trace);
	int max_try = atomic_read(&obj->i2c_retry);

	while(retry++ < max_try)
	{
		ret = DiscreteALSPS_hwmsen_read_block(client, addr, buf, count);
		if(ret == 0)
            break;
		udelay(100);
	}

	if(unlikely(trc))
	{
		if((retry != 1) && (trc & STK_TRC_DEBUG))
		{
			APS_LOG("(recv) %d/%d\n", retry-1, max_try); 

		}
	}

	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	transmitted, else error code. */
	return (ret == 0) ? count : ret;
}
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_master_send(struct i2c_client *client, u16 addr, u8 *buf ,int count)
{
	int ret = 0, retry = 0;
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	int trc = atomic_read(&obj->trace);
	int max_try = atomic_read(&obj->i2c_retry);


	while(retry++ < max_try)
	{
		ret = hwmsen_write_block(client, addr, buf, count);
		if (ret == 0)
		    break;
		udelay(100);
	}

	if(unlikely(trc))
	{
		if((retry != 1) && (trc & STK_TRC_DEBUG))
		{
			APS_LOG("(send) %d/%d\n", retry-1, max_try);
		}
	}
	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	transmitted, else error code. */
	return (ret == 0) ? count : ret;
}
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_led(struct i2c_client *client, u8 data)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);
	int ret = 0;
    
    ret = DiscreteALSPS_master_send(client, obj->addr.ledctrl, &data, 1);
	if(ret < 0)
	{
		APS_ERR("write led = %d\n", ret);
		return -EFAULT;
	}
	
	return 0;    
}
/*----------------------------------------------------------------------------*/
#if 0 //hct-drv.
int DiscreteALSPS_read_als(struct i2c_client *client, u16 *data)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);    
	int ret = 0;
	u8 buf[2];
	int32_t als_comperator;	
	u16 als_data;
	int32_t ir_data, tmp;
	u32 als_data_u32;

#ifdef STK_IRS
	const int ir_enlarge = 1 << (STK_ALS_READ_IRS_IT_REDUCE - STK_IRS_IT_REDUCE);	
#endif

#ifdef STK_ALS_FIR
	int idx;   
	int firlen = atomic_read(&obj->firlength);   	
#endif
	if(NULL == client)
	{
		return -EINVAL;
	}	
	ret = DiscreteALSPS_master_recv(client, obj->addr.data1_als, buf, 0x02);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}

	als_data = (buf[0] << 8) | (buf[1]);
	
	if((obj->p_wv_r_bd_with_co & 0b101) == 0b101)
	{
		tmp = als_data * obj->p_wv_r_bd_ratio / 1024;
		if(tmp > 65535)
			als_data = 65535;
		else
			als_data = tmp;
	}
	
	if(obj->p_wv_r_bd_with_co & 0b010)
	{
		if(als_data < STK_ALS_THRESHOLD && obj->als_code_last > 10000)
		{
			ir_data = DiscreteALSPS_get_ir_value(obj, STK_ALS_READ_IRS_IT_REDUCE);
#ifdef STK_IRS				
			if(ir_data > 0)
				obj->ir_code = ir_data * ir_enlarge;
#endif			
			APS_LOG("%s: als_data=%d, als_code_last=%d,ir_data=%d\n", 
					__func__, als_data, obj->als_code_last, ir_data);	
			if(ir_data > 1000)
			{
				als_data = obj->als_code_last;
			}
		}
	}
	obj->als_code_last = als_data;
	
#ifdef STK_ALS_FIR
	if(obj->fir.num < firlen)
	{                
		obj->fir.raw[obj->fir.num] = als_data;
		obj->fir.sum += als_data;
		obj->fir.num++;
		obj->fir.idx++;
	}
	else
	{
		idx = obj->fir.idx % firlen;
		obj->fir.sum -= obj->fir.raw[idx];
		obj->fir.raw[idx] = als_data;
		obj->fir.sum += als_data;
		obj->fir.idx++;
		als_data = (obj->fir.sum / firlen);
	}	
#endif
	
	
	if(obj->ir_code)
	{
		obj->als_correct_factor = 1000;
		if(als_data < STK_IRC_MAX_ALS_CODE && als_data > STK_IRC_MIN_ALS_CODE && 
			obj->ir_code > STK_IRC_MIN_IR_CODE)
		{
			als_comperator = als_data * STK_IRC_ALS_NUMERA / STK_IRC_ALS_DENOMI;
			if(obj->ir_code > als_comperator)
				obj->als_correct_factor = STK_IRC_ALS_CORREC;
		}
		APS_LOG("%s: als=%d, ir=%d, als_correct_factor=%d", __func__, als_data, obj->ir_code, obj->als_correct_factor);
		obj->ir_code = 0;
	}	
	als_data_u32 = als_data;
	als_data_u32 = als_data_u32 * obj->als_correct_factor / 1000;
	*data = (u16)als_data_u32;
	
	if(atomic_read(&obj->trace) & STK_TRC_ALS_DATA)
	{
		APS_DBG("ALS: 0x%04X\n", (u32)(*data));
	}
	
	return 0;    
}
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_als(struct i2c_client *client, u8 data)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);
	int ret = 0;
    
    ret = DiscreteALSPS_master_send(client, obj->addr.alsctrl, &data, 1);
	if(ret < 0)
	{
		APS_ERR("write als = %d\n", ret);
		return -EFAULT;
	}
	
	return 0;    
}
#endif
#ifdef STK_GES		
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_read_gsctrl(struct i2c_client *client, u8 *data)
{
	//struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);    
	int ret = 0;
	u8 buf;
	
	if(NULL == client)
	{
		return -EINVAL;
	}	
	ret = DiscreteALSPS_master_recv(client, STK_GSCTRL_REG, &buf, 0x01);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}
	else
	{
		*data = buf;
	}
	
	return 0;    
}
#endif
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_read_state(struct i2c_client *client, u8 *data)
{
	//struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);    
	int ret = 0;
	u8 buf;
	
	if(NULL == client)
	{
		return -EINVAL;
	}	
	ret = DiscreteALSPS_master_recv(client, STK_STATE_REG, &buf, 0x01);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}
	else
	{
		*data = buf;
	}
	
	return 0;    
}
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_read_flag(struct i2c_client *client, u8 *data)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);    
	int ret = 0;
	u8 buf;
	
	if(NULL == client)
	{
		return -EINVAL;
	}	
	ret = DiscreteALSPS_master_recv(client, obj->addr.flag, &buf, 0x01);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}
	else
	{
		*data = buf;
	}
	
	if(atomic_read(&obj->trace) & STK_TRC_ALS_DATA)
	{
		APS_DBG("PS NF flag: 0x%04X\n", (u32)(*data));
	}
	
	return 0;    
}
/*----------------------------------------------------------------------------*/
#ifdef STK_GES		
int DiscreteALSPS_read_flag2(struct i2c_client *client, u8 *data)
{
	//struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);    
	int ret = 0;
	u8 buf;
	
	if(NULL == client)
	{
		return -EINVAL;
	}	
	ret = DiscreteALSPS_master_recv(client, STK_FLAG2_REG, &buf, 0x01);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}
	else
	{
		*data = buf;
	}
	
	return 0;    
}
#endif
/*----------------------------------------------------------------------------*/

#if 0 //hct-drv.

static int DiscreteALSPS_read_otp25(struct i2c_client *client)
{
	// struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);   
	int ret, otp25;
	u8 data;

	data = 0x2;
    ret = DiscreteALSPS_master_send(client, 0x0, &data, 1);
	if (ret < 0)	
	{
		APS_ERR("write 0x0 = %d\n", ret);
		return -EFAULT;
	} 
	
	data = 0x25;
    ret = DiscreteALSPS_master_send(client, 0x90, &data, 1);
	if (ret < 0)	
	{
		APS_ERR("write 0x90 = %d\n", ret);
		return -EFAULT;
	} 

	data = 0x82;
    ret = DiscreteALSPS_master_send(client, 0x92, &data, 1);
	if (ret < 0)	
	{
		APS_ERR("write 0x0 = %d\n", ret);
		return -EFAULT;
	} 
	usleep_range(1000, 5000);

	ret = DiscreteALSPS_master_recv(client, 0x91, &data, 1);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}
	otp25 = data;
	
	data = 0x0;
    ret = DiscreteALSPS_master_send(client, 0x0, &data, 1);
	if (ret < 0)
	{
		APS_ERR("write 0x0 = %d\n", ret);
		return -EFAULT;
	} 	
	
	APS_LOG("%s: otp25=0x%x\n", __func__, otp25);
	if(otp25 & 0x80)
		return 1;
	return 0;
}

/*----------------------------------------------------------------------------*/

int DiscreteALSPS_read_id(struct i2c_client *client)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);	
	int ret = 0;
	u8 buf[2];
	u8 pid_msb;
	
	if(NULL == client)
	{
		return -EINVAL;
	}	
	obj->p_wv_r_bd_with_co = 0;
	obj->p_wv_r_bd_ratio = 1024;	
	
	ret = DiscreteALSPS_master_recv(client, STK_PDT_ID_REG, buf, 0x02);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}
	obj->pid = buf[0];
	APS_LOG("%s: PID=0x%x, VID=0x%x\n", __func__, buf[0], buf[1]);
	
	if(obj->pid == STK3310SA_PID || obj->pid == STK3311SA_PID)
		obj->ledctrl_val &= 0x3F;		
	
	if(buf[0] == STK3311WV_PID)
		obj->p_wv_r_bd_with_co |= 0b100;
	if(buf[1] == 0xC3)
		obj->p_wv_r_bd_with_co |= 0b010;
		
	if(DiscreteALSPS_read_otp25(client) == 1)
	{
		obj->p_wv_r_bd_with_co |= 0b001;
		obj->p_wv_r_bd_ratio = 1024;
	}
	APS_LOG("%s: p_wv_r_bd_with_co = 0x%x\n", __func__, obj->p_wv_r_bd_with_co);	
	
	if(buf[0] == 0)
	{
		APS_ERR( "PID=0x0, please make sure the chip is DiscreteALSPS!\n");
		return -2;			
	}		
	
	pid_msb = buf[0] & 0xF0;
	switch(pid_msb)
	{
	case 0x10:
	case 0x20:
	case 0x30:
		return 0;
	default:
		APS_ERR( "invalid PID(%#x)\n", buf[0]);	
		return -1;
	}	
	return 0;    
}

/*----------------------------------------------------------------------------*/
int DiscreteALSPS_read_ps(struct i2c_client *client, u16 *data)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);    
	int ret = 0;
	u8 buf[2];
	
	if(NULL == client)
	{
		APS_ERR("i2c client is NULL\n");
		return -EINVAL;
	}	
	ret = DiscreteALSPS_master_recv(client, obj->addr.data1_ps, buf, 0x02);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}
	else
	{
		*data = (buf[0] << 8) | (buf[1]);
	}
	
	if(atomic_read(&obj->trace) & STK_TRC_ALS_DATA)
	{
		APS_DBG("PS: 0x%04X\n", (u32)(*data));
	}
	
	return 0;     
}
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_ps(struct i2c_client *client, u8 data)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	int ret = 0;

    ret = DiscreteALSPS_master_send(client, obj->addr.psctrl, &data, 1);
	if (ret < 0)
	{
		APS_ERR("write ps = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}
#endif

/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_wait(struct i2c_client *client, u8 data)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	int ret = 0;

    ret = DiscreteALSPS_master_send(client, obj->addr.wait, &data, 1);
	if (ret < 0)
	{
		APS_ERR("write wait = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}

/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_int(struct i2c_client *client, u8 data)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	int ret = 0;

    ret = DiscreteALSPS_master_send(client, obj->addr.intmode, &data, 1);
	if (ret < 0)
	{
		APS_ERR("write intmode = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}
/*----------------------------------------------------------------------------*/
#ifdef STK_GES		
int DiscreteALSPS_write_gsctrl(struct i2c_client *client, u8 data)
{
	//struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	int ret = 0;

    ret = DiscreteALSPS_master_send(client, STK_GSCTRL_REG, &data, 1);
	if (ret < 0)
	{
		APS_ERR("write state = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}
#endif
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_state(struct i2c_client *client, u8 data)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	int ret = 0;

    ret = DiscreteALSPS_master_send(client, obj->addr.state, &data, 1);
	if (ret < 0)
	{
		APS_ERR("write state = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}
/*----------------------------------------------------------------------------*/
#if 0 //hct-drv disabled.
int DiscreteALSPS_write_flag(struct i2c_client *client, u8 data)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	int ret = 0;

    ret = DiscreteALSPS_master_send(client, obj->addr.flag, &data, 1);
	if (ret < 0)
	{
		APS_ERR("write ps = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}
/*----------------------------------------------------------------------------*/
#ifdef STK_GES		
int DiscreteALSPS_write_flag2(struct i2c_client *client, u8 data)
{
	//struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	int ret = 0;

    ret = DiscreteALSPS_master_send(client, STK_FLAG2_REG, &data, 1);
	if (ret < 0)
	{
		APS_ERR("write ps = %d\n", ret);
		return -EFAULT;
	} 
	return 0;    
}
#endif
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_sw_reset(struct i2c_client *client)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	u8 buf = 0, r_buf = 0;	
	int ret = 0;

	buf = 0x7F;
    ret = DiscreteALSPS_master_send(client, obj->addr.wait, (char*)&buf, sizeof(buf));
	if (ret < 0)
	{
		APS_ERR("i2c write test error = %d\n", ret);
		return -EFAULT;
	} 	
	
    ret = DiscreteALSPS_master_recv(client, obj->addr.wait, &r_buf, 1);
	if (ret < 0)
	{
		APS_ERR("i2c read test error = %d\n", ret);
		return -EFAULT;
	}	
	
	if(buf != r_buf)
	{
        APS_ERR("i2c r/w test error, read-back value is not the same, write=0x%x, read=0x%x\n", buf, r_buf);		
		return -EIO;
	}
	
	buf = 0;
    ret = DiscreteALSPS_master_send(client, obj->addr.soft_reset, (char*)&buf, sizeof(buf));
	if (ret < 0)
	{
		APS_ERR("write software reset error = %d\n", ret);
		return -EFAULT;
	} 
	msleep(13);
	return 0;    
}

/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_ps_high_thd(struct i2c_client *client, u16 thd)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	u8 buf[2];
	int ret = 0;
	
    buf[0] = (u8) ((0xFF00 & thd) >> 8);
    buf[1] = (u8) (0x00FF & thd);	

    ret = DiscreteALSPS_master_send(client, obj->addr.thdh1_ps, &buf[0], 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %d\n",  ret);
		return -EFAULT;
	}
	
    ret = DiscreteALSPS_master_send(client, obj->addr.thdh2_ps, &(buf[1]), 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %d\n", ret);
		return -EFAULT;
	}	
	
	return 0;    
}
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_ps_low_thd(struct i2c_client *client, u16 thd)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	u8 buf[2];
	int ret = 0;
	
    buf[0] = (u8) ((0xFF00 & thd) >> 8);
    buf[1] = (u8) (0x00FF & thd);	
    ret = DiscreteALSPS_master_send(client, obj->addr.thdl1_ps, &buf[0], 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, ret);
		return -EFAULT;
	}
	
    ret = DiscreteALSPS_master_send(client, obj->addr.thdl2_ps, &(buf[1]), 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, ret);
		return -EFAULT;
	}	
	
	return 0;    
}
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_als_high_thd(struct i2c_client *client, u16 thd)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	u8 buf[2];
	int ret = 0;
	
    buf[0] = (u8) ((0xFF00 & thd) >> 8);
    buf[1] = (u8) (0x00FF & thd);	
    ret = DiscreteALSPS_master_send(client, obj->addr.thdh1_als, &buf[0], 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, ret);
		return -EFAULT;
	}
	
    ret = DiscreteALSPS_master_send(client, obj->addr.thdh2_als, &(buf[1]), 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, ret);
		return -EFAULT;
	}	
	
	return 0;    
}
/*----------------------------------------------------------------------------*/
int DiscreteALSPS_write_als_low_thd(struct i2c_client *client, u16 thd)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	u8 buf[2];
	int ret = 0;
	
    buf[0] = (u8) ((0xFF00 & thd) >> 8);
    buf[1] = (u8) (0x00FF & thd);	
    ret = DiscreteALSPS_master_send(client, obj->addr.thdl1_als, &buf[0], 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, ret);
		return -EFAULT;
	}
	
    ret = DiscreteALSPS_master_send(client, obj->addr.thdl2_als, &(buf[1]), 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, ret);
		return -EFAULT;
	}	
	
	return 0;    
}
#endif
/*----------------------------------------------------------------------------*/
#if 0
int DiscreteALSPS_write_foffset(struct i2c_client *client, u16 ofset)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	u8 buf[2];
	int ret = 0;
	
    buf[0] = (u8) ((0xFF00 & ofset) >> 8);
    buf[1] = (u8) (0x00FF & ofset);	
    ret = DiscreteALSPS_master_send(client, obj->addr.data1_offset, &buf[0], 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, ret);
		return -EFAULT;
	}
	
    ret = DiscreteALSPS_master_send(client, obj->addr.data2_offset, &(buf[1]), 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, ret);
		return -EFAULT;
	}	
	
	return 0;   	
}

/*----------------------------------------------------------------------------*/

int DiscreteALSPS_write_aoffset(struct i2c_client *client,  u16 ofset)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);        
	u8 buf[2];
	int ret = 0;
	u8 s_buf = 0, re_en;
    ret = DiscreteALSPS_master_recv(client, obj->addr.state, &s_buf, 1);
	if (ret < 0)
	{
		APS_ERR("i2c read state error = %d\n", ret);
		return -EFAULT;
	}		
	re_en = (s_buf & STK_STATE_EN_AK_MASK) ? 1: 0;
	if(re_en)
	{
		s_buf &= (~STK_STATE_EN_AK_MASK); 		
		ret = DiscreteALSPS_master_send(client, obj->addr.state, &s_buf, 1);
		if (ret < 0)
		{
			APS_ERR("write state = %d\n", ret);
			return -EFAULT;
		} 			
		msleep(3);		
	}	

    buf[0] = (u8) ((0xFF00 & ofset) >> 8);
    buf[1] = (u8) (0x00FF & ofset);	
    ret = DiscreteALSPS_master_send(client, 0x0E, &buf[0], 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, ret);
		return -EFAULT;
	}
	
    ret = DiscreteALSPS_master_send(client, 0x0F, &(buf[1]), 1);
	if (ret < 0)
	{
		APS_ERR("WARNING: %s: %d\n", __func__, ret);
		return -EFAULT;
	}	
	if(!re_en)
		return 0;
	s_buf |= STK_STATE_EN_AK_MASK; 		
	ret = DiscreteALSPS_master_send(client, obj->addr.state, &s_buf, 1);
	if (ret < 0)
	{
		APS_ERR("write state = %d\n", ret);
		return -EFAULT;
	} 			
	return 0;  	
}
#endif
/*----------------------------------------------------------------------------*/
static void DiscreteALSPS_power(struct alsps_hw *hw, unsigned int on) 
{
#if 0
	int ret;
	if(on == 1){
		ret = regulator_enable(DiscreteALSPS_regulator);	/*enable regulator*/
		if (ret)
			printk("regulator_enable() failed!\n");
	}else{
		ret = regulator_disable(DiscreteALSPS_regulator);       /*enable regulator*/
         if (ret)
            printk("regulator_disable() failed!\n");
	}
#endif
}

/*----------------------------------------------------------------------------*/
#if 0
static int DiscreteALSPS_enable_als(struct i2c_client *client, int enable)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);
	int err, cur = 0, old = atomic_read(&obj->state_val);
	int trc = atomic_read(&obj->trace);

	APS_LOG("%s: enable=%d\n", __func__, enable);
	
#ifdef STK_GES		
	if(obj->ges_enabled)
	{
		APS_LOG( "%s: since ges is enabled, ALS is disabled\n", __func__);
		obj->re_enable_als = enable ? true : false;		
		return 0;
	}
#endif		
	cur = old & (~(STK_STATE_EN_ALS_MASK | STK_STATE_EN_WAIT_MASK)); 
	if(enable)
	{
		cur |= STK_STATE_EN_ALS_MASK;
	}
	else if (old & STK_STATE_EN_PS_MASK)
	{
		cur |= STK_STATE_EN_WAIT_MASK;   
	}
	if(trc & STK_TRC_DEBUG)
	{
		APS_LOG("%s: %08X, %08X, %d\n", __func__, cur, old, enable);
	}
	
	if(0 == (cur ^ old))
	{
		return 0;
	}
		
#ifdef STK_IRS		
	if(enable && !(old & STK_STATE_EN_PS_MASK))
	{		
		err =  DiscreteALSPS_get_ir_value(obj, STK_IRS_IT_REDUCE);
		if(err > 0)
			obj->ir_code = err;
	}			
#endif
		
	if(enable && obj->hw->polling_mode_als == 0)
	{
		DiscreteALSPS_write_als_high_thd(client, 0x0);
		DiscreteALSPS_write_als_low_thd(client, 0xFFFF);
	}
	err = DiscreteALSPS_write_state(client, cur);
	if(err < 0)
		return err;
	else
		atomic_set(&obj->state_val, cur);
	
	if(enable)
	{
		
		obj->als_last = 0;
		if(obj->hw->polling_mode_als)
		{
			atomic_set(&obj->als_deb_on, 1);
			atomic_set(&obj->als_deb_end, jiffies+atomic_read(&obj->als_debounce)*HZ/1000);
		}
		else
		{
			//set_bit(STK_BIT_ALS,  &obj->pending_intr);
			//schedule_delayed_work(&obj->eint_work,220*HZ/1000); 
		}
	}

	if(trc & STK_TRC_DEBUG)
	{
		APS_LOG("enable als (%d)\n", enable);
	}

	return err;
}

/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_enable_ps(struct i2c_client *client, int enable, int validate_reg)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);
	int err, cur = 0, old = atomic_read(&obj->state_val);
	int trc = atomic_read(&obj->trace);
	int report_value = 1;
//	hwm_sensor_data sensor_data;

	APS_LOG("%s: enable=%d\n", __FUNCTION__, enable);	

#ifdef STK_TUNE0		
	if (!(obj->psi_set) && !enable)
	{
		APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
		//hrtimer_cancel(&obj->ps_tune0_timer);	
	//	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);				
		//cancel_work_sync(&obj->stk_ps_tune0_work);
	}	
#endif

	if(obj->first_boot == true)
	{		
		obj->first_boot = false;
	}


	cur = old;		
	cur &= (~(0x45)); 
	if(enable)
	{
		cur |= (STK_STATE_EN_PS_MASK);
		if(!(old & STK_STATE_EN_ALS_MASK))
			cur |= STK_STATE_EN_WAIT_MASK;
		if(1 == obj->hw->polling_mode_ps)
		{
		//	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
			#ifdef CONFIG_PM_WAKELOCKS
			#else
				wake_lock(&ps_lock);
			#endif

		}
	}
	else
	{
		if(1 == obj->hw->polling_mode_ps)
		{		
		//	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
			#ifdef CONFIG_PM_WAKELOCKS
			#else
				wake_unlock(&ps_lock);
			#endif
		}
	}
	
	if(trc & STK_TRC_DEBUG)
	{
		APS_LOG("%s: %08X, %08X, %d\n", __func__, cur, old, enable);
	}
	
	if(0 == (cur ^ old))
	{
		return 0;
	}
	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
	err = DiscreteALSPS_write_state(client, cur);
	if(err < 0)
		return err;
	else
		atomic_set(&obj->state_val, cur);

#if 1	
	if(enable)
	{	
#ifdef STK_TUNE0		
	#ifndef CALI_EVERY_TIME
		if (!(obj->psi_set))
			//hrtimer_start(&obj->ps_tune0_timer, obj->ps_tune0_delay, HRTIMER_MODE_REL);	

		if ((err = DiscreteALSPS_write_ps_high_thd(obj->client, atomic_read(&obj->ps_high_thd_val)))) 
		{	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
			APS_ERR("write high thd error: %d\n", err);
			return err;
		}
		if ((err = DiscreteALSPS_write_ps_low_thd(obj->client, atomic_read(&obj->ps_low_thd_val))))
		{	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
			APS_ERR("write low thd error: %d\n", err);
			return err;
		}
	//	APS_LOG("%s: set HT=%d, LT=%d\n", __func__, atomic_read(&obj->ps_high_thd_val), atomic_read(&obj->ps_low_thd_val));		
	#else
		if(true)
		{
			obj->psi_set = 0;
			obj->psa = 0;
			obj->psi = 0xFFFF;
			
			if(obj->boot_cali == 1)
			{//	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
				atomic_set(&obj->ps_high_thd_val, obj->ps_high_thd_boot);
				atomic_set(&obj->ps_low_thd_val, obj->ps_low_thd_boot);
			}
			else
			{	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
			//	atomic_set(&obj->ps_high_thd_val, 0xFFFF);
			//	atomic_set(&obj->ps_low_thd_val, 0xFFFF);
			}

			if ((err = DiscreteALSPS_write_ps_high_thd(obj->client, atomic_read(&obj->ps_high_thd_val)))) 
			{	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
				APS_ERR("write high thd error: %d\n", err);
				return err;
			}
			if ((err = DiscreteALSPS_write_ps_low_thd(obj->client, atomic_read(&obj->ps_low_thd_val))))
			{	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
				APS_ERR("write low thd error: %d\n", err);
				return err;
			}
			APS_LOG("%s: set HT=%d, LT=%d\n", __func__, atomic_read(&obj->ps_high_thd_val), atomic_read(&obj->ps_low_thd_val));
			
			//hrtimer_start(&obj->ps_tune0_timer, obj->ps_tune0_delay, HRTIMER_MODE_REL);			
		}
	#endif
#endif		
		APS_LOG("%s: HT=%d, LT=%d\n", __func__, atomic_read(&obj->ps_high_thd_val), atomic_read(&obj->ps_low_thd_val));	
		
		if(obj->hw->polling_mode_ps)
		{	APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
			atomic_set(&obj->ps_deb_on, 1);
			atomic_set(&obj->ps_deb_end, jiffies+atomic_read(&obj->ps_debounce)*HZ/1000);
		}
		else
		{		
				msleep(4);
				APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
				if((err = DiscreteALSPS_read_ps(obj->client, &obj->ps)))
				{
					APS_ERR("DiscreteALSPS read ps data: %d\n", err);
					return err;
				}
				APS_LOG("%s: line=%d\n", __FUNCTION__, __LINE__);
				report_value = DiscreteALSPS_get_ps_value(obj, obj->ps);
				if(report_value < 0)
				{
					APS_ERR("DiscreteALSPS get ps value: %d\n", report_value);
					return err;
				}	

				APS_LOG("%s:ps raw 0x%x -> value 0x%x \n",__FUNCTION__, obj->ps, report_value);

				if(ps_report_interrupt_data(report_value))
				{	
					APS_ERR("call ps_report_interrupt_data fail\n");
				} 		

			#ifdef STK_PS_DEBUG
				APS_LOG("%s: after ps_report_interrupt_data, show all reg\n", __FUNCTION__);
				show_allreg(); //for debug
			#endif
		}
	}

	if(trc & STK_TRC_DEBUG)
	{
		APS_LOG("enable ps  (%d)\n", enable);
	}
#endif
	return 0;
}
#endif

/*----------------------------------------------------------------------------*/
#ifdef STK_GES
static int32_t DiscreteALSPS_enable_ges(struct i2c_client *client, int enable, int mode)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);	
	u8 reg;	
	int err;
	int org_state_reg = atomic_read(&obj->state_val);
	int org_mode = 0;
	
	APS_LOG("%s: enable=%d\n", __FUNCTION__, enable);		

	
	if(enable == obj->ges_enabled)
		return 0;
	
	if(enable)
	{
		if(org_state_reg & STK_STATE_EN_PS_MASK)
		{
		//	APS_LOG( "%s: force disable PS\n", __func__);
		//	DiscreteALSPS_enable_ps(obj->client, 0, 1);
		//	obj->re_enable_ps = true;
			APS_LOG( "%s: since PS is enabled, Ges is disabled, mode = %d\n", __func__, mode);
			obj->re_enable_ges2 = mode;	
			return 0;
		}
		if(org_state_reg & STK_STATE_EN_ALS_MASK) 
		{
			APS_LOG( "%s: force disable ALS\n", __func__);						
			DiscreteALSPS_enable_als(obj->client, 0);		
			obj->re_enable_als = true;
		}
		
		if((err = DiscreteALSPS_write_wait(client, 0)))
		{
			APS_ERR("write wait error: %d\n", err);
			return err;
		}		
		
		if((err = DiscreteALSPS_write_int(obj->client, 0)))
		{
			APS_ERR("write int mode error: %d\n", err);
			return err;        
		}				
			
		reg = STK_STATE_EN_WAIT_MASK | STK_STATE_EN_PS_MASK; 
		err = DiscreteALSPS_write_state(client, reg);
		if(err < 0)
			return err;
		atomic_set(&obj->state_val, reg);	
		obj->ges_enabled = mode;		
		if(mode == 2)
		{
			err = DiscreteALSPS_read_gsctrl(client, &reg);
			if(err < 0)
				return err;		
			reg &= 0xF3;
			if(obj->hw->polling_mode_ps == 1)
				reg |= 0x04;
			else
				reg |= 0x0C;
			err = DiscreteALSPS_write_gsctrl(client, reg);
			if(err < 0)
				return err;			
			if(obj->hw->polling_mode_ps == 1)
			{
				//hrtimer_start(&obj->ps_tune0_timer, obj->ps_tune0_delay, HRTIMER_MODE_REL);			
			}
			else
			{
#if defined(CONFIG_OF)
				enable_irq(obj->irq);
#elif defined(CUST_EINT_ALS_TYPE)
				mt_eint_unmask(CUST_EINT_ALS_NUM);
#else
				mt65xx_eint_unmask(CUST_EINT_ALS_NUM);
#endif
			}
		}			
	}
	else
	{
		org_mode = obj->ges_enabled;
		if(org_mode == 2)
		{	
			if(obj->hw->polling_mode_ps == 1)
			{
				//hrtimer_cancel(&obj->ps_tune0_timer);					
				//cancel_work_sync(&obj->stk_ps_tune0_work);					
			}
			else
			{
#ifdef MT6516        
				MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  
#elif ((defined MT6573) || (defined MT6575) || (defined MT6577) || (defined MT6589) || (defined MT6572))
				mt65xx_eint_mask(CUST_EINT_ALS_NUM);    
#else				
				mt_eint_mask(CUST_EINT_ALS_NUM);
#endif						
			}
		}
		
		err = DiscreteALSPS_write_state(client, 0);
		if(err < 0)
			return err;	
		atomic_set(&obj->state_val, 0);				
		if((err = DiscreteALSPS_write_wait(client, obj->wait_val)))
		{
			APS_ERR("write wait error: %d\n", err);
			return err;
		}	
				
		if((err = DiscreteALSPS_write_int(obj->client, obj->int_val)))
		{
			APS_ERR("write int mode error: %d\n", err);
			return err;        
		}				
		if(org_mode == 2)
		{		
			err = DiscreteALSPS_read_gsctrl(client, &reg);
			if(err < 0)
				return err;					
	
			reg &= 0xF3;
			err = DiscreteALSPS_write_gsctrl(client, reg);		
			if(err < 0)
				return err;		
		}
		obj->ges_enabled = 0;
		/*
		if(obj->re_enable_ps)
		{
			APS_LOG( "%s: re-enable PS\n", __func__);
			DiscreteALSPS_enable_ps(obj->client, 1, 1);
			obj->re_enable_ps = false;
		}
		*/
		if(obj->re_enable_als) 
		{
			APS_LOG( "%s: re-enable ALS\n", __func__);
			DiscreteALSPS_enable_als(obj->client, 1);
			obj->re_enable_als = false;
		}
	}	

	return 0;
}
#endif /* #ifdef STK_GES */

/*----------------------------------------------------------------------------*/

#if 0
static int DiscreteALSPS_ps_calibration(struct i2c_client *client) 
{
	return 0;
#if 0
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);	
	u8 reg;	
	int err = 0;
	int org_state_reg = atomic_read(&obj->state_val);
	bool re_en_ps = false, re_en_als = false;
	int counter = 0;
	uint32_t ps_sum = 0;
	const int PS_CALI_COUNT = 5;
	
#ifdef STK_GES	
	bool re_en_ges = false;
	
	if(obj->ges_enabled)
	{
		DiscreteALSPS_enable_ges(obj->client, 0, 1);
		re_en_ges = true;
	}
#endif	
	if(org_state_reg & STK_STATE_EN_PS_MASK)
	{
		APS_LOG( "%s: force disable PS\n", __func__);
		DiscreteALSPS_enable_ps(obj->client, 0, 1);
		re_en_ps = true;
	}
	
	if(org_state_reg & STK_STATE_EN_ALS_MASK) 
	{
		APS_LOG( "%s: force disable ALS\n", __func__);						
		DiscreteALSPS_enable_als(obj->client, 0);		
		re_en_als = true;
	}
	
	disable_irq_nosync(DiscreteALSPS_obj->irq);
/*
#ifdef MT6516        
	MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  
#elif ((defined MT6573) || (defined MT6575) || (defined MT6577) || (defined MT6589) || (defined MT6572))
	mt65xx_eint_mask(CUST_EINT_ALS_NUM);    
#else				
	mt_eint_mask(CUST_EINT_ALS_NUM);
#endif	*/
	
	reg = STK_STATE_EN_WAIT_MASK | STK_STATE_EN_PS_MASK; 
	err = DiscreteALSPS_write_state(client, reg);
	if(err < 0)
		goto err_out;
	
	while(counter < PS_CALI_COUNT)
	{
		msleep(60);
		if((err = DiscreteALSPS_read_ps(obj->client, &obj->ps)))		
			goto err_out;
//#ifdef STK_PS_POLLING_LOG			
		APS_LOG( "%s: ps=%d\n", __func__, obj->ps);		
//#endif		
		ps_sum += obj->ps;		
		counter++;
	}
	
	obj->ps_cali = (ps_sum / PS_CALI_COUNT);	
	
err_out:	
#if defined(CONFIG_OF)
	enable_irq(obj->irq);
#elif defined(CUST_EINT_ALS_TYPE)
	mt_eint_unmask(CUST_EINT_ALS_NUM);
#else
	mt65xx_eint_unmask(CUST_EINT_ALS_NUM);
#endif
	
	if(re_en_als) 
	{
		APS_LOG( "%s: re-enable ALS\n", __func__);
		DiscreteALSPS_enable_als(obj->client, 1);
	}
		
	if(re_en_ps)
	{
		APS_LOG( "%s: re-enable PS\n", __func__);
		DiscreteALSPS_enable_ps(obj->client, 1, 1);
	}
#ifdef STK_GES	
	if(re_en_ges)	
		DiscreteALSPS_enable_ges(obj->client, 1, 1);		
#endif		
	return err;
#endif
}

/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_check_intr(struct i2c_client *client, u8 *status) 
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);
	int err;

	//if (mt_get_gpio_in(GPIO_ALS_EINT_PIN) == 1) /*skip if no interrupt*/  
	//    return 0;

	err = DiscreteALSPS_read_flag(client, status);	
	if (err < 0)
	{
		APS_ERR("WARNING: read flag reg error: %d\n", err);
		return -EFAULT;
	}
	APS_LOG("%s: read status reg: 0x%x\n", __func__, *status);
    
	if(*status & STK_FLG_ALSINT_MASK)
	{
		set_bit(STK_BIT_ALS, &obj->pending_intr);
	}
	else
	{
	   clear_bit(STK_BIT_ALS, &obj->pending_intr);
	}
	
	if(*status & STK_FLG_PSINT_MASK)
	{
		set_bit(STK_BIT_PS,  &obj->pending_intr);
	}
	else
	{
	    clear_bit(STK_BIT_PS, &obj->pending_intr);
	}
	
	if(atomic_read(&obj->trace) & STK_TRC_DEBUG)
	{
		APS_LOG("check intr: 0x%02X => 0x%08lX\n", *status, obj->pending_intr);
	}

	return 0;
}


static int DiscreteALSPS_clear_intr(struct i2c_client *client, u8 status, u8 disable_flag) 
{
    int err = 0;

    status = status | (STK_FLG_ALSINT_MASK | STK_FLG_PSINT_MASK | STK_FLG_OUI_MASK | STK_FLG_IR_RDY_MASK);
    status &= (~disable_flag);
	//APS_LOG(" set flag reg: 0x%x\n", status);
	if((err = DiscreteALSPS_write_flag(client, status)))
		APS_ERR("DiscreteALSPS_write_flag failed, err=%d\n", err);
    return err;
}
#endif

/*----------------------------------------------------------------------------*/
#ifdef STK_CHK_REG	
#if 0
static int DiscreteALSPS_chk_reg_valid(struct DiscreteALSPS_priv *obj) 
{    
	int ret = 0;	
	u8 buf[9];
	
	if(NULL == obj)
	{
		return -EINVAL;
	}	
	memset(buf, 0, sizeof(buf));

	ret = DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, 1, &buf[0], 7);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}	
	ret = DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, 8, &buf[7], 2);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}		

	if(buf[0] != atomic_read(&obj->psctrl_val))
	{
		APS_ERR("%s: invalid reg 0x01=0x%2x\n", __func__, buf[0]);
		return 0xFF;
	}
	if(buf[1] != atomic_read(&obj->alsctrl_val))
	{
		APS_ERR("%s: invalid reg 0x02=0x%2x\n", __func__, buf[1]);
		return 0xFF;
	}
	if(buf[2] != obj->ledctrl_val)
	{
		APS_ERR("%s: invalid reg 0x03=0x%2x\n", __func__, buf[2]);
		return 0xFF;
	}		
	if(buf[3] != obj->int_val)
	{
		APS_ERR("%s: invalid reg 0x04=0x%2x\n", __func__, buf[3]);
		return 0xFF;
	}
	if(buf[4] != obj->wait_val)
	{
		APS_ERR("%s: invalid reg 0x05=0x%2x\n", __func__, buf[4]);
		return 0xFF;
	}	
	if(buf[5] != (atomic_read(&obj->ps_high_thd_val) & 0xFF00) >> 8)
	{
		APS_ERR("%s: invalid reg 0x06=0x%2x\n", __func__, buf[5]);
		return 0xFF;
	}	
	if(buf[6] != (atomic_read(&obj->ps_high_thd_val) & 0x00FF))
	{
		APS_ERR("%s: invalid reg 0x07=0x%2x\n", __func__, buf[6]);
		return 0xFF;
	}	
	if(buf[7] != (atomic_read(&obj->ps_low_thd_val) & 0xFF00) >> 8)
	{
		APS_ERR("%s: invalid reg 0x08=0x%2x\n", __func__, buf[7]);
		return 0xFF;
	}	
	if(buf[8] != (atomic_read(&obj->ps_low_thd_val) & 0x00FF))
	{
		APS_ERR("%s: invalid reg 0x09=0x%2x\n", __func__, buf[8]);
		return 0xFF;
	}	
		
	return 0;
}
#endif

#if 0
static int DiscreteALSPS_validate_n_handle(struct i2c_client *client) 
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client); 
	int err;
	
	err = DiscreteALSPS_chk_reg_valid(obj);
	if(err < 0)
	{
		APS_ERR("DiscreteALSPS_chk_reg_valid fail: %d\n", err);        
		return err;
	}
	
	if(err == 0xFF)
	{		
		APS_ERR("%s: Re-init chip\n", __func__);
		DiscreteALSPS_init_client(obj->client);		
		//obj->psa = 0;
		//obj->psi = 0xFFFF;		
		if((err = DiscreteALSPS_write_ps_high_thd(client, atomic_read(&obj->ps_high_thd_val))))
		{
			APS_ERR("write high thd error: %d\n", err);
			return err;        
		}
		
		if((err = DiscreteALSPS_write_ps_low_thd(client, atomic_read(&obj->ps_low_thd_val))))
		{
			APS_ERR("write low thd error: %d\n", err);
			return err;        
		}		
		
		return 0xFF;
	}
	return 0;
}
#endif
#endif /* #ifdef STK_CHK_REG	*/
#if 0
/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_set_als_int_thd(struct i2c_client *client, u16 als_data_reg) 
{
	s32 als_thd_h, als_thd_l;	
		
    als_thd_h = als_data_reg + STK_ALS_CODE_CHANGE_THD;
    als_thd_l = als_data_reg - STK_ALS_CODE_CHANGE_THD;
    if (als_thd_h >= (1<<16))
        als_thd_h = (1<<16) -1;
    if (als_thd_l <0)
        als_thd_l = 0;
	APS_LOG("DiscreteALSPS_set_als_int_thd:als_thd_h:%d,als_thd_l:%d\n", als_thd_h, als_thd_l);	
		
	DiscreteALSPS_write_als_high_thd(client, als_thd_h);
	DiscreteALSPS_write_als_low_thd(client, als_thd_l);

	return 0;
}
#endif
#if 0 //hct-drv.
static int DiscreteALSPS_ps_val(void)
{
	int mode;
	int32_t word_data, lii;
	u8 buf[4];
	int ret;
	
	ret = DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, 0x20, buf, 4);
	if(ret < 0)	
	{
		APS_ERR("%s fail, err=0x%x", __FUNCTION__, ret);
		return ret;	   
	}
	word_data = (buf[0] << 8) | buf[1];
	word_data += (buf[2] << 8) | buf[3];	
	
	mode = atomic_read(&DiscreteALSPS_obj->psctrl_val) & 0x3F;
	if(mode == 0x30)	
	{
		lii = 100;	
	}
	else if (mode == 0x31)
	{
		lii = 200;		
	}
	else if (mode == 0x32)
	{
		lii = 400;				
	}
	else if (mode == 0x33)
	{
		lii = 800;			
	}
	else
	{
		APS_ERR("%s: unsupported PS_IT(0x%x)\n", __FUNCTION__, mode);
		return -1;
	}
	
	if(word_data > lii)	
	{
		APS_LOG( "%s: word_data=%d, lii=%d\n", __FUNCTION__, word_data, lii);		
		return 0xFFFF;	
	}
	return 0;
}
#endif
#ifdef STK_GES		
#if 0 //hct-drv.
static int32_t stk_ges_poll_func(struct DiscreteALSPS_priv *obj)
{
	u8 disable_flag2 = 0, org_flag2_reg, w_flag2_reg;
	int ret;
		
	if(obj->ges_enabled == 2)
	{
		ret = DiscreteALSPS_read_flag2(obj->client, &org_flag2_reg);
		if(ret < 0)
			return ret;	
		
		disable_flag2 = org_flag2_reg & (STK_FLG2_INT_GS_MASK | 
			STK_FLG2_GS10_MASK | STK_FLG2_GS01_MASK);
		if(org_flag2_reg & STK_FLG2_GS10_MASK)
		{
			APS_LOG( "%s: >>>>>>>>>>>>\n", __func__);
		}
		if(org_flag2_reg & STK_FLG2_GS01_MASK)
		{
			APS_LOG( "%s: <<<<<<<<<<<<\n", __func__);
		}
		atomic_set(&obj->gesture2, (disable_flag2 & 
			(STK_FLG2_GS10_MASK | STK_FLG2_GS01_MASK)));  		
		
		if(disable_flag2)
		{
			w_flag2_reg	= org_flag2_reg | (STK_FLG2_INT_GS_MASK | STK_FLG2_GS10_MASK | STK_FLG2_GS01_MASK);
			w_flag2_reg &= (~disable_flag2);
			ret = DiscreteALSPS_write_flag2(obj->client, w_flag2_reg);	
			if(ret < 0)
				return ret;
		}			
	}		
	
	return 0;
}
#endif
#endif

#ifdef STK_TUNE0	
#if 0 //hct-drv.

static int stk_ps_tune_zero_final(struct DiscreteALSPS_priv *obj)
{
	int err;
	
	obj->tune_zero_init_proc = false;
	if((err = DiscreteALSPS_write_int(obj->client, obj->int_val)))
	{
		APS_ERR("write int mode error: %d\n", err);
		return err;        
	}	
	
	if((err = DiscreteALSPS_write_state(obj->client, atomic_read(&obj->state_val))))
	{
		APS_ERR("write stete error: %d\n", err);
		return err;        
	}		
	
	if(obj->data_count == -1)
	{
		APS_LOG("%s: exceed limit\n", __func__);
		//hrtimer_cancel(&obj->ps_tune0_timer);	
		return 0;
	}	
	
	obj->psa = obj->ps_stat_data[0];
	obj->psi = obj->ps_stat_data[2];							

#ifndef CALI_EVERY_TIME
	atomic_set(&obj->ps_high_thd_val, obj->ps_stat_data[1] + obj->stk_ht_n_ct); 
	atomic_set(&obj->ps_low_thd_val, obj->ps_stat_data[1] + obj->stk_lt_n_ct); 		
#else						
	obj->ps_high_thd_boot = obj->ps_stat_data[1] + obj->stk_ht_n_ct;
	obj->ps_low_thd_boot = obj->ps_stat_data[1] + obj->stk_lt_n_ct;
	atomic_set(&obj->ps_high_thd_val, obj->ps_high_thd_boot); 
	atomic_set(&obj->ps_low_thd_val, obj->ps_low_thd_boot); 
#endif

	if((err = DiscreteALSPS_write_ps_high_thd(obj->client, atomic_read(&obj->ps_high_thd_val))))
	{
		APS_ERR("write high thd error: %d\n", err);
		return err;        
	}	
	if((err = DiscreteALSPS_write_ps_low_thd(obj->client, atomic_read(&obj->ps_low_thd_val))))
	{
		APS_ERR("write low thd error: %d\n", err);
		return err;        
	}
	obj->boot_cali = 1;
	
	APS_LOG("%s: set HT=%d,LT=%d\n", __func__, atomic_read(&obj->ps_high_thd_val),  atomic_read(&obj->ps_low_thd_val));		
	//hrtimer_cancel(&obj->ps_tune0_timer);					
	return 0;
}

static int32_t stk_tune_zero_get_ps_data(struct DiscreteALSPS_priv *obj)
{
	int err;
	
	err = DiscreteALSPS_ps_val();	
	if(err == 0xFFFF)
	{	
		obj->data_count = -1;
		stk_ps_tune_zero_final(obj);
		return 0;	
	}
	
	if((err = DiscreteALSPS_read_ps(obj->client, &obj->ps)))
	{
		APS_ERR("DiscreteALSPS read ps data: %d\n", err);
		return err;
	}	
	APS_LOG("%s: ps #%d=%d\n", __func__, obj->data_count, obj->ps);
	
	obj->ps_stat_data[1]  +=  obj->ps;			
	if(obj->ps > obj->ps_stat_data[0])
		obj->ps_stat_data[0] = obj->ps;
	if(obj->ps < obj->ps_stat_data[2])
		obj->ps_stat_data[2] = obj->ps;						
	obj->data_count++;	
	
	if(obj->data_count == 5)
	{
		obj->ps_stat_data[1]  /= obj->data_count;			
		stk_ps_tune_zero_final(obj);
	}		
	
	return 0;
}

static int stk_ps_tune_zero_init(struct DiscreteALSPS_priv *obj)
{
	u8 w_state_reg;	
	int err;
	
	obj->psa = 0;
	obj->psi = 0xFFFF;	
	obj->psi_set = 0;	
	obj->tune_zero_init_proc = true;		
	obj->ps_stat_data[0] = 0;
	obj->ps_stat_data[2] = 9999;
	obj->ps_stat_data[1] = 0;
	obj->data_count = 0;
	obj->boot_cali = 0;
	
#ifdef CALI_EVERY_TIME
	obj->ps_high_thd_boot = obj->hw->ps_threshold_high;
	obj->ps_low_thd_boot = obj->hw->ps_threshold_low;
	if(obj->ps_high_thd_boot <= 0)
	{
		obj->ps_high_thd_boot = obj->stk_ht_n_ct*3;
		obj->ps_low_thd_boot = obj->stk_lt_n_ct*3;
	}
#endif
	
	if((err = DiscreteALSPS_write_int(obj->client, 0)))
	{
		APS_ERR("write int mode error: %d\n", err);
		return err;        
	}	
	
	w_state_reg = (STK_STATE_EN_PS_MASK | STK_STATE_EN_WAIT_MASK);			
	if((err = DiscreteALSPS_write_state(obj->client, w_state_reg)))
	{
		APS_ERR("write stete error: %d\n", err);
		return err;        
	}			
	//hrtimer_start(&obj->ps_tune0_timer, obj->ps_tune0_delay, HRTIMER_MODE_REL);		
	return 0;	
}


static int stk_ps_tune_zero_func_fae(struct DiscreteALSPS_priv *obj)
{
	int32_t word_data;
	u8 flag;
	bool ps_enabled = false;
	u8 buf[2];
	int ret, diff;
	
	ps_enabled = (atomic_read(&obj->state_val) & STK_STATE_EN_PS_MASK) ? true : false;	

#ifndef CALI_EVERY_TIME
	if(obj->psi_set || !(ps_enabled))
#else
	if(!(ps_enabled))
#endif
	{
		return 0;
	}	
	
	ret = DiscreteALSPS_read_flag(obj->client, &flag);
	if(ret < 0)
	{
		APS_ERR( "%s: get flag failed, err=0x%x\n", __func__, ret);
		return ret;
	}
	if(!(flag&STK_FLG_PSDR_MASK))
	{
		return 0;
	}
	
	ret = DiscreteALSPS_ps_val();	
	if(ret == 0)
	{
		ret = DiscreteALSPS_master_recv(obj->client, 0x11, buf, 2);
		if(ret < 0)
		{
			APS_ERR( "%s fail, err=0x%x", __func__, ret);
			return ret;	   
		}
		word_data = (buf[0] << 8) | buf[1];
		//APS_LOG("%s: word_data=%d\n", __func__, word_data);
		
		if(word_data == 0)
		{
			//APS_ERR( "%s: incorrect word data (0)\n", __func__);
			return 0xFFFF;
		}
		
		if(word_data > obj->psa)
		{
			obj->psa = word_data;
			APS_LOG("%s: update psa: psa=%d,psi=%d\n", __func__, obj->psa, obj->psi);
		}
		if(word_data < obj->psi)
		{
			obj->psi = word_data;	
			APS_LOG("%s: update psi: psa=%d,psi=%d\n", __func__, obj->psa, obj->psi);	
		}	
	}	
	
	diff = obj->psa - obj->psi;
	if(diff > obj->stk_max_min_diff)
	{
		obj->psi_set = obj->psi;
		atomic_set(&obj->ps_high_thd_val, obj->psi + obj->stk_ht_n_ct); 
		atomic_set(&obj->ps_low_thd_val, obj->psi + obj->stk_lt_n_ct); 
		
#ifdef CALI_EVERY_TIME
		if( (atomic_read(&obj->ps_high_thd_val) > (obj->ps_high_thd_boot + 500)) && (obj->boot_cali == 1) )
		{
			atomic_set(&obj->ps_high_thd_val, obj->ps_high_thd_boot); 
			atomic_set(&obj->ps_low_thd_val, obj->ps_low_thd_boot); 
			APS_LOG("%s: change THD to boot HT=%d, LT=%d\n", __func__, atomic_read(&obj->ps_high_thd_val), atomic_read(&obj->ps_low_thd_val));
		}
		else
		{
			APS_LOG("%s: boot HT=%d, LT=%d, boot_cali=%d\n", __func__, obj->ps_high_thd_boot, obj->ps_low_thd_boot, obj->boot_cali);
		}
#endif

		if((ret = DiscreteALSPS_write_ps_high_thd(obj->client, atomic_read(&obj->ps_high_thd_val))))
		{
			APS_ERR("write high thd error: %d\n", ret);
			return ret;        
		}		
		if((ret = DiscreteALSPS_write_ps_low_thd(obj->client, atomic_read(&obj->ps_low_thd_val))))
		{
			APS_ERR("write low thd error: %d\n", ret);
			return ret;        
		}	
#ifdef STK_DEBUG_PRINTF				
		APS_LOG("%s: FAE tune0 psa-psi(%d) > DIFF found\n", __func__, diff);
#endif					
		APS_LOG("%s: set HT=%d, LT=%d\n", __func__, atomic_read(&obj->ps_high_thd_val), atomic_read(&obj->ps_low_thd_val));
		//hrtimer_cancel(&obj->ps_tune0_timer);
	}
	
	return 0;
}	
#endif

#endif	/*#ifdef STK_TUNE0	*/

#if 0 //hct-drv
static void stk_ps_tune0_work_func(struct work_struct *work)
{
	struct DiscreteALSPS_priv *obj = container_of(work, struct DiscreteALSPS_priv, stk_ps_tune0_work);		
#ifdef STK_GES		
	if(obj->ges_enabled)
		stk_ges_poll_func(obj);
#endif
	if(obj->tune_zero_init_proc)
		stk_tune_zero_get_ps_data(obj);
	else
		stk_ps_tune_zero_func_fae(obj);
	return;
}	

static enum hrtimer_restart stk_ps_tune0_timer_func(struct hrtimer *timer)
{
	struct DiscreteALSPS_priv *obj = container_of(timer, struct DiscreteALSPS_priv, ps_tune0_timer);
	//queue_work(obj->stk_ps_tune0_wq, &obj->stk_ps_tune0_work);	
	hrtimer_forward_now(&obj->ps_tune0_timer, obj->ps_tune0_delay);
	return HRTIMER_RESTART;	
}
/*----------------------------------------------------------------------------*/
void DiscreteALSPS_eint_func(void)
{
	struct DiscreteALSPS_priv *obj = DiscreteALSPS_obj;
	APS_LOG(" interrupt fuc\n");
	if(!obj)
	{
		return;
	}
	//schedule_work(&obj->eint_work);
	//if(obj->hw->polling_mode_ps == 0 || obj->hw->polling_mode_als == 0)
	//	schedule_delayed_work(&obj->eint_work,0);
	if(atomic_read(&obj->trace) & STK_TRC_EINT)
	{
		APS_LOG("eint: als/ps intrs\n");
	}
}
/*----------------------------------------------------------------------------*/
static void DiscreteALSPS_eint_work(struct work_struct *work)
{
	struct DiscreteALSPS_priv *obj = DiscreteALSPS_obj;
	int err;
	int report_value = 1;
	//hwm_sensor_data sensor_data;
	u8 flag_reg;
	u8 disable_flag=0;
	//memset(&sensor_data, 0, sizeof(sensor_data));

	APS_LOG(" eint work\n");
	
	if((err = DiscreteALSPS_check_intr(obj->client, &flag_reg)))
	{
		APS_ERR("DiscreteALSPS_check_intr fail: %d\n", err);
		goto err_i2c_rw;
	}

    APS_LOG(" &obj->pending_intr =%lx\n",obj->pending_intr);
#if 1	
	if(((1<<STK_BIT_PS) &  obj->pending_intr) && (obj->hw->polling_mode_ps == 0))
	{
		APS_LOG("stk ps change\n");
		disable_flag |= STK_FLG_PSINT_MASK;
		
		if((err = DiscreteALSPS_read_ps(obj->client, &obj->ps)))
		{
			APS_ERR("DiscreteALSPS read ps data: %d\n", err);
			goto err_i2c_rw;
		}
		
		report_value = (flag_reg & STK_FLG_NF_MASK)? 1 : 0;

		APS_LOG("%s:ps raw 0x%x -> value 0x%x \n",__FUNCTION__, obj->ps, report_value);
		//let up layer to know
/*
		if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
		{	
			APS_ERR("call hwmsen_get_interrupt_data fail = %d\n", err);
		}
*/
		if(ps_report_interrupt_data(report_value))
				{	
					APS_ERR("call ps_report_interrupt_data fail\n");
				} 
								
	}
	if(disable_flag)
	{
		if((err = DiscreteALSPS_clear_intr(obj->client, flag_reg, disable_flag)))
		{
			APS_ERR("fail: %d\n", err);
			goto err_i2c_rw;
		}		
	}
		
	msleep(1);

#if defined(CONFIG_OF)
	enable_irq(obj->irq);
#endif
#endif

err_i2c_rw:	
	msleep(30);
#if defined(CONFIG_OF)
	enable_irq(obj->irq);
#endif
	return;
}
#endif


#if 0
#if defined(CONFIG_OF)
static irqreturn_t DiscreteALSPS_eint_handler(int irq, void *desc)
{
	printk("----------------DiscreteALSPS_eint_handler-----------------------\n");
	disable_irq_nosync(DiscreteALSPS_obj->irq);
	DiscreteALSPS_eint_func();

	return IRQ_HANDLED;
}
#endif
/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_init_client(struct i2c_client *client)
{
	struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);
	int err;
	int ps_ctrl;
	//u8 int_status;
	
	if((err = DiscreteALSPS_write_sw_reset(client)))
	{
		APS_ERR("software reset error, err=%d", err);
		return err;
	}

	if((err = DiscreteALSPS_read_id(client)))
	{
		APS_ERR("DiscreteALSPS_read_id error, err=%d", err);
		return err;
	}		
	
	
	if((err = DiscreteALSPS_write_state(client, atomic_read(&obj->state_val))))
	{
		APS_ERR("write stete error: %d\n", err);
		return err;        
	}	
	
/*	
	if((err = DiscreteALSPS_check_intr(client, &int_status)))
	{
		APS_ERR("check intr error: %d\n", err);
		return err;
	}
	
	if((err = DiscreteALSPS_clear_intr(client, int_status, STK_FLG_PSINT_MASK | STK_FLG_ALSINT_MASK)))
	{
		APS_ERR("clear intr error: %d\n", err);	
		return err;
	}
*/	
	ps_ctrl = atomic_read(&obj->psctrl_val);
	if(obj->hw->polling_mode_ps == 1)
		ps_ctrl &= 0x3F;
	
	if((err = DiscreteALSPS_write_ps(client, ps_ctrl)))
	{
		APS_ERR("write ps error: %d\n", err);
		return err;        
	}
	
	if((err = DiscreteALSPS_write_als(client, atomic_read(&obj->alsctrl_val))))
	{
		APS_ERR("write als error: %d\n", err);
		return err;
	}	
	
	if((err = DiscreteALSPS_write_led(client, obj->ledctrl_val)))
	{
		APS_ERR("write led error: %d\n", err);
		return err;
	}	
	
	if((err = DiscreteALSPS_write_wait(client, obj->wait_val)))
	{
		APS_ERR("write wait error: %d\n", err);
		return err;
	}	
#ifndef STK_TUNE0	
	if((err = DiscreteALSPS_write_ps_high_thd(client, atomic_read(&obj->ps_high_thd_val))))
	{
		APS_ERR("write high thd error: %d\n", err);
		return err;        
	}
	
	if((err = DiscreteALSPS_write_ps_low_thd(client, atomic_read(&obj->ps_low_thd_val))))
	{
		APS_ERR("write low thd error: %d\n", err);
		return err;        
	}
#endif	
	if((err = DiscreteALSPS_write_int(client, obj->int_val)))
	{
		APS_ERR("write int mode error: %d\n", err);
		return err;        
	}		
	
	/*
	u8 data;
	data = 0x60;
    err = DiscreteALSPS_master_send(client, 0x87, &data, 1);
	if (err < 0)
	{
		APS_ERR("write 0x87 = %d\n", err);
		return -EFAULT;
	} 
	*/
#ifdef STK_ALS_FIR
	memset(&obj->fir, 0x00, sizeof(obj->fir));  
#endif
#ifdef STK_TUNE0
	if(obj->first_boot == true)
		stk_ps_tune_zero_init(obj);
#endif	
#ifdef STK_GES	
	obj->re_enable_ges = false;	
	obj->re_enable_ges2 = 0;	
	atomic_set(&obj->gesture2, 0);
#endif
	obj->re_enable_ps = false;
	obj->re_enable_als = false;
	obj->als_code_last = 500;
	
	return 0;
}
#endif
/******************************************************************************
 * Sysfs attributes
*******************************************************************************/
static ssize_t DiscreteALSPS_show_config(struct device_driver *ddri, char *buf)
{
    //hct-drv need run both, but now we only show one.
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    //DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_config(ddri, buf);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->show_config(ddri, buf);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_store_config(struct device_driver *ddri, const char *buf, size_t count)
{
    //hct-drv need run both
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    DiscreteALSPS_func(DISCRETEALSPS_ALS)->store_config(ddri, buf, count);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->store_config(ddri, buf, count);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_trace(struct device_driver *ddri, char *buf)
{
    //hct-drv need run both, but now we only show one.
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
   // DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_trace(ddri, buf); //HCT TODO
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->show_trace(ddri, buf);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_store_trace(struct device_driver *ddri, const char *buf, size_t count)
{
    //hct-drv need run both, set the trace.
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    DiscreteALSPS_func(DISCRETEALSPS_ALS)->store_trace(ddri, buf, count);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->store_trace(ddri, buf, count);
}

/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_ir(struct device_driver *ddri, char *buf)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_ir(ddri, buf);
}

/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_als(struct device_driver *ddri, char *buf)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_als(ddri, buf);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_ps(struct device_driver *ddri, char *buf)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->show_ps(ddri, buf);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_reg(struct device_driver *ddri, char *buf)
{
    //hct-drv we can run both, because the buf is no use.
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_reg(ddri, buf);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->show_reg(ddri, buf);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_send(struct device_driver *ddri, char *buf)
{
    //hct-drv we can run both, because the buf is no use.
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_send(ddri, buf);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->show_send(ddri, buf);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_store_send(struct device_driver *ddri, const char *buf, size_t count)
{
    //hct-drv we can run both, because the buf is no use.
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    DiscreteALSPS_func(DISCRETEALSPS_ALS)->store_send(ddri, buf, count);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->store_send(ddri, buf, count);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_recv(struct device_driver *ddri, char *buf)
{
    //hct-drv not sure is als or ps, we use ps temp.
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->show_recv(ddri, buf);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_store_recv(struct device_driver *ddri, const char *buf, size_t count)
{
    //hct-drv not sure is als or ps, we use ps temp.
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->store_recv(ddri, buf, count);
}
/*----------------------------------------------------------------------------*/
#if 0 //def STK_PS_DEBUG
static int show_allreg(void)
{
	int ret = 0;
	u8 rbuf[0x22];
	int cnt;	
	int len = 0;
	
	memset(rbuf, 0, sizeof(rbuf));
	if(!DiscreteALSPS_obj)
	{
		APS_ERR("DiscreteALSPS_obj is null!!\n");
		return 0;
	}
	ret = DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, 0, &rbuf[0], 7);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}	
	ret = DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, 7, &rbuf[7], 7);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}	
	ret = DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, 14, &rbuf[14], 7);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}	
	ret = DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, 21, &rbuf[21], 7);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}
	ret = DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, 28, &rbuf[28], 4);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}
	
	ret = DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, STK_PDT_ID_REG, &rbuf[32], 2);
	if(ret < 0)
	{
		APS_DBG("error: %d\n", ret);
		return -EFAULT;
	}	
	
	for(cnt=0;cnt<0x20;cnt++)
	{
		APS_LOG("reg[0x%x]=0x%x\n", cnt, rbuf[cnt]);
	}	
	APS_LOG("reg[0x3E]=0x%x\n", rbuf[cnt]);
	APS_LOG("reg[0x3F]=0x%x\n", rbuf[cnt++]);

	return 0;
}
#endif
static ssize_t DiscreteALSPS_show_allreg(struct device_driver *ddri, char *buf)
{
    //hct-drv we need run both, no only show ps.
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    //DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_allreg(ddri, buf);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->show_allreg(ddri, buf);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_status(struct device_driver *ddri, char *buf)
{
    //hct-drv we need run both, no only show ps.
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    //DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_status(ddri, buf);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->show_status(ddri, buf);
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_alslv(struct device_driver *ddri, char *buf)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_alslv(ddri, buf);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_store_alslv(struct device_driver *ddri, const char *buf, size_t count)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->store_alslv(ddri, buf, count);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_alsval(struct device_driver *ddri, char *buf)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_alsval(ddri, buf);
}
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_store_alsval(struct device_driver *ddri, const char *buf, size_t count)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->store_alsval(ddri, buf, count);
}

#ifdef STK_TUNE0
static ssize_t DiscreteALSPS_show_cali(struct device_driver *ddri, char *buf)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->show_cali(ddri, buf);
}

static ssize_t DiscreteALSPS_ps_maxdiff_store(struct device_driver *ddri, const char *buf, size_t count)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_maxdiff_store(ddri, buf, count);
}


static ssize_t DiscreteALSPS_ps_maxdiff_show(struct device_driver *ddri, char *buf)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_maxdiff_show(ddri, buf);
}

static ssize_t DiscreteALSPS_ps_ltnct_store(struct device_driver *ddri, const char *buf, size_t count)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_ltnct_store(ddri, buf, count);
}

static ssize_t DiscreteALSPS_ps_ltnct_show(struct device_driver *ddri, char *buf)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_ltnct_show(ddri, buf);
}

static ssize_t DiscreteALSPS_ps_htnct_store(struct device_driver *ddri, const char *buf, size_t count)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_htnct_store(ddri, buf, count);
}

static ssize_t DiscreteALSPS_ps_htnct_show(struct device_driver *ddri, char *buf)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_htnct_show(ddri, buf);
}

#endif

#ifdef STK_ALS_FIR
/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_show_firlen(struct device_driver *ddri, char *buf)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->show_firlen(ddri, buf);
}

/*----------------------------------------------------------------------------*/
static ssize_t DiscreteALSPS_store_firlen(struct device_driver *ddri, const char *buf, size_t count)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->store_firlen(ddri, buf, count);
}
#endif /* #ifdef STK_ALS_FIR */
/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(als,     S_IWUSR | S_IRUGO, DiscreteALSPS_show_als,   NULL);
static DRIVER_ATTR(ps,      S_IWUSR | S_IRUGO, DiscreteALSPS_show_ps,    NULL);
static DRIVER_ATTR(ir,      S_IWUSR | S_IRUGO, DiscreteALSPS_show_ir,    NULL);
static DRIVER_ATTR(config,  S_IWUSR | S_IRUGO, DiscreteALSPS_show_config,DiscreteALSPS_store_config);
static DRIVER_ATTR(alslv,   S_IWUSR | S_IRUGO, DiscreteALSPS_show_alslv, DiscreteALSPS_store_alslv);
static DRIVER_ATTR(alsval,  S_IWUSR | S_IRUGO, DiscreteALSPS_show_alsval,DiscreteALSPS_store_alsval);
static DRIVER_ATTR(trace,   S_IWUSR | S_IRUGO, DiscreteALSPS_show_trace, DiscreteALSPS_store_trace);
static DRIVER_ATTR(status,  S_IWUSR | S_IRUGO, DiscreteALSPS_show_status,  NULL);
static DRIVER_ATTR(send,    S_IWUSR | S_IRUGO, DiscreteALSPS_show_send,  DiscreteALSPS_store_send);
static DRIVER_ATTR(recv,    S_IWUSR | S_IRUGO, DiscreteALSPS_show_recv,  DiscreteALSPS_store_recv);
static DRIVER_ATTR(reg,     S_IWUSR | S_IRUGO, DiscreteALSPS_show_reg,   NULL);
static DRIVER_ATTR(allreg,  S_IWUSR | S_IRUGO, DiscreteALSPS_show_allreg,   NULL);
#ifdef STK_TUNE0
static DRIVER_ATTR(cali,    S_IWUSR | S_IRUGO, DiscreteALSPS_show_cali,  NULL);
static DRIVER_ATTR(maxdiff,    S_IWUSR | S_IRUGO, DiscreteALSPS_ps_maxdiff_show,  DiscreteALSPS_ps_maxdiff_store);
static DRIVER_ATTR(ltnct,    S_IWUSR | S_IRUGO, DiscreteALSPS_ps_ltnct_show,  DiscreteALSPS_ps_ltnct_store);
static DRIVER_ATTR(htnct,    S_IWUSR | S_IRUGO, DiscreteALSPS_ps_htnct_show,  DiscreteALSPS_ps_htnct_store);
#endif
#ifdef STK_ALS_FIR
static DRIVER_ATTR(firlen,    S_IWUSR | S_IRUGO, DiscreteALSPS_show_firlen,  DiscreteALSPS_store_firlen);
#endif
/*----------------------------------------------------------------------------*/
static struct driver_attribute *DiscreteALSPS_attr_list[] = {
    &driver_attr_als,
    &driver_attr_ps,    
    &driver_attr_ir,    
    &driver_attr_trace,        /*trace log*/
    &driver_attr_config,
    &driver_attr_alslv,
    &driver_attr_alsval,
    &driver_attr_status,
    &driver_attr_send,
    &driver_attr_recv,
    &driver_attr_allreg,
//    &driver_attr_i2c,
    &driver_attr_reg,
#ifdef STK_TUNE0
    &driver_attr_cali,
    &driver_attr_maxdiff,
    &driver_attr_ltnct,
    &driver_attr_htnct,
#endif	
#ifdef STK_ALS_FIR
    &driver_attr_firlen,
#endif	
};

/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(DiscreteALSPS_attr_list)/sizeof(DiscreteALSPS_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if((err = driver_create_file(driver, DiscreteALSPS_attr_list[idx])))
		{            
			APS_ERR("driver_create_file (%s) = %d\n", DiscreteALSPS_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(DiscreteALSPS_attr_list)/sizeof(DiscreteALSPS_attr_list[0]));

	if (!driver)
	return -EINVAL;

	for (idx = 0; idx < num; idx++) 
	{
		driver_remove_file(driver, DiscreteALSPS_attr_list[idx]);
	}
	
	return err;
}


#ifdef STK_GES		
static ssize_t stk_ges_poll_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int len = 0, ii = 0, jj = 0;
	
	while(stk_ges_op[ii].ops[0] != 0)
	{
		len += scnprintf(buf + len, PAGE_SIZE - len, "%x ", ii);
		for(jj=0;jj<4;jj++)
			len += scnprintf(buf + len, PAGE_SIZE - len, "%x ", stk_ges_op[ii].ops[jj]);
		len += scnprintf(buf + len, PAGE_SIZE - len, "\n");
		ii++;
	}
	return len;
}

static ssize_t stk_ges_poll_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int32_t ret, i = 0, index = 0;	
	char *token;
	unsigned long value = 0;
		
	while(buf != '\0')
	{
		token = strsep((char **)&buf, " ");
		if((ret = kstrtoul(token, 16, &value)) < 0)
		{
		//	printk(KERN_ERR "%s:kstrtoul failed, ret=0x%x\n", __func__, ret);
			return ret;	
		}
		
		if(i == 0)
		{
			if(value >= 10)
			{
				memset(stk_ges_op, 0, sizeof(stk_ges_op));				
				break;
			}
			else
				index = value;
		}
		else
		{
			stk_ges_op[index].ops[i-1] = value;
		}
		i++;
		if(i == 5)
			break;
	}
	if(i != 5)
	{
	//	printk(KERN_ERR "%s: invalid length(%d)\n", __func__, i);
		memset(&(stk_ges_op[index]), 0, sizeof(union stk_ges_operation));				
	}
	return size;
}
		
static ssize_t stk_ges_code_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct DiscreteALSPS_priv *obj =  dev_get_drvdata(dev);	
    int ret;
	unsigned int gest0 = 0, gest1 = 0, gest2 = 0;
	
	if(!obj->ges_enabled)
		return 0;
	
	ret = DiscreteALSPS_get_ges_value(obj, &gest0, &gest1, &gest2);	
	if(ret < 0)
		return ret;
	else if(ret == 0xFFFF)
		atomic_set(&obj->gesture2, 0);		
		
    return scnprintf(buf, PAGE_SIZE, "%5d,%5d,%5d\n", gest0, gest1, atomic_read(&obj->gesture2));
}

static ssize_t stk_ges_code_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct DiscreteALSPS_priv *obj =  dev_get_drvdata(dev);
	uint8_t ges;
	unsigned long value = 0;
	int ret;
	
	ret = kstrtoul(buf, 16, &value);
	if(ret < 0)
	{
		APS_ERR( "%s:kstrtoul failed, ret=%d\n", __func__, ret);	
		return ret;	
	}	
	
	if(obj->ges_enabled)
	{
		switch(value)
		{
		case 3:
			//APS_LOG( "%s: ges input event, not detected\n",__func__);			
		case 0:
			return size;
		case 1:
			ges = KEY_PAGEUP;	
			atomic_set(&obj->gesture2, 0);
			APS_LOG( "%s: ges input event >>>\n",__func__);		
			break;
		case 2:
			ges = KEY_PAGEDOWN;	
			atomic_set(&obj->gesture2, 0);
			APS_LOG( "%s: ges input event <<<\n",__func__);		
			break;
		case 32:
			ges = KEY_VOLUMEDOWN;
			APS_LOG( "%s: ges input event near\n",__func__);				
			break;
		case 48:
			ges = KEY_VOLUMEUP;
			APS_LOG( "%s: ges input event far\n",__func__);				
			break;
		default:
			APS_ERR( "%s, invalid value %d\n", __func__, (int)value);
			return -EINVAL;
		}

		input_report_key(obj->ges_input_dev, ges, 1);
		input_report_key(obj->ges_input_dev, ges, 0);
		input_sync(obj->ges_input_dev);
	}
    return size;
}

static ssize_t stk_ges_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct DiscreteALSPS_priv *obj =  dev_get_drvdata(dev);
	unsigned long value = 0;
	int ret;
	
	ret = kstrtoul(buf, 16, &value);
	if(ret < 0)
	{
		APS_ERR( "%s:kstrtoul failed, ret=%d\n", __func__, ret);	
		return ret;	
	}	
    APS_LOG( "%s: Enable GES : %d\n", __func__, (int)value);
	
	switch(value)
	{
	case 0:
		if(obj->ges_enabled == 1)
			DiscreteALSPS_enable_ges(obj->client, 0, 1);
		else
			DiscreteALSPS_enable_ges(obj->client, 0, 2);
		break;
	case 1:
		DiscreteALSPS_enable_ges(obj->client, 1, 1);
		break;
	case 2:
		DiscreteALSPS_enable_ges(obj->client, 1, 2);
		break;
	default:
		APS_ERR( "%s, invalid value %d\n", __func__, *buf);
		return -EINVAL;	
	}

    return size;
}

static ssize_t stk_ges_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct DiscreteALSPS_priv *obj =  dev_get_drvdata(dev);
	
	return scnprintf(buf, PAGE_SIZE, "%d\n", obj->ges_enabled);		
}

static ssize_t stk_recv_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if(!DiscreteALSPS_obj)
	{
		APS_ERR("DiscreteALSPS_obj is null!!\n");
		return 0;
	}
	return scnprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&DiscreteALSPS_obj->recv_reg));     	
}

static ssize_t stk_recv_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int addr;
	u8 dat;
	if(!DiscreteALSPS_obj)
	{
		APS_ERR("DiscreteALSPS_obj is null!!\n");
		return 0;
	}
	else if(1 != sscanf(buf, "%x", &addr))
	{
		APS_ERR("invalid format: '%s'\n", buf);
		return 0;
	}
	DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, (u16)addr, (char*)&dat, sizeof(dat));
	//APS_LOG("recv(%02X) = %d, 0x%02X\n", addr, 
	//DiscreteALSPS_master_recv(DiscreteALSPS_obj->client, (u16)addr, (char*)&dat, sizeof(dat)), dat);
	atomic_set(&DiscreteALSPS_obj->recv_reg, dat);	
	return size;
}

static ssize_t stk_send_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t stk_send_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int addr, cmd;
	u8 dat;

	if(!DiscreteALSPS_obj)
	{
		APS_ERR("DiscreteALSPS_obj is null!!\n");
		return 0;
	}
	else if(2 != sscanf(buf, "%x %x", &addr, &cmd))
	{
		APS_ERR("invalid format: '%s'\n", buf);
		return 0;
	}

	dat = (u8)cmd;
	APS_LOG("send(%02X, %02X) = %d\n", addr, cmd, 
	DiscreteALSPS_master_send(DiscreteALSPS_obj->client, (u16)addr, &dat, sizeof(dat)));
	
	return size;
}

static struct device_attribute ges_enable_attribute = __ATTR(enable,0664,stk_ges_enable_show,stk_ges_enable_store);
static struct device_attribute ges_code_attribute = __ATTR(code, 0664, stk_ges_code_show, stk_ges_code_store);
static struct device_attribute ges_poll_attribute = __ATTR(poll, 0664, stk_ges_poll_show, stk_ges_poll_store);
static struct device_attribute ges_recv_attribute = __ATTR(recv,0664,stk_recv_show,stk_recv_store);
static struct device_attribute ges_send_attribute = __ATTR(send,0664,stk_send_show, stk_send_store);

static struct attribute *stk_ges_attrs [] =
{
    &ges_enable_attribute.attr,	
    &ges_code_attribute.attr,
    &ges_poll_attribute.attr,	
	&ges_recv_attribute.attr,
	&ges_send_attribute.attr,
    NULL
};

static struct attribute_group stk_ges_attribute_group = 
{
	.name = "driver",	
	.attrs = stk_ges_attrs,
};
#endif	/* #ifdef STK_GES */
/****************************************************************************** 
 * Function Configuration
******************************************************************************/
#ifdef STK_GES		
static uint32_t DiscreteALSPS_get_ges_value(struct DiscreteALSPS_priv *obj, unsigned int *ges0, unsigned int *ges1, unsigned int *ges2)
{
	u8 buf[4];
	int err, retry = 10;
	u8 flag;

	do {
		err = DiscreteALSPS_read_flag(obj->client, &flag);	
		if(err < 0)
			return err;
		if(flag & STK_FLG_PSDR_MASK)
			break;
		//APS_LOG( "%s: ps isnot ready\n", __func__);
		retry--;
		usleep_range(350, 1000);
	} while(retry > 0);

	err = DiscreteALSPS_master_recv(obj->client, obj->addr.data1_ps, buf, 0x02);
	if(err < 0)
	{
		APS_DBG("error: %d\n", err);
		return -EFAULT;
	}

	err = DiscreteALSPS_master_recv(obj->client, 0x24, buf, 0x04);
	if(err < 0)
	{
		APS_DBG("error: %d\n", err);
		return -EFAULT;
	}
	*ges0 = (buf[0]<<8) | buf[1];	
	*ges1 = (buf[2]<<8) | buf[3];	
	//APS_LOG( "%s: ges=%d,%d\n",__func__, *ges0, *ges1);	
	return 0;
}
#endif

#if 0

static int DiscreteALSPS_get_als_value(struct DiscreteALSPS_priv *obj, u16 als)
{
	int idx;
	int invalid = 0;
	for(idx = 0; idx < obj->als_level_num; idx++)
	{
		if(als < obj->hw->als_level[idx])
		{
			break;
		}
	}
	
	if(idx >= obj->als_value_num)
	{
		APS_ERR("exceed range\n"); 
		idx = obj->als_value_num - 1;
	}
	
	if(1 == atomic_read(&obj->als_deb_on))
	{
		unsigned long endt = atomic_read(&obj->als_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->als_deb_on, 0);
		}
		
		if(1 == atomic_read(&obj->als_deb_on))
		{
			invalid = 1;
		}
	}

	if(!invalid)
	{
		if (atomic_read(&obj->trace) & STK_TRC_CVT_ALS)
		{
			APS_DBG("ALS: %05d => %05d\n", als, obj->hw->als_value[idx]);
		}
		
		return obj->hw->als_value[idx];
	}
	else
	{
		if(atomic_read(&obj->trace) & STK_TRC_CVT_ALS)
		{
			APS_DBG("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);    
		}
		return -1;
	}
}

/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_get_ps_value_only(struct DiscreteALSPS_priv *obj, u16 ps)
{
	int mask = atomic_read(&obj->ps_mask);
	int invalid = 0, val;
	int err;
	u8 flag;
	
	err = DiscreteALSPS_read_flag(obj->client, &flag);
	if(err)
		return err;
	val = (flag & STK_FLG_NF_MASK)? 1 : 0;	
	
	if(atomic_read(&obj->ps_suspend))
	{
		invalid = 1;
	}
	else if(1 == atomic_read(&obj->ps_deb_on))
	{
		unsigned long endt = atomic_read(&obj->ps_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->ps_deb_on, 0);
		}
		
		if (1 == atomic_read(&obj->ps_deb_on))
		{
			invalid = 1;
		}
	}		
	
	if(!invalid)
	{
		if(unlikely(atomic_read(&obj->trace) & STK_TRC_CVT_PS))
		{
			if(mask)
			{
				APS_DBG("PS:  %05d => %05d [M] \n", ps, val);
			}
			else
			{
				APS_DBG("PS:  %05d => %05d\n", ps, val);
			}
		}
		return val;
		
	}	
	else
	{
		APS_ERR(" ps value is invalid, PS:  %05d => %05d\n", ps, val);
		if(unlikely(atomic_read(&obj->trace) & STK_TRC_CVT_PS))
		{
			APS_DBG("PS:  %05d => %05d (-1)\n", ps, val);    
		}
		return -1;
	}	
}

/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_get_ps_value(struct DiscreteALSPS_priv *obj, u16 ps)
{
	int mask = atomic_read(&obj->ps_mask);
	int invalid = 0, val;
	int err;
	u8 flag;

	err = DiscreteALSPS_read_flag(obj->client, &flag);
	if(err)
		return err;
	
	val = (flag & STK_FLG_NF_MASK)? 1 : 0;	
	
	APS_LOG("%s: read_flag = 0x%x, val = %d\n", __FUNCTION__, flag, val);
	
/*	if((err = DiscreteALSPS_clear_intr(obj->client, flag, STK_FLG_OUI_MASK)))
	{
		APS_ERR("fail: %d\n", err);
		return err;
	}	*/

	if(atomic_read(&obj->ps_suspend))
	{
		invalid = 1;
	}
	else if(1 == atomic_read(&obj->ps_deb_on))
	{
		unsigned long endt = atomic_read(&obj->ps_deb_end);
		if(time_after(jiffies, endt))
		{
			atomic_set(&obj->ps_deb_on, 0);
		}
		
		if (1 == atomic_read(&obj->ps_deb_on))
		{
			invalid = 1;
		}
	}
		
	
	if(!invalid)
	{
		if(unlikely(atomic_read(&obj->trace) & STK_TRC_CVT_PS))
		{
			if(mask)
			{
				APS_DBG("PS:  %05d => %05d [M] \n", ps, val);
			}
			else
			{
				APS_DBG("PS:  %05d => %05d\n", ps, val);
			}
		}
		return val;
		
	}	
	else
	{
		APS_ERR(" ps value is invalid, PS:  %05d => %05d\n", ps, val);
		if(unlikely(atomic_read(&obj->trace) & STK_TRC_CVT_PS))
		{
			APS_DBG("PS:  %05d => %05d (-1)\n", ps, val);    
		}
		return -1;
	}	
}
#endif

#if 0 //hct-drv disabled.
/*----------------------------------------------------------------------------*/

static int32_t DiscreteALSPS_set_irs_it_slp(struct DiscreteALSPS_priv *obj, uint16_t *slp_time, int32_t ials_it_reduce)
{
	uint8_t irs_alsctrl;
	int32_t ret;
		
	irs_alsctrl = (atomic_read(&obj->alsctrl_val) & 0x0F) - ials_it_reduce;		
	switch(irs_alsctrl)
	{
	case 2:
		*slp_time = 1;
		break;			
	case 3:
		*slp_time = 2;
		break;	
	case 4:
		*slp_time = 3;
		break;	
	case 5:
		*slp_time = 6;
		break;	
	case 6:
		*slp_time = 12;
		break;
	case 7:
		*slp_time = 24;			
		break;
	case 8:
		*slp_time = 48;			
		break;
	case 9:
		*slp_time = 96;			
		break;				
	default:
		APS_ERR( "%s: unknown ALS IT=0x%x\n", __func__, irs_alsctrl);
		ret = -EINVAL;	
		return ret;
	}
	irs_alsctrl |= (atomic_read(&obj->alsctrl_val) & 0xF0);
	ret = i2c_smbus_write_byte_data(obj->client, STK_ALSCTRL_REG, irs_alsctrl);
	if (ret < 0)
	{
		APS_ERR( "%s: write i2c error\n", __func__);
		return ret;		
	}		
	return 0;
}


static int32_t DiscreteALSPS_get_ir_value(struct DiscreteALSPS_priv *obj, int32_t als_it_reduce)
{
    int32_t word_data, ret;
	uint8_t w_reg, retry = 0;	
	uint16_t irs_slp_time = 100;
	//bool re_enable_ps = false;
	u8 flag;
	u8 buf[2];
	
/*	re_enable_ps = (atomic_read(&obj->state_val) & STK_STATE_EN_PS_MASK) ? true : false;	
	if(re_enable_ps)
	{
#ifdef STK_TUNE0		
		if (!(obj->psi_set))
		{
			hrtimer_cancel(&obj->ps_tune0_timer);					
			cancel_work_sync(&obj->stk_ps_tune0_work);
		}		
#endif		
		DiscreteALSPS_enable_ps(obj->client, 0, 1);
	}
*/	
	ret = DiscreteALSPS_set_irs_it_slp(obj, &irs_slp_time, als_it_reduce);
	if(ret < 0)
		goto irs_err_i2c_rw;
		
	w_reg = atomic_read(&obj->state_val) | STK_STATE_EN_IRS_MASK;		
    ret = i2c_smbus_write_byte_data(obj->client, STK_STATE_REG, w_reg);
    if (ret < 0)
	{
		APS_ERR( "%s: write i2c error\n", __func__);
		goto irs_err_i2c_rw;
	}	
	msleep(irs_slp_time);	
	
	do
	{
		msleep(3);		
		ret = DiscreteALSPS_read_flag(obj->client, &flag);	
		if (ret < 0)
		{
			APS_ERR("WARNING: read flag reg error: %d\n", ret);
			goto irs_err_i2c_rw;
		}	
		retry++;
	}while(retry < 10 && ((flag&STK_FLG_IR_RDY_MASK) == 0));
	
	if(retry == 10)
	{
		APS_ERR( "%s: ir data is not ready for a long time\n", __func__);
		ret = -EINVAL;
		goto irs_err_i2c_rw;
	}

	ret = DiscreteALSPS_clear_intr(obj->client, flag, STK_FLG_IR_RDY_MASK);	
    if (ret < 0)
	{
		APS_ERR( "%s: write i2c error\n", __func__);
		goto irs_err_i2c_rw;
	}		
	
	ret = DiscreteALSPS_master_recv(obj->client, STK_DATA1_IR_REG, buf, 2);
	if(ret < 0)	
	{
		APS_ERR( "%s fail, ret=0x%x", __func__, ret); 
		goto irs_err_i2c_rw;		
	}
	word_data =  (buf[0] << 8) | buf[1];

	ret = i2c_smbus_write_byte_data(obj->client, STK_ALSCTRL_REG, atomic_read(&obj->alsctrl_val));
	if (ret < 0)
	{
		APS_ERR( "%s: write i2c error\n", __func__);
		goto irs_err_i2c_rw;
	}
	//if(re_enable_ps)
		//DiscreteALSPS_enable_ps(obj->client, 1, 0);		
	return word_data;

irs_err_i2c_rw:	
	//if(re_enable_ps)
	//	DiscreteALSPS_enable_ps(obj->client, 1, 0);	
	return ret;
}
#endif

/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int DiscreteALSPS_open(struct inode *inode, struct file *file)
{
	file->private_data = DiscreteALSPS_i2c_client;

	if (!file->private_data)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}
	
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
static long DiscreteALSPS_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        switch (cmd)
        {
            case ALSPS_SET_PS_MODE:
            case ALSPS_GET_PS_MODE:
            case ALSPS_GET_PS_DATA:    
            case ALSPS_GET_PS_RAW_DATA:    
            case ALSPS_GET_PS_THRESHOLD_HIGH:
            case ALSPS_GET_PS_THRESHOLD_LOW:			
            case ALSPS_GET_PS_TEST_RESULT: 			
            case ALSPS_IOCTL_CLR_CALI: 
            case ALSPS_IOCTL_GET_CALI: 			
            case ALSPS_IOCTL_SET_CALI:      
                return DiscreteALSPS_func(DISCRETEALSPS_PS)->unlocked_ioctl(file, cmd, arg);

            case ALSPS_SET_ALS_MODE:
            case ALSPS_GET_ALS_MODE:
            case ALSPS_GET_ALS_DATA: 
            case ALSPS_GET_ALS_RAW_DATA:    
                return DiscreteALSPS_func(DISCRETEALSPS_ALS)->unlocked_ioctl(file, cmd, arg);
            default:
                return  -EFAULT;
        }
}
/*----------------------------------------------------------------------------*/
static struct file_operations DiscreteALSPS_fops = {
#if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0))
	.owner = THIS_MODULE,
#endif
	.open = DiscreteALSPS_open,
	.release = DiscreteALSPS_release,
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,36))	
	.unlocked_ioctl = DiscreteALSPS_unlocked_ioctl,
#else
	.ioctl = DiscreteALSPS_ioctl,
#endif

};
/*----------------------------------------------------------------------------*/
static struct miscdevice DiscreteALSPS_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &DiscreteALSPS_fops,
};
/*----------------------------------------------------------------------------*/
#if 0
static int DiscreteALSPS_i2c_suspend(struct i2c_client *client, pm_message_t msg) 
{
	APS_FUN();    
/*
	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(!obj)
		{
			APS_ERR("null pointer!!\n");
			return -EINVAL;
		}
		
		atomic_set(&obj->als_suspend, 1);
		if((err = DiscreteALSPS_enable_als(client, 0)))
		{
			APS_ERR("disable als: %d\n", err);
			return err;
		}

		atomic_set(&obj->ps_suspend, 1);
		if((err = DiscreteALSPS_enable_ps(client, 0, 1)))
		{
			APS_ERR("disable ps:  %d\n", err);
			return err;
		}
		
		DiscreteALSPS_power(obj->hw, 0);
	}
	
*/
	return 0;
}
/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_i2c_resume(struct i2c_client *client)
{
	APS_FUN();
/*
	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}

	DiscreteALSPS_power(obj->hw, 1);
	if((err = DiscreteALSPS_init_client(client)))
	{
		APS_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->als_suspend, 0);
	if(test_bit(STK_BIT_ALS, &obj->enable))
	{
		if((err = DiscreteALSPS_enable_als(client, 1)))
		{
			APS_ERR("enable als fail: %d\n", err);        
		}
	}
	atomic_set(&obj->ps_suspend, 0);
	if(test_bit(STK_BIT_PS,  &obj->enable))
	{
		if((err = DiscreteALSPS_enable_ps(client, 1, 1)))
		{
			APS_ERR("enable ps fail: %d\n", err);                
		}
	}
*/
	return 0;
}

/*----------------------------------------------------------------------------*/
static void DiscreteALSPS_early_suspend(struct early_suspend *h) 
{   /*early_suspend is only applied for ALS*/
	int err;
	struct DiscreteALSPS_priv *obj = container_of(h, struct DiscreteALSPS_priv, early_drv);   	
	int old = atomic_read(&obj->state_val);
	APS_FUN();    

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}
#ifdef STK_CHK_REG		
	err = DiscreteALSPS_validate_n_handle(obj->client);
	if(err < 0)	
	{
		APS_ERR("DiscreteALSPS_validate_n_handle fail: %d\n", err); 
	}
	else if (err == 0xFF)
	{
		if(old & STK_STATE_EN_PS_MASK)
			DiscreteALSPS_enable_ps(obj->client, 1, 0);
	}
#endif /* #ifdef STK_CHK_REG	*/	

#ifdef STK_GES	
	if(obj->ges_enabled == 1)
	{
		obj->re_enable_ges = obj->ges_enabled;		
		DiscreteALSPS_enable_ges(obj->client, 0, 1);	
	}
	else if(obj->ges_enabled == 2)
	{
		obj->re_enable_ges = obj->ges_enabled;	
		DiscreteALSPS_enable_ges(obj->client, 0, 2);	
	}
#endif
	if(old & STK_STATE_EN_ALS_MASK)
	{
		atomic_set(&obj->als_suspend, 1);    
		if((err = DiscreteALSPS_enable_als(obj->client, 0)))
		{
			APS_ERR("disable als fail: %d\n", err); 
		}
		APS_LOG( "%s: Enable ALS : 0\n", __func__);		
	}
}
/*----------------------------------------------------------------------------*/
static void DiscreteALSPS_late_resume(struct early_suspend *h)
{   /*early_suspend is only applied for ALS*/
	int err;
	struct DiscreteALSPS_priv *obj = container_of(h, struct DiscreteALSPS_priv, early_drv);         	
	APS_FUN();

	if(!obj)
	{
		APS_ERR("null pointer!!\n");
		return;
	}
#ifdef STK_CHK_REG		
	err = DiscreteALSPS_validate_n_handle(obj->client);
	if(err < 0)	
	{
		APS_ERR("DiscreteALSPS_validate_n_handle fail: %d\n", err); 
	}
	else if (err == 0xFF)
	{
		if(atomic_read(&obj->state_val) & STK_STATE_EN_PS_MASK)
			DiscreteALSPS_enable_ps(obj->client, 1, 0);
	}	
#endif /* #ifdef STK_CHK_REG	*/	
	
#ifdef STK_GES	
		if(obj->re_enable_ges == 1)
		{
			DiscreteALSPS_enable_ges(obj->client, 1, 1);				
			obj->re_enable_ges = 0;		
		}
		else if(obj->re_enable_ges == 2)
		{
			DiscreteALSPS_enable_ges(obj->client, 1, 2);				
			obj->re_enable_ges = 0;				
		}
#endif	
	
	if(atomic_read(&obj->als_suspend))
	{
		atomic_set(&obj->als_suspend, 0);
		if(test_bit(STK_BIT_ALS, &obj->enable))
		{
			APS_LOG( "%s: Enable ALS : 1\n", __func__);		
			if((err = DiscreteALSPS_enable_als(obj->client, 1)))
			{
				APS_ERR("enable als fail: %d\n", err);        

			}
		}
	}
}

int DiscreteALSPS_ps_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct DiscreteALSPS_priv *obj = (struct DiscreteALSPS_priv *)self;
	
	//APS_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{				
				value = *(int *)buff_in;
				if(value)
				{
					if((err = DiscreteALSPS_enable_ps(obj->client, 1, 1)))
					{
						APS_ERR("enable ps fail: %d\n", err); 
						return -1;
					}
					set_bit(STK_BIT_PS, &obj->enable);
				}
				else
				{
					if((err = DiscreteALSPS_enable_ps(obj->client, 0, 1)))
					{
						APS_ERR("disable ps fail: %d\n", err); 
						return -1;
					}
					clear_bit(STK_BIT_PS, &obj->enable);
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				sensor_data = (hwm_sensor_data *)buff_out;				
				
				if((err = DiscreteALSPS_read_ps(obj->client, &obj->ps)))
				{
					err = -1;
				}
				else
				{
					value = DiscreteALSPS_get_ps_value(obj, obj->ps);
					if(value < 0)
					{
						err = -1;
					}
					else
					{
						sensor_data->values[0] = value;
						sensor_data->value_divide = 1;
						sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
#ifdef STK_PS_POLLING_LOG						
						APS_LOG("%s:ps raw 0x%x -> value 0x%x \n",__FUNCTION__, obj->ps, sensor_data->values[0]);					
#endif				
					}	
				}				
			}
			break;
		default:
			APS_ERR("proximity sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}


int DiscreteALSPS_als_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	hwm_sensor_data* sensor_data;
	struct DiscreteALSPS_priv *obj = (struct DiscreteALSPS_priv *)self;
	u8 flag;
				
	//APS_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				APS_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;				
				if(value)
				{
					if((err = DiscreteALSPS_enable_als(obj->client, 1)))
					{
						APS_ERR("enable als fail: %d\n", err); 
						return -1;
					}
					set_bit(STK_BIT_ALS, &obj->enable);
				}
				else
				{
					if((err = DiscreteALSPS_enable_als(obj->client, 0)))
					{
						APS_ERR("disable als fail: %d\n", err); 
						return -1;
					}
					clear_bit(STK_BIT_ALS, &obj->enable);
				}
				
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				APS_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{			
				sensor_data = (hwm_sensor_data *)buff_out;
#ifdef STK_GES			
				if(obj->ges_enabled)
				{
					sensor_data->values[0] = DiscreteALSPS_get_als_value(obj, obj->als_last);				
					sensor_data->value_divide = 1;
					sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;				
					break;
				}
#endif							
				err = DiscreteALSPS_read_flag(obj->client, &flag);
				if(err)
					return err;
				
				if(!(flag & STK_FLG_ALSDR_MASK))
					return -1;				
				
				if((err = DiscreteALSPS_read_als(obj->client, &obj->als)))
				{
					err = -1;
				}
				else
				{
					if(obj->als < 3)
					{
						obj->als_last = obj->als;
						sensor_data->values[0] = DiscreteALSPS_get_als_value(obj, 0);
					}
					else if(abs(obj->als - obj->als_last) >= STK_ALS_CODE_CHANGE_THD)
					{
						obj->als_last = obj->als;
						sensor_data->values[0] = DiscreteALSPS_get_als_value(obj, obj->als);
					}
					else
					{
						sensor_data->values[0] = DiscreteALSPS_get_als_value(obj, obj->als_last);
					}					
					sensor_data->value_divide = 1;
					sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
				}				
			}
			break;
		default:
			APS_ERR("light sensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}
	
	return err;
}
#endif

/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_i2c_detect(struct i2c_client *client, struct i2c_board_info *info) 
{    
	strcpy(info->type, DiscreteALSPS_DEV_NAME);
	return 0;
}

/*----------------------------------------------------------------------------*/
#ifdef STK_GES
static int DiscreteALSPS_set_input_device(struct DiscreteALSPS_priv *obj)
{
	int err;
	
	obj->ges_input_dev = input_allocate_device();
	if (obj->ges_input_dev==NULL)
	{
		APS_ERR( "%s: could not allocate ps device\n", __func__);		
		err = -ENOMEM;
		return err;		
	}
	obj->ges_input_dev->name = "stk_ges";
	obj->ges_input_dev->evbit[0] = BIT_MASK(EV_KEY);
	set_bit(KEY_PAGEUP, obj->ges_input_dev->keybit);
	set_bit(KEY_PAGEDOWN, obj->ges_input_dev->keybit);
	set_bit(KEY_VOLUMEUP, obj->ges_input_dev->keybit);
	set_bit(KEY_VOLUMEDOWN, obj->ges_input_dev->keybit);
	/*
		set_bit(KEY_LEFT, obj->ges_input_dev->keybit);
		set_bit(KEY_RIGHT, obj->ges_input_dev->keybit);
		set_bit(KEY_UP, obj->ges_input_dev->keybit);
		set_bit(KEY_DOWN, obj->ges_input_dev->keybit);
	 */	
	err = input_register_device(obj->ges_input_dev);	
	if (err<0)
	{
		APS_ERR( "%s: can not register ps input device\n", __func__);	
		return err;
	}
	
	err = sysfs_create_group(&obj->ges_input_dev->dev.kobj, &stk_ges_attribute_group);
	if (err < 0) 
	{
		APS_ERR( "%s:could not create sysfs group for ps\n", __func__);
		return err;
	}	
	input_set_drvdata(obj->ges_input_dev, obj);	
	return 0;
}
#endif	
	
static int als_open_report_data(int open)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->als_open_report_data(open);
}

// if use  this typ of enable , Gsensor only enabled but not report inputEvent to HAL

static int als_enable_nodata(int en)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->als_enable_nodata(en);
}

static int als_set_delay(u64 ns)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->als_set_delay(ns);
}

static int als_get_data(int* value, int* status)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->als_get_data(value, status);
}

static int ps_open_report_data(int open)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_open_report_data(open);
}

// if use  this typ of enable , Gsensor only enabled but not report inputEvent to HAL

static int ps_enable_nodata(int en)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_enable_nodata(en);
}

static int ps_set_delay(u64 ns)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_set_delay(ns);
}

static int ps_get_data(int* value, int* status)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_get_data(value, status);
}

static int als_batch(int flag, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->als_batch(flag, samplingPeriodNs, maxBatchReportLatencyNs);
}

static int als_flush(void)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_ALS)->als_flush();
}



static int ps_batch(int flag, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
    APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
    return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_batch(flag,samplingPeriodNs,maxBatchReportLatencyNs);
}

static int ps_flush(void)
{
	APS_LOG("DiscreteALSPS.c. %s. \n", __FUNCTION__);
	return DiscreteALSPS_func(DISCRETEALSPS_PS)->ps_flush();
}

/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct DiscreteALSPS_priv *obj;
	//struct device_node *node = NULL;
	//struct hwmsen_object obj_ps, obj_als;
	int err = 0;
	struct als_control_path als_ctl={0};
	struct als_data_path als_data={0};
	struct ps_control_path ps_ctl={0};
	struct ps_data_path ps_data={0};

	APS_LOG("%s: driver version: %s\n", __FUNCTION__, DRIVER_VERSION);

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}

	err = get_alsps_dts_func(client->dev.of_node, hw);
	if (err < 0) {
		APS_ERR("DiscreteALSPS_i2c_probe. get dts info fail\n");
		goto exit_init_failed;
	}


    if(!DiscreteALSPS_is_allowed_probe()){
            APS_LOG("DiscreteALSPS_i2c_probe() ffff.  not both ready. return error.  !\n"); 
            goto exit_init_failed;
    }
    

	memset(obj, 0, sizeof(*obj));
	DiscreteALSPS_obj = obj;
//	obj->hw = get_cust_alsps_hw();
	obj->hw = hw;
	DiscreteALSPS_get_addr(obj->hw, &obj->addr);

	//INIT_DELAYED_WORK(&obj->eint_work, DiscreteALSPS_eint_work);
	printk("client->addr======%x\n",client->addr);
	client->addr = 0x48;
	printk("client->addr======%x\n",client->addr);
	obj->client = client;
	i2c_set_clientdata(client, obj);	
	atomic_set(&obj->als_debounce, 200);
	atomic_set(&obj->als_deb_on, 0);
	atomic_set(&obj->als_deb_end, 0);
	atomic_set(&obj->ps_debounce, 10);
	atomic_set(&obj->ps_deb_on, 0);
	atomic_set(&obj->ps_deb_end, 0);
	atomic_set(&obj->ps_mask, 0);
	atomic_set(&obj->trace, 0x00);
	atomic_set(&obj->als_suspend, 0);
	atomic_set(&obj->init_done, 0);
	//obj->irq_node = of_find_matching_node(obj->irq_node, alsps_of_match);
	//obj->irq_node = of_find_compatible_node(NULL, NULL, "mediatek,als_ps");

	atomic_set(&obj->state_val, 0);
	atomic_set(&obj->psctrl_val, 0x31);
	atomic_set(&obj->alsctrl_val, 0x39);
	obj->ledctrl_val = 0xFF;	
	obj->wait_val = 0xF;
	obj->int_val = 0;
	obj->first_boot = true;			 
	obj->als_correct_factor = 1000;
	atomic_set(&obj->ps_high_thd_val, obj->hw->ps_threshold_high);//  obj->hw->ps_high_thd_val
	atomic_set(&obj->ps_low_thd_val, obj->hw->ps_threshold_low);	
	atomic_set(&obj->recv_reg, 0);  
#ifdef STK_ALS_FIR	
	atomic_set(&obj->firlength, STK_FIR_LEN);	
#endif	

#ifdef STK_TUNE0
	obj->stk_max_min_diff = STK_MAX_MIN_DIFF;
	obj->stk_lt_n_ct = STK_LT_N_CT;
	obj->stk_ht_n_ct = STK_HT_N_CT;
#endif
	if(obj->hw->polling_mode_ps == 0)
	{
		APS_LOG("%s: enable PS interrupt\n", __FUNCTION__);
	}
	obj->int_val |= STK_INT_PS_MODE1;
	
	if(obj->hw->polling_mode_als == 0)
	{
	  obj->int_val |= STK_INT_ALS;		
	  APS_LOG("%s: enable ALS interrupt\n", __FUNCTION__);
	}	

	APS_LOG("%s: state_val=0x%x, psctrl_val=0x%x, alsctrl_val=0x%x, ledctrl_val=0x%x, wait_val=0x%x, int_val=0x%x\n", 
		__FUNCTION__, atomic_read(&obj->state_val), atomic_read(&obj->psctrl_val), atomic_read(&obj->alsctrl_val), 
		obj->ledctrl_val, obj->wait_val, obj->int_val);
	
	APS_LOG("DiscreteALSPS_i2c_probe() OK!\n");
    
	obj->enable = 0;
	obj->pending_intr = 0;
	obj->als_level_num = sizeof(obj->hw->als_level)/sizeof(obj->hw->als_level[0]);
	obj->als_value_num = sizeof(obj->hw->als_value)/sizeof(obj->hw->als_value[0]);   
	BUG_ON(sizeof(obj->als_level) != sizeof(obj->hw->als_level));
	memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
	BUG_ON(sizeof(obj->als_value) != sizeof(obj->hw->als_value));
	memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
	atomic_set(&obj->i2c_retry, 3);
	if(atomic_read(&obj->state_val) & STK_STATE_EN_ALS_MASK)
	{
		set_bit(STK_BIT_ALS, &obj->enable);
	}
	APS_LOG("DiscreteALSPS_i2c_probe() 3333. !\n");	
	if(atomic_read(&obj->state_val) & STK_STATE_EN_PS_MASK)
	{
		set_bit(STK_BIT_PS, &obj->enable);
	}
	
	DiscreteALSPS_i2c_client = client;

	//obj->stk_ps_tune0_wq = create_singlethread_workqueue("stk_ps_tune0_wq");
	//INIT_WORK(&obj->stk_ps_tune0_work, stk_ps_tune0_work_func);
	//hrtimer_init(&obj->ps_tune0_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#ifdef STK_TUNE0
	//obj->ps_tune0_delay = ns_to_ktime(60 * NSEC_PER_MSEC);
	//obj->ps_tune0_timer.function = stk_ps_tune0_timer_func;
#endif		
//	if((err = DiscreteALSPS_init_client(client)))
//	{
//		goto exit_init_failed;
//	}
	APS_LOG("DiscreteALSPS_i2c_probe() 4444. !\n");	
	if((err = misc_register(&DiscreteALSPS_device)))
	{
		APS_ERR("DiscreteALSPS_device register failed\n");
		goto exit_misc_device_register_failed;
	}
	APS_LOG("DiscreteALSPS_i2c_probe() 5555. !\n");	

	if((err = DiscreteALSPS_create_attr(&(DiscreteALSPS_init_info.platform_diver_addr->driver))))
	//if((err = DiscreteALSPS_create_attr(&DiscreteALSPS_alsps_driver.driver)))
	{
		APS_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
	
	APS_LOG("DiscreteALSPS_i2c_probe() 6666. !\n");	

	
	als_ctl.open_report_data= als_open_report_data;
	als_ctl.enable_nodata = als_enable_nodata;
	als_ctl.set_delay  = als_set_delay;
	als_ctl.is_report_input_direct = false;
	als_ctl.batch = als_batch;
	als_ctl.flush = als_flush;
#ifdef CUSTOM_KERNEL_SENSORHUB
	als_ctl.is_support_batch = obj->hw->is_batch_supported_als;
#else
    als_ctl.is_support_batch = false;
#endif
	APS_LOG("DiscreteALSPS_i2c_probe() 7777. !\n");	
	
	err = als_register_control_path(&als_ctl);
	if(err)
	{
		APS_ERR("register fail = %d\n", err);
		goto exit_sensor_obj_attach_fail;
	}
	APS_LOG("DiscreteALSPS_i2c_probe() 8888. !\n");	

	als_data.get_data = als_get_data;
	als_data.vender_div = 100;
	err = als_register_data_path(&als_data);	
	if(err)
	{
		APS_ERR("tregister fail = %d\n", err);
		goto exit_sensor_obj_attach_fail;
	}
	APS_LOG("DiscreteALSPS_i2c_probe() 9999. !\n");	

	
	ps_ctl.open_report_data= ps_open_report_data;
	ps_ctl.enable_nodata = ps_enable_nodata;
	ps_ctl.set_delay  = ps_set_delay;
	ps_ctl.batch = ps_batch;
	ps_ctl.flush = ps_flush;
//	ps_ctl.is_report_input_direct = true;
#ifdef CUSTOM_KERNEL_SENSORHUB
	ps_ctl.is_support_batch = obj->hw->is_batch_supported_ps;
#else
    ps_ctl.is_support_batch = false;
#endif
	APS_LOG("DiscreteALSPS_i2c_probe() aaaa. !\n");	

	if(1 == obj->hw->polling_mode_ps)
	{
		#ifdef CONFIG_PM_WAKELOCKS
		#else
			wake_lock_init(&ps_lock,WAKE_LOCK_SUSPEND,"ps wakelock");
		#endif

		ps_ctl.is_polling_mode = 1;
		ps_ctl.is_report_input_direct = false;
	}
	else
	{
		ps_ctl.is_polling_mode = 0;//PS interrupt mode
		ps_ctl.is_report_input_direct = true;
	}
	APS_LOG("DiscreteALSPS_i2c_probe() bbbb. !\n");	
	
	err = ps_register_control_path(&ps_ctl);
	if(err)
	{
		APS_ERR("register fail = %d\n", err);
		goto exit_sensor_obj_attach_fail;
	}
	APS_LOG("DiscreteALSPS_i2c_probe() cccc. !\n");	

	ps_data.get_data = ps_get_data;
	ps_data.vender_div = 100;
	err = ps_register_data_path(&ps_data);	
	if(err)
	{
		APS_ERR("tregister fail = %d\n", err);
		goto exit_sensor_obj_attach_fail;
	}
	APS_LOG("DiscreteALSPS_i2c_probe() dddd. !\n");	
#if 0
	err = batch_register_support_info(ID_LIGHT,als_ctl.is_support_batch, 100, 0);
	if(err)
	{
		APS_ERR("register light batch support err = %d\n", err);
	}
	
	err = batch_register_support_info(ID_PROXIMITY,ps_ctl.is_support_batch, 100, 0);
	if(err)
	{
		APS_ERR("register proximity batch support err = %d\n", err);
	}
#endif
#ifdef STK_GES
	err = DiscreteALSPS_set_input_device(obj);
	if(err < 0)
		goto exit_set_input_failed;
#endif
	APS_LOG("DiscreteALSPS_i2c_probe() eeee. !\n");	

    DiscreteALSPS_init_flag = 0;

	APS_LOG("%s: OK\n", __FUNCTION__);

	return 0;
	
#ifdef STK_GES	
	exit_set_input_failed:
	input_unregister_device(obj->ges_input_dev);	
	input_free_device(obj->ges_input_dev);		
#endif	
	exit_sensor_obj_attach_fail:
	exit_create_attr_failed:
	misc_deregister(&DiscreteALSPS_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(client);
//	exit_kfree:
	kfree(obj);
	exit:
	DiscreteALSPS_i2c_client = NULL;           
	#ifdef MT6516        
	MT6516_EINTIRQMask(CUST_EINT_ALS_NUM);  /*mask interrupt if fail*/
	#endif

    DiscreteALSPS_init_flag = -1;

	APS_ERR("%s: err = %d\n", __FUNCTION__, err);
	return err;
}
/*----------------------------------------------------------------------------*/
static int DiscreteALSPS_i2c_remove(struct i2c_client *client)
{
	int err;	
#ifdef STK_TUNE0
	//struct DiscreteALSPS_priv *obj = i2c_get_clientdata(client);		
	//destroy_workqueue(obj->stk_ps_tune0_wq);	
#endif		
#ifdef STK_GES		
	input_unregister_device(obj->ges_input_dev);	
	input_free_device(obj->ges_input_dev);		
#endif	
	//if((err = DiscreteALSPS_delete_attr(&DiscreteALSPS_i2c_driver.driver)))
	if((err = DiscreteALSPS_delete_attr(&(DiscreteALSPS_init_info.platform_diver_addr->driver))))
	{
		APS_ERR("DiscreteALSPS_delete_attr fail: %d\n", err);
	} 

	misc_deregister(&DiscreteALSPS_device);
	
	DiscreteALSPS_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));

	return 0;
}

static int  DiscreteALSPS_local_uninit(void)
{
//	struct alsps_hw *hw = get_cust_alsps_hw();
	APS_FUN();    
	DiscreteALSPS_power(hw, 0);    
	i2c_del_driver(&DiscreteALSPS_i2c_driver);
	DiscreteALSPS_i2c_client = NULL;
	return 0;
}

/*----------------------------------------------------------------------------*/

static int DiscreteALSPS_local_init(void) 
{
//	struct alsps_hw *hw = get_cust_alsps_hw();
	struct DiscreteALSPS_i2c_addr addr;

	APS_FUN();    
	DiscreteALSPS_power(hw, 1);    
	DiscreteALSPS_get_addr(hw, &addr);
	#if (LINUX_VERSION_CODE<KERNEL_VERSION(3,0,0))	
	DiscreteALSPS_force[0] = hw->i2c_num;
	DiscreteALSPS_force[1] = hw->i2c_addr[0];
	#endif
	if(i2c_add_driver(&DiscreteALSPS_i2c_driver))
	{
		APS_ERR("add driver error\n");
		return -1;
	} 

    if(-1 == DiscreteALSPS_init_flag)
    {
		APS_ERR("DiscreteALSPS_local_init fail with DiscreteALSPS_init_flag=%d\n", DiscreteALSPS_init_flag);
        return -1;
    }
	return 0;
}

/*----------------------------------------------------------------------------*/
static int __init DiscreteALSPS_init(void)
{
#if 0
	const char *name = "mediatek,cust_DiscreteALSPS";

	APS_FUN();
	printk("DiscreteALSPS_init In");

	hw = get_alsps_dts_func(name, hw);
	if (!hw)
		APS_ERR("get dts info fail\n");
#endif
   	alsps_driver_add(&DiscreteALSPS_init_info);// hwmsen_alsps_add(&DiscreteALSPS_init_info);

#if 0
	DiscreteALSPS_regulator = regulator_get(&alspsPltFmDev->dev,"vDiscreteALSPS");
	if(!DiscreteALSPS_regulator)
		printk("ZYQ DiscreteALSPS_regulator is NULL\n");
	ret = regulator_set_voltage(DiscreteALSPS_regulator, 2800000, 2800000); /*set 2.8v*/
	if (ret){
		printk("regulator_set_voltage() failed!\n");
	}	
#endif
	
	printk("DiscreteALSPS_init Out");

	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit DiscreteALSPS_exit(void)
{
	APS_FUN();
}
/*----------------------------------------------------------------------------*/
device_initcall_sync(DiscreteALSPS_init);

/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("MingHsien Hsieh");
MODULE_DESCRIPTION("SensorTek DiscreteALSPS proximity and light sensor driver");
MODULE_LICENSE("GPL");
