#include "ep_battery.h"
#include "ep_motor.h"
#include "ep_gps.h"
#include "ep_tick.h"
#include "math.h" 
#include "ep_protection.h"
#include "ep_pstorage.h"
#include "ep_rs485.h"
#include "main.h"
#include "ep_version.h"

QuickAvg_t Battery_RemainingTime;
QuickAvg_t Battery_SmoothedCurrent;

extern Battery_Handle_t Battery_Handle;
extern Comm_Handle_t Comm_Handle;
extern Motor_Handle_t Motor_Handle;
extern GPS_Handle_t Gps_Handle;

const uint16_t CRC16_Tab[256] =
{               
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

uint16_t CalCRC16(uint8_t* Data, uint8_t length)
{
    uint8_t i = 0;
    uint16_t Crc = 0;    
    for(i = 0; i < length; i++)
    {
        Crc = CRC16_Tab[(uint8_t)(Crc >> 8) ^ Data[i]] ^ (Crc << 8);
    }    
    return Crc;
}

uint8_t Battery_YD_Trigger(Battery_Handle_t* Handle)
{
    if((Systick_GetTick() - Handle->TrigCommTime) >= EP_BATT_UPDATE_INTERVAL)
    {                
        RS485_ReleaseMode(RS485_REMOTE);
        RS485_LockMode(RS485_BATTERY);  
        return 1;
    }
    else
    {
        return 0;
    }
}

void Battery_YD_Update(Battery_Handle_t* Handle)
{    
    uint32_t CurrTick;
    uint16_t CommInterval;
    
    if(RS485_GetMode() != RS485_BATTERY)
    {
        //return to remote if it is in remote's comm time
        return;    
    }
    else if((Handle->BatteryObj.Status == Battery_Status_CommFailed) || ((Handle->BatteryObj.Status == Battery_Status_NonEpBatt) && (Systick_GetTick() >= 4000)))
    {            
        //allocate 485 to remote for batt comm failed or power on time >= 4000 without proper batt comm
        RS485_ReleaseMode(RS485_BATTERY);  
        RS485_LockMode(RS485_REMOTE);   
        RS485_SetRx();
        return;    
    }    
    
    if(Handle->CommFailCnt > 50)//200)
    {
        Handle->RxPtr = 0;
        Handle->TxPtr = 0;  
                
        RS485_ReleaseMode(RS485_BATTERY);  
        RS485_LockMode(RS485_REMOTE);     
        RS485_SetRx();
        
        if((Handle->TrigCommTime != 0) && (Handle->TrigCommTime - Handle->LastCommTime > 4000) && (Handle->BatteryObj.Type == Battery_TypeOwnBatt))
        {
            //comm failure conditions: no reply for more than 4 seconds
            Handle->BatteryObj.Status = Battery_Status_CommFailed;
            return;
        }
        else if((Handle->BatteryObj.Type == Battery_TypeOwnBatt) || ((Handle->BatteryObj.Status == Battery_Status_NonEpBatt) && (Systick_GetTick() < 4000)))
        {
            //reset conditions for batt comm retry
            Handle->CommFailCnt = 0;
            Handle->TrigCommTime = 0;
        }
    }
    CurrTick = Systick_GetTick();
    CommInterval = CurrTick - Handle->TrigCommTime;
    if((RS485_GetMode() == RS485_BATTERY) && (Handle->TrigCommTime) && (CommInterval > 20))
    {
            //no response when is in 485 rx mode and wait for 20ms           
            Handle->TxPtr = 0;
            Handle->RxPtr = 0; 
            RS485_SetRx();
            Handle->CommFailCnt++;
    }
    
    if((CommInterval >= EP_BATT_UPDATE_INTERVAL) && (Handle->TxPtr == 0))
	{ 
        Handle->Ret.Data.CS1 = 0;
        Handle->Ret.Data.CS2 = 0;
        Handle->TrigCommTime = CurrTick;  
	  
		RS485_SetTx();
		NRF_UART0->TXD = Handle->Cmd.Raw[Handle->TxPtr++]; 
	}    
}

void Battery_YD_Decode(Battery_Handle_t* Handle)
{    
    uint16_t CRC16 = CalCRC16(Handle->Ret.Raw, EP_BATT_RET_LEN - 2);
    uint8_t CRC1 = CRC16 >> 8;
    uint8_t CRC2 = CRC16 & 0xff;
    if((CRC1 == Handle->Ret.Raw[EP_BATT_RET_LEN - 2]) && (CRC2 == Handle->Ret.Raw[EP_BATT_RET_LEN - 1]))
    {   
        if(Handle->BatteryObj.Status != Battery_Status_On)
            Handle->BatteryObj.Status = Battery_Status_On;
        
        Handle->CommFailCnt = 0;
        Handle->BatteryObj.Voltage = (Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 1] | (Handle->Ret.Raw[EP_BATT_RET_DATA_POS] << 8)) / 10;
        
        int16_t TempS16 = ((int16_t)(Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 3] | (Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 2] << 8))) / 10;
        Handle->BatteryObj.Current = TempS16 < 0 ? -TempS16 : TempS16;
        
        Handle->BatteryObj.MaxOutputPower = (Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 5] | (Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 4] << 8));
		uint16_t ThrottlePercentage;
		if(navy_commrelay_msg.product_model == product_navy_60)
		{
             ThrottlePercentage = ((uint32_t)Handle->BatteryObj.MaxOutputPower) * 100 / 600;
		}
		else if(navy_commrelay_msg.product_model == product_navy_30)
		{
			ThrottlePercentage = ((uint32_t)Handle->BatteryObj.MaxOutputPower) * 100 / 300;
		}
		
		 if((ThrottlePercentage % 10) >= 5)
			ThrottlePercentage = ThrottlePercentage / 10 + 1;
		else
			ThrottlePercentage = ThrottlePercentage / 10;


		
       
        Comm_SetThrottlePercentageBatt(&Comm_Handle, (uint8_t)ThrottlePercentage);
        
        Handle->BatteryObj.SOC = Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 6];
        Handle->BatteryObj.SOH = Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 7];
        Handle->BatteryObj.AbsCapacity = (Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 9] | (Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 8] << 8)) * 10;
        Handle->BatteryObj.RemainingCapacity = (Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 13] | (Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 12] << 8)) * 10;
        Handle->BatteryObj.RemainingTime = (Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 17] | (Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 16] << 8));
        Handle->BatteryObj.HighestTemperature = Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 18];
        Handle->BatteryObj.LowerestTemperature = Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 19];
        Handle->BatteryObj.Warning = (Battery_Warning_e)(Handle->Ret.Raw[EP_BATT_RET_DATA_POS + 20] & Battery_Warning_OverCurrent);
        Handle->LastCommTime = Systick_GetTick();
        
        if(Handle->BatteryObj.Warning)
        {
            Handle->BatteryObj.Status = Battery_Status_FunctionalError;
        }
        else
        {
            Handle->BatteryObj.Status = Battery_Status_On;
        }
        
        if(Handle->BatteryObj.Type != Battery_TypeOwnBatt)
        {
            Handle->BatteryObj.Type = Battery_TypeOwnBatt;
        }
    }
}

