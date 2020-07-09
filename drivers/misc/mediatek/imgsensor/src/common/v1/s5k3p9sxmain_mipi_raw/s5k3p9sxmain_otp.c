#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/slab.h>
#include "kd_camera_typedef.h"


#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"


#define PFX "s5k3p9sxmain_otp"
#define LOG_INF(fmt, args...)   pr_debug(PFX "[%s] " fmt, __func__, ##args)

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);


#define USHORT             unsigned short
#define BYTE               unsigned char

#define EEPROM_WRITE_ID         0xA2
#define IMGSENSOR_WRITE_ID      0x20

#define OTP_DATA                s5k3p9sxmain_eeprom_data
#define OTP_PLATFORM_CHOICE     0x0b     //bit0:awb, bit1:af, bit3:lsc
#define OTP_FLAG_ADDR           0x0000
#define AWB_ADDR                0x0006
#define AWB_CHECK_ADDR          0x0012
#define LSC_ADDR                0x0013
#define LSC_CHECK_ADDR          0x075F
#define AF_ADDR                 0x000E
#define AF_CHECK_ADDR           0x0012
#define CHECKSUM_METHOD(x,addr)  \
( (x % 255 + 1) == read_cmos_sensor_8(addr) )

#define GOLDEN_AWB_R			0x4b
#define GOLDEN_AWB_GR			0xa0
#define GOLDEN_AWB_GB			0x9f
#define GOLDEN_AWB_B			0x61

BYTE OTP_DATA[2048]= {0};

static char awbflag = 0;

static kal_uint16 read_cmos_sensor_8(kal_uint16 addr)
{
    kal_uint16 get_byte=0;
    char pusendcmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    iReadRegI2C(pusendcmd , 2, (u8*)&get_byte,1,EEPROM_WRITE_ID);
    return get_byte;
}

static void write_cmos_sensor_8(kal_uint16 addr, kal_uint8 para)
{
    char pusendcmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};
    iWriteRegI2C(pusendcmd , 3, IMGSENSOR_WRITE_ID);
}

void s5k3p9sxmain_get_otp_data(void)
{
	int i;
	kal_uint32 checksum = 0;

    if(0x01 != read_cmos_sensor_8(OTP_FLAG_ADDR))
	{
        LOG_INF("OTP DATA Invalid!!! %d\n",read_cmos_sensor_8(OTP_FLAG_ADDR) );
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
    		OTP_DATA[i + 6] = read_cmos_sensor_8(AF_ADDR + i);

	for(i = 1; i < 18; i++){
		checksum += read_cmos_sensor_8(OTP_FLAG_ADDR + i);

	}
	

    	if(CHECKSUM_METHOD(checksum, AF_CHECK_ADDR))
    	{
    		LOG_INF("AF Checksum OK\n");
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
	}

	for(i = 1; i < 18; i++){
		checksum += read_cmos_sensor_8(OTP_FLAG_ADDR + i);
	}

        if(CHECKSUM_METHOD(checksum, AWB_CHECK_ADDR))
        {
			LOG_INF("AWB OTP Checksum OK\n");
			LOG_INF("Unit_R = 0x%x, Unit_Gr = 0x%x, Unit_Gb = 0x%x, Unit_B = 0x%x\n", OTP_DATA[10], OTP_DATA[11], OTP_DATA[12], OTP_DATA[13]);
			LOG_INF("Golden_R = 0x%x, Golden_Gr = 0x%x, Golden_Gb = 0x%x, Golden_B = 0x%x\n", OTP_DATA[14], OTP_DATA[15], OTP_DATA[16], OTP_DATA[17]);

			OTP_DATA[14] = GOLDEN_AWB_R;
			OTP_DATA[15] = GOLDEN_AWB_GR;
			OTP_DATA[16] = GOLDEN_AWB_GB;
			OTP_DATA[17] = GOLDEN_AWB_B;
        }
        else
        {
			LOG_INF("AWB OTP Checksum Failed!!!\n");
			OTP_DATA[4] = OTP_DATA[4] & (~0x01);
        }
    }
	else
	{
		checksum = 0;
		for(i = 0; i < 6; i++)
		{
        	OTP_DATA[i + 10] = read_cmos_sensor_8(AWB_ADDR + i);
			LOG_INF("[SensorOTP]AWB data[%d] = 0x%x\n", i, OTP_DATA[i + 10]);
		}

		for(i = 0; i < 28; i++)
			checksum += read_cmos_sensor_8(AWB_ADDR + i);

		LOG_INF("AWB checksum = %d, read Checksum = %d\n", checksum, read_cmos_sensor_8(0x39));

		if(CHECKSUM_METHOD(checksum, AWB_CHECK_ADDR))
		{
			awbflag = 1;
			LOG_INF("AWB OTP Checksum OK\n");
			LOG_INF("R/G = 0x%x, B/G = 0x%x, GB/GR = 0x%x\n", OTP_DATA[10] | OTP_DATA[11] << 8, OTP_DATA[12] | OTP_DATA[13] << 8, OTP_DATA[14] | OTP_DATA[15] << 8);
		}
		else
			LOG_INF("AWB OTP Checksum Failed!!!\n");
	}

	//LSC
    if(OTP_PLATFORM_CHOICE & 0x8)
    {
    	checksum = 0;
    	for(i = 0; i < 1868; i++)
    	{
    		OTP_DATA[20 + i] = read_cmos_sensor_8(LSC_ADDR + i);
    		checksum += OTP_DATA[20 + i];
			//LOG_INF("LSC data[%d] = 0x%x\n", i, OTP_DATA[20 + i]);
    	}
	LOG_INF("AWB OTP checksum = %d, OTP_DATA[075f] = %d\n",checksum, read_cmos_sensor_8(LSC_CHECK_ADDR));
        if(CHECKSUM_METHOD(checksum, LSC_CHECK_ADDR))
    		LOG_INF("LSC Checksum OK\n");
    	else
    	{
    		LOG_INF("LSC Checksum Failed!!!  LSC Check Data = 0x%x\n", read_cmos_sensor_8(0x0787));
    		OTP_DATA[4] = OTP_DATA[4] & (~0x08);
    	}
    }
}

