/**********************************************************

***********************************************************/
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf51_bitfields.h"
#include "nrf_gpio.h"
#include "nrf51.h"
#include "spi_master.h"
/*******user header file************/
#include "ep_comm.h"
#include "main.h"
#include "ep_gps.h"
#include "ep_tick.h"
#include "ep_temp.h"
#include "ep_battery.h"
#include "ep_motor.h"
#include "ep_protection.h"
#include "softdevice_handler.h"
#include "ep_pstorage.h"
#include "ep_gzll.h"
#include "ep_rs485.h"
#include "ep_ble_stack.h"
#include "ep_buzzer.h"
#include "ep_version.h"
#include "ep_spi_slave.h"
#include "ep_wdt.h"

/*******end of user header file ************/
GPS_Handle_t Gps_Handle;
Comm_Handle_t Comm_Handle;
Motor_Handle_t Motor_Handle;
Battery_Handle_t Battery_Handle;

uint32_t PrevTick=0;
uint32_t CurrTick=0;
uint32_t gzll_addr0,gzll_addr1;
uint32_t mcu_id;

extern RS485_Mode_e RS485_Mode;
extern void Comm_CheckMode(Comm_Handle_t* handle);
extern void Remote485_CommFailCheck(void);

void HardFault_Handler()
{
   while(1)
	{  
		NVIC_SystemReset();				
	}  
}

int main(void)
{		
	//Product_Init(&navy_drv_msg,0,0,product_null,year_2019,month_1);
	//Product_Init(&navy_commrelay_msg,SOFTWARE_VER,HARDWARE_VER,PRODUCT_MODEL,SOFTWARE_YEAR,SOFTWARE_MONTH);	
	//Battery_Init(&Battery_Handle);
	
    mcu_id = NRF_FICR->DEVICEID[0] & 0xFFFFF;
	
	GZLL_param_init(&GZLL_Handle);
	ble_stack_init();
	PSTORAGE_Init();
	PSTORAGE_DataUdate();                                                   
	softdevice_handler_sd_disable();
    
	Gzll_NomalModeConfig(&GZLL_Handle);	
	Comm_Init(&Comm_Handle);
    
	Motor_Init(&Motor_Handle);
	Systick_Init();
	spi_slave_example_init();  //SPI初始化
 	WDT_Init();
	nrf_gpio_cfg_output(0);
	//nrf_gpio_pin_clear(12);
	while(1)
	{
		CurrTick = Systick_GetTick();

		if(CurrTick != PrevTick)
		{
			PrevTick = CurrTick;
														
			if(CurrTick % 5 == 0) 
			{		
				PSTORAGE_Save();	       				
			}		
						
			if(CurrTick % 20 == 0)//通信方式选择
			{	                
				Comm_CheckDisconnected(&Comm_Handle);
				Remote485_CommFailCheck();
				Motor_CheckComm();//check motor comm fail.
			}
			
			if(CurrTick % 30 == 0)     //配对状态
			{
				Gzll_CommFailCheck();   //30ms,无线通信检测失败函数
				Gzll_PairIntoCheck();   //30ms,无线通信函数
				Gzll_TimeOutCheck();    //30ms,连接超时函数
				WDT_Feed();
			}        			
		}
        
	}
}

