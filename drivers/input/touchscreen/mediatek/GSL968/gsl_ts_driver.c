/******************************************************************************

  Copyright (C), 2010-2012, Silead, Inc.

 ******************************************************************************
Filename      : gsl1680-d0.c
Version       : R2.0
Aurthor       : mark_huang
Creattime     : 2012.6.20
Description   : Driver for Silead I2C touchscreen.

 ******************************************************************************/

#include "tpd.h"
#include <linux/interrupt.h>
//#include <cust_eint.h>
#include <linux/of_irq.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/seq_file.h>
//#include <mach/mt_pm_ldo.h>
//#include <mach/mt_typedefs.h>
//#include <mach/mt_boot.h>
//#include <pmic_drv.h>
#include "gsl_ts_driver.h"
//#define GTP_GESTURE_WAKEUP
//#include <mach/upmu_common.h>
//#include <mach/upmu_common_sw.h>
//#include <mach/upmu_sw.h>
//#include <mach/upmu_hw.h>
//#include <mach/mt_pm_ldo.h> /* hwPowerOn */
//#include <mach/upmu_common.h>
//#include <mach/upmu_sw.h>
//#include <mach/upmu_hw.h>
//#include <mach/mt_gpio.h>

#if defined(__HCT_TP_GESTURE_SUPPORT__)
#if __HCT_TP_GESTURE_SUPPORT__
	#define GTP_GESTURE_WAKEUP    1	// gesture wakeup module
#else
	#define GTP_GESTURE_WAKEUP    0	// gesture wakeup module
#endif
#else
#define GTP_GESTURE_WAKEUP    0	// gesture wakeup module
#endif

#ifndef __HCT_TP_VDD_USE_VIO28_SUPPORT__
#define __HCT_TP_VDD_USE_VIO28_SUPPORT__ 0
#endif
#ifdef __HCT_PS_ENABLE__
#define TPD_PROXIMITY
#endif

#include "mtk_boot_common.h"
//#include "upmu_common.h"
static struct tpd_driver_t gsl_driver;
int hct_version[4]={0};
#ifdef __HCT_TP_GSL968_CUSTOME__
char hct_tp_script[7]={0};
#endif
extern struct tpd_device *tpd;
extern void tpgesture_hander(void);
#ifdef GSL_COMPATIBLE_BY_ADC
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
#endif

static struct gsl_ts_data *ddata = NULL;

//static int boot_mode = NORMAL_BOOT;
static int touch_irq;
#define GSL_DEV_NAME "gsl1680"

#define I2C_TRANS_SPEED 400	//100 khz or 400 khz
#define TPD_REG_BASE 0x00

#define TPD_KEY_COUNT           3
#define TPD_KEYS                {KEY_MENU,KEY_HOMEPAGE,KEY_BACK}
#define TPD_KEYS_DIM            {{2000,66,50,100},{2000,300,50,100},{2000,418,50,100}}
//#define ANDROID_4

//#define GSL_DAC_IDT_TP

//static struct tpd_bootloader_data_t g_bootloader_data;
static const struct i2c_device_id gsl_device_id[] = {{"gsl1680",0},{}};
static unsigned short force[] = {0,0x80,I2C_CLIENT_END,I2C_CLIENT_END};
static const unsigned short * const forces[] = { force, NULL };
#ifdef ANDROID_4
static struct i2c_board_info __initdata i2c_tpd = { I2C_BOARD_INFO("mtk-tpd", (0x80>>1))};
#else
//static struct i2c_client_address_data addr_data = { .forces = forces, };
#endif
#define Point_Report
#ifdef Point_Report
static unsigned int gsl_point = 0;
#endif
static volatile int gsl_halt_flag = 0;
static struct mutex gsl_i2c_lock;

#if GTP_GESTURE_WAKEUP
typedef enum{
	GE_DISABLE = 0,
	GE_ENABLE = 1,
	GE_WAKEUP = 2,
	GE_NOWORK =3,
}GE_T;
static GE_T gsl_gesture_status = GE_DISABLE;
static unsigned int gsl_gesture_flag = 1;
extern int gsl_obtain_gesture(void);
extern void gsl_FunIICRead(unsigned int (*fun) (unsigned int *,unsigned int,unsigned int));

#ifdef __HCT_LDO_TP_GESTURE__
extern struct pinctrl *pinctrl1;
extern struct pinctrl_state *ldotpgesture_en_high, *ldotpgesture_en_low;
#endif
extern void gsl_GestureExternInt(unsigned int *model,int len);
unsigned int gsl_model_extern[]={
	0x10,0x56,
	0x08000f00,0x0f0c2f1f,0x12125040,0x13127060,0x16149181,0x1a18b1a1,0x1c1bd2c2,0x201df2e2,
	0x3324f7fe,0x4f41e7ef,0x6d5ed8df,0x8a7bc8d0,0xa698b7c0,0xc3b4a6af,0xe0d2959d,0xffee848d,
	0x10,0x57,
	0x00062610,0x03015c41,0x06049277,0x0f09c8ad,0x2918f7e0,0x5142e4fb,0x685eb2cb,0x77707c97,
	0x857d9177,0x978dc5ab,0xb4a1f3de,0xdbcbd5ec,0xebe4a2bd,0xf4f06c87,0xfaf73651,0xfffd001b,
	
	0x10,0x49,
	0x0e00f4ff,0x2f1ee4ec,0x4f3ed4dc,0x6f5fc4cc,0x8f7fb3bc,0xae9ea3ab,0xcebe949b,0xf0df858d,
	0xf0ff707a,0xcfdf6268,0xadbe525a,0x8d9e434a,0x6d7d353c,0x4c5c262e,0x2c3c151e,0x0c3c000b,
	0x10,0x49,
	0x775df6f9,0xab91e6ef,0xdac3cedb,0xf9eda2b9,0xfdff6d88,0xf1f93a53,0xcce21424,0x9ab50209,
	0x65800101,0x354d1409,0x0f1f3a25,0x01056e53,0x0100a288,0x1407d3bc,0x4128f1e4,0x765bfffb,	

	0x10,0x1041,
	0xfdff859f,0xe0f2566c,0xbdcf2d41,0x90a90f1d,0x5a750106,0x253f0400,0x020d2b11,0x10015c46,
	0x3823806e,0x664e9f91,0x9a80b2ab,0xd0b5bab8,0xaac5bbbc,0x7590bebb,0x445bd3c4,0x244fffe5,
	0x10,0x1042,
	0xe5ff795e,0xb0cb4f54,0x7c96444a,0x4e63293a,0x223a0917,0x39271601,0x5c4a402c,0x7a6c6e55,
	0x837fa388,0x8084d7bd,0x5871fdef,0x293febfd,0x1019bcd5,0x040987a1,0x0101526d,0x11271e38,
	0x10,0x1044,
	0x86867995,0x8386415d,0x687a1026,0x324e0003,0x03151d07,0x04005539,0x240e836f,0x553ca293,
	0x8c70b5ac,0xc4a8bebb,0xfce0bfbf,0xcae6bfbf,0x91adbfbf,0x5975bdbe,0x253dccbf,0x0534ffe4,
	0x10,0x1041,
	0x0007775b,0x1004ae93,0x3520dac6,0x6b50f4e7,0xa487fdfe,0xdec1f3f9,0xfdf8c7e3,0xd7ee9dae,
	0xa4be818f,0x6f8a6672,0x3552595d,0x33185556,0x6d505555,0xa78a5053,0xddc43a48,0xdd05001d,
	0x10,0x1042,
	0x1a00d1d1,0x4f34d6d3,0x8369dfda,0xb89eebe4,0xecd2fef4,0xd5edecf9,0xb0bec6de,0x99a597af,
	0x8a8f637d,0x87882e49,0xa28c0214,0xd4bc0c01,0xece53b22,0xfcf56f55,0xfffea489,0xf91ed9be,
	0x10,0x1044,
	0x93958166,0x9a94b79c,0xb5a5e7d1,0xe6cbfff8,0xfefbd2ed,0xeef79eb8,0xcce07386,0x9fb75562,
	0x6b854049,0x35503539,0x02193331,0x381d3535,0x6f543736,0xa58a3938,0xdbc13239,0xf409001b,
	0x10,0x1045,
	0x6d6d1e00,0x6d6d5b3d,0x6e6d987a,0x7e73d4b7,0xab8ffef0,0xe4cae9fa,0xfcf3afcd,0xfdff7290,
	0xe2f03a55,0xb5ce0f21,0x79970104,0x3d5a0900,0x1324361c,0x02077153,0x0200ae90,0x1c2be9cc,
	0x10,0x1045,
	0x9999d9f8,0x97989ab9,0x90965b7a,0x7f8a1c3b,0x48680006,0x0d291704,0x00045535,0x07019375,
	0x2112ceb2,0x5638f5e6,0x9575fffc,0xd1b4f0fc,0xf5e7bbd9,0xfffc7c9b,0xf3fc3e5c,0xd407021f,

	0x10,0x47,        
	0xc9e00105,0x9ab11008,0x6f832c1c,0x525d533c,0x765f6c68,0xa68e5b64,0xd2bc4552,0xf8e62435,                  
	0xf2fa462e,0xdfea745d,0xc4d29e89,0xa4b5c5b3,0x8093e9d9,0x526bfef7,0x2139f2fc,0x002fcae0,	
	0x10,0x36,        
	0x18009fa5,0x4930969a,0x79619193,0xaa929190,0xdbc39893,0xfdf0baa3,0xeafae5d1,0xbfd6fbf4,        
	0x8ea6ffff,0x6477eaf9,0x5356bcd5,0x5f588ea5,0x756a6176,0x94843a4c,0xb9a61829,0xe5ed000a,  

	
	0x0a,0x36,
	0xebff0002,0xc1d60100,0x98ac0c06,0x75862116,0x53633b2e,0x35435949,0x1f297c6a,0x0c15a38f,
	0x0004ccb7,0x0e03f2e0,0x3621fffd,0x5b4aecf9,0x776acbdc,0x797ba1b6,0x566b8a90,0x2c61928b,
	0x0a,0x36,
	0x8f901700,0x8186452e,0x737b725c,0x536a9287,0x273da59c,0x0414c5b3,0x1905e9db,0x452efbf4,
	0x735cfffd,0xa28afaff,0xceb8ecf4,0xf7e4d5e2,0xf9ffa7be,0xcce3979b,0x9eb59495,0x6fa79593,
	0x0a,0x36,
	0xebff0900,0xc1d61e14,0x99ad372b,0x73875044,0x4c5f695c,0x2c398977,0x121eaf9b,0x0005d9c3,
	0x1d08f8ed,0x4b34fffd,0x7762f5fb,0xa38ee7ee,0xceb9d5de,0xdfe2b6ca,0xb3caa6aa,0x85bca5a5,	

	0x10,0x4f,
	0x5d76fefd,0x2d45e5f4,0x0b1abdd4,0x01058aa3,0x02005670,0x1b0b293d,0x4a320f19,0x7d630005,
	0xae960f05,0xd7c5311e,0xf4e85e46,0xfefc9278,0xf6fec4ac,0xcee4e8d7,0x9cb6fbf6,0x6882fefe,
	0x10,0x4f,
	0x795ffffc,0xaf94fdff,0xddc7dff0,0xf8eeafc9,0xfffd7a94,0xf8fe455f,0xd3e81c2f,0xa1bb0610,
	0x6b860600,0x3b521e0f,0x16264731,0x020a7a60,0x0000b095,0x1706e0cb,0x492efaf0,0x7f84fefe,

	0x10,0x00,
	0xf4ff0d00,0xe6ee2f1f,0xd8df5240,0xc8d17262,0xbac29483,0xacb3b6a4,0x9ea4d8c7,0x8f99f9ea,
	0x7883e7f6,0x666fc7d6,0x595fa4b5,0x49518495,0x383f6474,0x27304254,0x161e2132,0x012d0411,

	0x10,'>',
	0x17070600,0x3928160f,0x5949271f,0x796a372f,0x9a8a483f,0xb9aa5951,0xd8c86b62,
	0xfae97e74,
	0xe3f48d85,0xc1d29c95,0x9fb0aca4,0x7e8ebab3,0x5d6dcbc2,0x3c4cdbd3,0x1f2dece4,
	0x0010fff5,

	0x10,0x5e,
	0x0700efff,0x150ecdde,0x231badbd,0x362c8c9d,0x493f6e7e,0x58514d5e,0x69602d3e,
	0x79710b1d,
	0x92861002,0xa39b3121,0xb2ab5240,0xc0b87463,0xd1c99483,0xdfd9b6a5,0xebe5d8c7,
	0xfff2f8e9,
};
#endif


