/*
 * Copyright (C) 2010 MediaTek, Inc.
 *
 * Author: Terry Chang <terry.chang@mediatek.com>
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

#include "kpd.h"
#ifdef CONFIG_PM_WAKELOCKS
#include <linux/pm_wakeup.h>
#else
#include <linux/wakelock.h>
#endif
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/debugfs.h>

/* hct-drv add for hall by zcy start @20181010 */
#include <linux/slab.h>
#include <linux/of_gpio.h>
/* hct-drv add for hall by zcy end */
#include <linux/hct_include/hct_project_all_config.h>
#define KPD_NAME	"mtk-kpd"
#define HALL_NAME	"mtk-hall"  //hct-drv add for hall by zcy start @20181010

/* hct-drv add for customkey F1 by zcy begin */
#ifdef __HCT_CUSTOMKEY_F1_SUPPORT__
	#if __HCT_CUSTOMKEY_F1_SUPPORT__
extern int hct_customkey_f1_init(struct platform_device *pdev);
	#endif
#endif
/* hct-drv add for customkey F1 by zcy end */
#ifdef CONFIG_LONG_PRESS_MODE_EN
struct timer_list Long_press_key_timer;
atomic_t vol_down_long_press_flag = ATOMIC_INIT(0);
#endif

int kpd_klog_en;
void __iomem *kp_base;
static unsigned int kp_irqnr;
struct input_dev *kpd_input_dev;
static struct dentry *kpd_droot;
static struct dentry *kpd_dklog;
unsigned long call_status;
static bool kpd_suspend;
static unsigned int kp_irqnr;
static u32 kpd_keymap[KPD_NUM_KEYS];
static u16 kpd_keymap_state[KPD_NUM_MEMS];

struct input_dev *kpd_input_dev;
#ifdef CONFIG_PM_WAKELOCKS
struct wakeup_source kpd_suspend_lock;
#else
struct wake_lock kpd_suspend_lock;
#endif
struct keypad_dts_data kpd_dts_data;

/* for keymap handling */
static void kpd_keymap_handler(unsigned long data);
static DECLARE_TASKLET(kpd_keymap_tasklet, kpd_keymap_handler, 0);

static void kpd_memory_setting(void);
static int kpd_pdrv_probe(struct platform_device *pdev);
static int kpd_pdrv_suspend(struct platform_device *pdev, pm_message_t state);
static int kpd_pdrv_resume(struct platform_device *pdev);
static struct platform_driver kpd_pdrv;

/* hct-drv add for hall by zcy start @20181010*/
#if __HCT_HALL_SUPPORT__
#include <linux/switch.h>

#define CLAM_CLOSE 1
#define CLAM_OPEN  0

#define CLAM_USE_KEY  KEY_PRINT
#define CLAM_CLOSE_KEY KEY_BASSBOOST
#define CLAM_OPEN_KEY  KEY_FASTFORWARD
//#define CLAM_CLOSE_KEY KEY_VOLUMEUP  //only for test
//#define CLAM_OPEN_KEY KEY_VOLUMEDOWN //only for test

static int hall_pdrv_probe(struct platform_device *pdev);
static int hall_pdrv_remove(struct platform_device *pdev);




struct hct_hall_struct
{
    u8 state;
    struct work_struct eint_work;
    struct delayed_work init_work;
};

static u8 hct_hall_state = CLAM_OPEN;
static struct switch_dev hct_hall_dev;
static struct hct_hall_struct * phct_hall_data;


#ifdef __HCT_TP_GLOVE_FOR_HALL__
int hct_hall_flag = 0;
extern int hct_fts_enter_glove_mode(int mode);
extern void gt1x_power_reset(void);
#endif

#endif
/* hct-drv add for hall by zcy end */

/* hct-drv add for customkey F1 by zcy begin */
#ifdef __HCT_CUSTOMKEY_F1_SUPPORT__
#if __HCT_CUSTOMKEY_F1_SUPPORT__
void customkey_handler(bool pressed, int key_value)
{
	printk("customkey_handler report key. \n");
	if(pressed)
	{
		printk("customkey pressed. \n");
		input_report_key(kpd_input_dev, key_value, 1);
	}
	else
	{
		printk("customkey released. \n");
		input_report_key(kpd_input_dev, key_value, 0);
	}
	
	input_sync(kpd_input_dev);
}
#endif
#endif
/* hct-drv add for customkey F1 by zcy end */

