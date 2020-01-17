#include "ep_gps.h"
#include "ep_tick.h"
#include "nrf_delay.h"
#include "ep_pstorage.h"
#include "ep_motor.h"
#include "ep_battery.h"
#include "time.h"
	
#define GPS_I2C_TIMEOUT		200	
#define GPS_IIC_SCL_PIN     29
#define GPS_IIC_SDA_PIN     28



volatile uint8_t GpsRxBuf [250];
volatile uint8_t* Gps_AlignedBufPtr;
extern GPS_Handle_t Gps_Handle;
extern Motor_Handle_t Motor_Handle;
extern Battery_Handle_t Battery_Handle;
extern twi_config_t cfg;
extern PSTORAGE_Handle_t PSTORAGE_Handle;
//extern PSTORAGE_Obj StoredTandCData;	
	
void GPS_CFG_GNSS(void);
void GPS_CFG_INF(void);
void GPS_CFG_NAV5(void);
void GPS_CFG_NAVX5(void);
void GPS_CFG_ODO(void);
void GPS_CFG_RATE(void);
void GPS_CFG_PRT(void);
void GPS_CFG_RST(void);
void GPS_CFG_CFG(void);
void GPS_CFG_MSG(const uint8_t msg_class,const uint8_t msg_id, const uint8_t msg_rate);
void GPS_RST_ODO(void);

void GPS_ClearData(GPS_Handle_t* Handle)
{
	Handle->TravelledDistance = 0;	 
	Handle->longitude = 0;
	Handle->latitude = 0;
	Handle->fixType = 0;
	Handle->numSV = 0;
	Handle->headMot = 0;
	Handle->pdop = 0;
	Handle->gSpeed = 0;
	Handle->TTFF = 0;
	Handle->timeStarted = 0;
	Handle->gdop = 0;
	Handle->pAcc = 0;
	Handle->sAcc = 0;
	Handle->dAcc = 0;
}

void GPS_SettingConfig(GPS_Handle_t* Handle, uint8_t Options)
{	
    Handle->CommState = 0;
	Handle->CommFailCnt = 0;  
	Handle->CommFailCntCummulated = 0;  
	Handle->FirstPackLocation = 0; 
	Handle->DataMisalignCnt = 0;     
	Handle->RxLength = 0;
	Handle->DataPtr = 0;
	Handle->RecoveryFailCnt = 0;
	Handle->ReceivedMsg = 0;
	Handle->DecodedMsg = 0;
	Handle->iTOW = 999;
    Handle->WeekNum = 999;
    
    GPS_ClearData(Handle);
	
	QuickAvg_Init(&Handle->AvgSpeed, 2, 0);
	GPS_CFG_GNSS();
	GPS_CFG_INF();
	GPS_CFG_NAV5();
	GPS_CFG_NAVX5();
	GPS_CFG_ODO();
	GPS_CFG_RATE();
	GPS_CFG_PRT();
    if(Options & GPS_INIT_RESET_ODO)
        GPS_RST_ODO();
	
	//Set the required information
	GPS_CFG_MSG(GPS_CLASS_NAV,GPS_NAV_ODO, 1);		//odometer
	GPS_CFG_MSG(GPS_CLASS_NAV,GPS_NAV_PVT, 1);		//position velocity time
	GPS_CFG_MSG(GPS_CLASS_NAV,GPS_NAV_STATUS, 1);	//ttff timestarted
	GPS_CFG_MSG(GPS_CLASS_NAV,GPS_NAV_SOL, 1);		//gdop
    
    GPS_CFG_CFG();    
}

void GPS_Init(GPS_Handle_t* Handle, uint8_t Options)
{  	
	cfg.blocking_mode = TWI_BLOCKING_DISABLED;      //interrupt mode
	cfg.frequency = TWI_FREQ_250KHZ;                //fclk=250khz
	cfg.pinselect_scl = GPS_IIC_SCL_PIN;
	cfg.pinselect_sda = GPS_IIC_SDA_PIN;    
	twi_master_init(&cfg);
    Systick_Delay(650);
    GPS_SettingConfig(Handle, Options);
}

