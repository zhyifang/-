/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic
 * Semiconductor ASA.Terms and conditions of usage are described in detail
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRENTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $LastChangedRevision: 230 $
 */

 
/** 
 * @file
 * @brief Implementation of Gazell Pairing Library (gzp), Host functions.
 * @defgroup gzp_source_host Gazell Pairing Host implementation
 * @{
 * @ingroup gzp_04_source
 */

 
#include "nrf_gzp.h"
#include "nrf_gzll.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "nrf_assert.h"
#include "nrf_ecb.h"
#include "nrf_nvmc.h"
#include "nrf_gpio.h"
#include "ep_crypt_host.h"
#include "ep_comm.h"
#include "ep_gzll.h"
/**
 * Function for reading value of internal session counter.
 * @param dst   Current session counter.
 */

void gzp_get_host_id(uint8_t *dst);
static void gzp_set_host_id(const uint8_t* src);

/***********************
*  name : CRYPT_init
*  dsc  : 
*  
************************/
 void CRYPT_init()
{
  // gzp_host_chip_id_read(system_address, GZP_SYSTEM_ADDRESS_WIDTH);
}
/***********************
*  name : CRYPT_RxData_Decode
*  dsc  : Decode RX data 
*  uint8_t *rx_data : comm rx data
************************/
void CRYPT_RxData_Decode(uint8_t *rx_data)
{
	switch(rx_data[0])
	{
		case    GZP_CMD_ENCRYPTED_USER_DATA:
				CRYPT_UserData_RX(rx_data);
		        //Gzll_set_prefix_byte(gzp_dyn_key[0]);
		        Gzll_commState_set(GZLL_State_Connected);
		        Gzll_CommFailCountClear();
				break;

		case    GZP_CMD_HOST_ID_REQ: //0x02	
				CRYPT_HostId_req_process(rx_data);
				break;

		case    GZP_CMD_HOST_ID_FETCH: //0x03
				CRYPT_HostId_fetch_process(rx_data);
				break;

		case    GZP_CMD_KEY_UPDATE_PREPARE:
				CRYPT_KeyUpdate_prepare_process(rx_data);
				break;

		case    GZP_CMD_KEY_UPDATE:
				CRYPT_KeyUpdate_process(rx_data);
		        Gzll_prefix_set(true);
				break;
		
		default: break;
	}
} 
/***********************
*  name : CRYPT_UserData_TX
*  uint8_t *user_data : 5byte user data
*  tx_data[] : 0 : cmd 1.2.3:validation 45678:user data 5byte
************************/
void  CRYPT_UserData_TX(const uint8_t *user_data)
{
	uint8_t tx_packet[GZP_CRYPT_USER_DATA_REAL_LENTH];
	//0 : cmd 1.2.3:validation 45678:data 5byte
	tx_packet[0] = (uint8_t)GZP_CMD_ENCRYPTED_USER_DATA;//0 cmd header 
	gzp_add_validation_id(&tx_packet[GZP_CMD_ENCRYPTED_USER_DATA_VALIDATION_ID]);//123  validation id 
	memcpy(&tx_packet[GZP_CMD_ENCRYPTED_USER_DATA_PAYLOAD], (uint8_t*)user_data, GZP_CRYPT_USER_DATA_LENTH);//45678 user data 

	gzp_crypt_select_key(GZP_DATA_EXCHANGE); 
	gzp_crypt(&tx_packet[1], &tx_packet[1],GZP_CRYPT_USER_DATA_LENTH + GZP_VALIDATION_ID_LENGTH);//CRYPT data
	nrf_gzll_add_packet_to_tx_fifo(PIPE,tx_packet,GZP_CRYPT_USER_DATA_REAL_LENTH);
}
/***********************
*  name :  CRYPT_UserData_RX
*  dsc  :  Decryption rx data 
*  rx_data[] : 0 : cmd 1.2.3:validation 45678:user data 5byte
************************/
uint8_t temp_dat[7];
static void  CRYPT_UserData_RX(uint8_t *rx_data)
{
	//recrypt rx data 
	gzp_crypt_select_key(GZP_DATA_EXCHANGE);
	gzp_crypt(&rx_data[1], &rx_data[1], GZP_CRYPT_USER_DATA_LENTH + GZP_VALIDATION_ID_LENGTH);
	memcpy(temp_dat,rx_data,7);
	// Validate response in order to know whether packet was correctly decrypted by host
	if(gzp_validate_id(&rx_data[GZP_CMD_ENCRYPTED_USER_DATA_RESP_SESSION_TOKEN]))
	{
		memcpy(&Comm_Handle.Rec.Raw[0],&rx_data[2], GZP_CRYPT_USER_DATA_LENTH);  
	} 
}
/***********************
*  name :  CRYPT_KeyUpdate_prepare_process
*  dsc  :  handle  KeyUpdate_prepare from device , random session token to device
*  
************************/
static void CRYPT_KeyUpdate_prepare_process(uint8_t * rx_data)
{
	uint8_t tx_payload[GZP_CMD_KEY_UPDATE_PREPARE_RESP_PAYLOAD_LENGTH];
	if(rx_data[1] == rx_data[0] + 1)
	{
		tx_payload[0] = (uint8_t)GZP_CMD_KEY_UPDATE_PREPARE_RESP;
		gzp_random_numbers_generate(&tx_payload[GZP_CMD_KEY_UPDATE_PREPARE_RESP_SESSION_TOKEN],5);
		gzp_crypt_set_session_token(&tx_payload[GZP_CMD_KEY_UPDATE_PREPARE_RESP_SESSION_TOKEN]);
		nrf_gzll_add_packet_to_tx_fifo(PIPE,tx_payload,GZP_CMD_KEY_UPDATE_PREPARE_RESP_PAYLOAD_LENGTH);
	}
		
}
/***********************
*  name :  CRYPT_KeyUpdate_process
*  dsc  :  update dyn key from device 
*  
************************/
static void CRYPT_KeyUpdate_process(uint8_t* rx_data)
{
	uint8_t tx_data[1];
	tx_data[0] = GZP_CMD_KEY_UPDATE_SUCCESS;
	gzp_crypt_select_key(GZP_KEY_EXCHANGE);
	gzp_crypt(&rx_data[1], &rx_data[1], GZP_CMD_KEY_UPDATE_PAYLOAD_LENGTH - 1);
	if(gzp_validate_id(&rx_data[GZP_CMD_KEY_UPDATE_VALIDATION_ID]))
	{
	   gzp_crypt_set_dyn_key(&rx_data[GZP_CMD_KEY_UPDATE_NEW_KEY]);
	}
	nrf_gzll_add_packet_to_tx_fifo(PIPE,tx_data,1);
}


