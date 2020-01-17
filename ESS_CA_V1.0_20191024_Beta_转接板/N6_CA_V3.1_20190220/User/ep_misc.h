#ifndef __EP_MISC_H_
#define __EP_MISC_H_

#include "nrf.h"

typedef struct {
	uint32_t Sum;
	uint16_t Mean;
	uint16_t Size;
} QuickAvg_t;

void QuickAvg_Init(QuickAvg_t* DeviceAvg, uint8_t Size, uint16_t InitValue) ;
void QuickAvg_Set(QuickAvg_t* DeviceAvg, uint16_t Value) ;
uint16_t QuickAvg_Update(QuickAvg_t* DeviceAvg, uint16_t Element);
uint16_t QuickAvg_Get(QuickAvg_t* DeviceAvg) ;
uint32_t uintabs(uint32_t a, uint32_t b);

#endif  /*__EP_MISC_H_*/