static void kpd_memory_setting(void)
{
	kpd_init_keymap(kpd_keymap);
	kpd_init_keymap_state(kpd_keymap_state);
}

static ssize_t kpd_store_call_state(struct device_driver *ddri,
		const char *buf, size_t count)
{
	int ret;

	ret = kstrtoul(buf, 10, &call_status);
	if (ret) {
		kpd_print("kpd call state: Invalid values\n");
		return -EINVAL;
	}

	switch (call_status) {
	case 1:
		kpd_print("kpd call state: Idle state!\n");
		break;
	case 2:
		kpd_print("kpd call state: ringing state!\n");
		break;
	case 3:
		kpd_print("kpd call state: active or hold state!\n");
		break;

	default:
		kpd_print("kpd call state: Invalid values\n");
		break;
	}
	return count;
}

static ssize_t kpd_show_call_state(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%ld\n", call_status);
	return res;
}

#ifdef __HCT_TP_FW_VNO_SHOW_FACTORY_MODE__
int hct_tp_fw_version_num;

#ifdef CONFIG_TOUCHSCREEN_MTK_NT36xxx
extern int32_t get_nt36xx_fw_vno(void);
#endif

#ifdef CONFIG_TOUCHSCREEN_MTK_FOCALTECH_NEW_TS
extern int get_focalltech_fw_vno(void);
#endif

#ifdef CONFIG_TOUCHSCREEN_MTK_GT1X_V14
extern int get_gt1x_fw_vno(void);
#endif

int get_hct_tp_fw_version_num(void){
	int ret = 0;
//ft
#ifdef CONFIG_TOUCHSCREEN_MTK_FOCALTECH_NEW_TS
	if(!get_focalltech_fw_vno()){
		printk("hct-drv: get_focalltech_fw_vno is failed!,no focalltech tp\n");
	}else{
		ret = get_focalltech_fw_vno();
	}
#endif
//gtp
#ifdef CONFIG_TOUCHSCREEN_MTK_GT1X_V14
	if(!get_gt1x_fw_vno()){
		printk("hct-drv: get_gt1x_fw_vno is failed!,no gt1x tp\n");
	}else{
		ret = get_gt1x_fw_vno();
	}
#endif
//nvt
#ifdef CONFIG_TOUCHSCREEN_MTK_NT36xxx
	if(!get_nt36xx_fw_vno()){
		printk("hct-drv: get_nt36xx_fw_vno is failed!,no nt36xx tp\n");
	}else{
		ret = get_nt36xx_fw_vno();
	}
#endif
	printk("hct-drv: [%s] ret = 0x%x\n", __func__, ret);
	return ret;
}

static ssize_t show_hct_tp_fw_version_num(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	hct_tp_fw_version_num = get_hct_tp_fw_version_num();
	res = snprintf(buf, PAGE_SIZE, "V%d\n", hct_tp_fw_version_num);
	return res;
}
#endif

static DRIVER_ATTR(kpd_call_state, 0644, kpd_show_call_state,
		kpd_store_call_state);

#ifdef __HCT_TP_FW_VNO_SHOW_FACTORY_MODE__
static DRIVER_ATTR(hct_tp_fw_version_num, 0644, show_hct_tp_fw_version_num, NULL);
#endif

static struct driver_attribute *kpd_attr_list[] = {
	&driver_attr_kpd_call_state,
#ifdef __HCT_TP_FW_VNO_SHOW_FACTORY_MODE__
	&driver_attr_hct_tp_fw_version_num,
#endif
};

static int kpd_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(kpd_attr_list);

	if (driver == NULL)
		return -EINVAL;

	for (idx = 0; idx < num; idx++) {
		err = driver_create_file(driver, kpd_attr_list[idx]);
		if (err) {
			kpd_info("driver_create_file (%s) = %d\n",
				kpd_attr_list[idx]->attr.name, err);
			break;
		}
	}
	return err;
}

static int kpd_delete_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(kpd_attr_list);

	if (!driver)
		return -EINVAL;

	for (idx = 0; idx < num; idx++)
		driver_remove_file(driver, kpd_attr_list[idx]);

	return err;
}

