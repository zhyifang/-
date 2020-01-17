#include "nrf_gpio.h"
#include "ep_temp.h"
#include "ep_comm.h"
#include "ep_version.h"

uint8_t TempIndex = 0;                      //0--> motor; 1-->mos1; 2-->mos2
Temperature_t TempArray [9];                //array of temperature structs
static uint8_t TempOverheatFlag = 0;        //0--> not overheated; 1--> overheated; 
static uint8_t PowerLimitationFlag = 0;       //0--> no power limitation; 1--> with power limitation; 

static uint16_t TempAdValueTable[17] = {989,971,946,913,872,
	                                   823,766,704,639,573,
                                       508,447,391,340,295,
                                       255,221};
uint16_t MOT_OT_LIMIT,MOT_OT_TRIGGER,MOS_OT_LIMIT,MOS_OT_TRIGGER;
		
Temp_Index_e  MotProtectTempIndex,PowMosProtectTempIndex,DrvMosProtectTempIndex;
									   
uint32_t Temp_ConfigChannel(uint8_t ch);         

void Temp_TriggerAdcSampling(void)
{
	NRF_ADC->TASKS_START = 1;
}

void Temp_Init(void)
{				
	MotProtectTempIndex =  TMOT;
	PowMosProtectTempIndex = TMOS_P1;
	DrvMosProtectTempIndex = TMOS_A;
	
	NRF_ADC->INTENSET = ADC_INTENSET_END_Msk;
	NVIC_SetPriority(ADC_IRQn,2);
	NVIC_EnableIRQ(ADC_IRQn);	
	
	NRF_ADC->CONFIG	= Temp_ConfigChannel(TempIndex);					
	
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;	
	
	NRF_ADC->EVENTS_END  = 0;    
    Temp_TriggerAdcSampling();

	for(uint8_t i = 0; i < 8; i++)
	{
        QuickAvg_Init(&TempArray[i].MovingAvgStruct, 5, 0);       //adc value 880 corresponds to 15 degrees
        TempArray[i].ThrottlePercentage = 100;
    }
	
	TempProtectValueCheck();
}


void TempProtectValueCheck(void)
{
	if(navy_commrelay_msg.product_model == product_navy_60)
	{
		MOT_OT_LIMIT     = 1200;
		MOT_OT_TRIGGER   = MOT_OT_LIMIT - 100;
		
		MOS_OT_LIMIT     = 1050;
		MOS_OT_TRIGGER   = MOS_OT_LIMIT - 100;
	}
	else if(navy_commrelay_msg.product_model == product_navy_30)
	{
		MOT_OT_LIMIT     = 1050;
		MOT_OT_TRIGGER   = MOT_OT_LIMIT - 100;
		
		MOS_OT_LIMIT     = 1050;
		MOS_OT_TRIGGER   = MOS_OT_LIMIT - 100;
	}
}

void ADC_IRQHandler(void)
{
	uint16_t TempData;
	if (NRF_ADC->EVENTS_END != 0)
	{		
		NRF_ADC->TASKS_STOP = 1;	

        //update the moving average window
		TempData = NRF_ADC->RESULT;
		QuickAvg_Update(&TempArray[TempIndex].MovingAvgStruct, TempData);       

        //configure for the next channel
		TempIndex=(TempIndex + 1) % 8;              
		NRF_ADC->CONFIG	= Temp_ConfigChannel(TempIndex);

		NRF_ADC->EVENTS_END = 0;	
	}
}

uint32_t Temp_ConfigChannel(uint8_t ch)
{	
    uint32_t ChannelConfig = (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos)
                            |(ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) 
	                      |(ADC_CONFIG_REFSEL_SupplyOneThirdPrescaling << ADC_CONFIG_REFSEL_Pos) ;
	switch(ch)
	{
		case 0:	//p0.27 tmot	
			ChannelConfig |= (ADC_CONFIG_PSEL_AnalogInput1 << ADC_CONFIG_PSEL_Pos);	
			break;
		case 1:		//p0.03 TMOS_B
			ChannelConfig |= (ADC_CONFIG_PSEL_AnalogInput4 << ADC_CONFIG_PSEL_Pos);		
			break;
		case 2:		//p0.04 tmosp2
			ChannelConfig |= (ADC_CONFIG_PSEL_AnalogInput5 << ADC_CONFIG_PSEL_Pos);		
			break;
		case 3:		//p0.05 TMOS_A
			ChannelConfig |= (ADC_CONFIG_PSEL_AnalogInput6 << ADC_CONFIG_PSEL_Pos);		
			break;
		case 4:		//p0.06 tmosp1
			ChannelConfig |= (ADC_CONFIG_PSEL_AnalogInput7 << ADC_CONFIG_PSEL_Pos);		
			break;
		case 5:		//p0.02 TMOS_CL
			
			ChannelConfig |= (ADC_CONFIG_PSEL_AnalogInput3 << ADC_CONFIG_PSEL_Pos);			
			break;	
		case 6:		//p0.01 vdd
			ChannelConfig |= (ADC_CONFIG_PSEL_AnalogInput2 << ADC_CONFIG_PSEL_Pos);		
			break;
		case 7:		//p0.26 v15v
			ChannelConfig |= (ADC_CONFIG_PSEL_AnalogInput0 << ADC_CONFIG_PSEL_Pos);		
			break;	
	}	
	return ChannelConfig;
}


