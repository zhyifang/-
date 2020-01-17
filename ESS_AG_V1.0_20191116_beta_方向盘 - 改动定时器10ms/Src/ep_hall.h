#ifndef __EP_HALL_H__
#define __EP_HALL_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"

typedef enum 
{
	state_away,
	state_close
}hall_state_e;
typedef struct
{	
   hall_state_e  state;
	
}HALL_Handle_t;

extern HALL_Handle_t  HALL_Handle;

extern void HALL_Init(void);
extern void HALL_Read(HALL_Handle_t* handle);
hall_state_e  HALL_ReadState(HALL_Handle_t* handle) ;
#endif /* __EP_HALL_H__ */

