#ifndef __EP_RS485_H_
#define __EP_RS485_H_

#include <stdint.h>
#include "nrf_gpio.h"

#define RS485_TX_PIN	        8
#define RS485_RE_DE_PIN	        9
#define RS485_RX_PIN	        10

typedef enum 
{
	RS485_IDLE = 1,
	RS485_REMOTE,
	RS485_BATTERY,
}RS485_Mode_e;

#define RS485_REMOTE_RX         1
#define RS485_REMOTE_TX         2
#define RS485_BATTERY_RX        3
#define RS485_BATTERY_TX        4

void RS485_Init(void);
void RS485_SetRx(void); 
void RS485_SetTx(void);

uint8_t RS485_LockMode(RS485_Mode_e Mode);
uint8_t RS485_ReleaseMode(RS485_Mode_e Mode);
RS485_Mode_e RS485_GetMode(void);

void UART0_IRQHandler(void);

#endif  /*__EP_RS485_H_*/
