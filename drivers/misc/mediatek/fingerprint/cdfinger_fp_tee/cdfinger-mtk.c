#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wakelock.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/spi/spidev.h>
#include <linux/semaphore.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/input.h>
#include <linux/signal.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>

#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/fb.h>
#include <linux/notifier.h>
#include "../fingerprint.h"
#if defined(CONFIG_MTK_CLKMGR)
 /* mt_clkmgr.h will be removed after CCF porting is finished. */
#include <mach/mt_clkmgr.h>
#endif


#ifdef COMPAT_VENDOR
#include "fp_vendor.h"
#endif

extern void hct_waite_for_finger_dts_paser(void);
extern int hct_finger_set_spi_mode(int cmd);
extern int hct_finger_set_eint(int cmd);
extern int hct_finger_set_power(int cmd);
extern int hct_finger_set_reset(int cmd);

static u8 cdfinger_debug = 0x01;
static int isInKeyMode = 0; // key mode
static int screen_status = 1; // screen on
static int sign_sync = 0; // for poll
typedef struct key_report{
	int key;
	int value;
}key_report_t;

#define CDFINGER_DBG(fmt, args...) \
	do{ \
		if(cdfinger_debug & 0x01) \
			printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define CDFINGER_FUNCTION(fmt, args...) \
	do{ \
		if(cdfinger_debug & 0x02) \
			printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define CDFINGER_REG(fmt, args...) \
	do{ \
		if(cdfinger_debug & 0x04) \
			printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define CDFINGER_ERR(fmt, args...) \
    do{ \
		printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
    }while(0)

#define HAS_RESET_PIN

#define VERSION                         "cdfinger version 2.4"
#define DEVICE_NAME                     "fpsdev0"
#define SPI_DRV_NAME                    "cdfinger"

#define CDFINGER_IOCTL_MAGIC_NO          0xFB
#define CDFINGER_INIT                    _IOW(CDFINGER_IOCTL_MAGIC_NO, 0, uint8_t)
#define CDFINGER_GETIMAGE                _IOW(CDFINGER_IOCTL_MAGIC_NO, 1, uint8_t)
#define CDFINGER_INITERRUPT_MODE	     _IOW(CDFINGER_IOCTL_MAGIC_NO, 2, uint8_t)
#define CDFINGER_INITERRUPT_KEYMODE      _IOW(CDFINGER_IOCTL_MAGIC_NO, 3, uint8_t)
#define CDFINGER_INITERRUPT_FINGERUPMODE _IOW(CDFINGER_IOCTL_MAGIC_NO, 4, uint8_t)
#define CDFINGER_RELEASE_WAKELOCK        _IO(CDFINGER_IOCTL_MAGIC_NO, 5)
#define CDFINGER_CHECK_INTERRUPT         _IO(CDFINGER_IOCTL_MAGIC_NO, 6)
#define CDFINGER_SET_SPI_SPEED           _IOW(CDFINGER_IOCTL_MAGIC_NO, 7, uint8_t)
#define CDFINGER_REPORT_KEY_LEGACY              _IOW(CDFINGER_IOCTL_MAGIC_NO, 10, uint8_t)
#define CDFINGER_REPORT_KEY              _IOW(CDFINGER_IOCTL_MAGIC_NO, 19, key_report_t)
#define CDFINGER_POWERDOWN               _IO(CDFINGER_IOCTL_MAGIC_NO, 11)
#define CDFINGER_ENABLE_IRQ               _IO(CDFINGER_IOCTL_MAGIC_NO, 12)
#define CDFINGER_DISABLE_IRQ               _IO(CDFINGER_IOCTL_MAGIC_NO, 13)
#define CDFINGER_HW_RESET               _IOW(CDFINGER_IOCTL_MAGIC_NO, 14, uint8_t)
#define CDFINGER_GET_STATUS               _IO(CDFINGER_IOCTL_MAGIC_NO, 15)
#define CDFINGER_SPI_CLK               _IOW(CDFINGER_IOCTL_MAGIC_NO, 16, uint8_t)
#define CDFINGER_WAKE_LOCK	           _IOW(CDFINGER_IOCTL_MAGIC_NO,26,uint8_t)
#define CDFINGER_ENABLE_CLK				  _IOW(CDFINGER_IOCTL_MAGIC_NO, 30, uint8_t)
#define CDFINGER_POLL_TRIGGER			 _IO(CDFINGER_IOCTL_MAGIC_NO,31)
#define CDFINGER_NEW_KEYMODE		_IOW(CDFINGER_IOCTL_MAGIC_NO, 37, uint8_t)
#define KEY_INTERRUPT                   KEY_F11

enum work_mode {
	CDFINGER_MODE_NONE       = 1<<0,
	CDFINGER_INTERRUPT_MODE  = 1<<1,
	CDFINGER_KEY_MODE        = 1<<2,
	CDFINGER_FINGER_UP_MODE  = 1<<3,
	CDFINGER_READ_IMAGE_MODE = 1<<4,
	CDFINGER_MODE_MAX
};

static struct cdfinger_data {
	struct spi_device *spi;
	struct mutex buf_lock;
	unsigned int irq;
	int irq_enabled;

	u32 vdd_ldo_enable;
	u32 vio_ldo_enable;
	u32 config_spi_pin;

	struct pinctrl *fps_pinctrl;
	struct pinctrl_state *fps_reset_high;
	struct pinctrl_state *fps_reset_low;
	struct pinctrl_state *fps_power_on;
	struct pinctrl_state *fps_power_off;
	struct pinctrl_state *fps_vio_on;
	struct pinctrl_state *fps_vio_off;
	struct pinctrl_state *cdfinger_spi_miso;
	struct pinctrl_state *cdfinger_spi_mosi;
	struct pinctrl_state *cdfinger_spi_sck;
	struct pinctrl_state *cdfinger_spi_cs;
	struct pinctrl_state *cdfinger_irq;

	int thread_wakeup;
	int process_interrupt;
	int key_report;
	enum work_mode device_mode;
	struct timer_list int_timer;
	struct input_dev *cdfinger_inputdev;
	struct wake_lock cdfinger_lock;
	struct task_struct *cdfinger_thread;
	struct fasync_struct *async_queue;
	uint8_t cdfinger_interrupt;
	struct notifier_block notifier;
}*g_cdfinger;

static DECLARE_WAIT_QUEUE_HEAD(waiter);
static DECLARE_WAIT_QUEUE_HEAD(cdfinger_waitqueue);

#if 0
extern void mt_spi_enable_master_clk(struct spi_device *spidev);
extern void mt_spi_disable_master_clk(struct spi_device *spidev);

static void mtk_spi_clk_ctl(struct spi_device *spi, u8 bonoff)
{
	static int count;

	if (bonoff && (count == 0)) {
		mt_spi_enable_master_clk(spi);
		count = 1;
	} else if ((count > 0) && (bonoff == 0)) {
		mt_spi_disable_master_clk(spi);
		count = 0;
	}

}
#endif

// clk will follow platform... pls check this when you poarting
static void enable_clk(void)
{
#if (!defined(CONFIG_MT_SPI_FPGA_ENABLE))
#if defined(CONFIG_MTK_CLKMGR)
        clkmux_sel(MT_CLKMUX_SPI_GFMUX_SEL, MT_CG_UPLL_D12, "spi");
		enable_clock(MT_CG_SPI_SW_CG, "spi");
#endif
#endif
}

static void disable_clk(void)
{
#if (!defined(CONFIG_MT_SPI_FPGA_ENABLE))
#if defined(CONFIG_MTK_CLKMGR)
		disable_clock(MT_CG_SPI_SW_CG, "spi");
        clkmux_sel(MT_CLKMUX_SPI_GFMUX_SEL, MT_CG_SYS_26M, "spi");
#endif
#endif
}

static void cdfinger_disable_irq(struct cdfinger_data *cdfinger)
{
	CDFINGER_ERR("Leosin cdfinger_disable_irq()  In \n");
	if(cdfinger->irq_enabled == 1)
	{
		disable_irq_nosync(cdfinger->irq);
		cdfinger->irq_enabled = 0;
		CDFINGER_DBG("irq disable\n");
	}
}

static void cdfinger_enable_irq(struct cdfinger_data *cdfinger)
{
	CDFINGER_ERR("Leosin cdfinger_enable_irq()  In \n");
	if(cdfinger->irq_enabled == 0)
	{
		enable_irq(cdfinger->irq);
		cdfinger->irq_enabled =1;
		CDFINGER_DBG("irq enable\n");
	}
}
static int cdfinger_getirq_from_platform(struct cdfinger_data *cdfinger)
{
	CDFINGER_ERR("Leosin cdfinger_getirq_from_platform  In \n");
	if(!(cdfinger->spi->dev.of_node)){
		CDFINGER_ERR("of node not exist!\n");
		return -1;
	}

	cdfinger->irq = irq_of_parse_and_map(cdfinger->spi->dev.of_node, 0);
	if(cdfinger->irq < 0)
	{
		CDFINGER_ERR("parse irq failed! irq[%d]\n",cdfinger->irq);
		return -1;
	}
	CDFINGER_DBG("get irq success! irq[%d]\n",cdfinger->irq);
	CDFINGER_DBG("Leosin get irq success! irq[%d]\n",cdfinger->irq);
	//pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_irq);
	hct_finger_set_eint(2);
	return 0;
}

static int cdfinger_parse_dts(struct cdfinger_data *cdfinger)
{
	int ret = -1;

	cdfinger->spi->dev.of_node = of_find_compatible_node(NULL,NULL,"mediatek,hct_finger");
	if(!(cdfinger->spi->dev.of_node)){
		CDFINGER_ERR("of node not exist!\n");
		goto parse_err;
	}
	return 0;

parse_err:
	CDFINGER_ERR("parse dts failed!\n");

	return ret;
}

#ifdef COMPAT_VENDOR
static int spi_send_cmd(struct cdfinger_data *cdfinger,  u8 *tx, u8 *rx, u16 spilen)
{
	struct spi_message m;
	struct spi_transfer t = {
		.tx_buf = tx,
		.rx_buf = rx,
		.len = spilen,
	};

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	return spi_sync(cdfinger->spi, &m);
}
#endif

static void cdfinger_power_on(struct cdfinger_data *cdfinger)
{
	hct_finger_set_spi_mode(1);
	hct_finger_set_power(1);
}


static void cdfinger_reset(int count)
{
	hct_finger_set_reset(0);
	udelay(500*count);
	hct_finger_set_reset(1);
	udelay(500*count);
}


static void cdfinger_release_wakelock(struct cdfinger_data *cdfinger)
{
	CDFINGER_FUNCTION("enter\n");
	wake_unlock(&cdfinger->cdfinger_lock);
	CDFINGER_FUNCTION("exit\n");
}

static int cdfinger_mode_init(struct cdfinger_data *cdfinger, uint8_t arg, enum work_mode mode)
{
	CDFINGER_DBG("mode=0x%x\n", mode);
	cdfinger->process_interrupt = 1;
	cdfinger->device_mode = mode;
	cdfinger->key_report = 0;

	return 0;
}

static void cdfinger_wake_lock(struct cdfinger_data *cdfinger,int arg)
{
	CDFINGER_DBG("cdfinger_wake_lock enter----------\n");
	if(arg)
	{
		wake_lock(&cdfinger->cdfinger_lock);
	}
	else
	{
		wake_unlock(&cdfinger->cdfinger_lock);
		wake_lock_timeout(&cdfinger->cdfinger_lock, msecs_to_jiffies(3000));
	}
}

int cdfinger_report_key(struct cdfinger_data *cdfinger, unsigned long arg)
{
	key_report_t report;
	if (copy_from_user(&report, (key_report_t *)arg, sizeof(key_report_t)))
	{
		CDFINGER_ERR("%s err\n", __func__);
		return -1;
	}
	switch(report.key)
	{
	case KEY_UP:
		report.key=KEY_VOLUMEDOWN;
		break;
	case KEY_DOWN:
		report.key=KEY_VOLUMEUP;
		break;
	case KEY_RIGHT:
		report.key=KEY_PAGEUP;
		break;
	case KEY_LEFT:
		report.key=KEY_PAGEDOWN;
		break;
	default:
		break;
	}

	CDFINGER_FUNCTION("enter\n");
	input_report_key(cdfinger->cdfinger_inputdev, report.key, !!report.value);
	input_sync(cdfinger->cdfinger_inputdev);
	CDFINGER_FUNCTION("exit\n");

	return 0;
}

static int cdfinger_report_key_legacy(struct cdfinger_data *cdfinger, uint8_t arg)
{
	CDFINGER_FUNCTION("enter\n");
	input_report_key(cdfinger->cdfinger_inputdev, KEY_INTERRUPT, !!arg);
	input_sync(cdfinger->cdfinger_inputdev);
	CDFINGER_FUNCTION("exit\n");

	return 0;
}
static unsigned int cdfinger_poll(struct file *filp, struct poll_table_struct *wait)
{
	int mask = 0;
	poll_wait(filp, &cdfinger_waitqueue, wait);
	if (sign_sync == 1)
	{
		mask |= POLLIN|POLLPRI;
	} else if (sign_sync == 2)
	{
		mask |= POLLOUT;
	}
	sign_sync = 0;
	CDFINGER_DBG("mask %u\n",mask);
	return mask;
}
static long cdfinger_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct cdfinger_data *cdfinger = filp->private_data;
	int ret = 0;

	CDFINGER_FUNCTION("enter\n");
	CDFINGER_DBG("Leosin cdfinger_ioctl in...   cmd = %d \n", cmd);
	if(cdfinger == NULL)
	{
		CDFINGER_ERR("%s: fingerprint please open device first!\n", __func__);
		return -EIO;
	}

	mutex_lock(&cdfinger->buf_lock);
	switch (cmd) {
		case CDFINGER_INIT:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_INIT \n");
			break;
		case CDFINGER_GETIMAGE:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_GETIMAGE \n");
			break;
		case CDFINGER_INITERRUPT_MODE:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_INITERRUPT_MODE \n");
			sign_sync = 0;
			isInKeyMode = 1;  // not key mode
			cdfinger_reset(2);
			ret = cdfinger_mode_init(cdfinger,arg,CDFINGER_INTERRUPT_MODE);
			break;
		case CDFINGER_NEW_KEYMODE:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_NEW_KEYMODE \n");
			isInKeyMode = 0;
			ret = cdfinger_mode_init(cdfinger,arg,CDFINGER_INTERRUPT_MODE);
			break;
		case CDFINGER_INITERRUPT_FINGERUPMODE:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_INITERRUPT_FINGERUPMODE \n");
			ret = cdfinger_mode_init(cdfinger,arg,CDFINGER_FINGER_UP_MODE);
			break;
		case CDFINGER_RELEASE_WAKELOCK:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_RELEASE_WAKELOCK \n");
			cdfinger_release_wakelock(cdfinger);
			break;
		case CDFINGER_INITERRUPT_KEYMODE:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_INITERRUPT_KEYMODE \n");
			ret = cdfinger_mode_init(cdfinger,arg,CDFINGER_KEY_MODE);
			break;
		case CDFINGER_CHECK_INTERRUPT:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_CHECK_INTERRUPT \n");
			break;
		case CDFINGER_SET_SPI_SPEED:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_SET_SPI_SPEED \n");
			break;
		case CDFINGER_WAKE_LOCK:
			cdfinger_wake_lock(cdfinger,arg);
			CDFINGER_DBG("Leosin  cmd = CDFINGER_WAKE_LOCK arg=%ld \n",arg);
		case CDFINGER_REPORT_KEY:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_REPORT_KEY \n");
			ret = cdfinger_report_key(cdfinger, arg);
			break;
		case CDFINGER_REPORT_KEY_LEGACY:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_REPORT_KEY_LEGACY \n");
			ret = cdfinger_report_key_legacy(cdfinger, arg);
			break;
		case CDFINGER_POWERDOWN:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_POWERDOWN \n");
			break;
		case CDFINGER_ENABLE_IRQ:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_ENABLE_IRQ \n");
			cdfinger_enable_irq(cdfinger);
			break;
		case CDFINGER_DISABLE_IRQ:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_DISABLE_IRQ \n");
			cdfinger_disable_irq(cdfinger);
			break;
		case CDFINGER_ENABLE_CLK:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_ENABLE_CLK \n");
		case CDFINGER_SPI_CLK:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_SPI_CLK arg=%ld \n",arg);
			if (arg == 1)
				enable_clk();
			else if (arg == 0)
				disable_clk();
			break;
		case CDFINGER_HW_RESET:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_HW_RESET \n");
			cdfinger_reset(arg);
			break;
		case CDFINGER_GET_STATUS:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_GET_STATUS \n");
			ret = screen_status;
			break;
		case CDFINGER_POLL_TRIGGER:
			CDFINGER_DBG("Leosin  cmd = CDFINGER_POLL_TRIGGER \n");
			sign_sync = 2;
			wake_up_interruptible(&cdfinger_waitqueue);
			ret = 0;
			break;
		default:
			CDFINGER_DBG("Leosin  cmd = default \n");
			ret = -ENOTTY;
			break;
	}
	mutex_unlock(&cdfinger->buf_lock);
	CDFINGER_FUNCTION("exit\n");

	return ret;
}

static int cdfinger_open(struct inode *inode, struct file *file)
{
	CDFINGER_FUNCTION("enter\n");
	file->private_data = g_cdfinger;
	CDFINGER_FUNCTION("exit\n");

	return 0;
}

static ssize_t cdfinger_write(struct file *file, const char *buff, size_t count, loff_t * ppos)
{
	return 0;
}

static int cdfinger_async_fasync(int fd, struct file *filp, int mode)
{
	struct cdfinger_data *cdfinger = g_cdfinger;

	CDFINGER_FUNCTION("enter\n");
	return fasync_helper(fd, filp, mode, &cdfinger->async_queue);
}

static ssize_t cdfinger_read(struct file *file, char *buff, size_t count, loff_t * ppos)
{
	return 0;
}

static int cdfinger_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;

	return 0;
}

static const struct file_operations cdfinger_fops = {
	.owner = THIS_MODULE,
	.open = cdfinger_open,
	.write = cdfinger_write,
	.read = cdfinger_read,
	.release = cdfinger_release,
	.fasync = cdfinger_async_fasync,
	.unlocked_ioctl = cdfinger_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = cdfinger_ioctl,
#endif
	.poll = cdfinger_poll,
};

static struct miscdevice cdfinger_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &cdfinger_fops,
};

