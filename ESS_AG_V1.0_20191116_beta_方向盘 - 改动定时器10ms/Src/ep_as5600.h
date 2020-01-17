/*


	AS5600.H - §à§á§Ú§ã§Ñ§ß§Ú§Ö §â§Ö§Ô§Ú§ã§ä§â§à§Ó §æ§å§ß§Ü§è§Ú§Û §Õ§Ý§ñ §â§Ñ§Ò§à§ä§í §ã §Õ§Ñ§ä§é§Ú§Ü§à§Þ AS5600


*/
#include "stdint.h"

#define AS5600_ADDR	0x36		// §¡§Õ§â§Ö§ã §ß§Ñ §ê§Ú§ß§Ö

//	§°§á§Ú§ã§Ñ§ß§Ú§Ö §Ñ§Õ§â§Ö§ã§à§Ó §â§Ö§Ô§Ú§ã§ä§â§à§Ó AS5600

#define ZMCO		0x00
#define ZPOS_H		0x01		// §³§ä§Ñ§â§ä§à§Ó§Ñ§ñ §á§à§Ù§Ú§è§Ú§ñ
#define ZPOS_L		0x02
#define MPOS_H		0x03		// §³§ä§à§á§à§Ó§Ñ§ñ §á§à§Ù§Ú§è§Ú§ñ
#define MPOS_L		0x04
#define MANG_H		0x05
#define MANG_L		0x06
#define CONF_L		0x07
#define CONF_H		0x08

#define	RAWANG_H	0x0C		// "§³§í§â§à§Û" §å§Ô§à§Ý
#define RAWANG_L	0x0D
#define ANGLE_H		0x0E		// §µ§Ô§à§Ý §á§à§Ó§à§â§à§ä§Ñ
#define ANGLE_L		0x0F

#define STATUS		0x0B
#define AGC				0x1A
#define MAGN_H		0x1B
#define MAGN_L		0x1C

#define BURN		0xFF

//	§°§á§Ú§ã§Ñ§ß§Ú§Ö §Ò§Ú§ä§à§Ó§í§ç §Þ§Ñ§ã§à§Ü §â§Ö§Ô§Ú§ã§ä§â§à§Ó

#define	CONF_L_PM			0x03		// §²§Ö§Ø§Ú§Þ §á§Ú§ä§Ñ§ß§Ú§ñ:	00 = NOM, 01 = LPM1, 10 = LPM2, 11 = LPM3
#define CONF_H_SF			0x03		// §®§Ö§Õ§Ý§Ú§ä§Ö§Ý§î§ß§à§ã§ä§î §æ§Ú§Ý§î§ä§â§Ñ §á§à§Þ§Ö§ç: 00 = 16x (1); 01 = 8x; 10 = 4x; 11 = 2x
#define CONF_H_FTH		0x1C		// §±§à§â§à§Ô "§Ò§í§ã§ä§â§à§Ô§à" §æ§Ú§Ý§î§ä§â§Ñ
#define CONF_H_WD			0x20		// Watchdog

//	§°§á§Ú§ã§Ñ§ß§Ú§Ö §ã§Ý§å§Ø§Ö§Ò§ß§í§ç §Ü§à§ß§ã§ä§Ñ§ß§ä

#define	MAGNET_LOW	0x10
#define MAGNET_HIGH	0x20
#define MAGNET_NORM 0x30

#define HYST_MASK		0x0C
#define HYST_OFF		0x00
#define HYST_1LSB		0x04
#define HYST_2LSB		0x08
#define HYST_3LSB		0x0C

#define	OUT_STG_MASK				0x30
#define	OUT_STG_ANALOG			0x00
#define OUT_STG_ANALOG_RED	0x10
#define	OUT_STG_PWM					0x20

#define PWMF_MASK						0xC0
#define PWMF_115HZ					0x00
#define	PWMF_230HZ					0x40
#define	PWMF_460HZ					0x80
#define PWMF_920HZ					0xC0

#define AS5600_STATUS_MH 0X08
#define AS5600_STATUS_ML 0X10
#define AS5600_STATUS_MD 0X20

//typedef enum
//{
//	AS5600_STATUS_MH,
//	AS5600_STATUS_ML,
//	AS5600_STATUS_MD,
//}As5600_Status_e;

void AS5600_Init(void);

void AS5600_WriteReg(uint8_t Reg, uint8_t Data);
uint8_t AS5600_ReadReg(uint8_t Reg);

uint16_t AS5600_GetAngle(void);
uint16_t AS5600_GetRawAngle(void);
uint8_t AS5600_GetStatus(void);
void AS5600_SetHystheresis(uint8_t Hyst);
void AS5600_SetOutputStage(uint8_t OutStage);
void AS5600_SetPWMFreq(uint8_t Freq);