void GPS_Update(GPS_Handle_t* Handle)
{
	static uint32_t GpsStartTime;
	static uint8_t RegAdd;
	static uint8_t len[2];
 #if 0       
    if(Handle->CommFailCntCummulated < 100) 
        Handle->CommFailCntCummulated++;  
    else                                        //error exists or gps is disconnected
    {
        Handle->numSV = 255;
        return;
    }
#endif
	switch(Handle->CommState)
	{
		case 0: 	//get the rx length			
			RegAdd = GPS_I2C_RX_LEN;		
			twi_master_write_read(GPS_I2C_DEV_ADDR, &RegAdd, 1, len, 2);
        
			if(len[1] <= Handle->RxLength)              
			{	
				Handle->CommState++;
                if(Handle->CommFailCnt < 5) 
                    Handle->CommFailCnt++; 
//                else
//                    GPS_ClearData(Handle);
                
                if(Handle->CommFailCnt > 2) 
                {
                    Handle->ReceivedMsg = 0;
                    Handle->DecodedMsg = 0;
                }
				Handle->DataPtr = 0;
				GpsStartTime = Systick_GetTick();
				RegAdd = GPS_I2C_RX_DATA;
			}
			else
				break;
            
		case 1:		//start the transmission	
			twi_master_write_read(GPS_I2C_DEV_ADDR, &RegAdd, 1, (uint8_t*)GpsRxBuf, Handle->RxLength);
			Handle->CommState++;
			break;
        
		case 2:		//receive the data
			if(Systick_GetTick() - GpsStartTime > GPS_I2C_TIMEOUT)
			{
				Handle->DataPtr = 0;
				Handle->CommState = 0;
			}				
			break;
            
		case 3:		//decode the data	
			Handle->DataPtr = 0;
			Handle->CommState = 0;
			break;		
	}
	    if(Handle->CommFailCntCummulated < 100) 
        Handle->CommFailCntCummulated++;  
    else                                        //error exists or gps is disconnected
    {
        Handle->numSV = 255;
        return;
    }
}

