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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "nrf_gzll.h"
#include "nrf_gzp.h"
#include "nrf_delay.h"
#include "nrf_nvmc.h"
#include "nrf_gpio.h"
#include "ep_crypt_device.h"
#include "nrf_soc.h"
#include "ep_comm.h"
#include "ep_gzll.h"


CRYPT_Handle_t  CRYPT_Handle;
void CRYPT_Init(void)
{
	CRYPT_Handle.Crypt_Cmd_Type   =  Crypt_Cmd_HostID_Req;
	CRYPT_Handle.KeyUpdateCount   =  0;
}
void  CRYPT_Decode(uint8_t* rx_data)
{
	switch(rx_data[0])
	{
		case  GZP_CMD_ENCRYPTED_USER_DATA:             ///收到加密数据
			CRYPT_UserData_RX(rx_data);
			Comm_Decode(&Comm_Handle);
			break;

		case  GZP_CMD_HOST_ID_REQ_RESP:                //Host ID请求回复				    
			CRYPT_CmdSet(Crypt_Cmd_HostID_Fecth);
			break;

		case  GZP_CMD_HOST_ID_FETCH_RESP:              //host ID提取回复
			CRYPT_HostID_fetchResp_process(rx_data);
			CRYPT_CmdSet(Crypt_Cmd_keyUpdate_Prepare);
			break;

		case  GZP_CMD_KEY_UPDATE_PREPARE_RESP:         //密钥准备回复
			CRYPT_keyUpdate_PrepareResp(rx_data);
			CRYPT_CmdSet(Crypt_Cmd_keyUpdate);
			break;

		case  GZP_CMD_KEY_UPDATE_SUCCESS:             //密钥更新回复
			CRYPT_CmdSet(Crypt_Data);
		   // Gzll_prefix_set(true);
			break;

		default : break;
	}
	GZLL_Handle.rx_data.data[0] = 0;	
}

void  CRYPT_CmdRequest(void)
{
	if(CRYPT_Handle.Crypt_Cmd_Type == Crypt_Data)    //发送加密数据  
	{		
		CRYPT_UserData_TX(Comm_Handle.Cmd.Raw);

	}
	else if(CRYPT_Handle.Crypt_Cmd_Type == Crypt_Cmd_HostID_Req)
	{
		CRYPT_HostID_req_send();
		CRYPT_Handle.KeyUpdateCount = 0;

	}
	else if(CRYPT_Handle.Crypt_Cmd_Type == Crypt_Cmd_HostID_Fecth)
	{
		CRYPT_HostID_fetch_send();
		CRPTY_HostIdReq_FailCheck();///
	}
	else if(CRYPT_Handle.Crypt_Cmd_Type == Crypt_Cmd_keyUpdate_Prepare)
	{
		CRYPT_keyUpdate_Prepare();
		CRYPT_KeyUpdateFailCheck();///
	}
	else if(CRYPT_Handle.Crypt_Cmd_Type == Crypt_Cmd_keyUpdate)
	{
		CRYPT_keyUpdate();
		CRYPT_KeyUpdateFailCheck();///
	}
		
}

void CRYPT_HostID_req_send(void)
{
	uint8_t tx_packet[GZP_CMD_HOST_ID_REQ_PAYLOAD_LENGTH];
	// 0 :cmd  1,2,3,4,5 : session_token
	tx_packet[0] = (uint8_t)GZP_CMD_HOST_ID_REQ;
	sd_rand_application_vector_get(&tx_packet[GZP_CMD_HOST_ID_REQ_SESSION_TOKEN], GZP_SESSION_TOKEN_LENGTH); //5 byte rand data
	Gzll_SetTxData(tx_packet,GZP_CMD_HOST_ID_REQ_PAYLOAD_LENGTH);//1 head + 5 byte rand data
	gzp_crypt_set_session_token(&tx_packet[GZP_CMD_HOST_ID_REQ_SESSION_TOKEN]); //save session taken 
}
void CRYPT_HostID_fetch_send(void)
{
	uint8_t tx_packet[GZP_CMD_HOST_ID_REQ_PAYLOAD_LENGTH];

	tx_packet[0] = (uint8_t)GZP_CMD_HOST_ID_FETCH;
	gzp_add_validation_id(&tx_packet[GZP_CMD_HOST_ID_FETCH_VALIDATION_ID]);

	// Encrypt "host ID fetch" packet
	gzp_crypt_select_key(GZP_ID_EXCHANGE);
	gzp_crypt(&tx_packet[1], &tx_packet[1], GZP_CMD_HOST_ID_FETCH_PAYLOAD_LENGTH - 1);
	// 0: CMD  1,2,3:validation_id 
	//nrf_gzll_add_packet_to_tx_fifo(1,tx_packet,GZP_CMD_HOST_ID_REQ_PAYLOAD_LENGTH);
	Gzll_SetTxData(tx_packet,GZP_CMD_HOST_ID_FETCH_PAYLOAD_LENGTH);
}
void CRYPT_HostID_fetchResp_process(uint8_t* rx_data)
{
	gzp_crypt_select_key(GZP_ID_EXCHANGE);
	gzp_crypt(&rx_data[1], &rx_data[1], GZP_CMD_HOST_ID_FETCH_RESP_PAYLOAD_LENGTH - 1);
	if(gzp_validate_id(&rx_data[GZP_CMD_HOST_ID_FETCH_RESP_VALIDATION_ID]))
	{
		gzp_set_host_id(&rx_data[GZP_VALIDATION_ID_LENGTH+1]);//1+1
	}
}


