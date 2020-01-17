/**************************************
Company       : ePropulsion 
File name     : 
Author        :   
Version       :
Date          :
Description   :
Others        :
Function List :
***Update  Description*****
1.  Date:
    Author:
    Modification:
*************************************/
#include "ep_comm.h"
#include  "ep_pstorage.h"
#include  "nrf_gzll.h"
#include  "ep_timeslot.h"
#include  "ep_gzll.h"
#include  "nrf_soc.h"
#include  "ep_gps.h"
#include  "ep_motor.h"
#include  "ep_crypt_device.h"
#include  "ep_powerswitch.h"
#include  "ep_ble_init.h"
#include  "ep_ble_motor.h"
#include  "ep_ble_battery.h"
#include  "main.h"
#include  "ep_version.h"

extern uint16_t Angle;

/*  Variable Declaration  */
uint8_t Comm_CheckSum(uint8_t* data, uint8_t size);
Comm_Handle_t Comm_Handle;

void Comm_RS485_Check(Comm_Handle_t* handle)
{
	if(nrf_gpio_pin_read(RS485_CHECK_PIN) && handle->Mode == Comm_Mode_Gzll)	 //if 485 Access first
	{
		handle->Mode = Comm_Mode_RS485;
		RS485_Init(); 
		sd_radio_session_close();
		PowerSwitch_BatUVcountClear();
	}					
	else if(!nrf_gpio_pin_read(RS485_CHECK_PIN) && handle->Mode == Comm_Mode_RS485)//if gzll Access first
	{
		handle->Mode = Comm_Mode_Gzll;
		gazell_sd_radio_init();
		RS485_Disable();
	}				
}

Comm_Mode_e Comm_Read_Mode(void)
{
    return Comm_Handle.Mode;
}
void Comm_Init(Comm_Handle_t* handle)
{
	nrf_gpio_cfg_input(RS485_CHECK_PIN,NRF_GPIO_PIN_PULLDOWN); 

	if(nrf_gpio_pin_read(RS485_CHECK_PIN))
	{		
		RS485_Init();
		handle->Mode = Comm_Mode_RS485;		
	}
	if(!nrf_gpio_pin_read(RS485_CHECK_PIN))
	{
		gazell_sd_radio_init();
		handle->Mode = Comm_Mode_Gzll;
	}
	
	handle->State = Comm_State_Disconnected;
    handle->Pre_State =  Comm_State_Disconnected;	
	handle->Data_Type =  SysOnType;
	handle->Rec.Data.Addr = ADDR_MAIN_SYSTEM;
	Comm_ClearFailCnt(handle);
	handle->ReadAddrEnable = true;
}

uint8_t CommRec = 0;
void Comm_Decode(Comm_Handle_t* handle)
{
	static uint32_t  AddrTemp = 0;
	uint16_t msg = 0;
	
    if(handle->Rec.Data.CheckSum != (Comm_CheckSum(handle->Rec.Raw, MAINSYSTEM_CMD_LEN -1)&0x7f))
	{
	    return;//check sum error
	}
	CommRec = 1;

	msg = (handle->Rec.Data.MsgHi << 7)|handle->Rec.Data.MsgLo;
	if(((handle->Rec.Data.ReturnType >= Comm_Ret_MotorOriginal) && (handle->Rec.Data.ReturnType <= Comm_Ret_HandleSideSet))||(handle->Rec.Data.ReturnType == Comm_Ret_MotorStatus))
	{
		MOTOR_comm_RX_decode(&MOTOR_Handle,&Comm_Handle);
	}
	
	else if((handle->Rec.Data.ReturnType >= Comm_Ret_PairEnable) &&  (handle->Rec.Data.ReturnType <= Comm_Ret_ReadGzllAdrRecSuc))
	{
		switch(handle->Rec.Data.ReturnType)
		{
			case Comm_Ret_PairEnable:
			     if(handle->Rec.Data.MsgLo == MCU_ID[0])
				 {
					 Comm_DataTypeSet(GzllAddr1HReadType);	
				 }
				 break;
			case Comm_Ret_GzllAddrH:
				AddrTemp = (handle->Rec.Data.MsgHi << 21)|(handle->Rec.Data.MsgLo) << 14; 
			    Comm_DataTypeSet(GzllAddr1LReadType);	
				break;
			
			case Comm_Ret_GzllAddrL:
				AddrTemp |=  (handle->Rec.Data.MsgHi << 7)|(handle->Rec.Data.MsgLo); 
			    Gzll_SetGzllAddr1(AddrTemp);
			    Comm_DataTypeSet(GzllAddrReadSucType);
			    
				break;
		    case Comm_Ret_ReadGzllAdrRecSuc:
				 
			     if(Gzll_ReadPairState() == PairingState)
				 {
				    GZLL_Handle.GZLL_addr_change = true;
					Gzll_SetPairState(PairSucState);
				    Comm_DataTypeSet(SysOnType);
				 }
			     if(handle->Mode == Comm_Mode_RS485)
			     {			
                    GZLL_Handle.GZLL_addr_change = true;					 
					Gzll_Backup_gzllAddr1(&GZLL_Handle,pstorage_Wdata);
					PSTORAGE_set_CurHandle(PSTORAGE_Handle.ps_gzll_handle,GZLLADDR1_BLOCK_ADD);
					PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 
					Gzll_SetCommStatus(CommStatus_Nomal);
					Comm_DataTypeSet(SysOnType);
				 }
				 break;

		}
		
	}
	else if((handle->Rec.Data.ReturnType == Comm_Ret_CrSysOn) || (handle->Rec.Data.ReturnType == Comm_Ret_CrSysOff))
	{
		Comm_DataTypeSet(ThrottleValSetType);
	}
	else if(handle->Rec.Data.ReturnType == Comm_Ret_MotorRst)
	{	
		if(msg == true)
		Comm_DataTypeSet(SysOnType);
	}
	else if(handle->Rec.Data.ReturnType == Comm_Ret_MotorPosition)
	{
		THROTTLE_Handle.MotorPosition = (handle->Rec.Data.MsgHi << 7)|(handle->Rec.Data.MsgLo); 	
	}	
	handle->State = Comm_State_Connected;
	Comm_ClearFailCnt(handle);
}