static void cdfinger_async_Report(void)
{
	struct cdfinger_data *cdfinger = g_cdfinger;

	CDFINGER_FUNCTION("enter\n");
	kill_fasync(&cdfinger->async_queue, SIGIO, POLL_IN);
	CDFINGER_FUNCTION("exit\n");
}

static void int_timer_handle(unsigned long arg)
{
	struct cdfinger_data *cdfinger = g_cdfinger;

	CDFINGER_DBG("enter\n");
	if ((cdfinger->device_mode == CDFINGER_KEY_MODE) && (cdfinger->key_report == 1)) {
		input_report_key(cdfinger->cdfinger_inputdev, KEY_INTERRUPT, 0);
		input_sync(cdfinger->cdfinger_inputdev);
		cdfinger->key_report = 0;
	}

	if (cdfinger->device_mode == CDFINGER_FINGER_UP_MODE){
		cdfinger->process_interrupt = 0;
		cdfinger_async_Report();
	}
	CDFINGER_DBG("exit\n");
}

static int cdfinger_thread_func(void *arg)
{
	struct cdfinger_data *cdfinger = (struct cdfinger_data *)arg;

	do {
		wait_event_interruptible(waiter, cdfinger->thread_wakeup != 0);
		CDFINGER_DBG("cdfinger:%s,thread wakeup\n",__func__);
		cdfinger->thread_wakeup = 0;
		wake_lock_timeout(&cdfinger->cdfinger_lock, 3*HZ);

		if (cdfinger->device_mode == CDFINGER_INTERRUPT_MODE) {
			cdfinger->process_interrupt = 0;
			sign_sync = 1;
			wake_up_interruptible(&cdfinger_waitqueue);
			cdfinger_async_Report();
			del_timer_sync(&cdfinger->int_timer);
			continue;
		} else if ((cdfinger->device_mode == CDFINGER_KEY_MODE) && (cdfinger->key_report == 0)) {
			input_report_key(cdfinger->cdfinger_inputdev, KEY_INTERRUPT, 1);
			input_sync(cdfinger->cdfinger_inputdev);
			cdfinger->key_report = 1;
		}

	}while(!kthread_should_stop());

	CDFINGER_ERR("thread exit\n");
	return -1;
}