//hct-drv for 8613 OTG 20180124 begin
#if 1 //defined(CONFIG_USB_XHCI_MTK)
#define MOBILESOURCE_OPEN_KEY 		KEY_PROG1		//KEY_VOLUMEUP
#define MOBILESOURCE_CLOSE_KEY 		KEY_PROG2		//KEY_VOLUMEDOWN
void mobile_source_handler(bool open)
{
	printk("mobile_source_handler = %d\n", open);
	if(open)
	{
		if(!kpd_input_dev)
		{
			return;
		}
            input_report_key(kpd_input_dev, MOBILESOURCE_OPEN_KEY, 1);
            input_report_key(kpd_input_dev, MOBILESOURCE_OPEN_KEY, 0);
            input_sync(kpd_input_dev);
			
			printk("mobile_source_handler2222 = %d\n", MOBILESOURCE_OPEN_KEY);
	}
	else
	{
		if(!kpd_input_dev)
		{
			return;
		}
            input_report_key(kpd_input_dev, MOBILESOURCE_CLOSE_KEY, 1);
            input_report_key(kpd_input_dev, MOBILESOURCE_CLOSE_KEY, 0);
            input_sync(kpd_input_dev);
			printk("mobile_source_handler2222 = %d\n", MOBILESOURCE_OPEN_KEY);
	}
}
#endif
//hct-drv for 8613 OTG 20180124 end
/************************************************************************/
#ifdef CONFIG_LONG_PRESS_MODE_EN
void vol_down_long_press(unsigned long pressed)
{
	atomic_set(&vol_down_long_press_flag, 1);
}
#endif
/*****************************************/

#ifdef CONFIG_KPD_PWRKEY_USE_PMIC
void kpd_pwrkey_pmic_handler(unsigned long pressed)
{
	kpd_print("Power Key generate, pressed=%ld\n", pressed);
	if (!kpd_input_dev) {
		kpd_print("KPD input device not ready\n");
		return;
	}
	kpd_pmic_pwrkey_hal(pressed);
}
#endif

void kpd_pmic_rstkey_handler(unsigned long pressed)
{
	kpd_print("PMIC reset Key generate, pressed=%ld\n", pressed);
	if (!kpd_input_dev) {
		kpd_print("KPD input device not ready\n");
		return;
	}
	kpd_pmic_rstkey_hal(pressed);
}

static void kpd_keymap_handler(unsigned long data)
{
	u16 i, j;
	int32_t pressed;
	u16 new_state[KPD_NUM_MEMS], change, mask;
	u16 hw_keycode, linux_keycode;
	void *dest;

	kpd_get_keymap_state(new_state);
#ifdef CONFIG_PM_WAKELOCKS
	__pm_wakeup_event(&kpd_suspend_lock, 500);
#else
	wake_lock_timeout(&kpd_suspend_lock, HZ / 2);
#endif
	for (i = 0; i < KPD_NUM_MEMS; i++) {
		change = new_state[i] ^ kpd_keymap_state[i];
		if (change == 0U)
			continue;

		for (j = 0; j < 16U; j++) {
			mask = (u16) 1 << j;
			if ((change & mask) == 0U)
				continue;

			hw_keycode = (i << 4) + j;

			if (hw_keycode >= KPD_NUM_KEYS)
				continue;

			/* bit is 1: not pressed, 0: pressed */
			pressed = ((new_state[i] & mask) == 0U) ? 1 : 0;
			kpd_print("(%s) HW keycode = %d\n",
				(pressed == 1) ? "pressed" : "released",
					hw_keycode);

			linux_keycode = kpd_keymap[hw_keycode];
			if (linux_keycode == 0U)
				continue;
			input_report_key(kpd_input_dev, linux_keycode, pressed);
			input_sync(kpd_input_dev);
			kpd_print("report Linux keycode = %d\n", linux_keycode);

#ifdef CONFIG_LONG_PRESS_MODE_EN
			if (pressed) {
				init_timer(&Long_press_key_timer);
				Long_press_key_timer.expires = jiffies + 5*HZ;
				Long_press_key_timer.data =
					(unsigned long)pressed;
				Long_press_key_timer.function =
					vol_down_long_press;
				add_timer(&Long_press_key_timer);
			} else {
				del_timer_sync(&Long_press_key_timer);
			}
			if (!pressed &&
				atomic_read(&vol_down_long_press_flag)) {
				atomic_set(&vol_down_long_press_flag, 0);
			}
#endif
		}
	}

	dest = memcpy(kpd_keymap_state, new_state, sizeof(new_state));
	enable_irq(kp_irqnr);
}

static irqreturn_t kpd_irq_handler(int irq, void *dev_id)
{
	/* use _nosync to avoid deadlock */
	disable_irq_nosync(kp_irqnr);
	tasklet_schedule(&kpd_keymap_tasklet);
	return IRQ_HANDLED;
}

