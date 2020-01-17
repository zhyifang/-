/***************************
*  file name: ep_version.h
*  Desc     : define the ep product software version,Hardware version,
              product model,software date.
*
*
******************************/
#ifndef  __EP_VERSION_H_
#define  __EP_VERSION_H_
#include "stdint.h"
/***********
//add 3 return command


MOTOR_Ret_HS_Ver -> value = (HARDWARE_VER << 7) | (SOFTWARE_VER & 0x7F);
MOTOR_Ret_SwDate -> value = (SOFTWARE_YEAR << 4) | (SOFTWARE_MONTH & 0x0F);
MOTOR_Ret_PM     -> value = (PRODUCT_MODULE & 0x3F);
************/



//porduct model, value:0-63 6bit
typedef enum
{
	product_null          = 0,
    product_navy_60       = 1,     //navy6.0
    product_navy_30 ,              //navy3.0
    product_spirit10,              //spirit1.0
    product_spirit10_r,            //spirit1.0 wireless 
    product_spirit10_rw,           //spirit1.0 wire
    
}product_model_e;
//software year ,value 1-511 9bit
typedef enum
{
    year_2017 = 1,
    year_2018,
    year_2019,
    year_2020,
    year_2021,
    year_2022,
    year_2023,
    year_2024,
    year_2025,
    year_2026,
    year_2027,
    year_2028,
    year_2029,
    year_2030,
    year_2031,
    year_2032,

	
    year_offset = 2016,
}software_year_e;
//software month ,value 1-12  4bit
typedef enum
{
    month_1 = 1,
    month_2, 
    month_3, 
    month_4, 
    month_5, 
    month_6, 
    month_7, 
    month_8, 
    month_9, 
    month_10, 
    month_11, 
    month_12,
}software_month_e;
	

//software version, value:0-127 7bit. 1:ver 0.1  15:ver 1.5 
#define  SOFTWARE_VER       22   

//hardware version ,value:0-127 7bit. 1:ver 0.1  15:ver 1.5 
#define  HARDWARE_VER       15 

//product model ,value:0-31 5bit
#define  PRODUCT_MODULE     product_navy_60  

//software year ,value 1-511 9bit
#define  SOFTWARE_YEAR      year_2018
//software month ,value 1-12  4bit
#define  SOFTWARE_MONTH     month_9


typedef struct
{
    uint8_t            software_ver;
    uint8_t            hardware_ver;
    product_model_e   product_model;
    software_year_e    software_year;
    software_month_e   software_month;
}product_msg_t;

extern product_msg_t  drv_msg;
extern product_msg_t  commrelay_msg;
extern product_msg_t  handle_msg;

void Product_Init(product_msg_t* product_msg,
	              uint8_t   sv,uint8_t    hv,
				  product_model_e    model,
				  software_year_e   year,software_month_e month
				  );
void Product_SetSoftwareVer(product_msg_t* product_msg,uint8_t ver);
uint8_t Product_ReadSoftwareVer(product_msg_t* product_msg);
void Product_SetHardwareVer(product_msg_t* product_msg,uint8_t ver);
uint8_t Product_ReadHardwareVer(product_msg_t* product_msg);
void Product_SetProductModel(product_msg_t* product_msg,product_model_e model);
product_model_e Product_ReadProductModel(product_msg_t* product_msg);
void Product_SetSoftwareYear(product_msg_t* product_msg,software_year_e year);
software_year_e Product_ReadSoftwareYear(product_msg_t* product_msg);
void Product_SetSoftwareMonth(product_msg_t* product_msg,software_month_e month);
software_month_e Product_ReadSoftwareMonth(product_msg_t* product_msg);

#endif