static irqreturn_t cdfinger_interrupt_handler(unsigned irq, void *arg)
{
	struct cdfinger_data *cdfinger = (struct cdfinger_data *)arg;
        CDFINGER_ERR("Leosin cdfinger_interrupt_handler() In! irq = %d \n",irq);
	cdfinger->cdfinger_interrupt = 1;
	if (cdfinger->process_interrupt == 1)
	{
		CDFINGER_ERR("Leosin cdfinger->process_interrupt == 1 \n");
		mod_timer(&cdfinger->int_timer, jiffies + HZ / 10);
		cdfinger->thread_wakeup = 1;
		wake_up_interruptible(&waiter);
	}

	return IRQ_HANDLED;
}

static int cdfinger_create_inputdev(struct cdfinger_data *cdfinger)
{
	cdfinger->cdfinger_inputdev = input_allocate_device();
	if (!cdfinger->cdfinger_inputdev) {
		CDFINGER_ERR("cdfinger->cdfinger_inputdev create faile!\n");
		return -ENOMEM;
	}
	__set_bit(EV_KEY, cdfinger->cdfinger_inputdev->evbit);
	__set_bit(KEY_INTERRUPT, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_F1, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_F2, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_F3, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_F4, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_VOLUMEUP, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_VOLUMEDOWN, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_PAGEUP, cdfinger->cdfinger_inputdev->keybit);
    __set_bit(KEY_PAGEDOWN, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_UP, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_LEFT, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_RIGHT, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_DOWN, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_ENTER, cdfinger->cdfinger_inputdev->keybit);

	cdfinger->cdfinger_inputdev->id.bustype = BUS_HOST;
	cdfinger->cdfinger_inputdev->name = "cdfinger_inputdev";
	if (input_register_device(cdfinger->cdfinger_inputdev)) {
		CDFINGER_ERR("register inputdev failed\n");
		input_free_device(cdfinger->cdfinger_inputdev);
		return -1;
	}

	return 0;
}