static int kpd_open(struct input_dev *dev)
{
	/* void __user *uarg = (void __user *)arg; */
	return 0;
}

void kpd_get_dts_info(struct device_node *node)
{
	int32_t ret;

	of_property_read_u32(node, "mediatek,kpd-key-debounce",
		&kpd_dts_data.kpd_key_debounce);
	of_property_read_u32(node, "mediatek,kpd-sw-pwrkey",
		&kpd_dts_data.kpd_sw_pwrkey);
	of_property_read_u32(node, "mediatek,kpd-hw-pwrkey",
		&kpd_dts_data.kpd_hw_pwrkey);
	of_property_read_u32(node, "mediatek,kpd-sw-rstkey",
		&kpd_dts_data.kpd_sw_rstkey);
	of_property_read_u32(node, "mediatek,kpd-hw-rstkey",
		&kpd_dts_data.kpd_hw_rstkey);
	of_property_read_u32(node, "mediatek,kpd-use-extend-type",
		&kpd_dts_data.kpd_use_extend_type);
	of_property_read_u32(node, "mediatek,kpd-hw-dl-key1",
		&kpd_dts_data.kpd_hw_dl_key1);
	of_property_read_u32(node, "mediatek,kpd-hw-dl-key2",
		&kpd_dts_data.kpd_hw_dl_key2);
	of_property_read_u32(node, "mediatek,kpd-hw-dl-key3",
		&kpd_dts_data.kpd_hw_dl_key3);
	of_property_read_u32(node, "mediatek,kpd-hw-recovery-key",
		&kpd_dts_data.kpd_hw_recovery_key);
	of_property_read_u32(node, "mediatek,kpd-hw-factory-key",
		&kpd_dts_data.kpd_hw_factory_key);
	of_property_read_u32(node, "mediatek,kpd-hw-map-num",
		&kpd_dts_data.kpd_hw_map_num);
	ret = of_property_read_u32_array(node, "mediatek,kpd-hw-init-map",
		kpd_dts_data.kpd_hw_init_map,
			kpd_dts_data.kpd_hw_map_num);

	if (ret) {
		kpd_print("kpd-hw-init-map was not defined in dts.\n");
		memset(kpd_dts_data.kpd_hw_init_map, 0,
			sizeof(kpd_dts_data.kpd_hw_init_map));
	}

	kpd_print("deb= %d, sw-pwr= %d, hw-pwr= %d, hw-rst= %d, sw-rst= %d\n",
		  kpd_dts_data.kpd_key_debounce, kpd_dts_data.kpd_sw_pwrkey,
			kpd_dts_data.kpd_hw_pwrkey, kpd_dts_data.kpd_hw_rstkey,
				kpd_dts_data.kpd_sw_rstkey);
}

static int32_t kpd_gpio_init(struct device *dev)
{
	struct pinctrl *keypad_pinctrl;
	struct pinctrl_state *kpd_default;
	int32_t ret;

	if (dev == NULL) {
		kpd_print("kpd device is NULL!\n");
		ret = -1;
	} else {
		keypad_pinctrl = devm_pinctrl_get(dev);
		if (IS_ERR(keypad_pinctrl)) {
			ret = -1;
			kpd_print("Cannot find keypad_pinctrl!\n");
		} else {
			kpd_default = pinctrl_lookup_state(keypad_pinctrl,
				"default");
			if (IS_ERR(kpd_default)) {
				ret = -1;
				kpd_print("Cannot find ecall_state!\n");
			} else
				ret = pinctrl_select_state(keypad_pinctrl,
					kpd_default);
		}
	}
	return ret;
}

static int mt_kpd_debugfs(void)
{
#ifdef CONFIG_MTK_ENG_BUILD
	kpd_klog_en = 1;
#else
	kpd_klog_en = 0;
#endif
	kpd_droot = debugfs_create_dir("keypad", NULL);
	if (IS_ERR_OR_NULL(kpd_droot))
		return PTR_ERR(kpd_droot);

	kpd_dklog = debugfs_create_u32("debug", 0600, kpd_droot, &kpd_klog_en);

	return 0;
}

/* hct-drv add for hall by zcy start @20181010*/
#if __HCT_HALL_SUPPORT__
unsigned int hall_irq=0;
unsigned int hallsetdebounce;