#ifdef TPD_PROXIMITY
#include <hwmsensor.h>
#include <sensors_io.h>
#include <battery_common.h>
#include <alsps.h>
#include <asm/io.h>
#include <linux/wakelock.h>
#include <linux/ioctl.h>
extern int hwmsen_get_interrupt_data(int sensor, struct hwm_sensor_data *data);
static u8 tpd_proximity_flag = 0; //flag whether start alps
static u8 tpd_proximity_detect = 1;//0-->close ; 1--> far away
//static struct wake_lock ps_lock;
//static u8 gsl_psensor_data[8]={0};
extern int ps_flush_report(void);
static int ps_flush(void);
struct gsl_priv {
	struct work_struct eint_work;
	ulong enable;		/*enable mask */
	ulong pending_intr;	/*pending interrupt */
#if defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend early_drv;
#endif
	bool ps_flush;
};
	static struct gsl_priv *gsl_obj;
enum {
	CMC_BIT_ALS = 1,
	CMC_BIT_PS = 2,
} CMC_BIT;
#endif
#ifdef GSL_ALG_ID 
extern unsigned int gsl_mask_tiaoping(void);
extern unsigned int gsl_version_id(void);
extern void gsl_alg_id_main(struct gsl_touch_info *cinfo);
extern void gsl_DataInit(int *ret);


#endif

#ifdef GSL_TIMER
#define GSL_TIMER_CHECK_CIRCLE        200
static struct delayed_work gsl_timer_check_work;
static struct workqueue_struct *gsl_timer_workqueue = NULL;
static char int_1st[4];
static char int_2nd[4];
#endif

#ifdef TPD_PROC_DEBUG
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
static struct proc_dir_entry *gsl_config_proc = NULL;
#define GSL_CONFIG_PROC_FILE "gsl_config"
#define CONFIG_LEN 31
static char gsl_read[CONFIG_LEN];
static u8 gsl_data_proc[8] = {0};
static u8 gsl_proc_flag = 0;
#endif
static u8 int_type = 0;
#ifdef GSL_COMPATIBLE_CHIP
//static int gsl_compatible_flag = 0;
#endif

#ifdef GSL_THREAD_EINT
static DECLARE_WAIT_QUEUE_HEAD(waiter);
static struct task_struct *thread = NULL;
static int tpd_flag = 0;
#endif

#ifdef GSL_COMPATIBLE_BY_ADC
static void gsl_idt_adc_tp(void);
#endif