void Battery_Init(Battery_Handle_t* Handle)
{	
    Handle->TrigCommTime = 0;
    Handle->LastCommTime = 0;    
	Handle->BatteryObj.Type = Battery_TypeLiOn;//Battery_TypeOwnBatt;
	Handle->BatteryObj.Status = Battery_Status_NonEpBatt;
	Handle->BatteryObj.SOC = 0;
	Handle->BatteryObj.PrevSOC = 255;
	Handle->BatteryObj.DesignedVolt = 481;
	Handle->BatteryObj.Voltage = 0;    
	Handle->BatteryObj.Current = 0;
	Handle->BatteryObj.AbsCapacity = 130000;
	Handle->BatteryObj.RemainingCapacity = 0;
	Handle->BatteryObj.RemainingCapacityByVolt = 0;
	Handle->BatteryObj.RemainingCapacityByCurr = 0;
    Handle->BatteryObj.PowerOutputTime = 0;
	Handle->BatteryObj.RemainingTime = 0;
	Handle->BatteryObj.Cell.NominalVoltage = 0;
	Handle->BatteryObj.Cell.FullVoltage = 0;
	Handle->BatteryObj.Cell.EmptyVoltage = 0;
	Handle->BatteryObj.Cell.LinearRange = 0;
	Handle->BatteryObj.Cell.ExpoRange = 0;
	Handle->BatteryObj.Cell.NumberOfCells = 13;    
    
    Handle->Cmd.Data.BattAddr = EP_BATT_ADDR;
    Handle->Cmd.Data.FuncCode = EP_BATT_FUNC;
    Handle->Cmd.Data.Cmd = Battery_DataUpdate;
    Handle->Cmd.Data.DataLen = EP_BATT_LEN;
    Handle->Cmd.Data.CS1 = EP_BATT_CRC1;
    Handle->Cmd.Data.CS2 = EP_BATT_CRC2;
    Handle->TxPtr = 0;
    Handle->CommFailCnt = 0;    
    
	QuickAvg_Init(&Battery_RemainingTime, 100, 200);
	QuickAvg_Init(&Battery_SmoothedCurrent, 10, 0);    
}