uint8_t Comm_GetFailCount(Comm_Handle_t* handle)
{
	return handle->CommFailCnt;
}
void Comm_ClearFailCnt(Comm_Handle_t* handle)
{
	handle->CommFailCnt = 0;
}
uint8_t Comm_CheckSum(uint8_t* data, uint8_t size)
{
	uint8_t i;
	uint8_t result = 0;
	
	for(i = 0;i<size;i++)
	{
		 result = result ^ data[i];
	}
	return result;
}

Comm_State_e Comm_ReadState(void)
{
    return Comm_Handle.State;
}
static inline void RS485_SetRx(void) 
{ 
	NRF_UART0->TASKS_STOPTX = 1;
	NRF_UART0->TASKS_STARTRX = 1;
	nrf_gpio_pin_clear(RS485_RE_DE_PIN);
}

void SetCommRx(void)
{
	RS485_SetRx();
}

static inline void RS485_SetTx(void) 
{
	NRF_UART0->TASKS_STARTTX = 1;
	NRF_UART0->TASKS_STOPRX = 1;
	nrf_gpio_pin_set(RS485_RE_DE_PIN);
}

void SetCommTx(void)
{
	RS485_SetTx();
}

void Comm_TxDatSet(Comm_Handle_t* handle, Comm_Cmd_e cmd_type,uint16_t com_msg)
{
	handle-> Cmd_type  =  cmd_type;
	handle-> tx_msg    =  com_msg;
}
uint8_t* Comm_PrepareTxData(Comm_Handle_t* handle)
{
	handle->Cmd.Data.Addr            = ADDR_MAIN_SYSTEM;
	handle->Cmd.Data.RequestedType   = handle-> Cmd_type ;
	handle->Cmd.Data.MsgHi           = ( handle-> tx_msg >>7)&0x7f;
	handle->Cmd.Data.MsgLo           = ( handle-> tx_msg )&0x7f;
	handle->Cmd.Data.CheckSum        = Comm_CheckSum(handle->Cmd.Raw, MAINSYSTEM_CMD_LEN -1)&0x7f;
	return (uint8_t*)(handle->Cmd.Raw);	
}
 uint8_t  DelayCount = 0;
 uint8_t  checkFlag = 1;
