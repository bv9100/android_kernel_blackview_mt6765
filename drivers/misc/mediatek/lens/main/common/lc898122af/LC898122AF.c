/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/*
 * DW9714AF voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#include "lens_info.h"
#include "LC898122.h"

#include "Ois.h"
#include "OisDef.h"
//#include "OisFil.h"
//#include "OisFil_org.h"

#define AF_DRVNAME "LC898122AF_DRV"
#define AF_I2C_SLAVE_ADDR        0x48

#define AF_DEBUG
#ifdef AF_DEBUG
#define LOG_INF(format, args...) pr_debug(AF_DRVNAME " [%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif

//#define LC898122AF_OIS_ON


static struct i2c_client *g_pstAF_I2Cclient;
static int *g_pAF_Opened;
static spinlock_t *g_pAF_SpinLock;


static unsigned long g_u4AF_INF;
static unsigned long g_u4AF_MACRO = 1023;
static unsigned long g_u4TargetPosition;
static unsigned long g_u4CurrPosition;

void RamReadA_LC898122AF(unsigned short RamAddr, void *ReadData)
{
	int i4RetValue = 0;
	char pBuff[2] = { (char)(RamAddr >> 8), (char)(RamAddr & 0xFF) };
	unsigned short vRcvBuff = 0;
	unsigned long *pRcvBuff;

	pRcvBuff = (unsigned long *)ReadData;

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, pBuff, 2);
	if (i4RetValue < 0) {
		LOG_INF("[CAMERA SENSOR] read I2C send failed!!\n");
		return;
	}

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, (u8 *) &vRcvBuff, 2);
	if (i4RetValue != 2) {
		LOG_INF("[CAMERA SENSOR] I2C read failed!!\n");
		return;
	}
	*pRcvBuff = ((vRcvBuff & 0xFF) << 8) + ((vRcvBuff >> 8) & 0xFF);

	/* LOG_INF("I2C r2 (%x %x)\n", RamAddr, (unsigned int)*pRcvBuff); */
}

void RamWriteA_LC898122AF(unsigned short RamAddr, unsigned short RamData)
{
	int i4RetValue = 0;

	char puSendCmd[4] = { (char)((RamAddr >> 8) & 0xFF),
		(char)(RamAddr & 0xFF),
		(char)((RamData >> 8) & 0xFF),
		(char)(RamData & 0xFF)
	};

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 4);

	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return;
	}

}

void RamWrite32A_LC898122AF(unsigned short RamAddr, unsigned long RamData)
{
	int i4RetValue = 0;
	char puSendCmd[6] = { (char)((RamAddr >> 8) & 0xFF),
		(char)(RamAddr & 0xFF),
		(char)((RamData >> 24) & 0xFF),
		(char)((RamData >> 16) & 0xFF),
		(char)((RamData >> 8) & 0xFF),
		(char)(RamData & 0xFF)
	};

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 6);
	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return;
	}
}

void RamRead32A_LC898122AF(unsigned short RamAddr, void *ReadData)
{
	int i4RetValue = 0;
	char pBuff[2] = { (char)(RamAddr >> 8), (char)(RamAddr & 0xFF) };
	unsigned long *pRcvBuff, vRcvBuff = 0;

	pRcvBuff = (unsigned long *)ReadData;

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, pBuff, 2);
	if (i4RetValue < 0) {
		LOG_INF("[CAMERA SENSOR] read I2C send failed!!\n");
		return;
	}

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, (u8 *) &vRcvBuff, 4);
	if (i4RetValue != 4) {
		LOG_INF("[CAMERA SENSOR] I2C read failed!!\n");
		return;
	}
	*pRcvBuff = ((vRcvBuff & 0xFF) << 24)
	    + (((vRcvBuff >> 8) & 0xFF) << 16)
	    + (((vRcvBuff >> 16) & 0xFF) << 8)
	    + (((vRcvBuff >> 24) & 0xFF));

}

void RegWriteA_LC898122AF(unsigned short RegAddr, unsigned char RegData)
{
	int i4RetValue = 0;
	char puSendCmd[3] = { (char)((RegAddr >> 8) & 0xFF), (char)(RegAddr & 0xFF), RegData };

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 3);
	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return;
	}
}

void RegReadA_LC898122AF(unsigned short RegAddr, unsigned char *RegData)
{
	int i4RetValue = 0;
	char pBuff[2] = { (char)(RegAddr >> 8), (char)(RegAddr & 0xFF) };

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, pBuff, 2);
	if (i4RetValue < 0) {
		LOG_INF("[CAMERA SENSOR] read I2C send failed!!\n");
		return;
	}

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, (u8 *) RegData, 1);

	/* LOG_INF("I2C r (%x %x)\n", RegAddr, *RegData); */
	if (i4RetValue != 1) {
		LOG_INF("[CAMERA SENSOR] I2C read failed!!\n");
		return;
	}
}

