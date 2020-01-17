#ifndef  __EP_CRYPT_HOST_H_
#define  __EP_CRYPT_HOST_H_

#include "nrf.h"

/**
 * Function to process Host ID request from device.
 *
 * The Host shall retrieve the Host ID from NVM, or shall generate if
 * it does not yet exist.
 *
 * @param rx_payload Pointer to rx_payload contaning Host ID request.
 */
static void CRYPT_HostId_req_process(uint8_t *rx_data);
/**
 * Function to process Host ID fetch request from Device.
 *
 * The Device fetches the Host ID after the Host has generated/retrieved
 * the Host ID.
 *
 * @param rx_payload Pointer to rx_payload contaning Host ID fetch request.
 */
static void CRYPT_HostId_fetch_process(uint8_t* rx_data);
/**
 * Function to process Key Update Prepare packet.
 *
 * Device requests the Session Token to be used for the Key Update request.
 */
static void CRYPT_KeyUpdate_prepare_process(uint8_t * rx_data);
/**
 * Function to process Key Update packet.
 *
 * Device requests a Key Update and sends a new Dynamic Key. The Dynamic Key is 
 * updated on the Host.
 *
 * @param rx_payload Pointer to rx_payload containing Key Update request.
 */
static void CRYPT_KeyUpdate_process(uint8_t * rx_data);

void  CRYPT_RxData_Decode(uint8_t *rx_data);
void  CRYPT_UserData_TX(const uint8_t *src);
static void  CRYPT_UserData_RX(uint8_t *rx_data);
static void  CRYPT_GzllAddr_Resp(uint8_t *rx_data);

#endif
