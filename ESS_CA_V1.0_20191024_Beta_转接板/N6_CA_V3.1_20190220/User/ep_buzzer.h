#ifndef  __EP_BUZZER_H
#define  __EP_BUZZER_H

#include "nrf.h"
#include "nrf51.h"
#include "stdbool.h"

#define  BUZZER_PIN            11

#define  BUZZER_0              nrf_gpio_pin_clear(BUZZER_PIN)
#define  BUZZER_1              nrf_gpio_pin_set(BUZZER_PIN)
#define  BUZZER_Toggle         nrf_gpio_pin_toggle(BUZZER_PIN)


typedef struct  
{
    bool       BuzzerOnFlag;
	uint16_t   BuzzerOffCount;
	
}Buzzer_Handle_t;

extern Buzzer_Handle_t Buzzer_Handle; 

void  Buzzer_On(void);
void  Buzzer_Off(void);
void  Buzzer_Toggle(void);
void  Buzzer_Init(Buzzer_Handle_t* handle);
void  Buzzer_OffCheck(Buzzer_Handle_t* handle);
void  Buzzer_OnEn(Buzzer_Handle_t* handle);
#endif

