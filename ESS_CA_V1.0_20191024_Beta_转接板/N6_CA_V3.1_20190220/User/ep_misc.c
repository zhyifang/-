#include "ep_misc.h"

void QuickAvg_Init(QuickAvg_t* DeviceAvg, uint8_t Size, uint16_t InitValue) 
{
	DeviceAvg->Size = Size;
	DeviceAvg->Mean = InitValue;
	DeviceAvg->Sum = InitValue * Size;
}

void QuickAvg_Set(QuickAvg_t* DeviceAvg, uint16_t Value) 
{
	DeviceAvg->Sum = DeviceAvg->Size * Value ;
	DeviceAvg->Mean = Value;
}

uint16_t QuickAvg_Update(QuickAvg_t* DeviceAvg, uint16_t Element) 
{
	DeviceAvg->Sum = DeviceAvg->Sum - DeviceAvg->Mean + Element ;
	DeviceAvg->Mean = DeviceAvg->Sum / DeviceAvg->Size;
	return DeviceAvg->Mean;
}

uint16_t QuickAvg_Get(QuickAvg_t* DeviceAvg) 
{
	return DeviceAvg->Mean;
}

uint32_t uintabs(uint32_t a, uint32_t b)
{
    return a>b? a-b: b-a;
}
