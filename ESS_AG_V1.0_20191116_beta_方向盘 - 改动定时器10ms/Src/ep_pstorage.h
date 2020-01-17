#ifndef  __EP_PSTORAGE_H
#define  __EP_PSTORAGE_H

#include "nrf.h"
#include "pstorage.h"
#include "ep_lcd_6kw.h"
#include "ep_gzll.h"

#define  BLOCK_SIZE              0x10  //MIN SIZE IS 16
#define  BLOCK_COOUNT            60

#define  UNIT_BLOCK_ADD          0
#define  BATTERY_BLOCK_ADD       1
#define  GZLLADDR1_BLOCK_ADD     2 
#define  PERCENT_MAX_BLOCK_ADD   3
#define  ANG_BLOCK_ADD           4
#define  GPS_BLOCK_ADD           5

#define  TIMERECORD_BLOCK_ADD    6
//add other block 


typedef struct
{
     uint32_t           handleAdd;
	
	 pstorage_handle_t  ps_unit_handle;
	 pstorage_handle_t  ps_battery_handle;
	 pstorage_handle_t  ps_gzll_handle;
	 pstorage_handle_t  ps_percent_max_handle;
	 pstorage_handle_t  ps_ang_handle;
	 pstorage_handle_t  ps_gps_handle;
	 pstorage_handle_t  ps_TimeRecord_handle;
	 //add other block  
	 pstorage_handle_t  CurrentHandle;
	 volatile bool      write_flag;
	 
}PSTORAGE_Handle_t;

extern  PSTORAGE_Handle_t PSTORAGE_Handle;

extern  uint8_t   pstorage_Wdata[16];
extern  uint8_t   pstorage_Rdata[16];
extern  volatile bool flagRest;


void  PSTORAGE_set_Wflag(PSTORAGE_Handle_t* handle,bool flag);
bool  PSTORAGE_read_Wflag(PSTORAGE_Handle_t* handle);
void  PSTORAGE_set_CurHandle(pstorage_handle_t ps_handle,uint32_t add);
void  PSTORAGE_DataUdate(void);

uint32_t  PSTORAGE_Init(void);
uint32_t  PSTORAGE_Erase(pstorage_handle_t* pstorage_handle ,uint32_t block_add,bool all);
uint32_t  PSTORAGE_Read(pstorage_handle_t* pstorage_handle,uint32_t block_add,uint8_t *rData);
uint32_t  PSTORAGE_Write(pstorage_handle_t* pstorage_handle,uint32_t block_add,uint8_t *wData);
uint8_t   PSTORAGE_CheckSum(uint8_t * data, uint8_t length);
void      PSTORAGE_SaveData(PSTORAGE_Handle_t* handle);
//////////////////////////////
#endif

