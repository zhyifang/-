#ifndef  __EP_WDT_H
#define  __EP_WDT_H

#include "nrf.h"
#include <stdbool.h>

void WDT_Init(void);
void WDT_Feed(void);
bool WDT_ReadWdtRunStatues(void);

#endif

