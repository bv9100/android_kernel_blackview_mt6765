#include <linux/hct_include/hct_project_all_config.h>
#include "s5k3p9sxsub_otp_cam_cal.h"
//read from imgsensor.c

extern u8 s5k3p9sxsub_eeprom_data[2048];
unsigned int  s5k3p9sxsub_selective_read_region(struct i2c_client *client, unsigned int addr,
	unsigned char *data, unsigned int size)
{
    memcpy((void *)data,(void *)&s5k3p9sxsub_eeprom_data[addr],size);
	printk("s5k3p9sxsub_selective_read_region addr:%d,size %d data read = 0x%x\n",addr,size, *data);
    return 0;
}