#ifdef GSL_DEBUG
#define print_info(fmt, args...)   \
		do{                              \
		    printk("[tp-gsl][%s]"fmt,__func__, ##args);     \
		}while(0)
#else
#define print_info(fmt, args...)
#endif
#if GTP_GESTURE_WAKEUP
extern char tpgesture_value[10];
extern char tpgesture_status;
static char tpgesture_status_value[5];
#endif
#ifdef TPD_HAVE_BUTTON
extern void tpd_button(unsigned int x, unsigned int y, unsigned int down);
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif

#ifdef GSL_IDENTY_TP
static int gsl_tp_type = 0;
#endif

#ifdef  GSL_GPIO_IDT_TP
static int gsl_Rx_flag = 0;
static int gsl_Tx_flag = 0;
static int Rx_flag_0 = 0;
static int Tx_flag_0 = 0;
static int Rx_flag_1;
static int Tx_flag_1;
#endif
static const struct of_device_id tpd_of_match[] = {
	{.compatible = "mediatek,cap_touch_gsl968"},
	{},
};

/*****************************************************************************
Prototype    	: gsl_read_interface
Description  	: gsl chip tranfer function
Input        	: struct i2c_client *client
u8 reg
u8 *buf
u32 num
Output		: None
Return Value 	: static

 *****************************************************************************/
static int gsl_read_interface(struct i2c_client *client,
        u8 reg, u8 *buf, u32 num)
{
	int err = 0;
	int i;
	u8 temp = reg;
	mutex_lock(&gsl_i2c_lock);
	if(temp < 0x80)
	{
		temp = (temp+8)&0x5c;
			i2c_master_send(client,&temp,1);
			err = i2c_master_recv(client,&buf[0],4);
		temp = reg;
		i2c_master_send(client,&temp,1);
		err = i2c_master_recv(client,&buf[0],4);
	}
	for(i=0;i<num;)
	{
		temp = reg + i;
		i2c_master_send(client,&temp,1);
		if((i+8)<num)
			err = i2c_master_recv(client,(buf+i),8);
		else
			err = i2c_master_recv(client,(buf+i),(num-i));
		i+=8;
	}
	mutex_unlock(&gsl_i2c_lock);

	return err;
}

/*****************************************************************************
Prototype    : gsl_write_interface
Description  : gsl chip tranfer function
Input        : struct i2c_client *client
const u8 reg
u8 *buf
u32 num
Output       : None
Return Value : static

 *****************************************************************************/
static int gsl_write_interface(struct i2c_client *client,
        const u8 reg, u8 *buf, u32 num)
{
        struct i2c_msg xfer_msg[1] = {{0}};
	int err;
	u8 tmp_buf[num + 1];

	tmp_buf[0] = reg;
	memcpy(tmp_buf + 1, buf, num);

	xfer_msg[0].addr = client->addr;
	xfer_msg[0].len = num + 1;
	xfer_msg[0].flags = 0;//client->flags & I2C_M_TEN;
	xfer_msg[0].buf = tmp_buf;
	//xfer_msg[0].timing = 400;//I2C_TRANS_SPEED;
	mutex_lock(&gsl_i2c_lock);

	err = i2c_transfer(client->adapter, xfer_msg, 1);
	mutex_unlock(&gsl_i2c_lock);

	
	return err;
}

static ssize_t tpd_info_readVersion(void)
{
	char data_buf[4] = {0};
	int count = 0 ;
	data_buf[3] = 0;
	data_buf[2] = 0;
	data_buf[1] = 0;
	data_buf[0] = 0x3;
	gsl_write_interface(ddata->client, 0xf0, data_buf, 4);
	gsl_read_interface(ddata->client, 0x0, data_buf, 4); // read the value of (3,4)
	printk("tpd_info_readVersion (3,4) = 0x%02x%02x%02x%02x\n",data_buf[3],data_buf[2],data_buf[1],data_buf[0]);
        hct_version[3]=data_buf[3];
        hct_version[2]=data_buf[2];
	hct_version[1]=data_buf[1];
	hct_version[0]=data_buf[0];
	return count;
}
/*****************************************************************************
Prototype    : gsl_start_core
Description  : touchscreen chip work
Input        : struct i2c_client *client
Output       : None
Return Value : static

 *****************************************************************************/
static void gsl_start_core(struct i2c_client *client)
{
	u8 buf[4] = {0};
#if 0
	buf[0]=0;
	buf[1]=0x10;
	buf[2]=0xfe;
	buf[3]=0x1;
	gsl_write_interface(client,0xf0,buf,4);
	buf[0]=0xf;
	buf[1]=0;
	buf[2]=0;
	buf[3]=0;
	gsl_write_interface(client,0x4,buf,4);
	msleep(20);
#endif
	buf[0]=0x1;
	buf[1]=0x0;
	buf[2]=0x0;
	buf[3]=0x0;
	gsl_write_interface(client,0x88,buf,4);
	msleep(5);
	buf[0]=0;
	gsl_write_interface(client,0xe0,buf,4);
#ifdef GSL_ALG_ID
	gsl_DataInit(gsl_cfg_table[gsl_cfg_index].data_id);

	//gsl_DataInit(gsl_config_data_id);
#endif
}

/*****************************************************************************
Prototype    : gsl_reset_core
Description  : touchscreen chip soft reset
Input        : struct i2c_client *client
Output       : None
Return Value : static

 *****************************************************************************/
static void gsl_reset_core(struct i2c_client *client)
{
	u8 buf[4] = {0x00};

	buf[0] = 0x88;
	gsl_write_interface(client,0xe0,buf,4);
	msleep(5);

	buf[0] = 0x04;
	gsl_write_interface(client,0xe4,buf,4);
	msleep(5);

	buf[0] = 0;
	gsl_write_interface(client,0xbc,buf,4);
	msleep(5);
}

static void gsl_clear_reg(struct i2c_client *client)
{
	u8 buf[4]={0};
	//clear reg
	buf[0]=0x88;
	gsl_write_interface(client,0xe0,buf,4);
	msleep(20);
	buf[0]=0x3;
	gsl_write_interface(client,0x80,buf,4);
	msleep(5);
	buf[0]=0x4;
	gsl_write_interface(client,0xe4,buf,4);
	msleep(5);
	buf[0]=0x0;
	gsl_write_interface(client,0xe0,buf,4);
	msleep(20);
	//clear reg
}

/*****************************************************************************
Prototype    : gsl_load_fw
Description  : gsl chip load the firmware throught I2C
Input        : struct i2c_client *client
Output       : None
Return Value : static

 *****************************************************************************/
#if 0
#define DMA_TRANS_LEN 0x20
static void gsl_load_fw(struct i2c_client *client,const struct fw_data *GSL_DOWNLOAD_DATA,int data_len)
{
	u8 buf[DMA_TRANS_LEN*4] = {0};
	u8 send_flag = 1;
	u8 addr=0;
	u32 source_line = 0;
	u32 source_len = data_len;//ARRAY_SIZE(GSL_DOWNLOAD_DATA);

	print_info("=============gsl_load_fw start==============\n");

	for (source_line = 0; source_line < source_len; source_line++)
	{
		/* init page trans, set the page val */
		if (GSL_PAGE_REG == GSL_DOWNLOAD_DATA[source_line].offset)
		{
			memcpy(buf,&GSL_DOWNLOAD_DATA[source_line].val,4);
			gsl_write_interface(client, GSL_PAGE_REG, buf, 4);
			send_flag = 1;
		}
		else
		{
			if (1 == send_flag % (DMA_TRANS_LEN < 0x20 ? DMA_TRANS_LEN : 0x20))
	    			addr = (u8)GSL_DOWNLOAD_DATA[source_line].offset;

			memcpy((buf+send_flag*4 -4),&GSL_DOWNLOAD_DATA[source_line].val,4);

			if (0 == send_flag % (DMA_TRANS_LEN < 0x20 ? DMA_TRANS_LEN : 0x20))
			{
	    		gsl_write_interface(client, addr, buf, DMA_TRANS_LEN * 4);
				send_flag = 0;
			}

			send_flag++;
		}
	}

	print_info("=============gsl_load_fw end==============\n");

}
#else
static void gsl_load_fw(struct i2c_client *client,const struct fw_data *GSL_DOWNLOAD_DATA,int data_len)
{
	u8 buf[4] = {0};
	//u8 send_flag = 1;
	u8 addr=0;
	u32 source_line = 0;
	u32 source_len = data_len;//ARRAY_SIZE(GSL_DOWNLOAD_DATA);

	print_info("=============gsl_load_fw start==============\n");
	print_info("gsl_load_fw gsl_cfg_index = %d\n",gsl_cfg_index);
	for (source_line = 0; source_line < source_len; source_line++)
	{
		/* init page trans, set the page val */
    	addr = (u8)GSL_DOWNLOAD_DATA[source_line].offset;
		memcpy(buf,&GSL_DOWNLOAD_DATA[source_line].val,4);
    	gsl_write_interface(client, addr, buf, 4);
	}
}
#endif

static void gsl_sw_init(struct i2c_client *client)
{
	//int temp;
	static volatile int gsl_sw_flag=0;
	if(1==gsl_sw_flag)
		return;
	gsl_sw_flag=1;
	print_info("gsl_sw_init gsl_sw_flag = %d\n",gsl_sw_flag);
#if 0
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	msleep(20);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(20);
#endif

	gsl_clear_reg(client);
	gsl_reset_core(client);

	gsl_load_fw(client,gsl_cfg_table[gsl_cfg_index].fw,
		gsl_cfg_table[gsl_cfg_index].fw_size);
	//temp = ARRAY_SIZE(GSLX68X_FW);
	//gsl_load_fw(client,GSLX68X_FW,temp);

	gsl_start_core(client);
	gsl_sw_flag=0;
}

#ifdef  GSL_GPIO_IDT_TP
static void gsl_identify_tp(struct i2c_client *client)
{
	int i;
	u8 buf[4] = {0};
	u8 Rx_buf[4]={0};
	u8 Tx_buf[4]={0};
	gsl_reset_core(client);
	gsl_clear_reg(client);
	/***********pull up*************/
	buf[3] = 0x1;
	buf[2] = 0xfe;
	buf[1] = 0x0;
	buf[0] = 0x01;
	gsl_write_interface(client,0xf0,buf,4);
	Rx_buf[0] = 0x0; //pull up,set buf[1]=1
	gsl_write_interface(client,0x4,Rx_buf,4);    //set Rx pull up
	msleep(100);

	buf[3] = 0x1;
	buf[2] = 0xfe;
	buf[1] = 0x10;
	buf[0] = 0x0;
	gsl_write_interface(client,0xf0,buf,4);

	Tx_buf[3] =0x0;
	Tx_buf[2] =0x1;
	Tx_buf[1] =0x11;
	Tx_buf[0] =0x12;  //0x00011112  //pull up,wrire 0x0011112
	gsl_write_interface(client,0x58,Tx_buf,4);   //set Tx pull up
	msleep(100);
	for(i=0;i<3;i++)
	{
		buf[3] = 0x1;
		buf[2] = 0xfe;
		buf[1] = 0x4;
		buf[0] = 0x0;
		gsl_write_interface(client,0xf0,buf,4);
		gsl_read_interface(client,0x4,buf,4);
	}
	print_info("buf_0 = 0x%02x\n",buf[0]);
	Tx_flag_0 = buf[0]&0x2;
	Rx_flag_0 = buf[0]&0x1;
	print_info("Rxbuf_0 =0x%02x,Txbuf_0 =0x%02x\n",Rx_flag_0,Tx_flag_0);

	/**************pull down*******************/
	buf[3] = 0x1;
	buf[2] = 0xfe;
	buf[1] = 0x0;
	buf[0] = 0x01;
	gsl_write_interface(client,0xf0,buf,4);
	Rx_buf[0] = 0x2; //pull down,set buf[1]=1
	gsl_write_interface(client,0x4,Rx_buf,4);
	msleep(100);

	buf[3] = 0x1;
	buf[2] = 0xfe;
	buf[1] = 0x10;
	buf[0] = 0x0;
	gsl_write_interface(client,0xf0,buf,4);
	Tx_buf[3] =0x0;
	Tx_buf[2] =0x1;
	Tx_buf[1] =0x11;
	Tx_buf[0] =0x10;  //0x00011110  //pull down,write 0x0011110
	gsl_write_interface(client,0x58,Tx_buf,4);   // set Rx pull down 
	msleep(100);
	for(i=0;i<3;i++)
	{
		buf[3] = 0x1;
		buf[2] = 0xfe;
		buf[1] = 0x4;
		buf[0] = 0x0;
		gsl_write_interface(client,0xf0,buf,4);
		gsl_read_interface(client,0x4,buf,4);
	}	
	Tx_flag_1 = buf[1]&0x2;
	Rx_flag_1 = buf[0]&0x1;
	print_info("Rxbuf_1 =0x%02x,Txbuf_1 =0x%02x\n",buf[0],buf[1]);

	if(Rx_flag_0 == Rx_flag_1)
		if(Rx_flag_0 == 0x0 || Rx_flag_1 == 0x0)
			gsl_Rx_flag = 1;  //Rx pull down
		else
			gsl_Rx_flag = 2;  //Rx pull up
	else
		gsl_Rx_flag = 3;  //Rx no touch
	print_info("gsl_Rx_flag = %d\n",gsl_Rx_flag);
	if(Tx_flag_0 == Tx_flag_1)
	{
		if(Tx_flag_0 == 0x0 || Tx_flag_1 == 0x0)
		{
			gsl_Tx_flag = 1;  //Rx pull down
      gsl_cfg_index=0;
    }
		else
		{
			gsl_Tx_flag = 2;  //Rx pull up
		}
	}
	else
	{
		gsl_Tx_flag = 3;  //Tx no touch
    gsl_cfg_index=1;
  }
      //extern void app_get_ctp_name(char *name);
    //app_get_ctp_name("gsl2681_wsvga_gp313_t702");

	print_info("gsl_Tx_flag = %d\n",gsl_Tx_flag);
  
}
#endif

#ifdef GSL_IDENTY_TP

#define GSL_C		100
#ifdef __HCT_TP_SELECT__
#define GSL_CHIP_1	0xffc00c00  //HSD-915 TP
#define GSL_CHIP_2	0xff800700 //1691 TP
#else
#define GSL_CHIP_1	0xff80f801  //keleli TP
#define GSL_CHIP_2	0xff807803  //old TP
#endif
#define GSL_CHIP_3	0xffc0f841	//saihua
#define GSL_CHIP_4	0xffffffff	//keleli	//后加的
static unsigned int gsl_count_one(unsigned int flag)
{
	unsigned int tmp=0;
	int i =0;
	for(i=0;i<32;i++){
		if(flag&(0x1<<i))
			tmp++;
	}
	return tmp;
}

static int gsl_identify_tp(struct i2c_client *client)
{
	u8 buf[4];
#ifdef __HCT_TP_GSL968_CUSTOME__
        int buff[4]={0};
        int i =0;
#endif
	int err=1;
	int flag=0;
	unsigned int tmp,tmp0;
	unsigned int tmp1,tmp2,tmp3,tmp4;
	u32 num;
identify_tp_repeat:
	gsl_clear_reg(client);
	gsl_reset_core(client);
	num = ARRAY_SIZE(GSL_IDT_FW);
	gsl_load_fw(client,GSL_IDT_FW,num);
	gsl_start_core(client);
	msleep(20);
	gsl_read_interface(client,0xb4,buf,4);
	print_info("the test 0xb4 = {0x%02x%02x%02x%02x}\n",buf[3],buf[2],buf[1],buf[0]);
	if(((buf[3]<<8)|buf[2])>1){
		print_info("[TP-GSL][%s] is start ok\n",__func__);
		gsl_read_interface(client,0xb8,buf,4);
		tmp = (buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|buf[0];
		print_info("the test 0xb8 = {0x%02x%02x%02x%02x}\n",buf[3],buf[2],buf[1],buf[0]);
#ifdef __HCT_TP_GSL968_CUSTOME__
		for(i=0;i<4;i++){
			buff[i]=buf[i];
		}
                buff[0]+=((buff[1]<<8) | (buff[2]<<16) | (buff[3]<<24));
                if(buff[0]==0xff800800){
		sprintf(hct_tp_script,"HSD-915");
		}
                else if(buff[0]==0xff807801){
		sprintf(hct_tp_script,"DC-1691");			
		}
                else{
		}
#endif
		tmp1 = gsl_count_one(GSL_CHIP_1^tmp);
		tmp0 = gsl_count_one((tmp&GSL_CHIP_1)^GSL_CHIP_1);
		tmp1 += tmp0*GSL_C;
		print_info("[TP-GSL] tmp1 = %d\n",tmp1);

		tmp2 = gsl_count_one(GSL_CHIP_2^tmp);
		tmp0 = gsl_count_one((tmp&GSL_CHIP_2)^GSL_CHIP_2);
		tmp2 += tmp0*GSL_C;
		print_info("[TP-GSL] tmp2 = %d\n",tmp2);

		tmp3 = gsl_count_one(GSL_CHIP_3^tmp);
		tmp0 = gsl_count_one((tmp&GSL_CHIP_3)^GSL_CHIP_3);
		tmp3 += tmp0*GSL_C;
		print_info("[TP-GSL] tmp3 = %d\n",tmp3);

		tmp4 = gsl_count_one(GSL_CHIP_4^tmp);
		tmp0 = gsl_count_one((tmp&GSL_CHIP_4)^GSL_CHIP_4);
		tmp4 += tmp0*GSL_C;
		print_info("[TP-GSL] tmp4 = %d\n",tmp4);

		if(0xffffffff==GSL_CHIP_1)
		{
			tmp1=0xffff;
		}
		if(0xffffffff==GSL_CHIP_2)
		{
			tmp2=0xffff;
		}
		if(0xffffffff==GSL_CHIP_3)
		{
			tmp3=0xffff;
		}
		if(0xffffffff==GSL_CHIP_4)
		{
			tmp4=0xffff;
		}
		print_info("[TP-GSL] tmp1 = %d\n",tmp1);
		print_info("[TP-GSL] tmp2 = %d\n",tmp2);
		print_info("[TP-GSL] tmp3 = %d\n",tmp3);
		print_info("[TP-GSL] tmp4 = %d\n",tmp4);
		tmp = tmp1;
		if(tmp1>tmp2){
			tmp = tmp2;
		}
		if(tmp > tmp3){
			tmp = tmp3;
		}
		if(tmp>tmp4){
			tmp = tmp4;
		}

		if(tmp == tmp1){
			gsl_cfg_index = 0;
			//gsl_config_data_id = gsl_config_data_id_sfd;
			//gsl_tp_type = 1;
		}else if(tmp == tmp2){
			gsl_cfg_index = 1;
			//gsl_config_data_id = gsl_config_data_id_xz;
			//gsl_tp_type = 2;
		}else if(tmp == tmp3){
			//gsl_config_data_id = gsl_config_data_id_3;
			//gsl_tp_type = 3;
			gsl_cfg_index = 2;
		}else if(tmp == tmp4){
			//gsl_config_data_id = gsl_config_data_id_4;
			gsl_tp_type = 4;
		}
		err = 1;
	}else {
		flag++;
		if(flag < 3)
			goto identify_tp_repeat;
		err = 0;
	}
    //extern void app_get_ctp_name(char *name);
    //app_get_ctp_name("gsl2681_wsvga_gp313_t702");
	print_info("gsl_cfg_index = %d\n",gsl_cfg_index);
	return err;
}
#endif
#ifdef GSL_DAC_IDT_TP
static void gsl_identify_dac(struct i2c_client *client)
{	
	u8 writebuf[4] = {0};
	u8 readbuf[4] = {0};
	int ret;
		if(gsl_cfg_index == 0){
			return;
		}
	printk("======gsl_identify_dac======\n");
	gsl_clear_reg(client);
	gsl_reset_core(client);
	gsl_load_fw(client,gsl_cfg_table[gsl_cfg_index].fw,
		gsl_cfg_table[gsl_cfg_index].fw_size);
	gsl_start_core(client);
	msleep(20);
	writebuf[0] = 0x00;
	writebuf[1] = 0x10;
	writebuf[2] = 0xfe;
	writebuf[3] = 0x01;
	gsl_write_interface(client, 0xf0, writebuf, 4); 
      msleep(20);
	ret =gsl_read_interface(client,0x18, readbuf, 4);//0x1c
	if (ret < 0) 
		gsl_read_interface(client,0x18, readbuf, 4);//0x1c
	printk("======read 1fe1000,10: %x %x %x %x======\n",readbuf[3], readbuf[2], readbuf[1], readbuf[0]);
		if(readbuf[3]<10 )//readbuf[1]
		{
			gsl_cfg_index = 1;
		}
        	else
       	{
              	gsl_cfg_index = 2;  
        	}
}
#endif

/*****************************************************************************
Prototype    : check_mem_data
Description  : check mem data second to deal with power off
Input        : struct i2c_client *client
Output       : None
Return Value : static

 *****************************************************************************/
static void check_mem_data(struct i2c_client *client)
{
	char read_buf[4] = {0};
	gsl_read_interface(client, 0xb0, read_buf, 4);

	print_info("[gsl1680][%s] addr = 0xb0; read_buf = %02x%02x%02x%02x\n",
		__func__, read_buf[3], read_buf[2], read_buf[1], read_buf[0]);
	if (read_buf[3] != 0x5a || read_buf[2] != 0x5a || read_buf[1] != 0x5a || read_buf[0] != 0x5a)
	{
		gsl_sw_init(client);
		//gsl_init_chip(client);
	}
}


#ifdef TPD_PROC_DEBUG
//#define GSL_APPLICATION
#ifdef GSL_APPLICATION
static int gsl_read_MorePage(struct i2c_client *client,u32 addr,u8 *buf,u32 num)
{
	int i;
	u8 tmp_buf[4] = {0};
	u8 tmp_addr;
	for(i=0;i<num/8;i++){
		tmp_buf[0]=(char)((addr+i*8)/0x80);
		tmp_buf[1]=(char)(((addr+i*8)/0x80)>>8);
		tmp_buf[2]=(char)(((addr+i*8)/0x80)>>16);
		tmp_buf[3]=(char)(((addr+i*8)/0x80)>>24);
		gsl_write_interface(client,0xf0,tmp_buf,4);
		tmp_addr = (char)((addr+i*8)%0x80);
		gsl_read_interface(client,tmp_addr,(buf+i*8),8);
	}
	if(i*8<num){
		tmp_buf[0]=(char)((addr+i*8)/0x80);
		tmp_buf[1]=(char)(((addr+i*8)/0x80)>>8);
		tmp_buf[2]=(char)(((addr+i*8)/0x80)>>16);
		tmp_buf[3]=(char)(((addr+i*8)/0x80)>>24);
		gsl_write_interface(client,0xf0,tmp_buf,4);
		tmp_addr = (char)((addr+i*8)%0x80);
		gsl_read_interface(client,tmp_addr,(buf+i*8),4);
	}
	return 0;
}
#endif
static int char_to_int(char ch)
{
	if(ch>='0' && ch<='9')
		return (ch-'0');
	else
		return (ch-'a'+10);
}

//static int gsl_config_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
static int gsl_config_read_proc(struct seq_file *m,void *v)
{
	char temp_data[5] = {0};
	//int i;
	unsigned int tmp=0;
	if('v'==gsl_read[0]&&'s'==gsl_read[1])
	{
#ifdef GSL_ALG_ID
		tmp=gsl_version_id();
#else
		tmp=0x20121215;
#endif
		seq_printf(m,"version:%x\n",tmp);
	}
	else if('r'==gsl_read[0]&&'e'==gsl_read[1])
	{
		if('i'==gsl_read[3])
		{
#ifdef GSL_ALG_ID
			tmp=(gsl_data_proc[5]<<8) | gsl_data_proc[4];
			seq_printf(m,"gsl_cfg_table[gsl_cfg_index][%d] = ",tmp);
			if(tmp>=0&&tmp<gsl_cfg_table[gsl_cfg_index].data_size)
				seq_printf(m,"%d\n",gsl_cfg_table[gsl_cfg_index].data_id[tmp]); 
#endif
		}
		else
		{
			gsl_write_interface(ddata->client,0xf0,&gsl_data_proc[4],4);
			gsl_read_interface(ddata->client,gsl_data_proc[0],temp_data,4);
			seq_printf(m,"offset : {0x%02x,0x",gsl_data_proc[0]);
			seq_printf(m,"%02x",temp_data[3]);
			seq_printf(m,"%02x",temp_data[2]);
			seq_printf(m,"%02x",temp_data[1]);
			seq_printf(m,"%02x};\n",temp_data[0]);
		}
	}
#ifdef GSL_APPLICATION
	else if('a'==gsl_read[0]&&'p'==gsl_read[1]){
		char *buf;
		int temp1;
		tmp = (unsigned int)(((gsl_data_proc[2]<<8)|gsl_data_proc[1])&0xffff);
		buf=kzalloc(tmp,GFP_KERNEL);
		if(buf==NULL)
			return -1;
		if(3==gsl_data_proc[0]){
			gsl_read_interface(ddata->client,gsl_data_proc[3],buf,tmp);
			if(tmp < m->size){
				memcpy(m->buf,buf,tmp);
			}
		}else if(4==gsl_data_proc[0]){
			temp1=((gsl_data_proc[6]<<24)|(gsl_data_proc[5]<<16)|
				(gsl_data_proc[4]<<8)|gsl_data_proc[3]);
			gsl_read_MorePage(ddata->client,temp1,buf,tmp);
			if(tmp < m->size){
				memcpy(m->buf,buf,tmp);
			}
		}
		kfree(buf);
	}
#endif
	return 0;
}

static ssize_t  gsl_config_write_proc(struct file *file, const char __user  *buffer, size_t  count, loff_t *data)
{
	u8 buf[8] = {0};
	char temp_buf[CONFIG_LEN];
	char *path_buf;
	int tmp = 0;
	int tmp1 = 0;
	print_info("[tp-gsl][%s] \n",__func__);
	if(count > 512)
	{
		//print_info("size not match [%d:%ld]\n", CONFIG_LEN, count);
        	return -EFAULT;
	}
	path_buf=kzalloc(count,GFP_KERNEL);
	if(!path_buf)
	{
		print_info("alloc path_buf memory error \n");
		return -1;
	}
	if(copy_from_user(path_buf, buffer, count))
	{
		print_info("copy from user fail\n");
		goto exit_write_proc_out;
	}
	memcpy(temp_buf,path_buf,(count<CONFIG_LEN?count:CONFIG_LEN));
	print_info("[tp-gsl][%s][%s]\n",__func__,temp_buf);
#ifdef GSL_APPLICATION
	if('a'!=temp_buf[0]||'p'!=temp_buf[1]){
#endif
	buf[3]=char_to_int(temp_buf[14])<<4 | char_to_int(temp_buf[15]);	
	buf[2]=char_to_int(temp_buf[16])<<4 | char_to_int(temp_buf[17]);
	buf[1]=char_to_int(temp_buf[18])<<4 | char_to_int(temp_buf[19]);
	buf[0]=char_to_int(temp_buf[20])<<4 | char_to_int(temp_buf[21]);

	buf[7]=char_to_int(temp_buf[5])<<4 | char_to_int(temp_buf[6]);
	buf[6]=char_to_int(temp_buf[7])<<4 | char_to_int(temp_buf[8]);
	buf[5]=char_to_int(temp_buf[9])<<4 | char_to_int(temp_buf[10]);
	buf[4]=char_to_int(temp_buf[11])<<4 | char_to_int(temp_buf[12]);
#ifdef GSL_APPLICATION
	}
#endif
	if('v'==temp_buf[0]&& 's'==temp_buf[1])//version //vs
	{
		memcpy(gsl_read,temp_buf,4);
		print_info("gsl version\n");
	}
	else if('s'==temp_buf[0]&& 't'==temp_buf[1])//start //st
	{
#ifdef GSL_TIMER
		cancel_delayed_work_sync(&gsl_timer_check_work);
#endif
		gsl_proc_flag = 1;
		gsl_reset_core(ddata->client);
	}
	else if('e'==temp_buf[0]&&'n'==temp_buf[1])//end //en
	{
		msleep(20);
		gsl_reset_core(ddata->client);
		gsl_start_core(ddata->client);
    #ifdef GSL_TIMER
	queue_delayed_work(gsl_timer_workqueue, &gsl_timer_check_work, GSL_TIMER_CHECK_CIRCLE);
    #endif	
		gsl_proc_flag = 0;
	}
	else if('r'==temp_buf[0]&&'e'==temp_buf[1])//read buf //
	{
		memcpy(gsl_read,temp_buf,4);
		memcpy(gsl_data_proc,buf,8);
	}
	else if('w'==temp_buf[0]&&'r'==temp_buf[1])//write buf
	{
		gsl_write_interface(ddata->client,buf[4],buf,4);
	}

#ifdef GSL_ALG_ID
	else if('i'==temp_buf[0]&&'d'==temp_buf[1])//write id config //
	{
		tmp1=(buf[7]<<24)|(buf[6]<<16)|(buf[5]<<8)|buf[4];
		tmp=(buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|buf[0];

		if(tmp1>=0 && tmp1<gsl_cfg_table[gsl_cfg_index].data_size)
		{
			gsl_cfg_table[gsl_cfg_index].data_id[tmp1] = tmp;
		}
	}
#endif
#ifdef GSL_APPLICATION
	else if('a'==temp_buf[0]&&'p'==temp_buf[1]){
		if(1==path_buf[3]){
			tmp=((path_buf[5]<<8)|path_buf[4]);
			gsl_write_interface(ddata->client,path_buf[6],&path_buf[10],tmp);
		}else if(2==path_buf[3]){
			tmp = ((path_buf[5]<<8)|path_buf[4]);
			tmp1=((path_buf[9]<<24)|(path_buf[8]<<16)|(path_buf[7]<<8)
				|path_buf[6]);
			buf[0]=(char)((tmp1/0x80)&0xff);
			buf[1]=(char)(((tmp1/0x80)>>8)&0xff);
			buf[2]=(char)(((tmp1/0x80)>>16)&0xff);
			buf[3]=(char)(((tmp1/0x80)>>24)&0xff);
			buf[4]=(char)(tmp1%0x80);
			gsl_write_interface(ddata->client,0xf0,buf,4);
			gsl_write_interface(ddata->client,buf[4],&path_buf[10],tmp);
		}else if(3==path_buf[3]||4==path_buf[3]){
			memcpy(gsl_read,temp_buf,4);
			memcpy(gsl_data_proc,&path_buf[3],7);
		}
	}
#endif
exit_write_proc_out:
	kfree(path_buf);
	return count;
}
#endif


#ifdef GSL_TIMER
static void gsl_timer_check_func(struct work_struct *work)
{
	struct gsl_ts_data *ts = ddata;
	struct i2c_client *gsl_client = ts->client;
	static int i2c_lock_flag = 0;
	char read_buf[4]  = {0};
	char init_chip_flag = 0;
	int i,flag;
#ifdef  __HCT_TP_VDD_USE_VIO28_SUPPORT__
#if __HCT_TP_VDD_USE_VIO28_SUPPORT__
	print_info("tp vdd use vio28\n");
#else
	int ret = 0;
#endif
#endif
	print_info("----------------gsl_monitor_worker-----------------\n");	

	if(i2c_lock_flag != 0)
		return;
	else
		i2c_lock_flag = 1;

	gsl_read_interface(gsl_client, 0xb4, read_buf, 4);
	memcpy(int_2nd,int_1st,4);
	memcpy(int_1st,read_buf,4);
	print_info("======int_1st222: %x %x %x %x , int_2nd: %x %x %x %x ======\n",
			int_1st[3], int_1st[2], int_1st[1], int_1st[0], 
			int_2nd[3], int_2nd[2],int_2nd[1],int_2nd[0]);
	if(int_1st[3] == int_2nd[3] && int_1st[2] == int_2nd[2] &&
		int_1st[1] == int_2nd[1] && int_1st[0] == int_2nd[0])
	{
	
		init_chip_flag = 1;
		goto queue_monitor_work;
	}
	/*check 0xb0 register,check firmware if ok*/
	for(i=0;i<5;i++){
		gsl_read_interface(gsl_client, 0xb0, read_buf, 4);
			print_info("gsl_monitor_worker222 0xb0 = {0x%02x%02x%02x%02x};\n",
				read_buf[3],read_buf[2],read_buf[1],read_buf[0]);
		if(read_buf[3] != 0x5a || read_buf[2] != 0x5a || 
			read_buf[1] != 0x5a || read_buf[0] != 0x5a){
			flag = 1;
		}else{
			flag = 0;
			break;
		}

	}
	if(flag == 1){
		init_chip_flag = 1;
		goto queue_monitor_work;
	}
	
	/*check 0xbc register,check dac if normal*/
	for(i=0;i<5;i++){
		gsl_read_interface(gsl_client, 0xbc, read_buf, 4);
		print_info("gsl_monitor_worker222 0xbc = {0x%02x%02x%02x%02x};\n",
				read_buf[3],read_buf[2],read_buf[1],read_buf[0]);
		if(read_buf[3] != 0 || read_buf[2] != 0 || 
			read_buf[1] != 0 || read_buf[0] != 0){
			flag = 1;
			//flag = 0;
		}else{
			flag = 0;
			break;
		}
	}
	if(flag == 1){
		//gsl_reset_core(gsl_client);
		//gsl_start_core(gsl_client);
		init_chip_flag = 1;
		goto queue_monitor_work;
	}
queue_monitor_work:
	if(init_chip_flag){
		//pmic_set_register_value(PMIC_RG_VGP1_EN,0);
		tpd_gpio_output(GTP_RST_PORT, 0);
	#ifdef  __HCT_TP_VDD_USE_VIO28_SUPPORT__
	#if __HCT_TP_VDD_USE_VIO28_SUPPORT__
		print_info("tp vdd use vio28\n");
	#else
		ret = regulator_disable(tpd->reg);
	#endif
	#endif
		msleep(10);
		//pmic_set_register_value(PMIC_RG_VGP1_EN,1);
	        //pmic_set_register_value(PMIC_RG_VGP1_VOSEL,0x5);
	#ifdef  __HCT_TP_VDD_USE_VIO28_SUPPORT__
	#if __HCT_TP_VDD_USE_VIO28_SUPPORT__
		print_info("tp vdd use vio28\n");
	#else	
		ret = regulator_enable(tpd->reg);
	#endif
	#endif
		tpd_gpio_output(GTP_RST_PORT, 1);
                msleep(10);
		gsl_sw_init(gsl_client);
		memset(int_1st,0xff,sizeof(int_1st));
		
	}
	
	if(gsl_halt_flag==0){
		queue_delayed_work(gsl_timer_workqueue, &gsl_timer_check_work, 200);
	}
	i2c_lock_flag = 0;

}
#endif
#ifdef GSL_COMPATIBLE_BY_ADC
 static void gsl_idt_adc_tp(void)
{
	int vol_chx;
	int rd_data[4] = {0, 0, 0, 0};
	int i,flag;
	for(i=0;i<3;i++){
		if(IMM_GetOneChannelValue(0, rd_data, NULL) < 0){
			flag = 0;
		}else{
			flag = 1;
			break;
		}
	}

	if(flag == 0){
		print_info("[tp-gsl][%s]Get ADC channel 0 failed.\r\n",__func__);
		return;
	}

	vol_chx = rd_data[0]*1000 + rd_data[1]*10;	//the unit is mv.
	print_info("[tp-gsl][%s] vol = %d mv .\r\n",__func__,vol_chx);
	if(vol_chx>1000)
	{
		gsl_cfg_index = 1;		// taiguan tp 1
	}
	else
	{
		gsl_cfg_index = 0;		//qiutianwei tp
	}
	//extern void app_get_ctp_name(char *name);
    //app_get_ctp_name("gsl2681_wsvga_gp313_t702");
}
#endif

#ifdef TPD_PROXIMITY


static int tpd_get_ps_value(void)
{
	return tpd_proximity_detect;
}
static int tpd_enable_ps(int enable)
{
	u8 buf[4];
	if (enable) {
		//wake_lock(&ps_lock);

		buf[3] = 0x00;
		buf[2] = 0x00;
		buf[1] = 0x00;
		buf[0] = 0x4;
		gsl_write_interface(ddata->client, 0xf0, buf, 4);
		buf[3] = 0x0;
		buf[2] = 0x0;
		buf[1] = 0x0;
		buf[0] = 0x2;
		gsl_write_interface(ddata->client, 0, buf, 4);

		tpd_proximity_flag = 1;
		//add alps of function
		print_info("tpd-ps function is on\n");
	} else {
		tpd_proximity_flag = 0;
		//wake_unlock(&ps_lock);

		buf[3] = 0x00;
		buf[2] = 0x00;
		buf[1] = 0x00;
		buf[0] = 0x4;

		gsl_write_interface(ddata->client, 0xf0,buf,4);
		buf[3] =0x0;
		buf[2] = 0x0;
		buf[1] = 0x0;
		buf[0] = 0x0;
		gsl_write_interface(ddata->client, 0x00,buf,4);	
		printk("tpd-ps function is off\n");
	}
	return 0;
}

static int ps_open_report_data(int open)
{
    return 0;

			}
			// Do nothing

/* if use  this type of enable , Psensor only enabled but not report inputEvent to HAL */
static int ps_enable_nodata(int en)
			{
	printk("gsl_obj ps enable value = %d\n", en);
	if (en) {
		tpd_enable_ps(1);
		set_bit(CMC_BIT_PS, &gsl_obj->enable);
	} else {
		tpd_enable_ps(0);
		clear_bit(CMC_BIT_PS, &gsl_obj->enable);
					}
	return 0;
				}

static int ps_set_delay(u64 ns)
					{
    return 0;
					}
				//					clear_bit(CMC_BIT_PS, &obj->enable);

static int ps_get_data(int *value, int *status)
			{
	*value = tpd_get_ps_value();
	printk("tpd_proximity_detect = %d\n",*value);
	*status = SENSOR_STATUS_ACCURACY_MEDIUM;
	return 0;
			}

static int ps_batch(int flag, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
			{
	return 0;
			}

static int ps_flush(void)
{

	return 0;

}
#endif
#if GTP_GESTURE_WAKEUP
static unsigned int gsl_read_oneframe_data(unsigned int *data,
				unsigned int addr,unsigned int len)
{
	u8 buf[4];
	int i;
	printk("tp-gsl-gesture %s\n",__func__);
	for(i=0;i<len/2;i++){
		buf[0] = ((addr+i*8)/0x80)&0xff;
		buf[1] = (((addr+i*8)/0x80)>>8)&0xff;
		buf[2] = (((addr+i*8)/0x80)>>16)&0xff;
		buf[3] = (((addr+i*8)/0x80)>>24)&0xff;
		gsl_write_interface(ddata->client,0xf0,buf,4);
		gsl_read_interface(ddata->client,(addr+i*8)%0x80,(char *)&data[i*2],8);
	}
	if(len%2){
		buf[0] = ((addr+len*4 - 4)/0x80)&0xff;
		buf[1] = (((addr+len*4 - 4)/0x80)>>8)&0xff;
		buf[2] = (((addr+len*4 - 4)/0x80)>>16)&0xff;
		buf[3] = (((addr+len*4 - 4)/0x80)>>24)&0xff;
		gsl_write_interface(ddata->client,0xf0,buf,4);
		gsl_read_interface(ddata->client,(addr+len*4 - 4)%0x80,(char *)&data[len-1],4);
	}
	return len;
}
static void gsl_enter_doze(struct gsl_ts_data *ts)
{
	u8 buf[4] = {0};
#if 0
	u32 tmp;
	gsl_reset_core(ts->client);
	temp = ARRAY_SIZE(GSLX68X_FW_GESTURE);
	gsl_load_fw(ts->client,GSLX68X_FW_GESTURE,temp);
	gsl_start_core(ts->client);
	msleep(1000);		
#endif

	buf[0] = 0xa;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	gsl_write_interface(ts->client,0xf0,buf,4);
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0x1;
	buf[3] = 0x5a;
	gsl_write_interface(ts->client,0x8,buf,4);
	gsl_gesture_status = GE_NOWORK;
	msleep(50);
	gsl_gesture_status = GE_ENABLE;

}
static void gsl_quit_doze(struct gsl_ts_data *ts)
{
	u8 buf[4] = {0};

	gsl_gesture_status = GE_DISABLE;
	
	tpd_gpio_output(GTP_RST_PORT, 0);	
	msleep(10);
	tpd_gpio_output(GTP_RST_PORT, 1);
	msleep(5);
	
	buf[0] = 0xa;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	gsl_write_interface(ts->client,0xf0,buf,4);
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0x5a;
	gsl_write_interface(ts->client,0x8,buf,4);
	msleep(5);

#if 0
	gsl_reset_core(ddata->client);
	temp = ARRAY_SIZE(GSLX68X_FW_CONFIG);
	//gsl_load_fw();
	gsl_load_fw(ddata->client,GSLX68X_FW_CONFIG,temp);
	gsl_start_core(ddata->client);
#endif
}
#if  0
static ssize_t gsl_sysfs_tpgesture_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	u32 count = 0;
	count += scnprintf(buf,PAGE_SIZE,"tp gesture is on/off:\n");
	if(gsl_gesture_flag == 1){
		count += scnprintf(buf+count,PAGE_SIZE-count,
				" on \n");
	}else if(gsl_gesture_flag == 0){
		count += scnprintf(buf+count,PAGE_SIZE-count,
				" off \n");
	}
	count += scnprintf(buf+count,PAGE_SIZE-count,"tp gesture:");
	count += scnprintf(buf+count,PAGE_SIZE-count,
			"%c\n",gsl_gesture_c);
    	return count;
}
static ssize_t gsl_sysfs_tpgesturet_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
#if 1
	if(buf[0] == '0'){
		gsl_gesture_flag = 0;  
	}else if(buf[0] == '1'){
		gsl_gesture_flag = 1;
	}
#endif
	return count;
}
#endif

#endif

/*****************************************************************************
Prototype    : gsl_report_point
Description  : gsl1680 report touch event
Input        : union gsl_touch_info *ti
Output       : None
Return Value : static

 *****************************************************************************/
//static void gsl_report_point(union gsl_touch_info *ti)
static void gsl_report_point(struct gsl_touch_info *ti)
{
    int tmp = 0;
	static int gsl_up_flag = 0; //prevent more up event

    if (unlikely(ti->finger_num == 0))
    {
    	if(gsl_up_flag == 0)
	{
			return;
    	
	}
	gsl_up_flag = 0;
        input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
        input_report_key(tpd->dev, BTN_TOUCH, 0);
        input_mt_sync(tpd->dev);
	    if (FACTORY_BOOT == get_boot_mode()||
			RECOVERY_BOOT == get_boot_mode())
		{

			tpd_button(ti->x[tmp], ti->y[tmp], 0);

		}
	}
	else
	{
		gsl_up_flag = 1;
		for (tmp = 0; ti->finger_num > tmp; tmp++)
		{
			print_info("[gsl1680](x[%d],y[%d]) = (%d,%d);\n",
				ti->id[tmp], ti->id[tmp], ti->x[tmp], ti->y[tmp]);
			input_report_key(tpd->dev, BTN_TOUCH, 1);
			input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
			#if 0   ////modify by Edwin start
			if((boot_mode!=NORMAL_BOOT && (strncmp(CONFIG_MTK_LCM_PHYSICAL_ROTATION, "270", 3) == 0
				|| strncmp(CONFIG_MTK_LCM_PHYSICAL_ROTATION, "90", 3) == 0))
				|| (FACTORY_BOOT == get_boot_mode())) // mars@20131211
			{
				int temp;
				temp = ti->y[tmp];
				ti->y[tmp] = ti->x[tmp];
				ti->x[tmp] = TPD_RES_X-temp;
			}
			#endif    ////modify by Edwin end
			if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode()) // || ALARM_BOOT == get_boot_mode())

			{
#if 0
				tpd_button(ti->x[tmp], ti->y[tmp], 1);
				input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, ti->id[tmp] - 1);
				input_report_abs(tpd->dev, ABS_MT_POSITION_X, (ti->x[tmp])*600/1024);   //modify by Edwin
				input_report_abs(tpd->dev, ABS_MT_POSITION_Y, (ti->y[tmp])*1024/600);   //modify by Edwin
#endif
                                input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, ti->id[tmp] - 1);
                                input_report_abs(tpd->dev, ABS_MT_POSITION_X, ti->x[tmp]);
                                input_report_abs(tpd->dev, ABS_MT_POSITION_Y, ti->y[tmp]);

			}
			else
			{
				input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, ti->id[tmp] - 1);
				input_report_abs(tpd->dev, ABS_MT_POSITION_X, ti->x[tmp]);
				input_report_abs(tpd->dev, ABS_MT_POSITION_Y, ti->y[tmp]);
			}
			input_mt_sync(tpd->dev);
		}
	}
	input_sync(tpd->dev);
}

