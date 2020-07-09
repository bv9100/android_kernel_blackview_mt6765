
#include <linux/ioctl.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/atomic.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>

#define	GSENSORHUB_DIRECTION_NAME	"mtk-gsensor-direction"
static unsigned int gsensor_gpio_state_pin;
int gsensor_gpio_status = -1;

int gsensor_direction_for_gpio(void)
{
	gsensor_gpio_status = gpio_get_value(gsensor_gpio_state_pin);
	pr_err("[%s]gpio_gsensor:status=%d,gsensor_gpio_state_pin=%d~~~\n",__func__,gsensor_gpio_status,gsensor_gpio_state_pin);
	return gsensor_gpio_status;
}

static int hct_gpio_parse_dts(struct device_node *node, const char *gpio_name, bool gpio_mode)
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


static int gsensorhub_direction_pdrv_probe(struct platform_device *pdev)
{
	struct device_node *node = NULL;
	printk("[%s] probe begin\n",__func__);
	node = of_find_compatible_node(NULL, NULL, "mediatek,gpio_select_gsensor");
	if (node)
	{
		pr_err("[%s]gpio_gsensor node ok~~~\n",__func__);
		gsensor_gpio_state_pin = hct_gpio_parse_dts(node, "gsensor_select_gpio", false);
		if (-1 == gsensor_gpio_state_pin){
			pr_err("[%s]gpio_gsensor~~~\n",__func__);
		}
	}
	printk("[%s] probe end\n",__func__);
     return 0;

}

static int gsensorhub_direction_pdrv_remove(struct platform_device *pdev)
{

	return 0;
}


static const struct of_device_id gsensorhub_direction_of_match[] = {
	{.compatible = "mediatek,gpio_select_gsensor"},
	{},
};

static struct platform_driver gsensorhub_direction_pdrv = {
	.probe = gsensorhub_direction_pdrv_probe,
	.remove = gsensorhub_direction_pdrv_remove,
	.driver = {
		   .name = GSENSORHUB_DIRECTION_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = gsensorhub_direction_of_match,
		   },
};

//module_platform_driver(gsensorhub_direction_pdrv); 

static int __init gsensorhub_direction_init(void)
{
	printk("gsensorhub_direction driver init\n");

	if (platform_driver_register(&gsensorhub_direction_pdrv) != 0)
		printk("unable to register gsensorhub_direction driver.\n");
	return 0;
}
/* should never be called */
static void __exit gsensorhub_direction_exit(void)
{
	printk("gsensorhub_direction driver exit\n");

	platform_driver_unregister(&gsensorhub_direction_pdrv);
}

fs_initcall(gsensorhub_direction_init);
module_exit(gsensorhub_direction_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ACCELHUB gse driver");
MODULE_AUTHOR("hongxu.zhao@mediatek.com");