int16_t   TempCacul(uint16_t TempAdc)
{
	int16_t Temp;

	if((TempAdc <= 1016 )&&(TempAdc >= 1011))
	{
		Temp = (int16_t)(10*(1802 - TempAdc*1.8023));
		return Temp;
	}
	else if((TempAdc <= 1010 )&&(TempAdc >= 1002))
	{
		Temp = (int16_t)(10*(1152 - TempAdc*1.1589));
		return Temp;
	}
	else if((TempAdc <= 1002 )&&(TempAdc >= TempAdValueTable[0]))
	{
		Temp = (int16_t)(10*(770.95 - TempAdc*0.7788));
		return Temp;
	}
	else if((TempAdc < TempAdValueTable[0] )&&(TempAdc >= TempAdValueTable[16]))
	{
			for(uint8_t i = 0;i < 16;i++)
			{
				if((TempAdc < TempAdValueTable[i] )&&(TempAdc >= TempAdValueTable[i + 1]))
				{
					Temp = i*100 + (TempAdValueTable[i] - TempAdc) * 100 /(TempAdValueTable[i] - TempAdValueTable[i + 1]);
					return Temp;
				}
			}
	}
	return (-500);
}

int16_t Temp_Convert(uint16_t value, B_Value coe)
{
    static const uint16_t temp_table_3435[] = {936,934,931,926,918,
		                                       906,889,867,837,799,
	                                           754,702,645,585,525,
	                                           465,409,358,311,270,
	                                           234,202,175,152,132,
	                                           116};  // b=3435 from -50 to 200 degrees with 10-degree step

	uint16_t* temp_table;
  	int16_t temp = 0;
  	uint16_t i = 0;

  	if(coe == (B_Value)(B_3435))
  		temp_table = (uint16_t*)temp_table_3435;

  	if(value >= temp_table[0])
		temp = -500;
  	else if(value < temp_table[25])
		temp = (24 * 100 + (temp_table[24] - value) * 100 / (temp_table[24] - temp_table[25]));
	else
  	{
		while(i < 25)
		{
			if(value < temp_table[i])
				i++;
			else
			{
				temp = (((int16_t)(i) - 4) * 100 + (temp_table[i - 1] - value) * 100 / (temp_table[i - 1] - temp_table[i]));
				break;
			}
		}
  	}
  	return temp;
}

