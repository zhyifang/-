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

#include "nrf_gzp.h"
#include "nrf_gzll.h"
#include "nrf_ecb.h"
#include <string.h>
#include "nrf_soc.h"
#include "ep_crypt_device.h"
#include "nrf_gpio.h"
#define SOURCE_FILE NRF_SOURCE_FILE_GZP    ///< File identifer for asserts.


nrf_ecb_hal_data_t  nrf_ecb_hal_data;

/** 
 * Constant holding pre-defined "validation ID". 
 */
static const uint8_t gzp_validation_id[GZP_VALIDATION_ID_LENGTH] = GZP_VALIDATION_ID;

/** 
 * Constant holding pre-defined "secret key". 
 */
static const uint8_t gzp_secret_key[16] = GZP_SECRET_KEY;

/** 
 * Variable used for AES key selection 
 */
static gzp_key_select_t gzp_key_select;
void gzp_xor_cipher(uint8_t* dst, const uint8_t* src, const uint8_t* pad, uint8_t length)
{
    uint8_t i;

    for(i = 0; i < length; i++)
    {
        *dst = *src ^ *pad;
        dst++;
        src++;
        pad++;
    }
}

bool gzp_validate_id(const uint8_t* id)
{
    return (memcmp(id, (void*)gzp_validation_id, GZP_VALIDATION_ID_LENGTH) == 0);
}

void gzp_add_validation_id(uint8_t* dst)
{
    memcpy(dst, (void const*)gzp_validation_id, GZP_VALIDATION_ID_LENGTH); 
}

void gzp_crypt_set_session_token(const uint8_t * token)
{
    memcpy(CRYPT_Handle.gzp_session_token, (void const*)token, GZP_SESSION_TOKEN_LENGTH);
}

void gzp_crypt_set_dyn_key(const uint8_t* key)
{
    memcpy(CRYPT_Handle.gzp_dyn_key, (void const*)key, GZP_DYN_KEY_LENGTH); 
}

void gzp_crypt_get_session_token(uint8_t * dst_token)
{
    memcpy(dst_token, (void const*)CRYPT_Handle.gzp_session_token, GZP_SESSION_TOKEN_LENGTH);
}

void gzp_crypt_get_dyn_key(uint8_t* dst_key)
{
    memcpy(dst_key, (void const*)CRYPT_Handle.gzp_dyn_key, GZP_DYN_KEY_LENGTH); 
}

void gzp_crypt_select_key(gzp_key_select_t key_select)
{
    gzp_key_select = key_select;
}

void gzp_crypt(uint8_t* dst, const uint8_t* src, uint8_t length)
{
    uint8_t i;
    uint8_t key[16];
    uint8_t iv[16];

    // Build AES key based on "gzp_key_select"

    switch(gzp_key_select)
    {
    case GZP_ID_EXCHANGE:
        memcpy(key, (void const*)gzp_secret_key, 16);
        break;
    case GZP_KEY_EXCHANGE:
        memcpy(key, (void const*)gzp_secret_key, 16);
        gzp_get_host_id(key);
        break;
    case GZP_DATA_EXCHANGE:
        memcpy(key, (void const*)gzp_secret_key, 16);
        memcpy(key, (void const*)CRYPT_Handle.gzp_dyn_key, GZP_DYN_KEY_LENGTH);
        break;
    default:
        return;
    }  

    // Build init vector from "gzp_session_token"
    for(i = 0; i < 16; i++)
    {
        if(i < GZP_SESSION_TOKEN_LENGTH)
        {
            iv[i] = CRYPT_Handle.gzp_session_token[i];
        }
        else
        {
            iv[i] = 0;
        }
    }
		memcpy(nrf_ecb_hal_data.key,key,16);
		memcpy(nrf_ecb_hal_data.cleartext,iv,16);
	  // (void)nrf_ecb_init();
        sd_ecb_block_encrypt(&nrf_ecb_hal_data);
//    // Set up hal_aes using new key and init vector
//    (void)nrf_ecb_init();
//    nrf_ecb_set_key(key);
//    //hal_aes_setup(false, ECB, key, NULL); // Note, here we skip the IV as we use ECB mode

//    // Encrypt IV using ECB mode
//    (void)nrf_ecb_crypt(iv, iv);

    // Encrypt data by XOR'ing with AES output
//		memcpy(iv,nrf_ecb_hal_data.cleartext,16);
//    gzp_xor_cipher(dst, src, iv, length);
       gzp_xor_cipher(dst,src,nrf_ecb_hal_data.ciphertext, length);
}


