/**************************************
Company       : ePropulsion 
File name     : 
Author        :   
Version       :
Date          :
Description   :
Others        :
Function List :
***Update  Description*****
1.  Date:
    Author:
    Modification:

*  
*************************************/
#include "ep_hall.h"
#include "nrf_gpio.h"

#define  HALL_PIN_NUM			0

HALL_Handle_t  HALL_Handle;

void HALL_Init(void)
{
	 nrf_gpio_cfg_input(HALL_PIN_NUM,NRF_GPIO_PIN_NOPULL);
}
//0,´ÅÌú½Ó½ü
void HALL_Read(HALL_Handle_t* handle)
{
	 if(nrf_gpio_pin_read(HALL_PIN_NUM))
	 {
		handle->state = state_away;
	 }
	 else
	 {		 
		handle->state = state_close;
	 }
}
hall_state_e  HALL_ReadState(HALL_Handle_t* handle)  
{
	return handle->state;
}