void GPS_Decode(GPS_Handle_t* Handle)
{
	uint8_t	CK_A = 0, CK_B = 0;
	uint8_t CurrPackLocation, CurrPackLen;
	uint16_t i = 0, j = 0;
    uint32_t GroundSpeed = 0;
    
	
	//locate the first header
	while((GpsRxBuf[i] != GPS_SYNC_1) && (i < sizeof(GpsRxBuf)))  
		i++;
	
	//terminate the data stream for no correct header found
	if(i >= sizeof(GpsRxBuf))                                
		return;
	
	Gps_AlignedBufPtr = &GpsRxBuf[i];

    Handle->CommFailCnt = 0;
    Handle->CommFailCntCummulated = 0;
    Handle->RecoveryFailCnt = Handle->RecoveryFailCnt > 0? Handle->RecoveryFailCnt - 1: Handle->RecoveryFailCnt;
	Handle->FirstPackLocation = i;
    
    if((GpsRxBuf[0] != GPS_SYNC_1) && (GpsRxBuf[1] != GPS_SYNC_2))
    {
        if(Handle->DataMisalignCnt++ >= 5)
        {    
            TWI->ENABLE = 0;   
//            GPS_ClearData(Handle);          
            if(i >= 10)
                nrf_delay_ms(100);
            else
                nrf_delay_us(1);
            TWI->ENABLE = 1;
            Handle->DataMisalignCnt = 0;
        }
    }
    else
        Handle->DataMisalignCnt = 0;
        
	i = 0;    
    
	//decode the messages for each transmission Gps_AlignedBufPtr[0]
	while(1)
	{
		if((Gps_AlignedBufPtr[i] == GPS_SYNC_1) && (Gps_AlignedBufPtr[i+1] == GPS_SYNC_2) && (Gps_AlignedBufPtr[i+2] == GPS_CLASS_NAV))
		{
            switch(Gps_AlignedBufPtr[i + 3])
            {
                case GPS_NAV_STATUS:
                    Handle->ReceivedMsg |= 0x01;
                    break;
                case GPS_NAV_SOL:					
                    Handle->ReceivedMsg |= 0x02;
                    break;
                case GPS_NAV_PVT:					
                    Handle->ReceivedMsg |= 0x04;
                    break;	
                case GPS_NAV_ODO:					
                    Handle->ReceivedMsg |= 0x08;
                    break;	
                case GPS_NAV_DOP:
                    Handle->ReceivedMsg |= 0x10;
                    break;
            }
			CurrPackLocation = i;	
            if(i + 4 < 300)
                CurrPackLen = Gps_AlignedBufPtr[i + 4];
			CK_A = CK_B = 0;
			
            //calculate the checksum for the current data pack
			for(j = CurrPackLocation + 2; j < CurrPackLocation+CurrPackLen + 6; j++)
			{
				CK_A = (CK_A + Gps_AlignedBufPtr[j]) & 0xff;
				CK_B = (CK_A + CK_B) & 0xff;
			}
			
			if((CK_A == Gps_AlignedBufPtr[CurrPackLocation + CurrPackLen + 6]) && (CK_B == Gps_AlignedBufPtr[CurrPackLocation+CurrPackLen+7]))
			{
				switch(Gps_AlignedBufPtr[CurrPackLocation + 3])
				{
					case GPS_NAV_STATUS:
						Handle->TTFF = ((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+14] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+15]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+16]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+17]) << 24)) / 1000;
						Handle->timeStarted = ((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+18] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+19]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+20]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+21]) << 24)) / 1000;
						Handle->DecodedMsg |= 0x01;
						break;
                    
					case GPS_NAV_SOL:					
						Handle->pAcc = (uint32_t)((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+30] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+31]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+32]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+33]) << 24));
                        Handle->WeekNum = (uint16_t)Gps_AlignedBufPtr[CurrPackLocation+14] | ((uint16_t)(Gps_AlignedBufPtr[CurrPackLocation+15]) << 8);
                        Handle->DecodedMsg |= 0x02;
						break;
                    
					case GPS_NAV_PVT:	
						Handle->iTOW = ((uint32_t)((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+6] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+7]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+8]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+9]) << 24))) / 1000;
                        Handle->fixType = Gps_AlignedBufPtr[CurrPackLocation+26];
                        Handle->numSV = Gps_AlignedBufPtr[CurrPackLocation+29];			
						Handle->longitude = (int32_t)((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+30] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+31]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+32]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+33]) << 24));
						Handle->latitude = (int32_t)((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+34] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+35]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+36]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+37]) << 24));
						GroundSpeed = (uint32_t)((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+66] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+67]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+68]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+69]) << 24))/10;
						Handle->headMot = (int32_t)((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+70] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+71]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+72]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+73]) << 24))/1000;
						Handle->sAcc = (uint32_t)((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+74] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+75]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+76]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+77]) << 24))/10;
						Handle->pdop = ((uint16_t)Gps_AlignedBufPtr[CurrPackLocation+82] | ((uint16_t)(Gps_AlignedBufPtr[CurrPackLocation+83]) << 8));
						Handle->DecodedMsg |= 0x04;
						Handle->gSpeed = QuickAvg_Update(&Handle->AvgSpeed, GroundSpeed);
                        Handle->Time = (Handle->WeekNum * 604800 + Handle->iTOW) / 60;//分钟为单位？
						break;	
                        
					case GPS_NAV_ODO:					
						Handle->TravelledDistance = (uint32_t)((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+14] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+15]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+16]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+17]) << 24));
						Handle->dAcc = (uint32_t)((uint32_t)Gps_AlignedBufPtr[CurrPackLocation+22] | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+23]) << 8) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+24]) << 16) | ((uint32_t)(Gps_AlignedBufPtr[CurrPackLocation+25]) << 24));
						Handle->DecodedMsg |= 0x08;
						break;	
                    
					case GPS_NAV_DOP:
						Handle->DecodedMsg |= 0x10;
						Handle->gdop = ((uint16_t)Gps_AlignedBufPtr[CurrPackLocation+10] | ((uint16_t)(Gps_AlignedBufPtr[CurrPackLocation+11]) << 8));	
						break;
                    
					default:
						break;
				}
			}
			//move to next data pack
			i += (CurrPackLen + 8);
		}
		else
			i++;
        
		if(i >= (Handle->RxLength + Handle->FirstPackLocation))
			break;
	}
}

uint16_t GPS_GetGSpeedCM(GPS_Handle_t* Handle)
{
	return Handle->gSpeed;
}

uint8_t GPS_GetNumSV(GPS_Handle_t* Handle)
{
	return Handle->numSV;
}