void  CRYPT_CommFailCheck(uint16_t CommFailCount)
{
	if(CommFailCount < 100)
	{
		return;  
	}
	if(CRYPT_Handle.Crypt_Cmd_Type == Crypt_Data)
	{
		CRYPT_CmdSet(Crypt_Cmd_HostID_Req);
	}
}

void  CRYPT_KeyUpdateFailCheck(void)
{
	CRYPT_Handle.KeyUpdateCount++;

	if(CRYPT_Handle.KeyUpdateCount > 10)
	{
		CRYPT_CmdSet(Crypt_Cmd_HostID_Req);
	}
}

void  CRPTY_HostIdReq_FailCheck(void)
{
	CRYPT_Handle.HostIdReqCount++;
	if(CRYPT_Handle.HostIdReqCount > 10)
	{
		CRYPT_CmdSet(Crypt_Cmd_HostID_Req);
	}
}

void  CRPTY_GzllAddr_UpdateReq(void)
{
	uint8_t tx_data[2];
	tx_data[0] = GZP_CMD_GZLL_ADDRESS_UPDATE_REQ;
	tx_data[1] = GZP_CMD_GZLL_ADDRESS_UPDATE_REQ  +  1;
	Gzll_SetTxData(tx_data,2);
}
void  CRPTY_GzllAddr_RxResp(void)
{
	uint8_t tx_data[2];
	tx_data[0] = GZP_CMD_GZLL_ADDRESS_UPDATE_RESP;
	tx_data[1] = GZP_CMD_GZLL_ADDRESS_UPDATE_RESP  +  1;
	Gzll_SetTxData(tx_data,2);
}
void  CRYPT_CmdSet(Crypt_Cmd_Type_e Cmd_Type)
{
    CRYPT_Handle.Crypt_Cmd_Type = Cmd_Type;
}
void  CRYPT_keyUpdate_Prepare(void)
{
	uint8_t tx_data[2];
	// Send "prepare packet" to get session token to be used for key update
	tx_data[0] = (uint8_t)GZP_CMD_KEY_UPDATE_PREPARE;
	tx_data[1] = (uint8_t)GZP_CMD_KEY_UPDATE_PREPARE + 1;
	Gzll_SetTxData(tx_data,2);
}
void CRYPT_keyUpdate_PrepareResp(uint8_t* rx_data)
{
    gzp_crypt_set_session_token(&rx_data[GZP_CMD_KEY_UPDATE_PREPARE_RESP_SESSION_TOKEN]);
}
void CRYPT_keyUpdate(void)
{
	uint8_t tx_data[GZP_CMD_KEY_UPDATE_PAYLOAD_LENGTH];

	tx_data[0] = (uint8_t)GZP_CMD_KEY_UPDATE;
	gzp_add_validation_id(&tx_data[GZP_CMD_KEY_UPDATE_VALIDATION_ID]);
	sd_rand_application_vector_get(&tx_data[GZP_CMD_KEY_UPDATE_NEW_KEY],GZP_DYN_KEY_LENGTH); //random 15 byte dyn_key
	gzp_crypt_set_dyn_key(&tx_data[GZP_CMD_KEY_UPDATE_NEW_KEY]);

	gzp_crypt_select_key(GZP_KEY_EXCHANGE);
	gzp_crypt(&tx_data[1], &tx_data[1], GZP_CMD_KEY_UPDATE_PAYLOAD_LENGTH - 1);
	Gzll_SetTxData(tx_data,GZP_CMD_KEY_UPDATE_PAYLOAD_LENGTH ); 
}
void  CRYPT_UserData_TX(const uint8_t *tx_data)
{
    uint8_t tx_packet[GZP_CRYPT_USER_DATA_REAL_LENTH];
    //tx_packet_length = length + (uint8_t)GZP_ENCRYPTED_USER_DATA_PACKET_OVERHEAD;
	  //0 : cmd 1 validation 23456:data 5byte
    tx_packet[0] = (uint8_t)GZP_CMD_ENCRYPTED_USER_DATA;
    gzp_add_validation_id(&tx_packet[GZP_CMD_ENCRYPTED_USER_DATA_VALIDATION_ID]);
    memcpy(&tx_packet[GZP_CMD_ENCRYPTED_USER_DATA_PAYLOAD], (uint8_t*)tx_data, GZP_CRYPT_USER_DATA_LENTH);
    gzp_crypt_select_key(GZP_DATA_EXCHANGE);
 
    gzp_crypt(&tx_packet[1], &tx_packet[1],GZP_CRYPT_USER_DATA_LENTH + GZP_CMD_CRYPT_USER_DATA_HEADER_LENTH);
	Gzll_SetTxData(tx_packet,GZP_CRYPT_USER_DATA_REAL_LENTH);
}
void  CRYPT_UserData_RX(uint8_t *rx_data)
{ 
	gzp_crypt_select_key(GZP_DATA_EXCHANGE);
	gzp_crypt(&rx_data[1], &rx_data[1], GZP_CRYPT_USER_DATA_LENTH + GZP_CMD_CRYPT_USER_DATA_HEADER_LENTH);
	// Validate response in order to know whether packet was correctly decrypted by host
	if(gzp_validate_id(&rx_data[GZP_CMD_ENCRYPTED_USER_DATA_RESP_SESSION_TOKEN]))
	{
		memcpy(&Comm_Handle.Rec.Raw[0],&rx_data[2], GZP_CRYPT_USER_DATA_LENTH);  
	}
}

void gzp_set_host_id(const uint8_t * id)
{
    memcpy(CRYPT_Handle.gzp_host_id, id, GZP_HOST_ID_LENGTH);
}

void gzp_get_host_id(uint8_t * dst_id)
{
    memcpy(dst_id, CRYPT_Handle.gzp_host_id, GZP_HOST_ID_LENGTH);
}
