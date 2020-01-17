#ifndef  __EP_GPS_H_
#define  __EP_GPS_H_

#include "nrf.h"
#include "ep_comm.h"
typedef enum
{
	gps_Searching = 0,
	gps_Connected,
	gps_Disconnected,
}GPS_Status_e;

typedef struct
{
	bool            SaveDatFlag;
	bool            SendGpsDatFlag;
	GPS_Status_e    gps_status;
		
	uint16_t        speed;
	uint16_t        traveled_distance;
	uint16_t        traveled_time;
	uint16_t        remain_distance;
	uint16_t        remain_time;
	uint16_t        num_sv;
	uint16_t        latitude_H;
	uint16_t        latitude_L;
	uint16_t        longtitude_H;
	uint16_t        longtitude_L;
	uint16_t        heading;
	uint16_t        PDop;
	uint16_t        TTFF;      
	uint16_t        PAcc;
	uint16_t        SAcc;  
	uint16_t        GpsCheckVal;

	
	uint16_t        SaveTraveled_distance;
	uint16_t        SaveTraveled_time;
	uint32_t        SaveTime;
	uint32_t        TimeH;
	uint32_t        TimeL;
	uint32_t        Time;  //the time is the minute from 1980s to now 


}GPS_Handle_t;

extern  GPS_Handle_t  GPS_Handle;
void  GPS_SetSendDatStatus(uint8_t  bit);

void  GPS_Init(GPS_Handle_t*  handle);
void  GPS_comm_RX_decode(GPS_Handle_t* handle,Comm_Handle_t* comm_handle);
void  GPS_ConnectCheck(GPS_Handle_t* handle);
void  GPS_TimeCheck(GPS_Handle_t* handle);
void  GPS_Dat_Backup(GPS_Handle_t* handle,uint8_t* data);
void  GPS_Dat_Update(GPS_Handle_t* handle,uint8_t* data);
void  GPS_SetSendGpsDatFlag(bool Flag);
void  GPS_SetSendTDFlag(GPS_Handle_t* handle,bool flag);

#endif