void Comm_UpdateSendData(void)
{
	//static uint16_t TimeOutCount = 0;
	if(Gzll_ReadCommStatus() == CommStatus_Nomal)
	{
		//For delay ,make sure the commrelay save the pair address
		if(Comm_Handle.Data_Type == SysOnType)
		{
			Comm_TxDatSet(&Comm_Handle, 
							 Comm_Cmd_SetSysOn,
							THROTTLE_Handle.MotorPosition); 
		}
		else if(Comm_Handle.Data_Type == SysOffType)
		{
			Comm_TxDatSet(&Comm_Handle, 
							 Comm_Cmd_SetSysOff,
							 false); 
		}
		else if(Comm_Handle.Data_Type == ThrottleValSetType)//发送方向控制
		{
			if(MOTOR_Handle.state == MOTOR_State_On)
			{				
				 Comm_TxDatSet(&Comm_Handle, 
							  Comm_Cmd_SetMotorPower,
							 (THROTTLE_GetDir(&THROTTLE_Handle) << 7) | THROTTLE_GetPercent(&THROTTLE_Handle)); 
			}
			#if 1
			else //if(MOTOR_Handle.state == MOTOR_State_Stop)
			{
				 Comm_TxDatSet(&Comm_Handle, 
							 Comm_Cmd_SetMotorStop,
							 0); 
			}
			#endif
		}
		else if(Comm_Handle.Data_Type == MotorCalSetType)   //丝杆校正设置指令
		{
			#if 1
			Comm_TxDatSet(&Comm_Handle,
						  Comm_Cmd_SetVoltage,
						  THROTTLE_Handle.Voltage);
			Comm_DataTypeSet(NullType);	//单发
			#endif
		}
		else if(Comm_Handle.Data_Type == NeutralSetType)    //发送电机中位点设置指令
		{
			Comm_TxDatSet(&Comm_Handle,
						  Comm_Cmd_SetNeutral,
						  (THROTTLE_GetDir(&THROTTLE_Handle) << 7) | THROTTLE_GetPercent(&THROTTLE_Handle));
			Comm_DataTypeSet(NullType);
		}
		else if(Comm_Handle.Data_Type == HandleSideSetType) //左右舷设置指令
		{
			Comm_TxDatSet(&Comm_Handle,
						  Comm_Cmd_SetHandleSide,
						  THROTTLE_Handle.HandleSide);
			Comm_DataTypeSet(NullType);
		}
		else if(Comm_Handle.Data_Type == ErrorClearType)    //发送清除故障指令
		{
			Comm_TxDatSet(&Comm_Handle,
						  Comm_Cmd_SetErrClear,
						  100);
			Comm_DataTypeSet(NullType);
		}
	}
	//auto pair 
	else
	{
		static  uint8_t readCount = 0;
		if(Comm_Handle.Data_Type == GzllPairRequestType)    //pair request 
		{
			if(Gzll_ReadPairState() == PairingState)
			{
				Comm_TxDatSet(&Comm_Handle, 
						   Comm_Cmd_PairRequest,
						   MCU_ID[0]); 
			}		
		}	
		else if(Comm_Handle.Data_Type == GzllAddr1HReadType)//read address H
		{
			//readhcount++;
			Comm_TxDatSet(&Comm_Handle, 
						   Comm_Cmd_GetGzllAddrH,
						   0); 
			if(Comm_Handle.Mode == Comm_Mode_RS485) 
			{
				if((Comm_ReadState() == Comm_State_Connected))
				{
					readCount++;
					if(readCount > 10)
					{
						readCount = 0;
						Gzll_SetCommStatus(CommStatus_Nomal);
						Comm_DataTypeSet(ThrottleValSetType);
					}
				}	
			}
		}
		else if(Comm_Handle.Data_Type == GzllAddr1LReadType)//read address L
		{
			readCount = 0;
			Comm_TxDatSet(&Comm_Handle, 
						   Comm_Cmd_GetGzllAddrL,
						   0); 
		}
		else if(Comm_Handle.Data_Type ==GzllAddrReadSucType)//read address success 
		{
			Comm_TxDatSet(&Comm_Handle, 
						   Comm_Cmd_GetGzllAddrSuc,
						   0); 
		}
	}	
}