Battery_Type_e Battery_GetType(Battery_Handle_t* Handle)
{
    return Handle->BatteryObj.Type;    
}

void Battery_SetType(Battery_Handle_t* Handle, Battery_Type_e type)
{
    Handle->BatteryObj.Type = type;    
}

uint8_t Battery_GetNumOfCells(Battery_Handle_t* Handle)
{
    return Handle->BatteryObj.Cell.NumberOfCells;
}

void Battery_SetNumberOfCells(Battery_Handle_t* Handle, uint8_t num)
{
    Handle->BatteryObj.Cell.NumberOfCells = num;        
}

void Battery_UpdateSpec(Battery_Handle_t* Handle)
{
    float NumberOfCells;
    switch(Battery_Handle.BatteryObj.Type)
    {                
        case Battery_TypeLiOn:
            if((Battery_Handle.BatteryObj.DesignedVolt == 444) || (Battery_Handle.BatteryObj.DesignedVolt == 481) || (Battery_Handle.BatteryObj.DesignedVolt == 518))
                Battery_Handle.BatteryObj.Cell.NominalVoltage = BATTERY_LION_NOMINAL;  
            else if((Battery_Handle.BatteryObj.DesignedVolt == 432) || (Battery_Handle.BatteryObj.DesignedVolt == 468) || (Battery_Handle.BatteryObj.DesignedVolt == 504))
                Battery_Handle.BatteryObj.Cell.NominalVoltage = 360;  
            else if((Battery_Handle.BatteryObj.DesignedVolt == 456) || (Battery_Handle.BatteryObj.DesignedVolt == 494) || (Battery_Handle.BatteryObj.DesignedVolt == 532))
                Battery_Handle.BatteryObj.Cell.NominalVoltage = 380;
            NumberOfCells = (float)Battery_Handle.BatteryObj.DesignedVolt * 10 / Battery_Handle.BatteryObj.Cell.NominalVoltage;  
            Battery_Handle.BatteryObj.Cell.FullVoltage = BATTERY_LION_FULL;
            Battery_Handle.BatteryObj.Cell.LinearVoltage = BATTERY_LION_LINEAR;
            Battery_Handle.BatteryObj.Cell.EmptyVoltage = BATTERY_LION_EMPTY;
            Battery_Handle.BatteryObj.Cell.LinearRange = (uint8_t)(BATTERY_LION_FULL - BATTERY_LION_LINEAR);
            Battery_Handle.BatteryObj.Cell.ExpoRange = (uint8_t)(BATTERY_LION_LINEAR - BATTERY_LION_EMPTY);
            Battery_Handle.BatteryObj.LoadedVoltageDiff = BATTERY_LION_LOAD_VOLT_DIFF * NumberOfCells;
            break;
        
        case Battery_TypePb:
            NumberOfCells = (float)Battery_Handle.BatteryObj.DesignedVolt * 10 / BATTERY_PB_NOMINAL;  
            Battery_Handle.BatteryObj.Cell.FullVoltage = BATTERY_PB_FULL;
            Battery_Handle.BatteryObj.Cell.NominalVoltage = BATTERY_PB_NOMINAL;
            Battery_Handle.BatteryObj.Cell.LinearVoltage = BATTERY_PB_LINEAR;
            Battery_Handle.BatteryObj.Cell.EmptyVoltage = BATTERY_PB_EMPTY;
            Battery_Handle.BatteryObj.Cell.LinearRange = (uint8_t)(BATTERY_PB_FULL - BATTERY_PB_LINEAR);
            Battery_Handle.BatteryObj.Cell.ExpoRange = (uint8_t)(BATTERY_PB_LINEAR - BATTERY_PB_EMPTY);
            Battery_Handle.BatteryObj.LoadedVoltageDiff = BATTERY_PB_LOAD_VOLT_DIFF * NumberOfCells;
            break;
        
        case Battery_TypeLiE:
            NumberOfCells = (float)Battery_Handle.BatteryObj.DesignedVolt * 10 / BATTERY_LFE_NOMINAL;  
            Battery_Handle.BatteryObj.Cell.FullVoltage = BATTERY_LFE_FULL;
            Battery_Handle.BatteryObj.Cell.NominalVoltage = BATTERY_LFE_NOMINAL;
            Battery_Handle.BatteryObj.Cell.LinearVoltage = BATTERY_LFE_LINEAR;
            Battery_Handle.BatteryObj.Cell.EmptyVoltage = BATTERY_LFE_EMPTY;
            Battery_Handle.BatteryObj.Cell.LinearRange = (uint8_t)(BATTERY_LFE_FULL - BATTERY_LFE_LINEAR);
            Battery_Handle.BatteryObj.Cell.ExpoRange = (uint8_t)(BATTERY_LFE_LINEAR - BATTERY_LFE_EMPTY);
            Battery_Handle.BatteryObj.LoadedVoltageDiff = BATTERY_LFE_LOAD_VOLT_DIFF * NumberOfCells;
            break;
        
        case Battery_TypeA:
            break;
        case Battery_TypeB:
            break;
        case Battery_TypeC:
            break;
        case Battery_TypeD:
            break;          
        case Battery_TypeOwnBatt:
            break;                
    }
    if(NumberOfCells - (uint8_t)NumberOfCells >= 0.5f)
        Battery_Handle.BatteryObj.Cell.NumberOfCells = (uint8_t)(NumberOfCells) + 1;
    else
        Battery_Handle.BatteryObj.Cell.NumberOfCells = (uint8_t)(NumberOfCells);    
}