static unsigned int hall_state_pin;
static struct pinctrl *hall_pinctrl1;
static struct pinctrl_state *hall_pins_eint_int;
static struct pinctrl_state *pins_default;
//JK add for hall irq error unuseful 20171128
/*static*/ int hct_parse_dts(struct device_node *node, const char *gpio_name, bool gpio_mode)
{
	int gpio_num = 0;
	struct gpio_desc *desc;
	int ret = 0;

	if (node)
	{
		gpio_num = of_get_named_gpio(node, gpio_name, 0);
		if (gpio_num < 0)
		{
			printk("%s: of_get_named_gpio fail. \n", __func__);
			return -1;
		}
		else //获取GPIO成功
		{
			printk("%s: of_get_named_gpio GPIO is %d.\n", __func__, gpio_num);
			desc = gpio_to_desc(gpio_num);
			if (!desc)
			{
				printk("%s: gpio_desc is null.\n", __func__);
				return -1;
			}
			else //获取描述成功
				printk("%s: gpio_desc is not null.\n", __func__);

			if (gpio_is_valid(gpio_num))
				printk("%s: gpio number %d is valid. \n", __func__ ,gpio_num);

			ret = gpio_request(gpio_num, gpio_name);
			if (ret)
			{
				printk("%s: gpio_request fail. \n", __func__);
				return -1;
			}
			else //gpio_request 成功
			{
				if (!gpio_mode) //中断模式
				{
					ret = gpio_direction_input(gpio_num);
					if (ret)
					{
						printk("%s: gpio_direction_input fail. \n", __func__);
						return -1;
					}
				}
				else //GPIO 模式
				{
					ret = gpio_direction_output(gpio_num, 1);
					if (ret)
					{
						printk("%s: gpio_direction_output failed. \n", __func__);
						return -1;
					}

					gpio_set_value(gpio_num, 0);
					printk("%s: gpio_get_value =%d. \n", __func__, gpio_get_value(gpio_num));
				}

				return gpio_num; //返回GPIO num
			}
		}
	}
	else
	{
		printk("%s: get gpio num fail. \n", __func__);
		return -1;
	}
}
//JK add end
static irqreturn_t hall_interrupt_handler(int irq, void *dev_id)
{
        pr_err("hall_interrupt_handler, happened\n");

        disable_irq_nosync(hall_irq);
        hct_hall_state = !hct_hall_state;

        if(hct_hall_state == 0) // this means hall is close
            irq_set_irq_type(hall_irq, IRQF_TRIGGER_HIGH);
        else
            irq_set_irq_type(hall_irq, IRQF_TRIGGER_LOW);

        schedule_work(&(phct_hall_data->eint_work));
        return IRQ_HANDLED;
}


static int hall_irq_registration(void)
{
        int ret = 0;

        if(hct_hall_state == 0) // this means hall is close
            ret = request_irq(hall_irq, hall_interrupt_handler,IRQF_TRIGGER_HIGH, HALL_NAME, NULL);
        else
            ret = request_irq(hall_irq, hall_interrupt_handler,IRQF_TRIGGER_LOW, HALL_NAME, NULL);
        
        if (ret)
            pr_err("hall request_irq IRQ LINE register failed!.");
        else 
        {
            pr_err("[%s] hall request_irq register sucess!.", __func__);
        }
	return ret;
}



static void hct_hall_init_work_func(struct work_struct *work)
{
    printk("[erick-hall]:hall hct_hall_init_work_func enter--\n");
    
    hct_hall_dev.name = "hct-hall";
    hct_hall_dev.index = 0;
#if 0    
#if defined(CONFIG_MTK_LEGACY)
	hct_hall_dev.state = mt_get_gpio_in(hall_state_pin);
#else
	hct_hall_dev.state = __gpio_get_value(hall_state_pin);
#endif
#endif
    hct_hall_dev.state = gpio_get_value(hall_state_pin);

    printk("[erick-hall]: %s, line%d---, hall_state_pin= (%d) pin-state=%d\n", __func__, __LINE__,hall_state_pin, hct_hall_dev.state);

    hct_hall_state = hct_hall_dev.state;
    if(switch_dev_register(&hct_hall_dev))
    {
        	printk("[erick-hall]:hall switch_dev_register fail\n");
        	return;
    }
    hall_irq_registration();
	enable_irq_wake(hall_irq);
    enable_irq(hall_irq);

    printk("[erick-hall]:hall hct_hall_init_work_func exit--\n");

}

extern void tpd_nodify_clam_open(int isclamopen);

#ifdef __HCT_TP_GLOVE_FOR_HALL__
void hct_ctp_enter_glove_mode(int hall)
{
	int ret;
//gt
	hct_hall_flag = hall;
	gt1x_power_reset();
//ft
	ret = hct_fts_enter_glove_mode(!hall);
	printk("[hct-hall]: %s, hct_fts_enter_glove_mode = %d\n", __func__, ret);
	
	return ;
}
#endif

