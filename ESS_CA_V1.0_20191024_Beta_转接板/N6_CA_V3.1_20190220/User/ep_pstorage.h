#ifndef  __EP_PSTORAGE_H
#define  __EP_PSTORAGE_H

#include "nrf.h"
#include "pstorage.h"
#include "ep_gzll.h"

#define  BLOCK_SIZE             0x10  //MIN SIZE IS 16
#define  BLOCK_COOUNT           60

#define  GZLLADDR1_BLOCK_ADD    0
#define  MotCal_BLOCK_ADD       1


typedef struct
{
	 pstorage_handle_t  ps_gzll_handle;
	 pstorage_handle_t  ps_MotCal_handle;
	 
	 pstorage_handle_t  CurrentHandle;
	 uint8_t            handleAdd;
	 volatile bool      write_flag; 
}PSTORAGE_Handle_t;


extern  PSTORAGE_Handle_t PSTORAGE_Handle;
extern  volatile bool flagRest ;
extern  uint8_t   pstorage_Wdata[16];
extern  uint8_t   pstorage_Rdata[16];

void  PSTORAGE_set_Wflag(PSTORAGE_Handle_t* handle,bool flag);
bool  PSTORAGE_read_Wflag(PSTORAGE_Handle_t* handle);
void  PSTORAGE_set_CurHandle(pstorage_handle_t ps_handle,uint8_t add);
void  MOTOR_Backup_PowerLimit(uint8_t* data);
bool  MOTOR_Update_PowerLimit(uint8_t* data);
void  BatCap_Backup(uint8_t* data);
bool  BatCap_Update(uint8_t* data);
void  PSTORAGE_DataUdate(void);
uint32_t PSTORAGE_Init(void);
uint32_t PSTORAGE_Erase(pstorage_handle_t* pstorage_handle ,uint32_t block_add,bool all);
uint32_t PSTORAGE_Read(pstorage_handle_t* pstorage_handle,uint32_t block_add,uint8_t *rData);
uint32_t PSTORAGE_Write(pstorage_handle_t* pstorage_handle,uint32_t block_add,uint8_t *data);
void PSTORAGE_Save(void);


#endif