//ois start
void s4AF_Write_Word_Byte(unsigned short RegAddr, unsigned char RegData)
{
	int i4RetValue = 0;
	char puSendCmd[3] = { (char)((RegAddr >> 8) & 0xFF), (char)(RegAddr & 0xFF), RegData };

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 3);
	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return;
	}
}

void s4AF_Write_Word_Word(unsigned short RamAddr, unsigned short RamData)
{
	int i4RetValue = 0;

	char puSendCmd[4] = { (char)((RamAddr >> 8) & 0xFF),
		(char)(RamAddr & 0xFF),
		(char)((RamData >> 8) & 0xFF),
		(char)(RamData & 0xFF)
	};

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 4);
	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return;
	}

}

void s4AF_Write_Word_DWord(unsigned short RamAddr, unsigned long RamData)
{
	int i4RetValue = 0;
	char puSendCmd[6] = { (char)((RamAddr >> 8) & 0xFF),
		(char)(RamAddr & 0xFF),
		(char)((RamData >> 24) & 0xFF),
		(char)((RamData >> 16) & 0xFF),
		(char)((RamData >> 8) & 0xFF),
		(char)(RamData & 0xFF)
	};

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 6);
	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return;
	}
}


static void LC898122_write_settings(unsigned short size, struct reg_settings_ois_t *settings)
{
	int i = 0;
	LOG_INF("Enter\n");

	for (i = 0; i < size; i++) {

			switch (settings[i].data_type) {
			case BYTE_DATA:
			    s4AF_Write_Word_Byte(settings[i].reg_addr,(unsigned char)settings[i].reg_data);
				LOG_INF("addr[%d] =  0x%04x, data = 0x%02x\n",i,settings[i].reg_addr,(unsigned char)settings[i].reg_data);
				break;
			case WORD_DATA:
                s4AF_Write_Word_Word(settings[i].reg_addr,(unsigned short)settings[i].reg_data);
				LOG_INF("addr[%d] =  0x%04x, data = 0x%04x\n",i,settings[i].reg_addr,(unsigned short)settings[i].reg_data);
				break;
			case DWORD_DATA:
				 s4AF_Write_Word_DWord(settings[i].reg_addr,(unsigned long)settings[i].reg_data);
				 LOG_INF("addr[%d] =  0x%04x, data = 0x%08lx\n",i,settings[i].reg_addr,(unsigned long)settings[i].reg_data);
				break;

			default:
				LOG_INF("Unsupport data type: %d\n",
					settings[i].data_type);
				break;
			}


		if (settings[i].delay > 20)
			msleep(settings[i].delay);
		else if (0 != settings[i].delay)
			usleep_range(settings[i].delay * 1000,
				(settings[i].delay * 1000) + 1000);
	}

	LOG_INF("Exit\n");
}

#ifdef LC898122AF_OIS_ON
static unsigned char s4LC898OTP_ReadReg(unsigned short RegAddr)
{
	int i4RetValue = 0;
	unsigned char pBuff = (unsigned char)RegAddr;
	unsigned char RegData = 0xFF;

	g_pstAF_I2Cclient->addr = (0xA8 >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, &pBuff, 1);
	if (i4RetValue < 0) {
		LOG_INF("[CAMERA SENSOR] read I2C send failed!!\n");
		return 0xff;
	}

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, &RegData, 1);

	LOG_INF("OTPI2C r (%x %x)\n", RegAddr, RegData);
	if (i4RetValue != 1) {
		LOG_INF("[CAMERA SENSOR] I2C read failed!!\n");
		return 0xff;
	}
	return RegData;

}

