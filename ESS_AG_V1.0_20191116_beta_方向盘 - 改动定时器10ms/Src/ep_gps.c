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

*  
*************************************/
#include  "ep_gps.h"
#include  "nrf.h"
#include  "ep_comm.h"
#include  "ep_lcd_6kw.h"
#include  "ep_ble_navigation.h"
#include  "ep_pstorage.h"

GPS_Handle_t  GPS_Handle;

void GPS_Init(GPS_Handle_t*  handle)
{
	handle->gps_status          = gps_Disconnected;
	handle->speed               = 0;
	handle->traveled_distance   = 0;
	handle->traveled_time       = 0;
	handle->remain_distance     = 0;
	handle->remain_time         = 0;
	handle->num_sv              = 0xFF;
	handle->latitude_H          = 0;
	handle->latitude_L          = 0;
	handle->longtitude_H        = 0;
	handle->longtitude_L        = 0;
	handle->heading             = 0;
	handle->PDop                = 0;
	handle->TTFF                = 0;
	handle->PAcc                = 0;
	handle->PDop                = 0;
	handle->SAcc                = 0;
	handle->SaveDatFlag         = false;
	handle->SendGpsDatFlag      = true;
	handle->Time                = 0;

}

#if 0
void GPS_comm_RX_decode(GPS_Handle_t* handle,Comm_Handle_t* comm_handle)
{
	switch(comm_handle->Rec.Data.ReturnType)
	{

		case Comm_Ret_GpsSpeed:        /*10*/	
			 handle->speed = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
		     
			 break;	
		case Comm_Ret_GpsNumSV:
			 handle->num_sv = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
			 GPS_ConnectCheck(handle);
		     break;	
		case Comm_Ret_GpsLatitudeH:
			 handle->latitude_H = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
			 break;	
		case Comm_Ret_GpsLatitudeL:
			 handle->latitude_L = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
		     
			 break;	
		case Comm_Ret_GpsLongtitudeH:
			 handle->longtitude_H = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
			 break;	
		case Comm_Ret_GpsLongtitudeL:  /*15*/	
			 handle->longtitude_L = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
		     
			 break;	
		case Comm_Ret_GpsHeading:
			 handle->heading = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
		     BATTERY_SetBatTemp(&BATTERY_Handle,handle->heading/100,handle->heading%100);//////////////////////////////////////////////////
			 break;	
		case Comm_Ret_TravelledDistance:
			 handle->traveled_distance = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
			 break;	
		case Comm_Ret_TravelledTime:
			 handle->traveled_time = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
			 break;	
		case Comm_Ret_RemainingDistance:
			 handle->remain_distance = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
			 break;	
		case Comm_Ret_RemainingTime:   /*20*/	
			 handle->remain_time = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
		    
			 break;	
		case Comm_Ret_GpsTimeH:   /*20*/	
             handle->TimeH = (comm_handle->Rec.Data.MsgHi << 21)|(comm_handle->Rec.Data.MsgLo << 14);
			 break;	
		case Comm_Ret_GpsTimeL:   /*20*/	
             handle->TimeL = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
		     handle->Time = handle->TimeH | handle->TimeL;
		     GPS_TimeCheck(handle);
			 break;	
		default:break; 
	}
}
#endif
void GPS_SetSendGpsDatFlag(bool Flag)
{
	GPS_Handle.SendGpsDatFlag = Flag;
}
void  GPS_TimeCheck(GPS_Handle_t* handle)
{
	if(handle->SendGpsDatFlag == true)
	{		
		if(handle->Time > 10000)//if gps time up to 10000,indicate gps is nomal 
		{
			if(handle->Time >= handle->SaveTime)
			{
				//if new Traveled time > the saving Traveled time + 30 ,send 0 ;else send the saving time and the saving dis
				if((handle->Time - handle->SaveTime) > 30)//send 0
				{
					handle->SaveTraveled_distance = 0;
					handle->SaveTraveled_time = 0;
					//Comm_DataTypeSet(TravelTimeSetType);
				}
				else//send the saving time and the saving dis
				{
					//Comm_DataTypeSet(TravelTimeSetType);
				}
			}
		}
		
	}
	else
    {
		//if(handle->SaveDatFlag == true)
		{
			//handle->SaveDatFlag = false;
			handle->SaveTraveled_distance =  handle->traveled_distance;
			handle->SaveTraveled_time = handle->traveled_time;
			handle->SaveTime = handle->Time;
		}
		
	}
}
void  GPS_SetSendTDFlag(GPS_Handle_t* handle,bool flag)
{
	handle->SendGpsDatFlag = flag;
}
void  GPS_ConnectCheck(GPS_Handle_t* handle)
{
	static uint8_t check_count = 0;

		
	if(handle->num_sv == 0xFF)
	{
		handle->gps_status = gps_Disconnected;
	}
	else if(handle->num_sv > 5)
	{
		handle->gps_status = gps_Connected;
	}
	else if((handle->num_sv < 5)&&(check_count++ > 10))
	{
		handle->gps_status = gps_Searching;
	}

	
}
void GPS_Dat_Backup(GPS_Handle_t* handle,uint8_t* data)
{
    *(data+0) = (handle->SaveTime >> 24) & 0xff;                                             //time of week bit 31 to bit 24
    *(data+1) = (handle->SaveTime >> 16) & 0xff;                                             //time of week bit 23 to bit 16
    *(data+2) = (handle->SaveTime >> 8) & 0xff;                                              //time of week bit 15 to bit 8
    *(data+3) = handle->SaveTime & 0xff;                                                     //time of week bit 7 to bit 0  
    
    *(data+4) = (handle->SaveTraveled_time >> 8) & 0xff;                                    //battery on time bit 15 to bit 8
    *(data+5) = (handle->SaveTraveled_time & 0xff);                                        //battery on time bit 7 to bit 0
    
    *(data+6) = (handle->SaveTraveled_distance >> 8) & 0xff;     							//travelled distance bit 15 to bit 8
    *(data+7) = (handle->SaveTraveled_distance & 0xff);          							//travelled distance bit 7 to bit 0    
    
    *(data+8) = PSTORAGE_CheckSum(data, 8);                                                   //ensure the data validity
      
}


void GPS_Dat_Update(GPS_Handle_t* handle,uint8_t* data)
{   
    if(data[8] != PSTORAGE_CheckSum(data, 8))                                                 //ensure the data validity
    {
		handle->SaveTime = 0;
        handle->traveled_time = 0;
        handle->SaveTraveled_distance = 0;
        return;
    }    
    
	handle->SaveTime              = (uint32_t)(((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | ((uint32_t)data[3]));
    handle->SaveTraveled_time     = (uint16_t)(((uint16_t)data[4] << 8) | ((uint16_t)data[5]));
    handle->SaveTraveled_distance = (uint16_t)(((uint16_t)data[6] << 8) | ((uint16_t)data[7]));
}