/*****************************************************************************
Prototype    : gsl_report_work
Description  : to deal interrupt throught workqueue
Input        : struct work_struct *work
Output       : None
Return Value : static

 *****************************************************************************/
static void gsl_report_work(void)
{

	u8 i = 0;
	u16 ret = 0;
	u16 tmp = 0,tmp1 = 0;
	struct gsl_touch_info cinfo={{0}};
	u8 buf[4] = {0};
	u8 tmp_buf[44] ={0};
	u8 tmp1_buf[44] ={0};
	unsigned int temp80 = 0;
#ifdef TPD_PROXIMITY
//	int err;
	
	struct hwm_sensor_data sensor_data;
    /*added by bernard*/
	if (tpd_proximity_flag == 1)
	{

		gsl_read_interface(ddata->client,0xac,buf,4);
		printk("gslX680   buf[0] = %d buf[1] = %d,	buf[2] = %d  buf[3] = %d \n",buf[0],buf[1],buf[2],buf[3]);
		if (buf[0] == 1 && buf[1] == 0 && buf[2] == 0 && buf[3] == 0)
		{
			tpd_proximity_detect = 0;
			//sensor_data.values[0] = 0;
		}
		else
		{
			tpd_proximity_detect = 1;
			//sensor_data.values[0] = 1;
		}
		printk("gslX680    ps change   tpd_proximity_detect = %d  \n",tpd_proximity_detect);
		//get raw data
		printk(" ps change\n");
		//map and store data to hwm_sensor_data
		sensor_data.values[0] = tpd_get_ps_value();
		sensor_data.value_divide = 1;
		sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
		//let up layer to know
#if 0
		if((err = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data)))
		{
			print_info("call hwmsen_get_interrupt_data fail = %d\n", err);
		}
#endif
		if(ps_report_interrupt_data(tpd_get_ps_value()))
                {
                    print_info("call hwmsen_get_interrupt_data fail \n");
                }
            
		
	}
	/*end of added*/
