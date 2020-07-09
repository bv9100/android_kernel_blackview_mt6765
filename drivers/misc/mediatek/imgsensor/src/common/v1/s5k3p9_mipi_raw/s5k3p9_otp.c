#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/slab.h>
#include <linux/types.h>


#define PFX "s5k3p9_otp"
#define LOG_INF(fmt, args...)   pr_debug(PFX "[%s] " fmt, __func__, ##args)
#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);


#define USHORT             unsigned short
#define BYTE               unsigned char

#define EEPROM_WRITE_ID         0xA8

#define OTP_DATA                s5k3p9_eeprom_data
#define OTP_PLATFORM_CHOICE     0x09     //bit0:awb, bit1:af, bit3:lsc
#define OTP_FLAG_ADDR           0x0000
#define AWB_ADDR                0x0008
#define AWB_CHECK_ADDR          0x0010
#define LSC_ADDR                0x001e
#define LSC_CHECK_ADDR          0x076a
#define AF_ADDR                 0x0000
#define AF_CHECK_ADDR           0x0000
#define CHECKSUM_METHOD(x,addr)  \
(((x >> 8) & 0xff) == read_cmos_sensor_8(addr) && (x & 0xff) == read_cmos_sensor_8(addr + 1))


BYTE OTP_DATA[2048]= {0};

static kal_uint16 read_cmos_sensor_8(kal_uint16 addr)
{
    kal_uint16 get_byte=0;
    char pusendcmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    iReadRegI2C(pusendcmd , 2, (u8*)&get_byte,1,EEPROM_WRITE_ID);
    return get_byte;
}

void s5k3p9_get_otp_data(void)
{
	int i;
	kal_uint32 checksum = 0;

    if(0x01 != read_cmos_sensor_8(OTP_FLAG_ADDR))
	{
        LOG_INF("OTP DATA Invalid!!!\n");
        return;
    }

//0x010b00ff
	OTP_DATA[0] = 0xff;
	OTP_DATA[1] = 0x00;
	OTP_DATA[2] = 0x0b;
	OTP_DATA[3] = 0x01;

	OTP_DATA[4] = OTP_PLATFORM_CHOICE;

	//AF
    if(OTP_PLATFORM_CHOICE & 0x2)
    {
    	for(i = 0; i < 4; i++)
    	{
    		OTP_DATA[i + 6] = read_cmos_sensor_8(AF_ADDR + i);
    		checksum += OTP_DATA[i + 6];
    	}

    	if(CHECKSUM_METHOD(checksum, AF_CHECK_ADDR))
    	{
    		LOG_INF("AF Checksum OK\n");
    		OTP_DATA[6] ^= OTP_DATA[7];
    		OTP_DATA[7] ^= OTP_DATA[6];
    		OTP_DATA[6] ^= OTP_DATA[7];

    		OTP_DATA[8] = OTP_DATA[11];
    		OTP_DATA[9] = OTP_DATA[10];
            LOG_INF("AFInf = %d, AFMarco = %d\n", OTP_DATA[7] << 8 | OTP_DATA[6], OTP_DATA[9] << 8 | OTP_DATA[8]);
    	}
    	else
    	{
    		LOG_INF("AF Checksum Failed!!!\n");
    		OTP_DATA[4] = OTP_DATA[4] & (~0x02);
    	}
    }


//AWB
    if(OTP_PLATFORM_CHOICE & 0x1)
    {
        checksum = 0;

        for(i = 0; i < 8; i++)
        {
          OTP_DATA[i + 10] = read_cmos_sensor_8(AWB_ADDR + i);
          checksum += OTP_DATA[i + 10];
        }

        if(CHECKSUM_METHOD(checksum, AWB_CHECK_ADDR))
        {
          LOG_INF("AWB OTP Checksum OK\n");
          LOG_INF("Unit_R = 0x%x, Unit_Gr = 0x%x, Unit_Gb = 0x%x, Unit_B = 0x%x\n", OTP_DATA[10], OTP_DATA[11], OTP_DATA[12], OTP_DATA[13]);
          LOG_INF("Golden_R = 0x%x, Golden_Gr = 0x%x, Golden_Gb = 0x%x, Golden_B = 0x%x\n", OTP_DATA[14], OTP_DATA[15], OTP_DATA[16], OTP_DATA[17]);
        }
        else
        {
          LOG_INF("AWB OTP Checksum Failed!!!\n");
          OTP_DATA[4] = OTP_DATA[4] & (~0x01);
        }
    }
	//LSC
    if(OTP_PLATFORM_CHOICE & 0x8)
    {
    	checksum = 0;
    	for(i = 0; i < 1868; i++)
    	{
    		OTP_DATA[20 + i] = read_cmos_sensor_8(LSC_ADDR + i);
    		checksum += OTP_DATA[20 + i];
    	}

        if(CHECKSUM_METHOD(checksum, LSC_CHECK_ADDR))
    		LOG_INF("LSC Checksum OK\n");
    	else
    	{
    		LOG_INF("LSC Checksum Failed!!!\n");
    		OTP_DATA[4] = OTP_DATA[4] & (~0x08);
    	}
    }
}
