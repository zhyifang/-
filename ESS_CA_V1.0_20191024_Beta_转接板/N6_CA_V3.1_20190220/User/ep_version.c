#include "ep_version.h"

product_msg_t  navy_drv_msg;
product_msg_t  navy_commrelay_msg;

void Product_Init(product_msg_t* product_msg,
	              uint8_t   sv,uint8_t    hv,
				  product_model_e    model,
				  software_year_e   year,software_month_e month
				  )
{
	product_msg->software_ver    = sv;
	product_msg->hardware_ver    = hv;
	product_msg->product_model  = model;
	product_msg->software_year   = year;
	product_msg->software_month  = month;
}
void Product_SetSoftwareVer(product_msg_t* product_msg,uint8_t ver)
{
	product_msg->software_ver = ver;
}
uint8_t Product_ReadSoftwareVer(product_msg_t* product_msg)
{
	return product_msg->software_ver;
}

void Product_SetHardwareVer(product_msg_t* product_msg,uint8_t ver)
{
	product_msg->hardware_ver = ver;
}
uint8_t Product_ReadHardwareVer(product_msg_t* product_msg)
{
	return product_msg->hardware_ver;
}

void Product_SetProductModel(product_msg_t* product_msg,product_model_e model)
{
	product_msg->product_model = model;
}
product_model_e Product_ReadProductModel(product_msg_t* product_msg)
{
	return product_msg->product_model;
}

void Product_SetSoftwareYear(product_msg_t* product_msg,software_year_e year)
{
	product_msg->software_year = year;
}
software_year_e Product_ReadSoftwareYear(product_msg_t* product_msg)
{
	return product_msg->software_year;
}

void Product_SetSoftwareMonth(product_msg_t* product_msg,software_month_e month)
{
	product_msg->software_month = month;
}
software_month_e Product_ReadSoftwareMonth(product_msg_t* product_msg)
{
	return product_msg->software_month;
}
