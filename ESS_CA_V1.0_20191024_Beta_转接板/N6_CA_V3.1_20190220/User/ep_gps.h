#ifndef __EP_GPS_H_
#define __EP_GPS_H_

#include <stdint.h>
#include <stdbool.h>
#include "twi_master_int.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "ep_misc.h"

/*      GPS UBX Command defines     */

/*      GPS UBX Command Format:
    ------------------------------------------------------------------------------------------------------------
    | SYNC_CHAR_1 | SYNC_CHAR_2 |    CLASS    |     ID     |   LENGTH   |   Payload   |    CK_A    |    CK_B    |
    |   1 Byte    |   1 Byte    |   1 Byte    |    1 Byte  |  2 Bytes   |   n Bytes   |   1 Byte   |   1 Byte   |
    -------------------------------------------------------------------------------------------------------------  */

#define GPS_SYNC_1              0xB5
#define GPS_SYNC_2              0x62

#define GPS_CLASS_NAV           0x01    //Navigation Results: Position, Speed, Time, Acc, Heading, DOP, SVs used
#define GPS_CLASS_RXM           0x02    //Receiver Manager Messages: Satellite Status, RTC Status
#define GPS_CLASS_INF           0x04    //Information Messages: Printf-Style Messages, with IDs such as Error, Warning, Notice
#define GPS_CLASS_ACK           0x05    //Ack/Nack Messages: as replies to CFG Input Messages
#define GPS_CLASS_CFG           0x06    //Configuration Input Messages: Set Dynamic Model, Set DOP Mask, Set Baud Rate, etc.
#define GPS_CLASS_MON           0x0A    //Monitoring Messages: Comunication Status, CPU Load, Stack Usage, Task Status
#define GPS_CLASS_AID           0x0B    //AssistNow Aiding Messages: Ephemeris, Almanac, other A-GPS data input
#define GPS_CLASS_TIM           0x0D    //Timing Messages: Timepulse Output, Timemark Results
#define GPS_CLASS_ESF           0x10    //External Sensor Fusion Messages: External sensor measurements and status information

#define GPS_NAV_POSLLH          0x02    //Geodetic Position Solution
#define GPS_NAV_POSLLH_LEN      28    
#define GPS_NAV_DOP             0x04    //Dilution of precision
#define GPS_NAV_DOP_LEN     	18    
#define GPS_NAV_VELNED          0x12    //Velocity Solution in NED
#define GPS_NAV_VELNED_LEN      36    

#define GPS_NAV_PVT             0x07    //Position Velocity Time Information
#define GPS_NAV_PVT_LEN         92    
#define GPS_NAV_STATUS          0x03    //Receiver Navigation Status
#define GPS_NAV_STATUS_LEN      16    
#define GPS_NAV_SOL             0x06    //Navigation Solution Information
#define GPS_NAV_SOL_LEN     	52   
#define GPS_NAV_ODO				0x09 
#define GPS_NAV_ODO_LEN			20

#define GPS_I2C_DEV_ADDR        0x42
#define GPS_I2C_RX_LEN       	0xFD  
#define GPS_I2C_RX_DATA       	0xFF  

#define GPS_INIT_NORMAL         0x00
#define GPS_INIT_RESET_ODO      0x01

typedef struct
{	
	int32_t 	longitude;
	int32_t		latitude;
	uint8_t		fixType;
	uint8_t		numSV;
	int32_t		headMot;
	uint16_t	pdop;
	uint32_t	gSpeed;
	uint32_t	TTFF;
	uint32_t	timeStarted;        //unit: 1 second    
	uint16_t	gdop;
	uint32_t	pAcc;
	uint32_t	sAcc;
	uint32_t	iTOW;
	uint16_t	WeekNum;
	uint32_t	Time;
	uint32_t	dAcc;
	uint16_t	TravelledDistance;
	uint16_t	StoredTravelledDistance;
	QuickAvg_t 	AvgSpeed;
	uint8_t 	CommState;
	uint8_t 	CommFailCnt;
	uint8_t 	CommFailCntCummulated;
	uint8_t 	RecoveryFailCnt;    
	uint8_t 	DataMisalignCnt;    
	uint8_t 	ReceivedMsg;    
	uint8_t 	DecodedMsg; 
	uint16_t 	FirstPackLocation;
	uint16_t 	RxLength;
	uint8_t		DataPtr; 
}GPS_Handle_t;


void GPS_Init(GPS_Handle_t* gps, uint8_t Options);

void GPS_Decode(GPS_Handle_t* gps);
void GPS_RxByte(GPS_Handle_t* gps);
void GPS_Update(GPS_Handle_t* gps);

uint16_t GPS_GetGSpeedCM(GPS_Handle_t* gps);                    //unit: cm/s
uint8_t GPS_GetNumSV(GPS_Handle_t* gps);            
uint32_t GPS_GetTimeOfWeek(GPS_Handle_t* gps);   
uint16_t GPS_GetWeekNum(GPS_Handle_t* Handle); 
uint32_t GPS_GetOwnTime(GPS_Handle_t* Handle);                    
uint32_t GPS_GetLatitude(GPS_Handle_t* gps);
uint32_t GPS_GetLongtitude(GPS_Handle_t* gps);
int16_t GPS_GetHeading(GPS_Handle_t* gps);
uint16_t GPS_GetPDop(GPS_Handle_t* gps);
uint8_t GPS_GetTTFF(GPS_Handle_t* gps);
uint16_t GPS_GetPAcc(GPS_Handle_t* gps);
uint16_t GPS_GetSAcc(GPS_Handle_t* gps);
uint16_t GPS_GetTravelledDistance(GPS_Handle_t* gps);           //unit: m
void GPS_SetPrevTravelledDistance(GPS_Handle_t* Handle, uint16_t PrevDistance);

uint8_t Gps_GetFailCount(GPS_Handle_t* Handle);
uint8_t Gps_GetFailCountCummulated(GPS_Handle_t* Handle);



#endif /* __EP_GPS_H_ */