void hct_hall_eint_work_func(struct work_struct *work)
{
	int slid ;

/* slid: eint pin value:
0: means hall is close
1: means hall is open
*/

#if 0
#if defined(CONFIG_MTK_LEGACY)
        slid = mt_get_gpio_in(hall_state_pin);
#else
        slid = __gpio_get_value(hall_state_pin);
#endif
#endif
	slid = gpio_get_value(hall_state_pin);
	#ifdef __HCT_TP_GLOVE_FOR_HALL__
	hct_ctp_enter_glove_mode(slid);
	#endif
	printk("[hct-hall]: %s, ---slid=%d\n", __func__, slid);

	if(slid==0)
	{
	    input_report_key(kpd_input_dev, CLAM_CLOSE_KEY, 1);
	    input_report_key(kpd_input_dev, CLAM_CLOSE_KEY, 0);

          input_sync(kpd_input_dev);
	    switch_set_state((struct switch_dev *)&hct_hall_dev, CLAM_CLOSE);       
	    printk("[hct-hall]: ---close\n");
#if defined(CONFIG_HCT_GTP_HALL_CFG)
		tpd_nodify_clam_open(1);
#endif
	}
	else
	{        
	    input_report_key(kpd_input_dev, CLAM_OPEN_KEY, 1);
	    input_report_key(kpd_input_dev, CLAM_OPEN_KEY, 0);

          input_sync(kpd_input_dev);
	    switch_set_state((struct switch_dev *)&hct_hall_dev, CLAM_OPEN);
	    printk("[hct-hall]: ---open\n");		
#if defined(CONFIG_HCT_GTP_HALL_CFG)
		tpd_nodify_clam_open(0);
#endif
	}
    enable_irq(hall_irq);
}


