#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/slab.h>
#include <linux/types.h>
#include "kd_camera_typedef.h"


#define PFX "S5K3P3_pdafotp"
#define LOG_INF(format, args...)	pr_debug(PFX "[%s] " format, __FUNCTION__, ##args)

//#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
//extern void kdSetI2CSpeed(u16 i2cSpeed);
//extern int iBurstWriteReg_multi(u8 *pData, u32 bytes, u16 i2cId, u16 transfer_length);
extern int iMultiReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId, u8 number);


#define USHORT             unsigned short
#define BYTE               unsigned char
#define Sleep(ms) mdelay(ms)

#define S5K3P3_EEPROM_READ_ID  0xA1
#define S5K3P3_EEPROM_WRITE_ID   0xB0	// 0xA0
#define S5K3P3_I2C_SPEED        100
#define S5K3P3_MAX_OFFSET		0xFFFF

#define DATA_SIZE 2048
static BYTE s5k3P3_eeprom_data[DATA_SIZE]= {0};
static bool get_done = false;
static int last_size = 0;
static int last_offset = 0;


static bool selective_read_eeprom(kal_uint16 addr, BYTE* data)
{
	char pu_send_cmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    if(addr > S5K3P3_MAX_OFFSET)
        return false;
//	kdSetI2CSpeed(S5K3P3_I2C_SPEED);

	if(iReadRegI2C(pu_send_cmd, 2, (u8*)data, 1, S5K3P3_EEPROM_WRITE_ID)<0)
		return false;
    return true;
}

static bool _read_3P3_eeprom(kal_uint16 addr, BYTE* data, kal_uint32 size ){
	int i = 0;
	int offset = addr;
	for(i = 0; i < size; i++) {
		if(!selective_read_eeprom(offset, &data[i])){
			return false;
		}
		//LOG_INF("read_eeprom 0x%0x 0x%0x\n",offset, data[i]);
		offset++;
	}
	get_done = true;
	last_size = size;
	last_offset = addr;
    return true;
}

bool read_3P3_eeprom( kal_uint16 addr, BYTE* data, kal_uint32 size){
	addr = 0x791;
	size = 496;
	LOG_INF("read 3P3 eeprom, size1 = %d\n", size);
	if(!get_done || last_size != size || last_offset != addr) {
		if(!_read_3P3_eeprom(addr, s5k3P3_eeprom_data, size)){
			get_done = 0;
            last_size = 0;
            last_offset = 0;
			return false;
		}
	}
	addr = 0x983;
	size = 908;
	LOG_INF("read 3P3 eeprom, size2 = %d\n", size);
	if(!get_done || last_size != size || last_offset != addr) {
		if(!_read_3P3_eeprom(addr, s5k3P3_eeprom_data+496, size)){
			get_done = 0;
            last_size = 0;
            last_offset = 0;
			return false;
		}
	}

	memcpy(data, s5k3P3_eeprom_data, 1404);
    return true;
}