static int cdfinger_fb_notifier_callback(struct notifier_block* self,
                                        unsigned long event, void* data)
{
    struct fb_event* evdata = data;
    unsigned int blank;
    int retval = 0;
	
    if (event != FB_EVENT_BLANK /* FB_EARLY_EVENT_BLANK */) {
        return 0;
    }
    blank = *(int*)evdata->data;
    switch (blank) {
        case FB_BLANK_UNBLANK:
			CDFINGER_DBG("sunlin==FB_BLANK_UNBLANK==\n");
			mutex_lock(&g_cdfinger->buf_lock);
			screen_status = 1;
			if (isInKeyMode == 0) {
				sign_sync = 1;
				wake_up_interruptible(&cdfinger_waitqueue);
				cdfinger_async_Report();
			}
			mutex_unlock(&g_cdfinger->buf_lock);
            break;

        case FB_BLANK_POWERDOWN:
			CDFINGER_DBG("sunlin==FB_BLANK_POWERDOWN==\n");
			mutex_lock(&g_cdfinger->buf_lock);
			screen_status = 0;
			if (isInKeyMode == 0) {
				sign_sync = 1;
				wake_up_interruptible(&cdfinger_waitqueue);
				cdfinger_async_Report();
			}
			mutex_unlock(&g_cdfinger->buf_lock);
            break;
        default:
            break;
    }

    return retval;
}