uint32_t GPS_GetLatitude(GPS_Handle_t* Handle)
{
	return Handle->latitude;
}

uint32_t GPS_GetLongtitude(GPS_Handle_t* Handle)
{
	return Handle->longitude;
}

int16_t GPS_GetHeading(GPS_Handle_t* Handle)
{
	return Handle->headMot;
}

uint32_t GPS_GetTimeOfWeek(GPS_Handle_t* Handle)
{
	return Handle->iTOW;
}

uint16_t GPS_GetWeekNum(GPS_Handle_t* Handle)
{
	return Handle->WeekNum;
}

uint32_t GPS_GetOwnTime(GPS_Handle_t* Handle)
{
	return Handle->Time;
}

uint16_t GPS_GetPDop(GPS_Handle_t* Handle)
{
	return Handle->pdop;
}

uint8_t GPS_GetTTFF(GPS_Handle_t* Handle)
{
	return Handle->TTFF;
}

uint16_t GPS_GetPAcc(GPS_Handle_t* Handle)
{
	return Handle->pAcc;
}

uint16_t GPS_GetSAcc(GPS_Handle_t* Handle)
{
	return Handle->sAcc;
}

void GPS_SetPrevTravelledDistance(GPS_Handle_t* Handle, uint16_t PrevDistance)
{
	Handle->StoredTravelledDistance = PrevDistance * 10;
}

uint16_t GPS_GetTravelledDistance(GPS_Handle_t* Handle)
{
	return (Handle->TravelledDistance) ;//+ Handle->StoredTravelledDistance);
}

uint8_t Gps_GetFailCount(GPS_Handle_t* Handle)
{
	return Handle->CommFailCnt;
}

uint8_t Gps_GetFailCountCummulated(GPS_Handle_t* Handle)
{
	return Handle->CommFailCntCummulated;
}

void GPS_Config(uint8_t* buf, const uint8_t size)
{
	uint8_t CK_A = 0, CK_B = 0;
	uint8_t i;
	
	for(i = 2; i<(size-2); i++)
	{
		CK_A = CK_A + buf[i];
		CK_B = CK_B + CK_A;
	}
	buf[size-2] = CK_A;
	buf[size-1] = CK_B;	

	twi_master_write(GPS_I2C_DEV_ADDR, buf, size);	
}

//config the use of satellite systems
void GPS_CFG_GNSS(void)
{
	uint8_t msg[] = {0xB5,0x62,  0x06,0x3E,  0x2C,0x00,  0x00,  0x20,  0x20,  0x05,  
					0x00,  0x08,  0x10,  0x00,  0x01,0x00,0x01,0x01,  	//gps
					0x01,  0x01,  0x03,  0x00,  0x01,0x00,0x01,0x01,  	//sbas
					0x03,  0x08,  0x10,  0x00,  0x00,0x00,0x01,0x01,  	//beidou
					0x05,  0x00,  0x03,  0x00,  0x01,0x00,0x01,0x01,	//qzss
					0x06,  0x08,  0x0E,  0x00,  0x01,0x00,0x01,0x01,	//glonass
					0x00,0x00};
	GPS_Config(msg, sizeof(msg));
}

//config using ublox protocol for port 0 ddc
void GPS_CFG_INF(void)
{
	uint8_t msg[] = {0xB5,0x62,  0x06,0x02,  0x0A,0x00,  0x00,  0x00,0x00,0x00,  
					0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00};	
	GPS_Config(msg, sizeof(msg));
}

//config nav engine settings
void GPS_CFG_NAV5(void)
{
	uint8_t msg[] = {0xB5,0x62,  0x06,0x24,  0x24,0x00,  0xFF,0xFF,  0x05,  0x03,  
					0x00,0x00,0x00,0x00,  0x10,0x27,0x00,0x00,  0x05,  0x00,  0xFA,0x00,  
					0xFA,0x00,  0x64,0x00,  0x2C,0x01,  0x00,  0x3C,  0x00,  0x00,  
					0x00,0x00,  0xC8,0x00,  0x00,  0x00,0x00,0x00,0x00,0x00,  0x00,0x00};
	GPS_Config(msg, sizeof(msg));
}