#endif



#ifdef TPD_PROC_DEBUG
	if(gsl_proc_flag == 1){
		return;
	}
#endif
#ifdef GSL_IRQ_CHECK
	gsl_read_interface(ddata->client,0xbc,buf,4);
	if(0x80==buf[3]&&0==buf[2]&&0==buf[1]&&0==buf[0])
	{
		gsl_reset_core(ddata->client);
		gsl_start_core(ddata->client);
		msleep(20);
		check_mem_data(ddata->client);
		goto gsl_report_work_out;
	}
#endif

 	gsl_read_interface(ddata->client, 0x80, tmp_buf, 8);
	if(tmp_buf[0]>=2&&tmp_buf[0]<=10)
		gsl_read_interface(ddata->client, 0x88, &tmp_buf[8], (tmp_buf[0]*4-4));
	cinfo.finger_num = tmp_buf[0] & 0x0f;
	for(tmp=0;tmp<(cinfo.finger_num>10?10:cinfo.finger_num);tmp++)
	{
		cinfo.id[tmp] = tmp_buf[tmp*4+7] >> 4;
		cinfo.y[tmp] = (tmp_buf[tmp*4+4] | ((tmp_buf[tmp*4+5])<<8));
		cinfo.x[tmp] = (tmp_buf[tmp*4+6] | ((tmp_buf[tmp*4+7] & 0x0f)<<8));
	}