extern int hct_finger_probe_isok;//add for hct finger jianrong
static int cdfinger_probe(struct spi_device *spi)
{
	struct cdfinger_data *cdfinger = NULL;
	int status = -ENODEV;
#ifdef	COMPAT_VENDOR
	uint8_t chipid[4] = {0x74, 0x66, 0x66, 0x66};
#endif
	CDFINGER_DBG("enter\n");
	
	if(hct_finger_probe_isok)
    {

    	return -1;
    }

	
	
	hct_waite_for_finger_dts_paser();
	cdfinger = kzalloc(sizeof(struct cdfinger_data), GFP_KERNEL);
	if (!cdfinger) {
		CDFINGER_ERR("alloc cdfinger failed!\n");
		return -ENOMEM;;
	}

	g_cdfinger = cdfinger;
	spi_set_drvdata(spi, cdfinger);
	cdfinger->spi = spi;
	if(cdfinger_parse_dts(cdfinger))
	{
			CDFINGER_ERR("%s: parse dts failed!\n", __func__);
			goto free_cdfinger;
	}

	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;

	if(spi_setup(spi) != 0)
	{
		CDFINGER_ERR("%s: spi setup failed!\n", __func__);
		goto free_cdfinger;
	}
	cdfinger_power_on(cdfinger);
#ifdef HAS_RESET_PIN
	cdfinger_reset(50);
#endif
	enable_clk();   //if doujia  we can open this to debug
#ifdef COMPAT_VENDOR
    if(get_fp_spi_enable() == 1 ){
		status = spi_send_cmd(cdfinger, chipid, chipid, 4);
		if (status == 0) {
			if (chipid[3] == 0x80 || chipid[3] == 0x98 || chipid[3] == 0x56){
				CDFINGER_DBG("get id success(%x)\n",chipid[3]);		
				set_fp_vendor(FP_VENDOR_CDFINGER);
			} else {
				CDFINGER_DBG("get id failed(%x)\n",chipid[3]);
				status = -1;			
				goto free_cdfinger;
			}
		} else {
			CDFINGER_ERR("spi invoke err()\n");
			status = -1;
			goto free_cdfinger;	
		}
    } else {
		printk("ERROR--spi cann't called\n");
		CDFINGER_ERR("spi invoke err()\n");
        status = -1;
        goto free_cdfinger;
     }
#endif
	mutex_init(&cdfinger->buf_lock);
	wake_lock_init(&cdfinger->cdfinger_lock, WAKE_LOCK_SUSPEND, "cdfinger wakelock");

	status = misc_register(&cdfinger_dev);
	if (status < 0) {
		CDFINGER_ERR("%s: cdev register failed!\n", __func__);
		goto free_lock;
	}

	if(cdfinger_create_inputdev(cdfinger) < 0)
	{
		CDFINGER_ERR("%s: inputdev register failed!\n", __func__);
		goto free_device;
	}

	init_timer(&cdfinger->int_timer);
	cdfinger->int_timer.function = int_timer_handle;
	add_timer(&cdfinger->int_timer);
	if(cdfinger_getirq_from_platform(cdfinger)!=0)
		goto free_work;
	status = request_threaded_irq(cdfinger->irq, (irq_handler_t)cdfinger_interrupt_handler, NULL,
					IRQF_TRIGGER_RISING | IRQF_ONESHOT, "cdfinger-irq", cdfinger);
	if(status){
		CDFINGER_ERR("request_irq error\n");
		goto free_work;
	}

	enable_irq_wake(cdfinger->irq);
	cdfinger->irq_enabled = 1;
	
	cdfinger->cdfinger_thread = kthread_run(cdfinger_thread_func, cdfinger, "cdfinger_thread");
	if (IS_ERR(cdfinger->cdfinger_thread)) {
		CDFINGER_ERR("kthread_run is failed\n");
		goto free_irq;
	}
	cdfinger->notifier.notifier_call = cdfinger_fb_notifier_callback;
    fb_register_client(&cdfinger->notifier);


    hct_finger_probe_isok = 1;

	CDFINGER_DBG("exit\n");

	return 0;

free_irq:
	free_irq(cdfinger->irq, cdfinger);
free_work:
	del_timer(&cdfinger->int_timer);
	input_unregister_device(cdfinger->cdfinger_inputdev);
	cdfinger->cdfinger_inputdev = NULL;
	input_free_device(cdfinger->cdfinger_inputdev);
free_device:
	misc_deregister(&cdfinger_dev);
free_lock:
	wake_lock_destroy(&cdfinger->cdfinger_lock);
	mutex_destroy(&cdfinger->buf_lock);
free_cdfinger:
	kfree(cdfinger);
	cdfinger = NULL;

	return -1;
}