//enable assistnow autonomous
void GPS_CFG_NAVX5(void)
{
	uint8_t msg[] = {0xB5,0x62,  0x06,0x23,  0x28,0x00,  0x02,0x00,  0x4C,0x66,  
					0x00,0x00,0x00,0x00,  0x00,0x00,  0x03,  0x10,  0x00,  0x00,  0x00,  
					0x00,0x00,  0x00,  0x00,0x00,  0x00,0x00,0x00,0x00,0x00,0x00,  0x00,  
					0x01,  0x00,0x00,  0x00,0x00,  0x00,0x00,0x00,0x00,  0x00,0x00,0x00,  
					0x00,  0x19,0x06};

	GPS_Config(msg, sizeof(msg));
}

//enable odometer
void GPS_CFG_ODO(void)
{
	uint8_t msg[] = {0xB5,0x62,  0x06,0x1E,  0x14,0x00,  0x00,  0x00,0x00,0x00,  
					0x01,  0x00,  0x00,0x00,0x00,0x00,0x00,0x00,  0x0A,  0x32,  
					0x00,0x00,  0x99,  0x4C,  0x00,0x00,  0x5A,0x00};

	GPS_Config(msg, sizeof(msg));
}

//reset odometer
void GPS_RST_ODO(void)
{
	uint8_t msg[] = {0xB5,0x62,  0x01,0x10,  0x00,0x00,  0x11,0x34};

	GPS_Config(msg, sizeof(msg));
}


//config measurement rate = 200hz
void GPS_CFG_RATE(void)
{
	uint8_t msg[] = {0xB5,0x62,  0x06,0x08,  0x06,0x00,  0xc8,0x00,  0x01,0x00, 
					0x00,0x00,  0x00,0x00};
	GPS_Config(msg, sizeof(msg));
}

//config using port ddc with tx timeout
void GPS_CFG_PRT(void)
{
	uint8_t msg[] = {0xB5,0x62,	 0x06,0x00,  0x14,0x00,  0x00,  0x00,  0x00,0x00,  
					0x84,0x00,0x00,0x00,  0x00,0x00,0x00,0x00,  0x03,0x00,  0x01,0x00,  
					0x02,0x00,  0x00,0x00,  0x00,0x00};
	GPS_Config(msg, sizeof(msg));
}

//hotrestart the gps
void GPS_CFG_RST(void)
{
	uint8_t msg[] = {0xB5,0x62,	 0x06,0x04,  0x04,0x00,  0x00,0x00,  0x09,  0x00,  0x17,0x76};
	GPS_Config(msg, sizeof(msg));
}

//save the settings
void GPS_CFG_CFG(void)
{
	uint8_t msg[] = {0xB5,0x62,	 0x06,0x09,  0x0d,0x00,  0x00,0x00,0x00,0x00,  0xff,0xff,0x00,0x00, 
                    0x00,0x00,0x00,0x00,  0x03,  0x1d,0xab};
	GPS_Config(msg, sizeof(msg));
}

void GPS_CFG_MSG(const uint8_t msg_class,const uint8_t msg_id, const uint8_t msg_rate)
{
	uint8_t msg[] = {0xB5,0x62,  0x06,0x01,  3,0,  0x00,  0x00,  0x01,  0x00,0x00};
	msg[6] = msg_class;
	msg[7] = msg_id;
	msg[8] = msg_rate;
	switch(msg[7])
	{
		case GPS_NAV_POSLLH:
			Gps_Handle.RxLength += GPS_NAV_POSLLH_LEN + 8;
			break;
		case GPS_NAV_STATUS:
			Gps_Handle.RxLength += GPS_NAV_STATUS_LEN + 8;
			break;
		case GPS_NAV_DOP:
			Gps_Handle.RxLength += GPS_NAV_DOP_LEN + 8;
			break;
		case GPS_NAV_SOL:
			Gps_Handle.RxLength += GPS_NAV_SOL_LEN + 8;
			break;
		case GPS_NAV_VELNED:
			Gps_Handle.RxLength += GPS_NAV_VELNED_LEN + 8;
			break;
		case GPS_NAV_PVT:
			Gps_Handle.RxLength += GPS_NAV_PVT_LEN + 8;
			break;	
		case GPS_NAV_ODO:
			Gps_Handle.RxLength += GPS_NAV_ODO_LEN + 8;
			break;		
	}
	GPS_Config(msg, sizeof(msg));
}