#ifdef GSL_ALG_ID

	
	cinfo.finger_num = (tmp_buf[3]<<24)|(tmp_buf[2]<<16)|(tmp_buf[1]<<8)|(tmp_buf[0]);

 	gsl_read_interface(ddata->client, 0x80, tmp1_buf, 8);
	temp80 = (tmp1_buf[3]<<24)|(tmp1_buf[2]<<16)|(tmp1_buf[1]<<8)|(tmp1_buf[0]);
	print_info("tp-gsl temp80 = %x \n",temp80);
	print_info("tp-gsl  x = %d y = %d finger_num = %d\n",cinfo.x[tmp],cinfo.y[tmp],cinfo.finger_num);
	
	gsl_alg_id_main(&cinfo);
	
	print_info("tp-gsl  x = %d y = %d finger_num = %d\n",cinfo.x[tmp],cinfo.y[tmp],cinfo.finger_num);
 	gsl_read_interface(ddata->client, 0x80, tmp1_buf, 8);
	temp80 = (tmp1_buf[3]<<24)|(tmp1_buf[2]<<16)|(tmp1_buf[1]<<8)|(tmp1_buf[0]);
	print_info("tp-gsl temp80 = %x \n",temp80);
	
	tmp1=gsl_mask_tiaoping();
	print_info("[tp-gsl] tmp1=%x\n",tmp1);
	if(tmp1>0&&tmp1<0xffffffff)
	{
		buf[0]=0xa;
		buf[1]=0;
		buf[2]=0;
		buf[3]=0;
		gsl_write_interface(ddata->client,0xf0,buf,4);
		buf[0]=(u8)(tmp1 & 0xff);
		buf[1]=(u8)((tmp1>>8) & 0xff);
		buf[2]=(u8)((tmp1>>16) & 0xff);
		buf[3]=(u8)((tmp1>>24) & 0xff);
		gsl_write_interface(ddata->client,0x8,buf,4);
	}
#endif
#if GTP_GESTURE_WAKEUP
		print_info("gsl_gesture_status=%d,gsl_gesture_flag=%d\n",gsl_gesture_status,gsl_gesture_flag);
	
		if(GE_ENABLE == gsl_gesture_status && gsl_gesture_flag == 1){
		int tmp_c;
			//u8 key_data = 0;
		tmp_c = gsl_obtain_gesture();
		printk("gsl_obtain_gesture():tmp_c=0x%x\n",tmp_c);
			printk("gsl_obtain_gesture():tmp_c=0x%x\n",tmp_c);
	switch(tmp_c)
	{
		case (int)'*':
            		strcpy(tpgesture_value,"DOUBCLICK");
			tpgesture_hander();
			break;
		case (int)0xa1fc:
            		strcpy(tpgesture_value,"UP");
			tpgesture_hander();
			break;
		case (int)0xa1fd:
			strcpy(tpgesture_value,"DOWN");
			tpgesture_hander();
			break;
		case (int)0xa1fb:
			strcpy(tpgesture_value,"LEFT");
			tpgesture_hander();
			break;
		case (int)0xa1fa:
			strcpy(tpgesture_value,"RIGHT");
			tpgesture_hander();
			break;
		case (int)'C':
			strcpy(tpgesture_value,"c");
			tpgesture_hander();
			break;
		case (int)'O':
			strcpy(tpgesture_value,"o");
			tpgesture_hander();
			break;
		case (int)'W':
			strcpy(tpgesture_value,"w");
			tpgesture_hander();
			break;
		case (int)'E':
			strcpy(tpgesture_value,"e");
			tpgesture_hander();
			break;
              case (int)'V':
			strcpy(tpgesture_value,"v");
			tpgesture_hander();
			break;
		case (int)'M':
			strcpy(tpgesture_value,"m");
			tpgesture_hander();
			break;
		case (int)'Z':
			strcpy(tpgesture_value,"z");
			tpgesture_hander();
			break;
		case (int)'S':
			strcpy(tpgesture_value,"s");
			tpgesture_hander();
			break;
		default:
			break;
		}

			//GTP_GESTURE_WAKEUP_status = GE_WAKEUP;

#ifdef __HCT_TP_631GESTURE_DELAY__
		msleep(400);
#else
		msleep(300);
#endif
		return;
	}
