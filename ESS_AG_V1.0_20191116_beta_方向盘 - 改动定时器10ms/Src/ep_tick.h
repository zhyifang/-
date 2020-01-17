#ifndef __EP_TICK_H_
#define __EP_TICK_H_

#define TIME_OUT_5_SECOND 150
#define TIME_OUT_3_SECOND 100

void Systick_Init(void);
void SysTimer1Stop(void);
void SysTimer1Start(void);

extern void EventTimeOutCheckProcess(void);
extern uint32_t CountDown5SecondCnt;
extern uint32_t ConutDown3SecondCnt;

#endif  /*__EP_TICK_H_*/
