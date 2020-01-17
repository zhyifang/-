#ifndef __EP_CRYPT_DEVICE_H
#define __EP_CRYPT_DEVICE_H

#include "nrf.h"
#include "nrf_gzp.h"
#include "nrf_gzp.h"

typedef enum
{
	Crypt_Cmd_Null,
	Crypt_Cmd_HostID_Req,
	Crypt_Cmd_HostID_Fecth,
	Crypt_Cmd_keyUpdate_Prepare,
	Crypt_Cmd_keyUpdate,
	Crypt_Data
}Crypt_Cmd_Type_e;

typedef  struct
{
	uint8_t  gzp_session_token[GZP_SESSION_TOKEN_LENGTH];///
	uint8_t  gzp_dyn_key[GZP_DYN_KEY_LENGTH];
	uint8_t  gzp_system_address[GZP_SYSTEM_ADDRESS_WIDTH]; ///<
	uint8_t  gzp_host_id[GZP_HOST_ID_LENGTH];              ///<
	uint8_t  dyn_key[GZP_DYN_KEY_LENGTH];

	uint8_t  KeyUpdateCount;
	uint8_t  HostIdReqCount;
	Crypt_Cmd_Type_e  Crypt_Cmd_Type;	
}CRYPT_Handle_t;

extern CRYPT_Handle_t  CRYPT_Handle;

void  CRYPT_Decode(uint8_t * rx_packet);
void  CRYPT_Init(void);
void  CRYPT_HostID_req_send(void);
void  CRYPT_HostID_fetch_send(void);
void  CRYPT_HostID_fetchResp_process(uint8_t* rx_data);
void  CRYPT_CmdSet(Crypt_Cmd_Type_e Cmd_Type);
void  CRYPT_CommFailCheck(uint16_t CommFailCount);
void  CRYPT_keyUpdate_Prepare(void);
void  CRYPT_keyUpdate_PrepareResp(uint8_t* rx_data);
void  CRYPT_keyUpdate(void);
void  CRYPT_KeyUpdateFailCheck(void);
void  CRYPT_CmdRequest(void);
void  CRYPT_UserData_TX(const uint8_t *src);
void  CRYPT_UserData_RX(uint8_t *rx_data);
void  CRPTY_HostIdReq_FailCheck(void);

#endif