#endif
	gsl_report_point(&cinfo);

	if(strncmp(CONFIG_MTK_LCM_PHYSICAL_ROTATION, "270", 3)==0 || strncmp(CONFIG_MTK_LCM_PHYSICAL_ROTATION, "90", 3)==0)
	{
		//print_info("CONFIG_MTK_LCM_PHYSICAL_ROTATION = %d\n",CONFIG_MTK_LCM_PHYSICAL_ROTATION);
		for(i = 0;i < 5 ;i++)
		{
			ret = cinfo.x[i];
			cinfo.x[i] = cinfo.y[i];
			cinfo.y[i] = ret;
		}
	}
#ifdef Point_Report
if(gsl_point!=1)
{

	input_report_abs(tpd->dev, ABS_MT_TRACKING_ID,0);
       input_report_abs(tpd->dev, ABS_MT_POSITION_X,0);
       input_report_abs(tpd->dev, ABS_MT_POSITION_Y,0);
}
#endif
	gsl_report_point(&cinfo);
#ifdef GSL_IRQ_CHECK
gsl_report_work_out:
#endif
	//mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	//enable_irq(touch_irq);

}


#ifdef GSL_THREAD_EINT
static int touch_event_handler(void *unused)
{
	struct sched_param param = { .sched_priority = 4 };
	sched_setscheduler(current, SCHED_RR, &param);
	do
	{
		//mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
		//enable_irq(touch_irq);
		set_current_state(TASK_INTERRUPTIBLE);
		wait_event_interruptible(waiter, tpd_flag != 0);
		tpd_flag = 0;
		TPD_DEBUG_SET_TIME;
		set_current_state(TASK_RUNNING);
		gsl_report_work();
	} while (!kthread_should_stop());
	return 0;
}

#endif

/*****************************************************************************
Prototype    : tpd_eint_interrupt_handler
Description  : gsl1680 ISR
Input        : None
Output       : None
Return Value : static

 *****************************************************************************/
static void tpd_eint_interrupt_handler(void)
{

//	print_info("[gsl1680] TPD interrupt has been triggered\n");

	//mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	//disable_irq(touch_irq);
#ifdef GSL_THREAD_EINT
	tpd_flag=1;
    wake_up_interruptible(&waiter);
#else
	if (!work_pending(&ddata->work)) {
		queue_work(ddata->wq, &ddata->work);
	}
#endif
}

/*****************************************************************************
Prototype    : gsl_hw_init
Description  : gsl1680 set gpio
Input        : None
Output       : None
Return Value : static

 *****************************************************************************/
static void gsl_hw_init(void)
{
	#ifdef  __HCT_TP_VDD_USE_VIO28_SUPPORT__
	#if __HCT_TP_VDD_USE_VIO28_SUPPORT__
		print_info("tp vdd use vio28\n");
	#else
	int ret = 0;
        ret = regulator_set_voltage(tpd->reg, 2800000, 2800000);    /* set 2.8v */
        if (ret)
            print_info("regulator_set_voltage() failed!\n");
        ret = regulator_enable(tpd->reg);   /* enable regulator */
        if (ret)
            print_info("regulator_enable() failed!\n");
	#endif
	#endif
	//tpd_gpio_as_int(GTP_RST_PORT);
	tpd_gpio_output(GTP_RST_PORT, 0);
	tpd_gpio_output(GTP_RST_PORT, 1);
	msleep(100);
	tpd_gpio_as_int(GTP_INT_PORT);
	//tpd_gpio_output(GTP_INT_PORT, 1);
	msleep(100);
}

/*****************************************************************************
Prototype    : gsl_sw_init
Description  : gsl1680 load firmware
Input        : struct i2c_client *client
Output       : int
Return Value : static

 *****************************************************************************/


#ifdef GSL_COMPATIBLE_CHIP
static int gsl_compatible_id(struct i2c_client *client)
{
	int i,err;
	u8 buf[4]={0};
	for(i=0;i<5;i++)
	{
		err=gsl_read_interface(client,0xfc,buf,4);
		if(err>0)
		{
			print_info("[tp-gsl] i2c read 0xfc = 0x%02x%02x%02x%02x\n",
				buf[3],buf[2],buf[1],buf[0]);
			break;
		}
	}
	return (err<0?-1:0);
}
#endif
#ifdef TPD_PROC_DEBUG
static int gsl_server_list_open(struct inode *inode,struct file *file)
{
	return single_open(file,gsl_config_read_proc,NULL);
}
static const struct file_operations gsl_seq_fops = {
	.open = gsl_server_list_open,
	.read = seq_read,
	.release = single_release,
	.write = gsl_config_write_proc,
	.owner = THIS_MODULE,
};
#endif

/*****************************************************************************
Prototype    : gsl_probe
Description  : setup gsl1680 driver
Input        : struct i2c_client *client
const struct i2c_device_id *id
Output       : None
Return Value : static

 *****************************************************************************/
static int gsl_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;
struct device_node *node;
	s32 ret = 0;
//	unsigned char tp_data[4];

#ifdef TPD_PROXIMITY
		struct gsl_priv *obj;
		//struct hwmsen_object obj_ps;
		struct ps_control_path ps_ctl = { 0 };
		struct ps_data_path ps_data = { 0 };
			obj = kzalloc(sizeof(*obj), GFP_KERNEL);
			gsl_obj = obj;
#endif
	
	//client->timing = 400;

	ddata = kzalloc(sizeof(struct gsl_ts_data), GFP_KERNEL);
	if (!ddata) {
		print_info("alloc ddata memory error\n");
		return -ENOMEM;
	}
	mutex_init(&gsl_i2c_lock);
	
	client->addr = (0x80>>1);

	ddata->client = client;
	printk("ddata->client->addr = 0x%x \n",ddata->client->addr);
#ifdef __HCT_LDO_TP_GESTURE__
		pinctrl_select_state(pinctrl1, ldotpgesture_en_high);
#endif
	gsl_hw_init();

	i2c_set_clientdata(ddata->client, ddata);
	#if GTP_GESTURE_WAKEUP
        gsl_FunIICRead(gsl_read_oneframe_data);

gsl_GestureExternInt(gsl_model_extern,sizeof(gsl_model_extern)/sizeof(unsigned int)/18);
	#endif

	
#ifdef GSL_COMPATIBLE_CHIP
	err = gsl_compatible_id(ddata->client);
	if(err<0)
		goto  err_malloc;
#endif

#ifdef GSL_COMPATIBLE_BY_ADC
	gsl_idt_adc_tp();
#endif


#ifdef GSL_GPIO_IDT_TP
		gsl_identify_tp(ddata->client);
#endif

#ifdef GSL_IDENTY_TP
		gsl_identify_tp(ddata->client);
#else
gsl_cfg_index = 0;
#endif

#ifdef GSL_DAC_IDT_TP
		gsl_identify_dac(ddata->client);
#endif
	gsl_sw_init(ddata->client);
	msleep(20);
	check_mem_data(ddata->client);

#ifdef GSL_THREAD_EINT
	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	if (IS_ERR(thread)) {
		//err = PTR_ERR(thread);
		//TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", PTR_ERR(thread));
	}
#else
	INIT_WORK(&ddata->work, gsl_report_work);
	ddata->wq = create_singlethread_workqueue(GSL_DEV_NAME);
	if (!(ddata->wq))
	{
		print_info("  can't create workqueue\n");
	}
#endif


	//mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, EINTF_TRIGGER_RISING, tpd_eint_interrupt_handler, 1);
	//mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	node = of_find_matching_node(NULL, touch_of_match);
	//node = of_find_compatible_node(NULL, NULL, "mediatek,TOUCH-eint");
	//node = of_find_compatible_node(NULL, NULL, "mediatek,cap_touch");
	if (node) {
		touch_irq = irq_of_parse_and_map(node, 0);
	//mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	//disable_irq(touch_irq);
		ret = request_irq(touch_irq,
				  (irq_handler_t)tpd_eint_interrupt_handler,
				  !int_type ? IRQF_TRIGGER_RISING :
				  IRQF_TRIGGER_FALLING,
				  "TOUCH-eint", NULL);
		if (ret > 0) {
			ret = -1;
			printk("tpd request_irq IRQ LINE NOT AVAILABLE!.");
		}
	/* mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); */
	enable_irq(touch_irq);
	}

	ret = tpd_info_readVersion();
#ifdef __HCT_TP_GSL968_CUSTOME__
	 sprintf(gsl_driver.descript,"%s Vno: 0x %02x%02x%02x%02x", hct_tp_script,hct_version[3],hct_version[2],hct_version[1],hct_version[0]);
#else
	 sprintf(gsl_driver.descript,"Vno: 0x %02x%02x%02x%02x", hct_version[3],hct_version[2],hct_version[1],hct_version[0]);
#endif
	
#ifdef GSL_TIMER
	INIT_DELAYED_WORK(&gsl_timer_check_work, gsl_timer_check_func);
	gsl_timer_workqueue = create_workqueue("gsl_timer_check");
	queue_delayed_work(gsl_timer_workqueue, &gsl_timer_check_work, GSL_TIMER_CHECK_CIRCLE);
#endif
#ifdef TPD_PROC_DEBUG
	gsl_config_proc = proc_create(GSL_CONFIG_PROC_FILE, 0666, NULL, &gsl_seq_fops);
       if (gsl_config_proc == NULL)
	    {
	        print_info("create_proc_entry %s failed\n", GSL_CONFIG_PROC_FILE);
	    }
	   gsl_proc_flag = 0;

/*	gsl_config_proc = create_proc_entry(GSL_CONFIG_PROC_FILE, 0666, NULL);
	if (gsl_config_proc == NULL)
	{
		print_info("create_proc_entry %s failed\n", GSL_CONFIG_PROC_FILE);
	}
	else
	{
		gsl_config_proc->read_proc = gsl_config_read_proc;
		gsl_config_proc->write_proc = gsl_config_write_proc;
	}
	gsl_proc_flag = 0;
	*/
#endif

#ifdef GSL_ALG_ID

	gsl_DataInit(gsl_cfg_table[gsl_cfg_index].data_id);
	//gsl_DataInit(gsl_config_data_id);
#endif
#ifdef TPD_PROXIMITY
	//obj_ps.self = gsl1680p_obj;
       gsl_obj = kzalloc(sizeof(*gsl_obj), GFP_KERNEL);

	//obj_ps.polling = 1;//need to confirm what mode is!!!
	ps_ctl.is_use_common_factory = false;
	ps_ctl.open_report_data = ps_open_report_data;
	ps_ctl.enable_nodata = ps_enable_nodata;
	ps_ctl.set_delay = ps_set_delay;
	ps_ctl.batch = ps_batch;
	ps_ctl.flush = ps_flush;
	ps_ctl.is_report_input_direct = false;
	ps_ctl.is_support_batch = false;
	err = ps_register_control_path(&ps_ctl);

	if (err) {
		printk("register fail = %d\n", err);
	}

	ps_data.get_data = ps_get_data;
	ps_data.vender_div = 100;
	
	err = ps_register_data_path(&ps_data);
	if (err) {
		printk("tregister fail = %d\n", err);
	}