void  Comm_DataTypeSet(Comm_Data_Type_e  DataType)
{
    Comm_Handle.Data_Type = DataType;
}
Comm_Data_Type_e  Comm_DataTypeRead(Comm_Handle_t* handle)
{
    return handle->Data_Type;
}
void Comm_RS485ModeSend(Comm_Handle_t* handle)
{
	RS485_Init();
	handle->TxPtr=0;
	RS485_SetTx();
	NRF_UART0->TXD = handle->Cmd.Raw[handle->TxPtr++]; 

	if(handle->CommFailCnt < 200)
	{
		handle->CommFailCnt++;	
	}  
	if(handle->CommFailCnt > DISCONNECT_CNT_MAX)
	{
		if(handle->State == Comm_State_Connected)
		{
			PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); //save motor position
			PSTORAGE_set_CurHandle(PSTORAGE_Handle.ps_percent_max_handle,PERCENT_MAX_BLOCK_ADD);
			THROTTLE_Backup_PercentMax(&THROTTLE_Handle,pstorage_Wdata);//掉电存储
		}
		handle->State = Comm_State_Disconnected;
		handle->Pre_State =  Comm_State_Disconnected;	
	}
	CommRec = 0;
}
void Comm_GzllModeSend(Comm_Handle_t* handle)
{

	nrf_gzll_add_packet_to_tx_fifo(PIPE,GZLL_Handle.tx_data.data,GZLL_Handle.tx_data.len);
	
	if(handle->CommFailCnt < 200)
	{
		handle->CommFailCnt++;	
		
	}  
	if(handle->CommFailCnt > DISCONNECT_CNT_MAX)
	{		
	
		if(handle->State == Comm_State_Connected)
		{
			PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); //save motor position
			PSTORAGE_set_CurHandle(PSTORAGE_Handle.ps_percent_max_handle,PERCENT_MAX_BLOCK_ADD);
			THROTTLE_Backup_PercentMax(&THROTTLE_Handle,pstorage_Wdata);//掉电存储
		}
		handle->State = Comm_State_Disconnected;
		handle->Pre_State =  Comm_State_Disconnected;
        Motor_SetMotStatus(motor_nomal);
	}
	if(handle->CommFailCnt >= 3)
	{
		GZLL_Handle.tx_fail_count++;
	}
	CRYPT_CommFailCheck(handle->CommFailCnt);
	CommRec = 0;
}
static void RS485_Init(void)
{	
	nrf_gpio_cfg_output(RS485_TX_PIN);
	nrf_gpio_cfg_input(RS485_RX_PIN, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_output(RS485_RE_DE_PIN);

	NRF_UART0->PSELTXD = RS485_TX_PIN;
	NRF_UART0->PSELRXD = RS485_RX_PIN;

	NRF_UART0->INTENSET = UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos;
	NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos;

	NRF_UART0->CONFIG   = 0;
	NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud38400 << UART_BAUDRATE_BAUDRATE_Pos);
	NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->TASKS_STARTTX = 1;
	NRF_UART0->TASKS_STARTRX = 1;
	NRF_UART0->EVENTS_RXDRDY = 0;	

	NVIC_SetPriority(UART0_IRQn,1);
	NVIC_EnableIRQ(UART0_IRQn);
	RS485_SetTx();
}
/**********************************
*   pin RX,TX,RD,  the Power consume when RS485 disable and not into main loop 
*   type1 output hight:  4.399mA
*   type2 output low  :  3.920mA
*   type3 input  not pull   : 3.838mA
*   type4 input  pull down : 3.838mA if not Disable uart 4.120
*   type5 input  pull up   : 4.270mA  
*   for reduce the  Power consume ,the best set is type4 or  type3
************************************/
static void RS485_Disable(void)
{	
    nrf_gpio_cfg_input(RS485_TX_PIN,NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_cfg_input(RS485_RX_PIN,NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_cfg_input(RS485_RE_DE_PIN,NRF_GPIO_PIN_PULLDOWN);

    NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
    NRF_UART0->TASKS_STARTTX = 0;
    NRF_UART0->TASKS_STARTRX = 0;
    NRF_UART0->EVENTS_RXDRDY = 0;	
	
	NVIC_DisableIRQ(UART0_IRQn);
}

void UART0_IRQHandler(void)
{
	uint8_t rx_data;
	if (NRF_UART0->EVENTS_ERROR)
	{
		NRF_UART0->EVENTS_ERROR = 0;
		NRF_UART0->ERRORSRC = 0x0F;
	}
	if (NRF_UART0->EVENTS_RXDRDY)  /*Is the receiver's interrupt flag set?*/ 
	{
		NRF_UART0->EVENTS_RXDRDY = 0;
		rx_data = NRF_UART0->RXD;
		
		if((Comm_Handle.RxPtr == 0)&&(rx_data != ADDR_MAIN_SYSTEM))
		{
			Comm_Handle.RxPtr = 0; 
		}
		else
		{			
			Comm_Handle.Rec.Raw[Comm_Handle.RxPtr++] = rx_data;
		}
		if(Comm_Handle.RxPtr==MAINSYSTEM_RET_LEN)//from sent to rec all dat need 2.7ms
		{
			Comm_Decode(&Comm_Handle);
			Comm_Handle.RxPtr=0;
            RS485_Disable();//after rec dat disable uart for save power	
		}
		return;
	}

	if (NRF_UART0->EVENTS_TXDRDY)
	{
		NRF_UART0->EVENTS_TXDRDY = 0;
        
		if(Comm_Handle.TxPtr<MAINSYSTEM_CMD_LEN)
		{
			NRF_UART0->TXD = Comm_Handle.Cmd.Raw[Comm_Handle.TxPtr++];
		}
		else			
		{
			Comm_Handle.TxPtr =0;
			RS485_SetRx();	
		}
		
		return;
	}
	NRF_UART0->EVENTS_RXDRDY = 0;
	NRF_UART0->EVENTS_TXDRDY = 0;
}
