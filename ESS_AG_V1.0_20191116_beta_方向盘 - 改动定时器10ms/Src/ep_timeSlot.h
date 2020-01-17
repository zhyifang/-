#ifndef __EP_TIMESLOT_H__
#define __EP_TIMESLOT_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_gzll.h"
#include "nrf.h"

typedef struct 
{
   uint32_t evt_id;
}Timeslot_Handle_t;
extern Timeslot_Handle_t Timeslot_Handle;
extern volatile   uint8_t g_ui_signal_type,error1,error2,error_flag;
extern volatile  uint8_t  timeslot_open_flag,timeslot_open_flag1;
extern void  request_next(void);
extern uint32_t gazell_sd_radio_init(void);
extern void  timeslot_request_next(void);
extern void  timeslot_sys_event_handler(uint32_t evt_id); 
#endif
