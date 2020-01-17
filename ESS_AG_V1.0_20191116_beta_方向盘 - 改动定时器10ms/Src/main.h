#ifndef __MAIN_H
#define __MAIN_H
/**
V1.0   :
V1.1   :1.更改设置电池容量为0-2000
        2.取消蓝牙广播，解决有线时关机后无法开机问题
		3.修复无线通信三个频道有可能相同的情况
V1.2   :2016-8-8
        
**/   

#include "nrf.h"

//if the code finally ,DEBUG_MODE need to be 0,NOT_IDLING_MODE need to be 1
#define  DEBUG_MODE          0

#define  CODE_DATE_YEAR         2017     //代码日期，年月日
#define  CODE_DATE_MONTH        11
#define  CODE_DATE_DATE         13

#define  CODE_VERSION           181 //1.81 //代码版本




#define  ABS(x) ( (x)>0?(x):-(x)) 

extern void USER_ASSERT(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);

extern uint8_t MCU_ID[1];

extern void delay_ms(uint16_t time) ;
#endif



