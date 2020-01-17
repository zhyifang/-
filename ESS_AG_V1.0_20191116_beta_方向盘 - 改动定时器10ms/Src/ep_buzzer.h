#ifndef __EP_BUZZER_H__
#define __EP_BUZZER_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"

#define BUZZER_PIN 26

typedef struct
{
	uint8_t g_Buzzer_on_flag;
	uint8_t g_buzzer_once_flag;
	uint8_t g_buzzer_twice_flag;
	uint8_t g_buzzer_beep_cnt;
	uint8_t g_buzzer_beep_times;
	uint8_t buzzer_delay_cnt1;
	uint8_t buzzer_delay_cnt2;
	uint8_t buzzer_delay_cnt3;
}BUZZER_Handle_t;

extern BUZZER_Handle_t BuzzerHandle;
void  BUZZER_Init(void);
void  BUZZER_On(void);
void  BUZZER_Off(void);
void  BUZZER_Run(uint8_t soc);
void BUZZER_Err(void);
void BUZZER_beep_once(void);
void BUZZER_beep_twice(void);

#endif /*__EP_BUZZER_H__*/