static int hall_pdrv_probe(struct platform_device *pdev)
{
    struct hct_hall_struct * pdata;
    u32 ints[2] = { 0, 0 };
    u32 ints1[2] = { 0, 0 };
    
    struct device_node *node = NULL;
    int err = -EIO;
    printk("zcy add %s is start @20181010!!!\n",__func__);
    if(!(pdata = kzalloc(sizeof(*pdata), GFP_KERNEL)))
    {
        err = -ENOMEM;
        goto exit;
    }

    phct_hall_data = pdata;

    hall_pinctrl1 = devm_pinctrl_get(&pdev->dev);
    if (IS_ERR(hall_pinctrl1)) {
    	err = PTR_ERR(hall_pinctrl1);
    	dev_err(&pdev->dev, "fwq Cannot find accdet accdet_pinctrl1!\n");
    	return err;
    }
    printk("zcy add %s is 111111 !!!\n",__func__);
    pins_default = pinctrl_lookup_state(hall_pinctrl1, "default");
    if (IS_ERR(pins_default)) {
    	err = PTR_ERR(pins_default);
    	dev_err(&pdev->dev, "fwq Cannot find accdet pinctrl default!\n");
    }

    hall_pins_eint_int = pinctrl_lookup_state(hall_pinctrl1, "kpd_slide_as_int");
    if (IS_ERR(hall_pins_eint_int)) {
    	err = PTR_ERR(hall_pins_eint_int);
    	dev_err(&pdev->dev, "fwq Cannot find accdet pinctrl kpd_slide_as_int!\n");
    	return err;
    }
    pinctrl_select_state(hall_pinctrl1, hall_pins_eint_int);

    node = of_find_compatible_node(NULL, NULL, "mediatek,hct-hall");
    if (node)
    {
        of_property_read_u32_array(node, "debounce", ints, ARRAY_SIZE(ints));
        of_property_read_u32_array(node, "interrupts", ints1, ARRAY_SIZE(ints1));
        hall_irq = ints[0];
        hallsetdebounce = ints[1];
        gpio_set_debounce(hall_irq, hallsetdebounce);
    printk("zcy add %s is 22222 !!!\n",__func__);
#if 0
        if (of_property_read_u32_index(node, "hall_gpio", 1, &hall_state_pin)) {
#endif
		hall_state_pin = hct_parse_dts(node, "hall_gpio", false);

        if (-1 == hall_state_pin){
            pr_err("hall_irq_registration fail~~~\n");
            goto Node_Paser_failed;
        }

        hall_irq = irq_of_parse_and_map(node, 0);
    } 
    else
        goto Node_Paser_failed;

    err =0;
    
    printk("%s, hal_irq=%d, state_pin=%d\n", __FUNCTION__, hall_irq, hall_state_pin);
#if 1
    INIT_WORK(&phct_hall_data->eint_work, hct_hall_eint_work_func);
    INIT_DELAYED_WORK(&phct_hall_data->init_work, hct_hall_init_work_func);
    schedule_delayed_work(&phct_hall_data->init_work, msecs_to_jiffies(500));
#endif
     return err;

Node_Paser_failed:
    kfree(pdata);
exit:
     return err;

}

static int hall_pdrv_remove(struct platform_device *pdev)
{

	return 0;
}

#endif
/* hct-drv add for hall by zcy end */
static int kpd_pdrv_probe(struct platform_device *pdev)
{
	struct clk *kpd_clk = NULL;
	u32 i;
	int32_t err = 0;

	if (!pdev->dev.of_node) {
		kpd_notice("no kpd dev node\n");
		return -ENODEV;
	}

	kpd_clk = devm_clk_get(&pdev->dev, "kpd-clk");
	if (!IS_ERR(kpd_clk)) {
		err = clk_prepare_enable(kpd_clk);
		if (err)
			kpd_notice("get kpd-clk fail: %d\n", err);
	} else {
		kpd_notice("kpd-clk is default set by ccf.\n");
	}

	kp_base = of_iomap(pdev->dev.of_node, 0);
	if (!kp_base) {
		kpd_notice("KP iomap failed\n");
		return -ENODEV;
	};

	kp_irqnr = irq_of_parse_and_map(pdev->dev.of_node, 0);
	if (!kp_irqnr) {
		kpd_notice("KP get irqnr failed\n");
		return -ENODEV;
	}
	kpd_info("kp base: 0x%p, addr:0x%p,  kp irq: %d\n",
			kp_base, &kp_base, kp_irqnr);
	err = kpd_gpio_init(&pdev->dev);
	if (err != 0)
		kpd_print("gpio init failed\n");

	kpd_get_dts_info(pdev->dev.of_node);

	kpd_memory_setting();

	kpd_input_dev = devm_input_allocate_device(&pdev->dev);
	if (!kpd_input_dev) {
		kpd_notice("input allocate device fail.\n");
		return -ENOMEM;
	}

	kpd_input_dev->name = KPD_NAME;
	kpd_input_dev->id.bustype = BUS_HOST;
	kpd_input_dev->id.vendor = 0x2454;
	kpd_input_dev->id.product = 0x6500;
	kpd_input_dev->id.version = 0x0010;
	kpd_input_dev->open = kpd_open;
	kpd_input_dev->dev.parent = &pdev->dev;

	__set_bit(EV_KEY, kpd_input_dev->evbit);
#if defined(CONFIG_KPD_PWRKEY_USE_PMIC)
	__set_bit(kpd_dts_data.kpd_sw_pwrkey, kpd_input_dev->keybit);
	kpd_keymap[8] = 0;
#endif
	if (!kpd_dts_data.kpd_use_extend_type) {
		for (i = 17; i < KPD_NUM_KEYS; i += 9)
			kpd_keymap[i] = 0;
	}
	for (i = 0; i < KPD_NUM_KEYS; i++) {
		if (kpd_keymap[i] != 0)
			__set_bit(kpd_keymap[i], kpd_input_dev->keybit);
	}


//hct-drv for 8613 OTG begin
#if 1 //defined(CONFIG_USB_XHCI_MTK)
              __set_bit(MOBILESOURCE_OPEN_KEY, kpd_input_dev->keybit);
              __set_bit(MOBILESOURCE_CLOSE_KEY, kpd_input_dev->keybit);
#endif
//hct-drv for 8613 OTG end

/* hct-drv add fot hall by zcy start @20181010*/
#if  __HCT_HALL_SUPPORT__
        __set_bit(CLAM_CLOSE_KEY, kpd_input_dev->keybit);
        __set_bit(CLAM_OPEN_KEY, kpd_input_dev->keybit);
        __set_bit(CLAM_USE_KEY, kpd_input_dev->keybit);
#endif
/* hct-drv add fot hall by zcy end */
	if (kpd_dts_data.kpd_sw_rstkey)
		__set_bit(kpd_dts_data.kpd_sw_rstkey, kpd_input_dev->keybit);
#ifdef KPD_KEY_MAP
	__set_bit(KPD_KEY_MAP, kpd_input_dev->keybit);
#endif
#ifdef CONFIG_MTK_MRDUMP_KEY
	__set_bit(KEY_RESTART, kpd_input_dev->keybit);
#endif

	err = input_register_device(kpd_input_dev);
	if (err) {
		kpd_notice("register input device failed (%d)\n", err);
		return err;
	}
#ifdef CONFIG_PM_WAKELOCKS
	wakeup_source_init(&kpd_suspend_lock, "kpd wakelock");
#else
	wake_lock_init(&kpd_suspend_lock, WAKE_LOCK_SUSPEND, "kpd wakelock");
#endif
	/* register IRQ and EINT */
	kpd_set_debounce(kpd_dts_data.kpd_key_debounce);
	err = request_irq(kp_irqnr, kpd_irq_handler, IRQF_TRIGGER_NONE,
			KPD_NAME, NULL);
	if (err) {
		kpd_notice("register IRQ failed (%d)\n", err);
		input_unregister_device(kpd_input_dev);
		return err;
	}
#ifdef CONFIG_MTK_MRDUMP_KEY
	mt_eint_register();
#endif

/* hct-drv add for customkey F1 by zcy begin */
#ifdef __HCT_CUSTOMKEY_F1_SUPPORT__
	#if __HCT_CUSTOMKEY_F1_SUPPORT__
	kpd_print("__set_bit F1 begin.\n");
		hct_customkey_f1_init(pdev);
		__set_bit(KEY_F1, kpd_input_dev->keybit);
	kpd_print("__set_bit F1 end.\n");
	#endif
#endif
/* hct-drv add for customkey F1 by zcy end */

#ifdef CONFIG_MTK_PMIC_NEW_ARCH
	long_press_reboot_function_setting();
#endif
	err = kpd_create_attr(&kpd_pdrv.driver);
	if (err) {
		kpd_notice("create attr file fail\n");
		kpd_delete_attr(&kpd_pdrv.driver);
		return err;
	}
	/* Add kpd debug node */
	mt_kpd_debugfs();

	kpd_info("kpd_probe OK.\n");

	return err;
}

static int kpd_pdrv_suspend(struct platform_device *pdev, pm_message_t state)
{
	kpd_suspend = true;
#ifdef MTK_KP_WAKESOURCE
	if (call_status == 2) {
		kpd_print("kpd_early_suspend wake up source enable!! (%d)\n",
				kpd_suspend);
	} else {
		kpd_wakeup_src_setting(0);
		kpd_print("kpd_early_suspend wake up source disable!! (%d)\n",
				kpd_suspend);
	}
#endif
	kpd_print("suspend!! (%d)\n", kpd_suspend);
	return 0;
}

static int kpd_pdrv_resume(struct platform_device *pdev)
{
	kpd_suspend = false;
#ifdef MTK_KP_WAKESOURCE
	if (call_status == 2) {
		kpd_print("kpd_early_suspend wake up source enable!! (%d)\n",
				kpd_suspend);
	} else {
		kpd_print("kpd_early_suspend wake up source resume!! (%d)\n",
				kpd_suspend);
		kpd_wakeup_src_setting(1);
	}
#endif
	kpd_print("resume!! (%d)\n", kpd_suspend);
	return 0;
}

static const struct of_device_id kpd_of_match[] = {
	{.compatible = "mediatek,kp"},
	{},
};
/* hct-drv add for hall by zcy start @20181010*/
#if __HCT_HALL_SUPPORT__
static const struct of_device_id hall_of_match[] = {
	{.compatible = "mediatek,hct-hall"},
	{},
};

static struct platform_driver hall_pdrv = {
	.probe = hall_pdrv_probe,
	.remove = hall_pdrv_remove,
	.driver = {
		   .name = HALL_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = hall_of_match,
		   },
};
#endif
/* hct-drv add for hall by zcy end*/

static struct platform_driver kpd_pdrv = {
	.probe = kpd_pdrv_probe,
	.suspend = kpd_pdrv_suspend,
	.resume = kpd_pdrv_resume,
	.driver = {
		   .name = KPD_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = kpd_of_match,
		   },
};


module_platform_driver(kpd_pdrv);
#if __HCT_HALL_SUPPORT__
module_platform_driver(hall_pdrv); // hct-drv add for hall by zcy start @20181010
#endif
MODULE_AUTHOR("Mediatek Corporation");
MODULE_DESCRIPTION("MTK Keypad (KPD) Driver");
MODULE_LICENSE("GPL");
