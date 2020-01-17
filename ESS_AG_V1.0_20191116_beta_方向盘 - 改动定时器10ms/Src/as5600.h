#ifndef __AS5600
#define __AS5600
#include "myiic.h"   

#define AS5600_PRG_PIN  1       

#define AS5600_ADDR	0x36		// 设备地址

//AS5600是非接触式的位置传感器
//Configuration Registers
#define ZMCO		0x00
#define ZPOS_H		0x01		// start position
#define ZPOS_L		0x02
#define MPOS_H		0x03		// stop position
#define MPOS_L		0x04
#define MANG_H		0x05        // maximum angle
#define MANG_L		0x06
#define CONF_L		0x07        //CONF Register
#define CONF_H		0x08

//Output Registers
#define	RAWANG_H	0x0C		// 
#define RAWANG_L	0x0D
#define ANGLE_H		0x0E		// 
#define ANGLE_L		0x0F

//Status Registers
#define STATUS		0x0B
#define AGC			0x1A
#define MAGN_H		0x1B
#define MAGN_L		0x1C
//Burn Commands
#define BURN		0xFF

//CONF Register
#define	CONF_L_PM		0x03		// 	00 = NOM, 01 = LPM1, 10 = LPM2, 11 = LPM3
#define CONF_H_SF		0x03		//  00 = 16x (1); 01 = 8x; 10 = 4x; 11 = 2x
#define CONF_H_FTH	    0x1C		// 
#define CONF_H_WD		0x20		// Watchdog

//
#define	MAGNET_LOW	    0x10
#define MAGNET_HIGH	    0x08
#define MAGNET_NORM     0x20

#define HYST_MASK		0x0C
#define HYST_OFF		0x00
#define HYST_1LSB		0x04
#define HYST_2LSB		0x08
#define HYST_3LSB		0x0C

#define	OUT_STG_MASK			0x30
#define	OUT_STG_ANALOG			0x00
#define OUT_STG_ANALOG_RED	    0x10
#define	OUT_STG_PWM				0x20

#define PM_MASK					0x03
#define AS5600_PM_MODE_NOM      0x00
#define AS5600_PM_MODE_LPM1     0x01
#define AS5600_PM_MODE_LPM2     0x02
#define AS5600_PM_MODE_LPM3     0x03

#define PWMF_MASK				0xC0
#define PWMF_115HZ				0x00
#define	PWMF_230HZ				0x40
#define	PWMF_460HZ				0x80
#define PWMF_920HZ				0xC0

#define AS5600_STATUS_MH      0X08
#define AS5600_STATUS_ML      0X10
#define AS5600_STATUS_MD      0X20


void AS5600_Init(void);

u8 AS5600_ReadOneByte(u8 ReadAddr);

void AS5600_WriteOneByte(u8 WriteAddr,u8 DataToWrite);


u8 AS5600_Read(void);

uint8_t AS5600_GetAGC(void);
uint16_t AS5600_ReadConf(void);
uint16_t AS5600_ReadMagitue(void);
void AS5600_EnableWd(void);
void AS5600_SetPMMode(uint8_t mode);
uint8_t AS5600_GetStatus(void);
extern uint16_t AS5600_GetAngle(void);
extern void AS5600_SetOutputStage(uint8_t OutStage);
#endif