static void LC898122AF_otp(void)
{
	unsigned char otp_data[36] = {0};
	int i;
	unsigned int temp;

	for(i = 0x11; i <= 0x31; i++)
		otp_data[i - 0x11] = s4LC898OTP_ReadReg(i);

	otp_data[34] = s4LC898OTP_ReadReg(0x38);

//	for(i = 0; i < 36; i++)
//		LOG_INF("otp_data[%d] = 0x%x\n", i, otp_data[i]);

	RamAccFixMod(ON);

	temp = otp_data[1] << 8 | otp_data[0];
	RamWriteA_LC898122AF(0x1479, temp);	//Hall offset X

	temp = otp_data[3] << 8 | otp_data[2];
	RamWriteA_LC898122AF(0x14F9, temp);	//Hall offset Y

	temp = otp_data[5] << 8 | otp_data[4];
	RamWriteA_LC898122AF(0x147A, temp);	//Hall bias X

	temp = otp_data[7] << 8 | otp_data[6];
	RamWriteA_LC898122AF(0x14FA, temp);	//Hall bias Y

	temp = otp_data[9] << 8 | otp_data[8];
	RamWriteA_LC898122AF(0x1450, temp);	//Hall AD offset X

	temp = otp_data[11] << 8 | otp_data[10];
	RamWriteA_LC898122AF(0x14D0, temp);	//Hall AD offset Y

	temp = otp_data[13] << 8 | otp_data[12];
	RamWriteA_LC898122AF(0x10D3, temp);	//Loop gain X

	temp = otp_data[15] << 8 | otp_data[14];
	RamWriteA_LC898122AF(0x11D3, temp);	//Loop gain Y

	RamAccFixMod(OFF);

	RegWriteA_LC898122AF(0x02A0,otp_data[17]);	//Gyro offset X[15:8]

	RegWriteA_LC898122AF(0x02A1,otp_data[16]);	//Gyro offset X[7:0]

	RegWriteA_LC898122AF(0x02A2,otp_data[19]);	//Gyro offset Y[15:8]

	RegWriteA_LC898122AF(0x02A3,otp_data[18]);	//Gyro offset Y[7:0]

	RegWriteA_LC898122AF(0x0257,otp_data[20]);	//OSC Value[7:0]

	temp = otp_data[24] << 24 | otp_data[23] << 16 | otp_data[22] << 8 | otp_data[21];
	RamWrite32A_LC898122AF(0x1020, temp);	//Gyro gain X[31:0]

	temp = otp_data[28] << 24 | otp_data[27] << 16 | otp_data[26] << 8 | otp_data[25];
	RamWrite32A_LC898122AF(0x1120, temp);	//Gyro gain X[31:0]

	SetDOFSTDAF(otp_data[34]);	//Set VCM DAC offset
}

#endif

static void LC898122_init(void)
{
	unsigned char UcStbb0, UcClkon;

	LC898122_write_settings(lc898212_setting.init_setting_size, &(lc898212_setting.init_settings[0]));
	LOG_INF("LC898122_init \n");

	RegReadA_LC898122AF(STBB0, &UcStbb0);
	/* 0x0250       [ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ] */
	UcStbb0 &= 0x80;
	RegWriteA_LC898122AF(STBB0, UcStbb0);
	/* 0x0250       [ STBAFDRV | STBOISDRV | STBOPAAF | STBOPAY ][ STBOPAX | STBDACI | STBDACV | STBADC ] */
	RegWriteA_LC898122AF(PWMA, 0x00);	/* 0x0010               PWM Standby */
	RegWriteA_LC898122AF(CVA, 0x00);	/* 0x0020       LINEAR PWM mode standby */
	DrvSw(OFF);	/* Drvier Block Ena=0 */
	#ifdef	MONITOR_OFF
	#else
	RegWriteA_LC898122AF(PWMMONA, 0x00);	/* 0x0030       Monitor Standby */
	#endif
	/* RegWriteA_LC898122AF( DACMONFC, 0x01 ) ;           // 0x0032       DAC Monitor Standby */
	SelectGySleep(ON);	/* Gyro Sleep */
	RegReadA_LC898122AF(CLKON, &UcClkon);
	/* 0x020B       PWM Clock OFF + D-Gyro I/F OFF  SRVCLK can't OFF */
	UcClkon &= 0x1A;
	RegWriteA_LC898122AF(CLKON, UcClkon);
	/* 0x020B       PWM Clock OFF + D-Gyro I/F OFF  SRVCLK can't OFF */


#ifdef LC898122AF_OIS_ON
//OIS init start
	SelectModule(MODULE_20M);
	IniSetAf();
	IniSet();

	LC898122AF_otp();

	RemOff(ON);	//Remove gyro offset

	RegWriteA_LC898122AF(0x0304, 0x04);	//Enable FST(Fast settling time) function

	s4AF_Write_Word_Word(0x0380, (unsigned short)(1000 << 6));	//Move lens to the initial position in Z direction

	RtnCen(0x00);	//Enable servo control (Move lens to center position in X/Y direction)

	mdelay(150);

	RemOff(OFF);	//Clear removing gyro offset

	OisEna();	//Enable OIS control (OIS mode : Still mode, Pan/Tilt mode : ON)
#endif
}