uint16_t Battery_GetDesignedVolt(Battery_Handle_t* Handle)
{
    return Handle->BatteryObj.DesignedVolt;       
}

void Battery_SetDesignedVolt(Battery_Handle_t* Handle, uint16_t volt)
{
    Handle->BatteryObj.DesignedVolt = volt;      
}


uint8_t Battery_GetSoc(Battery_Handle_t* Handle)
{
    return Handle->BatteryObj.SOC;
}

void Battery_SetSoc(Battery_Handle_t* Handle, uint8_t soc)
{
    Handle->BatteryObj.SOC = soc;      
}


uint8_t Battery_GetPrevSoc(Battery_Handle_t* Handle)
{
    return Handle->BatteryObj.PrevSOC;
}

void Battery_SetPrevSoc(Battery_Handle_t* Handle, uint8_t soc)
{
    Handle->BatteryObj.PrevSOC = soc;      
}


float Battery_GetRemainingCapacity(Battery_Handle_t* Handle)
{
	return Handle->BatteryObj.RemainingCapacity;
}

void Battery_SetRemainingCap(Battery_Handle_t* Handle, float Rcap)
{
    Handle->BatteryObj.RemainingCapacity = Rcap;  
}


uint32_t Battery_GetAbsCapacity(Battery_Handle_t* Handle)
{	
	return Handle->BatteryObj.AbsCapacity;
}

void Battery_SetAbsCapacity(Battery_Handle_t* Handle, uint32_t cap)
{
    Handle->BatteryObj.AbsCapacity = cap;     
}

