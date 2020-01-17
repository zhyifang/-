#ifndef __EP_PROTECTION_H_
#define __EP_PROTECTION_H_

#include "nrf.h"

#define PROTECTIONSWITCH_PIN	7

void ProtectionSwtich_Start(uint8_t DelayOn);

void ProtectionSwtich_Init(void);
void ProtectionSwtich_TriggerOn(void);
void ProtectionSwtich_SetState(uint8_t ucState);
uint8_t ProtectionSwtich_GetState(void);

#endif  /*__EP_PROTECTION_H_*/