int read_3P3_eeprom_lenid_byid(void)
{
	int ret=0;
	int addr = 0x0005;
	int size = 1;
	LOG_INF("read 3P3 eeprom lenid, addr=0x%x, size=%d\n", addr, size);
	if(!get_done || last_size != size || last_offset != addr) {
		if(!_read_3P3_eeprom(addr, s5k3P3_eeprom_data, size)){
			get_done = 0;
            		last_size = 0;
            		last_offset = 0;
			return false;
		}
	}
	ret = s5k3P3_eeprom_data[0];
	LOG_INF("read 3P3 eeprom lenid=0x%x\n", ret);
   	return ret;
}
u8 S5K3P3SX_lsc_data[1868+21]={0};

 int s5k3p3_otp_read(void)
{
	kal_uint16 addr;
	kal_uint16 size;
	u8 data_temp=0;
	unsigned long  checksum_lsc = 0;

//0x010b00ff
	S5K3P3SX_lsc_data[0] = 0xff;  //zy  layout check
	S5K3P3SX_lsc_data[1] = 0x00;  //zy layout check
	S5K3P3SX_lsc_data[2] = 0x0b;  //zy layout check
	S5K3P3SX_lsc_data[3] = 0x1;  //zy layout check

	S5K3P3SX_lsc_data[4] = 0xa;  //bit1:awb bit1:af bit4:lsc zy  choose mode
	S5K3P3SX_lsc_data[5] = 0;  //reserved


	addr = 0x078a;  //af 
	size = 4;
	LOG_INF("read 3P3 eeprom, size1 = %d\n", size);
	if(!get_done || last_size != size || last_offset != addr) {
		if(!_read_3P3_eeprom(addr, S5K3P3SX_lsc_data+6, size)){
			get_done = 0;
            last_size = 0;
            last_offset = 0;
			return false;
		}
	}
	data_temp=S5K3P3SX_lsc_data[6];
	S5K3P3SX_lsc_data[6] = S5K3P3SX_lsc_data[7];//Infinity_L;
	S5K3P3SX_lsc_data[7] = data_temp;//Infinity_H;
	data_temp=S5K3P3SX_lsc_data[8];
	S5K3P3SX_lsc_data[8] = S5K3P3SX_lsc_data[9];//Near_L;
	S5K3P3SX_lsc_data[9] = data_temp;//Near_H;

	addr = 0x001d;  //awb
	size = 8;
	LOG_INF("read 3P3 eeprom, size1 = %d\n", size);
	if(!get_done || last_size != size || last_offset != addr) {
		if(!_read_3P3_eeprom(addr, S5K3P3SX_lsc_data+10, size)){
			get_done = 0;
            last_size = 0;
            last_offset = 0;
			return false;
		}
	}

	
	S5K3P3SX_lsc_data[18] = 0;  //reserved
	S5K3P3SX_lsc_data[19] = 0;  //reserved


	addr = 0x003B;  //lsc
	size = 1868;
	LOG_INF("read 3P3 eeprom, size1 = %d\n", size);
	if(!get_done || last_size != size || last_offset != addr) {
		if(!_read_3P3_eeprom(addr, S5K3P3SX_lsc_data+20, size)){
			get_done = 0;
            last_size = 0;
            last_offset = 0;
			return false;
		}
	}

	addr = 0x0787;	//lsc checksum
	size = 1;
	if(!get_done || last_size != size || last_offset != addr) {
		if(!_read_3P3_eeprom(addr, S5K3P3SX_lsc_data+1888, size)){
			get_done = 0;
			last_size = 0;
			last_offset = 0;
			return false;
		}
	}

	
//checksum
	{
	int i;
	for(i=20;i<1868+20;i++)
	{
		checksum_lsc += S5K3P3SX_lsc_data[i];
	}
		
	checksum_lsc = checksum_lsc%256;
	LOG_INF("checksum_lsc= %lu,S5K3P3SX_lsc_data[1888]=%d\n", checksum_lsc,S5K3P3SX_lsc_data[1888]);
	
	if (checksum_lsc!= S5K3P3SX_lsc_data[1888])
		S5K3P3SX_lsc_data[4] &= 0xf7;   //set bit4 0
		
	}

#if 0
	{
	int i;
	for(i=0;i<1868+20;i++)
	{
	//printk("S5K3P3_lsc_data[i]:%x,S5K3P3_lsc_data[i]:%d\n",S5K3P3SX_lsc_data[i],S5K3P3SX_lsc_data[i]);
	LOG_INF("S5K3P3SX_lsc_data[i]= %x,i=%d\n", S5K3P3SX_lsc_data[i],i);

	}
	}
#endif
    return true;
}

//WB OTP start
bool read_3P3_eeprom_wb(BYTE* data){
	int  checksum_wb = 0;
	int i, ret=0;
	int addr = 0x001C;
	int size = 30;
	LOG_INF("read 3P3 eeprom WB, addr=0x%x, size=%d\n", addr, size);
	if(!get_done || last_size != size || last_offset != addr) {
		if(!_read_3P3_eeprom(addr, s5k3P3_eeprom_data, size)){
			get_done = 0;
            		last_size = 0;
            		last_offset = 0;
			return false;
		}
	}
//checksum
	LOG_INF("read 3P3 eeprom WB flag=%d\n", s5k3P3_eeprom_data[0]);
	if(1 == s5k3P3_eeprom_data[0])	
	{
		for(i=1;i<7;i++)
		{
			checksum_wb += s5k3P3_eeprom_data[i];
//			LOG_INF("checksum_wb= %x,s5k3P3_eeprom_data[%d]=%x\n", checksum_wb, i, s5k3P3_eeprom_data[i]);
		}
		LOG_INF("checksum_wb= %x,s5k3P3_eeprom_data[29]=%x\n", checksum_wb, s5k3P3_eeprom_data[29]);
		if (checksum_wb == s5k3P3_eeprom_data[29])
		{
			LOG_INF("checksum is ok! s5k3P3_eeprom_data copy to data\n");
			memcpy(data, s5k3P3_eeprom_data+1, 6);
			ret = 1;
		}
	}

   	return ret;
}
//WB OTP end