void Battery_EstimateRemainingCapacity(Battery_Handle_t* Handle)
{
    uint8_t soc;
    uint32_t CurrTick = Systick_GetTick();
    uint16_t CurrVolt;
    float CapacityDrop;
    
    if((ProtectionSwtich_GetState() == 0) || (Handle->BatteryObj.Type == Battery_TypeOwnBatt))
        return;
        
    if((Motor_GetUbusDiff(&Motor_Handle) <= 10) && (Motor_GetUbusDiff(&Motor_Handle) >= -10))
    {
        if(Handle->BatteryObj.Current == 0)                 //Without load compensation 
            CurrVolt = Handle->BatteryObj.Voltage * 10;
        else                                                //With load compensation    
            CurrVolt = Handle->BatteryObj.Voltage * 10 + (uint32_t)(Handle->BatteryObj.LoadedVoltageDiff) * Motor_GetPower(&Motor_Handle) / 6000;
          
            Handle->BatteryObj.LoadedVolt = CurrVolt;    
        
        if(CurrVolt >= Handle->BatteryObj.Cell.NumberOfCells * Handle->BatteryObj.Cell.LinearVoltage)                        
        {
            soc = (uint8_t)((CurrVolt / Handle->BatteryObj.Cell.NumberOfCells - Handle->BatteryObj.Cell.LinearVoltage) * 90 / Handle->BatteryObj.Cell.LinearRange) + 10; 
            soc = soc > 100 ? 100: (soc < 10 ? 10: soc); 
        }  
        else if(CurrVolt >= Handle->BatteryObj.Cell.NumberOfCells * Handle->BatteryObj.Cell.EmptyVoltage)                    
        {
            soc = (uint8_t)((CurrVolt / Handle->BatteryObj.Cell.NumberOfCells - Handle->BatteryObj.Cell.EmptyVoltage) * 10 / Handle->BatteryObj.Cell.ExpoRange);                         
            soc = soc > 10 ? 10: soc; 
        }       
        else
            soc = 0; 
        Handle->BatteryObj.socbyvolt=soc;
        Handle->BatteryObj.RemainingCapacityByVolt = (float)(Handle->BatteryObj.AbsCapacity) * soc / 100;
            
        
        if(Handle->BatteryObj.Current == 0)          //Without load compensation 
        {
            if(Handle->BatteryObj.RemainingCapacityByCurr == 0.0f)          /* use volt to estimate the capacity when the system is just on */ 
            {            
                if((Handle->BatteryObj.PrevSOC <= 100) && (uintabs(Handle->BatteryObj.PrevSOC , soc) <= 15) && (Systick_GetSecond() <= 10))
                {
                    Handle->BatteryObj.RemainingCapacity = Handle->BatteryObj.PrevSOC  * Handle->BatteryObj.AbsCapacity / 100 + Handle->BatteryObj.AbsCapacity / 200;
                }
                else                    
                {
                    Handle->BatteryObj.RemainingCapacity = (float)Handle->BatteryObj.RemainingCapacityByVolt;          
                    soc = Handle->BatteryObj.RemainingCapacity * 100 / Handle->BatteryObj.AbsCapacity;
                    Handle->BatteryObj.SOC = soc;
                }
            }
            else    /* RcapDiffWithoutLoad >0 when RcapVolt>RcapCurr, probably a larger measure assuming RcapVolt is accurate. */            
            {                
                Handle->BatteryObj.RcapDiffWithoutLoad = Handle->BatteryObj.RemainingCapacityByVolt - Handle->BatteryObj.RemainingCapacityByCurr;
            }
        }
        else        
        {
            Handle->BatteryObj.RcapDiffWithLoad = Handle->BatteryObj.RemainingCapacityByVolt - Handle->BatteryObj.RemainingCapacityByCurr;

            /*  
                1/360 = 1/10*1000 * 100/1000/3600
                convert 0.1A to mA: 1/10*1000
                convert 100ms to hr: 100/1000/3600
            */
            
            CapacityDrop = (float)Handle->BatteryObj.Current / 360;  
            Handle->BatteryObj.Drop = CapacityDrop;
            
            if(Handle->BatteryObj.RcapDiffWithoutLoad != 0.0f)       //Without load compensation has a higher priority than With load compensation
            {              
                if(fabs(Handle->BatteryObj.RcapDiffWithoutLoad) < 100.0f)
                    Handle->BatteryObj.RcapDiffWithoutLoad = 0.0f;
                else if(Handle->BatteryObj.RcapDiffWithoutLoad > CapacityDrop * 50)
                    Handle->BatteryObj.RcapDiffWithoutLoad = CapacityDrop * 50;
                else if(Handle->BatteryObj.RcapDiffWithoutLoad < -CapacityDrop * 50)
                    Handle->BatteryObj.RcapDiffWithoutLoad = -CapacityDrop * 50;
                
                Handle->BatteryObj.TotalDrop = CapacityDrop - Handle->BatteryObj.RcapDiffWithoutLoad / 50;
                Handle->BatteryObj.RemainingCapacityByCurr = (float)Handle->BatteryObj.RemainingCapacity - Handle->BatteryObj.TotalDrop;         
                Handle->BatteryObj.RcapDiffWithoutLoad -= Handle->BatteryObj.RcapDiffWithoutLoad / 50;                
            }
            else
            {       
                if(fabs(Handle->BatteryObj.RcapDiffWithLoad) < 100.0f)
                    Handle->BatteryObj.RcapDiffWithLoad = 0.0f;                
                else if(Handle->BatteryObj.RcapDiffWithLoad > CapacityDrop * 50)
                    Handle->BatteryObj.RcapDiffWithLoad = CapacityDrop * 50;  
                else if(Handle->BatteryObj.RcapDiffWithLoad < -CapacityDrop * 50)
                    Handle->BatteryObj.RcapDiffWithLoad = -CapacityDrop * 50;  
                
                Handle->BatteryObj.TotalDrop = CapacityDrop - Handle->BatteryObj.RcapDiffWithLoad / 100;
                Handle->BatteryObj.RemainingCapacityByCurr = (float)Handle->BatteryObj.RemainingCapacity - Handle->BatteryObj.TotalDrop;
                Handle->BatteryObj.CompensationDrop = Handle->BatteryObj.RcapDiffWithLoad / 100;
                Handle->BatteryObj.RcapDiffWithLoad -= Handle->BatteryObj.RcapDiffWithLoad / 100;               
            }
            
            Handle->BatteryObj.RemainingCapacity = (uint32_t)(Handle->BatteryObj.RemainingCapacityByCurr);
        }   
        soc = (uint8_t)(Handle->BatteryObj.RemainingCapacity * 100 / Battery_GetAbsCapacity(&Battery_Handle));
        soc = soc > 100 ? 100: soc;             
        Handle->BatteryObj.SOC = soc;
    }
}

