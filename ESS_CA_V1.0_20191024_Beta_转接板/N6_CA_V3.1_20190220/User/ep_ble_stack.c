#include "nrf_gpio.h"
#include "ep_tick.h"
#include "ep_ble_stack.h"
#include <stdio.h>
#include "softdevice_handler.h"
#include "ep_pstorage.h"

void sys_evt_dispatch(uint32_t sys_evt)
{
	//	timeslot_sys_event_handler(sys_evt);
	  pstorage_sys_event_handler(sys_evt); 
}

 void ble_stack_init(void)
{
	SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_250MS_CALIBRATION, NULL);
	softdevice_sys_evt_handler_set(sys_evt_dispatch);
}		