#endif
#if GTP_GESTURE_WAKEUP
	input_set_capability(tpd->dev, EV_KEY, KEY_POWER);
	input_set_capability(tpd->dev, EV_KEY, KEY_C);
	input_set_capability(tpd->dev, EV_KEY, KEY_S);
	input_set_capability(tpd->dev, EV_KEY, KEY_V);
	input_set_capability(tpd->dev, EV_KEY, KEY_Z);
	input_set_capability(tpd->dev, EV_KEY, KEY_E);
	input_set_capability(tpd->dev, EV_KEY, KEY_M);
	input_set_capability(tpd->dev, EV_KEY, KEY_W);
	input_set_capability(tpd->dev, EV_KEY, KEY_O);
	input_set_capability(tpd->dev, EV_KEY, KEY_U);
	input_set_capability(tpd->dev, EV_KEY, KEY_LEFT);
	input_set_capability(tpd->dev, EV_KEY, KEY_RIGHT);
	input_set_capability(tpd->dev, EV_KEY, KEY_UP);
	input_set_capability(tpd->dev, EV_KEY, KEY_DOWN);
#endif


	//mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	//enable_irq(touch_irq);
#ifdef Point_Report
	gsl_point = 1;
#endif

	tpd_load_status = 1;
	printk("[wsl]GSL end tpd_load_status = %d \n", tpd_load_status);
	check_mem_data(ddata->client);

	return 0;

//err_xfer:
	//   kfree(ddata->ti);
#ifdef GSL_COMPATIBLE_CHIP
err_malloc:
#endif
	if (ddata)
		kfree(ddata);

	return err;
}

/*****************************************************************************
Prototype    : gsl_remove
Description  : remove gsl1680 driver
Input        : struct i2c_client *client
Output       : int
Return Value : static

 *****************************************************************************/
static int gsl_remove(struct i2c_client *client)
{
	print_info("[gsl1680] TPD removed\n");
	return 0;
}

/*****************************************************************************
Prototype    : gsl_detect
Description  : gsl1680 driver local setup without board file
Input        : struct i2c_client *client
int kind
struct i2c_board_info *info
Output       : int
Return Value : static

 *****************************************************************************/
static int gsl_detect (struct i2c_client *client, struct i2c_board_info *info)
{
//	int error;

//	print_info("%s, %d\n", __FUNCTION__, __LINE__);
//	strcpy(info->type, TPD_DEVICE);
        strcpy(info->type, "mtk-tpd");
	return 0;
}

static struct i2c_driver gsl_i2c_driver = {
	.probe = gsl_probe,
	.remove = gsl_remove,
	.id_table = gsl_device_id,
	.detect = gsl_detect,
    	.driver = {
    	    	.name = "gsl1680",
    	    	.owner = THIS_MODULE,
    	    	.of_match_table = tpd_of_match,
    	},
#ifndef ANDROID_4
	//.address_data = &addr_data,
	.address_list = (const unsigned short *)forces,
#endif
};

/*****************************************************************************
Prototype    : gsl_local_init
Description  : setup gsl1680 driver
Input        : None
Output       : None
Return Value : static

 *****************************************************************************/
static int gsl_local_init(void)
{
	int ret;
	print_info();
	print_info("[wsl] gsl_local_init start \n");
	//boot_mode = get_boot_mode();
	//print_info("boot_mode == %d \n", boot_mode);

	//if (boot_mode == SW_REBOOT)
	//boot_mode = NORMAL_BOOT;

#ifdef TPD_HAVE_BUTTON
	print_info("TPD_HAVE_BUTTON\n");
	tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);
#endif
	#ifdef  __HCT_TP_VDD_USE_VIO28_SUPPORT__
	#if __HCT_TP_VDD_USE_VIO28_SUPPORT__
		print_info("tp vdd use vio28\n");
	#else
	print_info("[wsl] regulator_get start \n");
    	tpd->reg = regulator_get(tpd->tpd_dev, "vtouch");
    if (IS_ERR(tpd->reg))
	print_info("tpd->reg error\n");
	print_info("[wsl] regulator_get end \n");
	#endif
	#endif		
       input_set_abs_params(tpd->dev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);
	if (tpd_dts_data.use_tpd_button) {
		/*initialize tpd button data */
		tpd_button_setting(tpd_dts_data.tpd_key_num,
				   tpd_dts_data.tpd_key_local,
				   tpd_dts_data.tpd_key_dim_local);
	}
	ret = i2c_add_driver(&gsl_i2c_driver);

	if (ret < 0) {
		print_info("unable to i2c_add_driver\n");
		return -ENODEV;
	}

	if (tpd_load_status == 0)
	{
		print_info(" tpd_load_status == 0, gsl_probe failed\n");
		i2c_del_driver(&gsl_i2c_driver);
		return -ENODEV;
	}

	/* define in tpd_debug.h */
	tpd_type_cap = 1;
	print_info("end %s, %d\n", __FUNCTION__, __LINE__);
	return 0;
}

/*****************************************************************************
Prototype    : gsl_suspend
Description  : gsl chip power manage, device goto sleep
Input        : struct i2c_client *client
Output       : int
Return Value : static

 *****************************************************************************/
static void gsl_suspend(struct device *h)
{
	int tmp;
#ifdef __HCT_TP_GSL_POWER_OFF__
	int ret;
#endif

#ifdef TPD_PROXIMITY
    if (tpd_proximity_flag == 1)
    {
        return;
    }
#endif
	gsl_halt_flag = 1;
	//version info
	print_info("[tp-gsl]the last time of debug:%x\n",TPD_DEBUG_TIME);
#ifdef GSL_ALG_ID
	tmp = gsl_version_id();
	print_info("[tp-gsl]the version of alg_id:%x\n",tmp);
#endif

	//version info

#ifdef TPD_PROC_DEBUG
	if(gsl_proc_flag == 1){
		return;
	}
#endif

#ifdef GSL_TIMER
	cancel_delayed_work_sync(&gsl_timer_check_work);
#endif
#if GTP_GESTURE_WAKEUP
	if(tpgesture_status == 1){
#ifdef __HCT_LDO_TP_GESTURE__
		pinctrl_select_state(pinctrl1, ldotpgesture_en_high);
#endif
		gsl_enter_doze(ddata);
		return;
	}
	else
	{
	print_info("[tp-gsl]tpgesture_status :%d\n",tpgesture_status );
#ifdef __HCT_LDO_TP_GESTURE__
		pinctrl_select_state(pinctrl1, ldotpgesture_en_low);
#endif
	}
#endif


	//mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	disable_irq(touch_irq);
//	gsl_reset_core(ddata->client);
//	msleep(20);
//	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
	tpd_gpio_output(GTP_RST_PORT, 0);
	//hwPowerDown(MT65XX_POWER_LDO_VGP1, "TP");
#ifdef __HCT_TP_GSL_POWER_OFF__
	ret = regulator_disable(tpd->reg);
#endif
}

/*****************************************************************************
Prototype    : gsl_suspend
Description  : gsl chip power manage, wake up device
Input        : struct i2c_client *client
Output       : int
Return Value : static

 *****************************************************************************/
static void gsl_resume(struct device *h)
{
#ifdef __HCT_TP_GSL_POWER_OFF__
	int ret;
#endif
    print_info();
#ifdef TPD_PROXIMITY
    if (tpd_proximity_flag == 1 && gsl_halt_flag == 0)
    {
        tpd_enable_ps(1);
	
        return;
    }
#endif


#ifdef TPD_PROC_DEBUG
	if(gsl_proc_flag == 1){
		
		return;
	}
#endif
#if GTP_GESTURE_WAKEUP
	if(gsl_gesture_flag == 1){
		gsl_quit_doze(ddata);
	}
#endif
	
	//mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
#ifdef __HCT_TP_GSL_POWER_OFF__	
ret = regulator_enable(tpd->reg);
#endif
#ifdef __HCT_LDO_TP_GESTURE__
pinctrl_select_state(pinctrl1, ldotpgesture_en_high);
#endif
	tpd_gpio_output(GTP_RST_PORT, 1);
	msleep(10);
	
	gsl_reset_core(ddata->client);
	gsl_start_core(ddata->client);
	msleep(5);
	
	check_mem_data(ddata->client);
	//mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	enable_irq(touch_irq);

#ifdef GSL_TIMER
	queue_delayed_work(gsl_timer_workqueue, &gsl_timer_check_work, GSL_TIMER_CHECK_CIRCLE);
#endif
	gsl_halt_flag = 0;
#ifdef TPD_PROXIMITY
	if(tpd_proximity_flag == 1)
    {
        tpd_enable_ps(1);
        return;
    }
#endif
	//dump_stack();
}
#if GTP_GESTURE_WAKEUP
static ssize_t show_tpgesture_value(struct device* dev, struct device_attribute *attr, char *buf)
{
	print_info("show tp gesture value is %s \n",tpgesture_value);
	return scnprintf(buf, PAGE_SIZE, "%s\n", tpgesture_value);
}
static ssize_t show_tpgesture_status_value(struct device* dev, struct device_attribute *attr, char *buf)
{
	print_info("show tp gesture status is %s \n",tpgesture_status_value);
	return scnprintf(buf, PAGE_SIZE, "%s\n", tpgesture_status_value);
}
static ssize_t store_tpgesture_status_value(struct device* dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if(!strncmp(buf, "on", 2))
	{
		sprintf(tpgesture_status_value,"on");
		tpgesture_status = 1;//status --- on
	}
	else
	{
		sprintf(tpgesture_status_value,"off");
		tpgesture_status = 0;//status --- off
	}
	return count;
}
static DEVICE_ATTR(tpgesture,  0664, show_tpgesture_value, NULL);
static DEVICE_ATTR(tpgesture_status,  0664, show_tpgesture_status_value, store_tpgesture_status_value);
static struct device_attribute *tpd_attr_list[] = {
	&dev_attr_tpgesture,
	&dev_attr_tpgesture_status,
};
#endif
static struct tpd_driver_t gsl_driver = {
	.tpd_device_name = GSL_DEV_NAME,
	.tpd_local_init = gsl_local_init,
	.suspend = gsl_suspend,
	.resume = gsl_resume,
#ifdef TPD_HAVE_BUTTON
	.tpd_have_button = 1,
#else
 	.tpd_have_button = 0,
#endif
#if GTP_GESTURE_WAKEUP
			.attrs = {
				.attr = tpd_attr_list,
				.num = (int)(sizeof(tpd_attr_list)/sizeof(tpd_attr_list[0])),
			},
#endif
};

/*****************************************************************************
Prototype    : gsl_driver_init
Description  : driver module entry
Input        : None
Output       : int
Return Value : static

 *****************************************************************************/
static int __init gsl_driver_init(void)
{
//	int ret;
	tpd_get_dts_info();
	if(tpd_driver_add(&gsl_driver) < 0)
		print_info("gsl_driver init error, return num is  \n");
	
	return 0;
}

/*****************************************************************************
Prototype    : gsl_driver_exit
Description  : driver module exit
Input        : None
Output       : None
Return Value : static

 *****************************************************************************/
static void __exit gsl_driver_exit(void)
{
	print_info();
	tpd_driver_remove(&gsl_driver);
}

module_init(gsl_driver_init);
module_exit(gsl_driver_exit);