uint16_t Battery_GetStatus(Battery_Handle_t* Handle)
{
	return Handle->BatteryObj.Status;
}

uint16_t Battery_GetRemainingDistance(Battery_Handle_t* Handle)
{
	return (uint16_t)((uint32_t)(Battery_GetRemainingTime(Handle)) * GPS_GetGSpeedCM(&Gps_Handle) * 60 / 1000);
}

void Battery_SetTravelledTimeOffset(Battery_Handle_t* Handle, uint32_t Offset)
{
   Handle->BatteryObj.StoredPowerOutputTime = Offset * 12000;//分钟转换成毫秒？
}

uint32_t Battery_GetTravelledTime(Battery_Handle_t* Handle)
{
   return Handle->BatteryObj.PowerOutputTime ;//+ Handle->BatteryObj.StoredPowerOutputTime;
}

void Battery_AddTravelledTime(Battery_Handle_t* Handle, uint32_t Increment)
{
   Handle->BatteryObj.PowerOutputTime += Increment;//单位毫秒？
}

uint16_t Battery_GetRemainingTime(Battery_Handle_t* Handle)
{
    uint16_t RemainingTime;
    if(Handle->BatteryObj.Type == Battery_TypeOwnBatt)
        return Handle->BatteryObj.RemainingTime;
    else
    {
        if(Handle->BatteryObj.Current == 0)
        {
            Handle->BatteryObj.RemainingTime = 0;
            QuickAvg_Set(&Battery_RemainingTime, 0);
        }
        else 
        {
            RemainingTime = Handle->BatteryObj.RemainingCapacity * 60 / (QuickAvg_Get(&Battery_SmoothedCurrent) * 100);
            RemainingTime = RemainingTime > 300? 300: RemainingTime;
            
            //set the current directly to avoid slow change
            if(uintabs(RemainingTime, Battery_RemainingTime.Mean) > 3)
                QuickAvg_Set(&Battery_RemainingTime, RemainingTime);
            else            
                Handle->BatteryObj.RemainingTime = QuickAvg_Update(&Battery_RemainingTime, RemainingTime);
        }
        return Handle->BatteryObj.RemainingTime;
    }
}

uint16_t Battery_GetVoltage(Battery_Handle_t* Handle)
{    
    return Handle->BatteryObj.Voltage;
}

void Battery_SetVoltage(Battery_Handle_t* Handle, uint16_t Voltage)
{    
    Handle->BatteryObj.Voltage = Voltage;
}

uint16_t Battery_GetCurrent(Battery_Handle_t* Handle)
{
    return Handle->BatteryObj.Current;
}

void Battery_SetCurrent(Battery_Handle_t* Handle, uint16_t Current)
{
    if(Current == 0)
    {
        Handle->BatteryObj.Current = 0;
        QuickAvg_Set(&Battery_SmoothedCurrent, 0);        
    }
    else
    {
        Handle->BatteryObj.Current = Current;
        QuickAvg_Update(&Battery_SmoothedCurrent, Current);   
    }        
}

uint16_t Battery_GetPower(Battery_Handle_t* Handle)
{
    return Handle->BatteryObj.Power;
}

void Battery_SetPower(Battery_Handle_t* Handle, uint16_t Power)
{
	Handle->BatteryObj.Power = Power;
}


uint8_t Battery_GetHighestTemp(Battery_Handle_t* Handle)
{
    return Handle->BatteryObj.HighestTemperature;
}

uint8_t Battery_GetLowestTemp(Battery_Handle_t* Handle)
{
    return Handle->BatteryObj.LowerestTemperature;
}