void s5k3p9sxmain_awb_load(void)
{
	if(awbflag)
	{
		kal_uint32 Unit_RG, Unit_BG, Unit_GG, Golden_RG, Golden_BG, Golden_GG;
		kal_uint32 R_Gain, B_Gain, G_Gain, G_Gain_R, G_Gain_B;

		Unit_RG   = (OTP_DATA[10] | OTP_DATA[11] << 8);
		Unit_BG   = (OTP_DATA[12] | OTP_DATA[13] << 8);
		Unit_GG   = (OTP_DATA[14] | OTP_DATA[15] << 8);
		Golden_RG = Unit_RG;	//wati Golden data
		Golden_BG = Unit_BG;
		Golden_GG = Unit_GG;
		LOG_INF("Unit_RG = 0x%x, Unit_BG = 0x%x, Unit_GG = 0x%x\n", Unit_RG, Unit_BG, Unit_GG);
		LOG_INF("Golden_RG = 0x%x, Golden_BG = 0x%x, Golden_GG = 0x%x\n", Golden_RG, Golden_BG, Golden_GG);
		if(Unit_RG < Golden_RG)
		{
			if(Unit_BG < Golden_BG)
			{
				R_Gain = 0x100 * Golden_RG / Unit_RG;
				B_Gain = 0x100 * Golden_BG / Unit_BG;
				G_Gain = 0x100;
			}
			else
			{
				R_Gain = 0x100 * (Golden_RG * Unit_BG) / (Golden_BG * Unit_RG);
				G_Gain = 0x100 * Unit_BG / Golden_BG;
				B_Gain = 0x100;
			}
		}
		else
		{
			if(Unit_BG < Golden_BG)
			{
				R_Gain = 0x100;
				G_Gain = 0x100 * Unit_RG / Golden_RG;
				B_Gain = 0x100 * (Golden_BG * Unit_RG) / (Golden_RG * Unit_BG);
			}
			else
			{
				G_Gain_R = 0x100 * Unit_RG / Golden_RG;
				G_Gain_B = 0x100 * Unit_BG / Golden_BG;
				if(G_Gain_R > G_Gain_B)
				{
					R_Gain = 0x100;
					G_Gain = 0x100 * Unit_RG / Golden_RG;
					B_Gain = 0x100 * (Golden_BG * Unit_RG) / (Golden_RG * Unit_BG);
				}
				else
				{
					R_Gain = 0x100 * (Golden_RG * Unit_BG) / (Golden_BG * Unit_RG);
					G_Gain = 0x100 * Unit_BG / Golden_BG;
					B_Gain = 0x100;
				}
			}
		}
		LOG_INF("R_Gain=0x%x, B_Gain=0x%x, G_Gain=0x%x\n", R_Gain, B_Gain, G_Gain);
		if(R_Gain>0x0100)
		{
			write_cmos_sensor_8(0x0210, (R_Gain & 0xff00)>>8);
			write_cmos_sensor_8(0x0211, (R_Gain & 0x00ff)); //R
		}
		if(G_Gain>0x0100)
		{
			write_cmos_sensor_8(0x020e, (G_Gain & 0xff00)>>8);
			write_cmos_sensor_8(0x020f, (G_Gain & 0x00ff));//GR
			write_cmos_sensor_8(0x0214, (G_Gain & 0xff00)>>8);
			write_cmos_sensor_8(0x0215, (G_Gain & 0x00ff));//GB
		}
		if(B_Gain>0x0100)
		{
			write_cmos_sensor_8(0x0212, (B_Gain & 0xff00)>>8);
			write_cmos_sensor_8(0x0213, (B_Gain & 0x00ff)); //B
		}
		write_cmos_sensor_8(0x020D, 0x01);
	}
	else
	{
		if((OTP_PLATFORM_CHOICE & 0x1) != 1)
			LOG_INF("AWB OTP Checksum Failed!!!  Don't load AWB OTP\n");
	}

}

bool read_3P9_eeprom( kal_uint16 addr, BYTE* data, kal_uint32 size)
{
	kal_uint32 checksum = 0;
	int i;

	for(i = 0; i < 496; i++)
	{
		data[i] = read_cmos_sensor_8(0x760 + i);
		checksum += data[i];
	}

    if(CHECKSUM_METHOD(checksum, 0x0950))
		LOG_INF("PDAF1 Checksum OK\n");
	else
		LOG_INF("PDAF1 Checksum Failed!!!\n");

	checksum = 0;
	for(i = 0; i < 908; i++)
	{
		data[i + 496] = read_cmos_sensor_8(0x951 + i);
		checksum += data[i + 496];
	}

    if(CHECKSUM_METHOD(checksum, 0x0CDD))
		LOG_INF("PDAF2 Checksum OK\n");
	else
		LOG_INF("PDAF2 Checksum Failed!!!\n");

  return true;
}