void Temp_Update(void)	
{
    uint8_t HighTempValue;
    /***********************************************************/
	//ntc b3435  sensor, motor temperature,B phase  temperature,power temperature,A phase
	/*
		TMOT,		 //motor temperature  temp_row1
		TMOS_B, 	 //NTC 10K	B phase 			temp1_row2									 
		TMOS_P1,	 //NTC 10K	power	  temp_row4
		TMOS_A, 	 //NTC 10K	A phase   temp_row3
		TMOS_P2L,	 //LMT87	power				temp1_row4
		TMOS_CL,	 //LMT87	C phase 			temp1_row3
		VDD,		 //Voltage	3.3v
		V15V,		 //Voltage	15V
		TMOTNEW,	 //motor temperature new        temp1_row1

	*/
	for(uint8_t i = 0;i < 4;i++ )
	{
		TempArray[i].Temperature = TempCacul(TempArray[i].MovingAvgStruct.Mean);
	}
	//lmt87 sensor,power temperature,A phase
	for(uint8_t i = 4;i < 6;i++ )
	{
		if(TempArray[i].MovingAvgStruct.Mean > 810)
		{
			TempArray[i].Temperature = (int16_t)(10*(814.03 - TempArray[i].MovingAvgStruct.Mean) / 5.221);
		}
		else
		{
			TempArray[i].Temperature = (uint16_t)(10*(814.03 - TempArray[i].MovingAvgStruct.Mean) / 4.221);		
		}	 
	}
	//vol sense 
	TempArray[6].Vol = TempArray[6].MovingAvgStruct.Mean * 332 / 10240;
    TempArray[7].Vol = TempArray[7].MovingAvgStruct.Mean * 363 / 10240;
	
    /***********************************************************/
	//motor OT protect 
    if(TempArray[MotProtectTempIndex].Temperature <= MOT_OT_TRIGGER)
    {
        PowerLimitationFlag &= ~MOT_OT_BIT; 
        TempOverheatFlag &= ~MOT_OT_BIT;  
        TempArray[MotProtectTempIndex].ThrottlePercentage = 100;   
    }
    else if((TempArray[MotProtectTempIndex].Temperature > MOT_OT_TRIGGER) && (TempArray[MotProtectTempIndex].Temperature <= MOT_OT_LIMIT))
    {    
        PowerLimitationFlag |= MOT_OT_BIT; 
        TempOverheatFlag &= ~MOT_OT_BIT;                 
        HighTempValue = MOT_OT_LIMIT - TempArray[MotProtectTempIndex].Temperature;
        HighTempValue = HighTempValue > 100 ? 100 : HighTempValue;
		//HighTempValue: 0-100
        TempArray[MotProtectTempIndex].ThrottlePercentage = HIGH_TEMP_POWER_LIMIT + HighTempValue /10 * POWER_LIMIT_FACTOR;
    }
    else    //TempArray[0].Temperature > MOT_OT_LIMIT
    {
        TempOverheatFlag |= MOT_OT_BIT;        
    }

	//power mos OT protect 
    if(TempArray[PowMosProtectTempIndex].Temperature <= MOS_OT_TRIGGER)
    {
        PowerLimitationFlag &= ~MOS1_OT_BIT; 
        TempOverheatFlag &= ~MOS1_OT_BIT;    
        TempArray[PowMosProtectTempIndex].ThrottlePercentage = 100;   
    }
    else if((TempArray[PowMosProtectTempIndex].Temperature > MOS_OT_TRIGGER) && (TempArray[PowMosProtectTempIndex].Temperature <= MOS_OT_LIMIT))
    {    
        PowerLimitationFlag |= MOS1_OT_BIT;  
        TempOverheatFlag &= ~MOS1_OT_BIT;                   
        HighTempValue = MOS_OT_LIMIT - TempArray[PowMosProtectTempIndex].Temperature;
        HighTempValue = HighTempValue > 100 ? 100 : HighTempValue;
        TempArray[PowMosProtectTempIndex].ThrottlePercentage = HIGH_TEMP_POWER_LIMIT + HighTempValue / 10  * POWER_LIMIT_FACTOR;
    }
    else    //TempArray[1].Temperature > MOS1_OT_BIT
    {
        TempOverheatFlag |= MOS1_OT_BIT;        
    }
	
    //driver mos OT protect 
    if(TempArray[DrvMosProtectTempIndex].Temperature <= MOS_OT_TRIGGER)
    {
        PowerLimitationFlag &= ~MOS2_OT_BIT;    
        TempOverheatFlag &= ~MOS2_OT_BIT;   
        TempArray[DrvMosProtectTempIndex].ThrottlePercentage = 100;   
    }
    else if((TempArray[DrvMosProtectTempIndex].Temperature > MOS_OT_TRIGGER) && (TempArray[DrvMosProtectTempIndex].Temperature <= MOS_OT_LIMIT))
    {    
        PowerLimitationFlag |= MOS2_OT_BIT;  
        TempOverheatFlag &= ~MOS2_OT_BIT;                   
        HighTempValue = MOS_OT_LIMIT - TempArray[DrvMosProtectTempIndex].Temperature;
        HighTempValue = HighTempValue > 100 ? 100 : HighTempValue;
        TempArray[DrvMosProtectTempIndex].ThrottlePercentage = HIGH_TEMP_POWER_LIMIT + HighTempValue / 10 * POWER_LIMIT_FACTOR;		
    }
    else    //TempArray[2].Temperature > MOS2_OT_BIT
    {
        TempOverheatFlag |= MOS2_OT_BIT;        
    }
    
    if(PowerLimitationFlag)     
    {    
    #if 1
        //mot		
        if((TempArray[MotProtectTempIndex].ThrottlePercentage <= TempArray[PowMosProtectTempIndex].ThrottlePercentage)
		&& (TempArray[MotProtectTempIndex].ThrottlePercentage <= TempArray[DrvMosProtectTempIndex].ThrottlePercentage))
		{
			Comm_SetThrottlePercentageTemp(&Comm_Handle, TempArray[MotProtectTempIndex].ThrottlePercentage);
		}
         //power mos        
        else if((TempArray[PowMosProtectTempIndex].ThrottlePercentage <= TempArray[MotProtectTempIndex].ThrottlePercentage)
			&&  (TempArray[PowMosProtectTempIndex].ThrottlePercentage <= TempArray[DrvMosProtectTempIndex].ThrottlePercentage))
		{
			Comm_SetThrottlePercentageTemp(&Comm_Handle, TempArray[PowMosProtectTempIndex].ThrottlePercentage); 
		}
        //driver mos         
        else if((TempArray[DrvMosProtectTempIndex].ThrottlePercentage <= TempArray[MotProtectTempIndex].ThrottlePercentage) 
		    &&  (TempArray[DrvMosProtectTempIndex].ThrottlePercentage <= TempArray[PowMosProtectTempIndex].ThrottlePercentage))
		{
			Comm_SetThrottlePercentageTemp(&Comm_Handle, TempArray[DrvMosProtectTempIndex].ThrottlePercentage);
		}
      #endif            
    }
    else
        Comm_SetThrottlePercentageTemp(&Comm_Handle, 100);

}



//return the temperature for channel with precision 0.1degree
int16_t Temp_Get(uint8_t channel)	
{
    return TempArray[channel].Temperature;
}
void Temp_Set(uint8_t channel,int16_t temp)	
{
    TempArray[channel].Temperature = temp;
}

uint8_t Temp_IsOverheated(void)
{    
    if(TempOverheatFlag)
        return 1;
    else
        return 0;
}