//ois end
static inline int getAFInfo(__user struct stAF_MotorInfo *pstMotorInfo)
{
	struct stAF_MotorInfo stMotorInfo;

	stMotorInfo.u4MacroPosition = g_u4AF_MACRO;
	stMotorInfo.u4InfPosition = g_u4AF_INF;
	stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
	stMotorInfo.bIsSupportSR = 1;

	stMotorInfo.bIsMotorMoving = 1;

	if (*g_pAF_Opened >= 1)
		stMotorInfo.bIsMotorOpen = 1;
	else
		stMotorInfo.bIsMotorOpen = 0;

	if (copy_to_user(pstMotorInfo, &stMotorInfo, sizeof(struct stAF_MotorInfo)))
		LOG_INF("copy to user failed when getting motor information\n");

	return 0;
}

static inline int moveAF(unsigned long a_u4Position)
{
	//int ret = 0;

	if ((a_u4Position > g_u4AF_MACRO) || (a_u4Position < g_u4AF_INF)) {
		LOG_INF("out of range\n");
		return -EINVAL;
	}

	if (*g_pAF_Opened == 1) {

		//ret = s4AF_ReadReg(&InitPos);

		LC898122_init();

		spin_lock(g_pAF_SpinLock);
		g_u4CurrPosition = 0;
		*g_pAF_Opened = 2;
		spin_unlock(g_pAF_SpinLock);
	}

	if(a_u4Position == 0)
		{
		LOG_INF("hct-drv s4AF_Write_Word_Word\n");
		s4AF_Write_Word_Word(0x0380, 900 << 5);	// g_u4TargetPosition << 5
		return 0;
		}
	if (g_u4CurrPosition == a_u4Position)
		return 0;

	spin_lock(g_pAF_SpinLock);
	g_u4TargetPosition = a_u4Position;
	spin_unlock(g_pAF_SpinLock);

	 LOG_INF("move [curr] %lu [target] %lu  DAC 0x%x\n", g_u4CurrPosition, g_u4TargetPosition, (unsigned int)((g_u4TargetPosition + 800) << 5));


	s4AF_Write_Word_Word(0x0380, (unsigned short)((g_u4TargetPosition + 800) << 5));	// g_u4TargetPosition << 5

//	SetTregAf((unsigned short)g_u4TargetPosition);	//Move lens

	spin_lock(g_pAF_SpinLock);
	g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
	spin_unlock(g_pAF_SpinLock);

	return 0;
}

static inline int setAFInf(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_INF = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

static inline int setAFMacro(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_MACRO = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

/* ////////////////////////////////////////////////////////////// */
long LC898122AF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command, unsigned long a_u4Param)
{
	long i4RetValue = 0;

	switch (a_u4Command) {
	case AFIOC_G_MOTORINFO:
		i4RetValue = getAFInfo((__user struct stAF_MotorInfo *) (a_u4Param));
		break;

	case AFIOC_T_MOVETO:
		i4RetValue = moveAF(a_u4Param);
		break;

	case AFIOC_T_SETINFPOS:
		i4RetValue = setAFInf(a_u4Param);
		break;

	case AFIOC_T_SETMACROPOS:
		i4RetValue = setAFMacro(a_u4Param);
		break;

	default:
		LOG_INF("No CMD\n");
		i4RetValue = -EPERM;
		break;
	}

	return i4RetValue;
}

/* Main jobs: */
/* 1.Deallocate anything that "open" allocated in private_data. */
/* 2.Shut down the device on last close. */
/* 3.Only called once on last time. */
/* Q1 : Try release multiple times. */
int LC898122AF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");

	if (*g_pAF_Opened == 2) {
		LOG_INF("Wait\n");
		//s4AF_WriteReg(0x80); /* Power down mode */
		LC898122_write_settings(lc898212_setting.disable_ois_setting_size, &(lc898212_setting.disable_ois_settings[0]));
	}

	if (*g_pAF_Opened) {
		LOG_INF("Free\n");

		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 0;
		spin_unlock(g_pAF_SpinLock);
	}

	LOG_INF("End\n");

	return 0;
}

int LC898122AF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient, spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_SpinLock = pAF_SpinLock;
	g_pAF_Opened = pAF_Opened;

	return 1;
}

int LC898122AF_GetFileName(unsigned char *pFileName)
{
	#if SUPPORT_GETTING_LENS_FOLDER_NAME
	char FilePath[256];
	char *FileString;

	sprintf(FilePath, "%s", __FILE__);
	FileString = strrchr(FilePath, '/');
	*FileString = '\0';
	FileString = (strrchr(FilePath, '/') + 1);
	strncpy(pFileName, FileString, AF_MOTOR_NAME);
	LOG_INF("FileName : %s\n", pFileName);
	#else
	pFileName[0] = '\0';
	#endif
	return 1;
}