static void CRYPT_HostId_req_process(uint8_t *rx_data)
{
	int i;
	uint8_t tx_data[1];
	uint8_t temp_host_id[GZP_HOST_ID_LENGTH];
	tx_data[0] = GZP_CMD_HOST_ID_REQ_RESP;

	gzp_crypt_set_session_token(&rx_data[GZP_CMD_HOST_ID_REQ_SESSION_TOKEN]);
	gzp_get_host_id(temp_host_id);
	for(i = 0; i < GZP_HOST_ID_LENGTH; i++)
	{
		if(temp_host_id[i] != 0xFF)
		{
			break;      
		}
	}
	if(i == GZP_HOST_ID_LENGTH) // If host not generated yet
	{
		gzp_random_numbers_generate(temp_host_id, 5);
		//lint -save -e506 Constantvalue Boolean
		if(GZP_HOST_ID_LENGTH > GZP_SESSION_TOKEN_LENGTH)
		{
		//lint -restore
			gzp_xor_cipher(temp_host_id, temp_host_id, &rx_data[GZP_CMD_HOST_ID_REQ_SESSION_TOKEN], GZP_SESSION_TOKEN_LENGTH);
		}
		else
		{
			gzp_xor_cipher(temp_host_id, temp_host_id, &rx_data[GZP_CMD_HOST_ID_REQ_SESSION_TOKEN], GZP_HOST_ID_LENGTH);
		}

		gzp_set_host_id(temp_host_id);
	} 

	nrf_gzll_add_packet_to_tx_fifo(PIPE,tx_data,1);
}

static void CRYPT_HostId_fetch_process(uint8_t* rx_payload)
{
	uint8_t tx_payload[GZP_CMD_HOST_ID_FETCH_RESP_PAYLOAD_LENGTH];
	gzp_crypt_select_key(GZP_ID_EXCHANGE);
	gzp_crypt(&rx_payload[1], &rx_payload[1], GZP_CMD_HOST_ID_FETCH_PAYLOAD_LENGTH - 1);
	if(gzp_validate_id(&rx_payload[GZP_CMD_HOST_ID_FETCH_VALIDATION_ID]))
	{ 
		//0 :CMD 1, validation 23456:hostid
		tx_payload[0] = (uint8_t)GZP_CMD_HOST_ID_FETCH_RESP;
		gzp_add_validation_id(&tx_payload[GZP_CMD_HOST_ID_FETCH_RESP_VALIDATION_ID]);
		gzp_get_host_id(&tx_payload[GZP_VALIDATION_ID_LENGTH+1]);

		gzp_crypt(&tx_payload[1], &tx_payload[1],GZP_CMD_HOST_ID_FETCH_RESP_PAYLOAD_LENGTH-1);

		nrf_gzll_add_packet_to_tx_fifo(PIPE,tx_payload,GZP_CMD_HOST_ID_FETCH_RESP_PAYLOAD_LENGTH);
	}
  
}

static void gzp_set_host_id(const uint8_t* src)
{
	if(*((uint8_t*)GZP_PARAMS_STORAGE_ADR) == 0xff)
	{
		nrf_nvmc_write_bytes(GZP_PARAMS_STORAGE_ADR + 1, src, GZP_HOST_ID_LENGTH);
		nrf_nvmc_write_byte(GZP_PARAMS_STORAGE_ADR, 0x00);
	}    
}

void gzp_get_host_id(uint8_t *dst)
{
  memcpy(dst, (uint8_t*)GZP_PARAMS_STORAGE_ADR + 1, GZP_HOST_ID_LENGTH);
}

void gzp_host_chip_id_read(uint8_t *dst, uint8_t n)
{
	uint8_t i;
	uint8_t random_number;

	if( *((uint8_t*)(GZP_PARAMS_STORAGE_ADR + GZP_HOST_ID_LENGTH + 1)) == 0xff)
	{
		nrf_nvmc_write_byte((GZP_PARAMS_STORAGE_ADR + GZP_HOST_ID_LENGTH + 1) , 0x00);

		for(i = 0; i < n; i++) 
		{
			gzp_random_numbers_generate(&random_number, 1);
			nrf_nvmc_write_byte((GZP_PARAMS_STORAGE_ADR + GZP_HOST_ID_LENGTH + 2 + i) , random_number);
		}
	}

	for(i = 0; i < n; i++) 
	{
		*(dst++) = *((uint8_t*)(GZP_PARAMS_STORAGE_ADR + GZP_HOST_ID_LENGTH + 2 + i));
	}
}

