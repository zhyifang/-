#ifndef __EP_TICK_H_
#define __EP_TICK_H_

void Systick_Init(void);
uint32_t Systick_GetSecond(void);
uint32_t Systick_GetTick(void);
void Systick_Delay(__IO uint32_t Delay);

#endif  /*__EP_TICK_H_*/