static int cdfinger_suspend (struct device *dev)
{
	return 0;
}

static int cdfinger_resume (struct device *dev)
{
	return 0;
}
static const struct dev_pm_ops cdfinger_pm = {
	.suspend = cdfinger_suspend,
	.resume = cdfinger_resume
};
struct of_device_id cdfinger_of_match[] = {
	{ .compatible = "mediatek,hct_finger", },             
	{ .compatible = "cdfinger,fps998e", },
	{ .compatible = "cdfinger,fps1098", },
	{ .compatible = "cdfinger,fps998", },
	{ .compatible = "cdfinger,fps980", },
	{ .compatible = "cdfinger,fps956", },
	{},
};
MODULE_DEVICE_TABLE(of, cdfinger_of_match);

static const struct spi_device_id cdfinger_id[] = {
	{SPI_DRV_NAME, 0},
	{}
};
MODULE_DEVICE_TABLE(spi, cdfinger_id);

static struct spi_driver cdfinger_driver = {
	.driver = {
		.name = SPI_DRV_NAME,
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
		.pm = &cdfinger_pm,
		.of_match_table = of_match_ptr(cdfinger_of_match),
	},
	.id_table = cdfinger_id,
	.probe = cdfinger_probe,
	.remove = NULL,
};

#ifndef DTS_PROBE 
static struct spi_board_info spi_board_cdfinger[] __initdata = {
	[0] = {
		.modalias = "cdfinger",
		.bus_num = 0,
		.chip_select = FINGERPRINT_CDFINGER_CS,
		.mode = SPI_MODE_0,
		.max_speed_hz = 6000000,
	},
};
#endif

static int __init cdfinger_spi_init(void)
{

#ifndef DTS_PROBE 
	spi_register_board_info(spi_board_cdfinger, ARRAY_SIZE(spi_board_cdfinger));
#endif

	return spi_register_driver(&cdfinger_driver);
}

static void __exit cdfinger_spi_exit(void)
{
	spi_unregister_driver(&cdfinger_driver);
}

late_initcall_sync(cdfinger_spi_init);
module_exit(cdfinger_spi_exit);

MODULE_DESCRIPTION("cdfinger tee Driver");
MODULE_AUTHOR("shuaitao@cdfinger.com");
MODULE_LICENSE("GPL");
MODULE_ALIAS("cdfinger");
