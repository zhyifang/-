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
 * $LastChangedRevision: 25890 $
 */


/** 
 * @file
 * @brief Implementation of Gazell Pairing Library (gzp), Common functions.
 * @defgroup gzp_source_common Gazell Pairing common functions implementation
 * @{
 * @ingroup gzp_04_source
 */


#include "nrf_gzp.h"
#include "nrf_gzll.h"
#include "nrf_ecb.h"
#include <string.h>
#include "nrf_soc.h"

#define SOURCE_FILE NRF_SOURCE_FILE_GZP    ///< File identifer for asserts.

nrf_ecb_hal_data_t  nrf_ecb_hal_data;
/******************************************************************************/
/** @name Global variables
 *  @{ */
/******************************************************************************/
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


/** @} */


/******************************************************************************/
/** @name Misc. external variables.
 *  @{ */
/******************************************************************************/
uint8_t gzp_session_token[GZP_SESSION_TOKEN_LENGTH];
uint8_t gzp_dyn_key[GZP_DYN_KEY_LENGTH];


__INLINE void nrf_gzp_disable_gzll(void)
{
    if(nrf_gzll_is_enabled())
    {
        nrf_gzll_disable();
        __WFI();
        while(nrf_gzll_is_enabled())
        {
        }
    }
}

#ifndef GZP_CRYPT_DISABLE

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
    memcpy(gzp_session_token, (void const*)token, GZP_SESSION_TOKEN_LENGTH);
}

void gzp_crypt_set_dyn_key(const uint8_t* key)
{
    memcpy(gzp_dyn_key, (void const*)key, GZP_DYN_KEY_LENGTH); 
}

void gzp_crypt_get_session_token(uint8_t * dst_token)
{
    memcpy(dst_token, (void const*)gzp_session_token, GZP_SESSION_TOKEN_LENGTH);
}

void gzp_crypt_get_dyn_key(uint8_t* dst_key)
{
    memcpy(dst_key, (void const*)gzp_dyn_key, GZP_DYN_KEY_LENGTH); 
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
        memcpy(key, (void const*)gzp_dyn_key, GZP_DYN_KEY_LENGTH);
        break;
    default:
        return;
    }  

    // Build init vector from "gzp_session_token"
    for(i = 0; i < 16; i++)
    {
        if(i < GZP_SESSION_TOKEN_LENGTH)
        {
            iv[i] = gzp_session_token[i];
        }
        else
        {
            iv[i] = 0;
        }
    }
//		memcpy(nrf_ecb_hal_data.key,key,16);
//		memcpy(nrf_ecb_hal_data.cleartext,iv,16);
//    sd_ecb_block_encrypt(&nrf_ecb_hal_data);
//    // Set up hal_aes using new key and init vector
    (void)nrf_ecb_init();
    nrf_ecb_set_key(key);
//    //hal_aes_setup(false, ECB, key, NULL); // Note, here we skip the IV as we use ECB mode

//    // Encrypt IV using ECB mode
    (void)nrf_ecb_crypt(iv, iv);

    // Encrypt data by XOR'ing with AES output
   gzp_xor_cipher(dst, src, iv, length);
  // gzp_xor_cipher(dst, src,nrf_ecb_hal_data.ciphertext, length);
}

void gzp_random_numbers_generate(uint8_t * dst, uint8_t n)
{
    uint8_t i;

    NRF_RNG->EVENTS_VALRDY=0;
    NRF_RNG->TASKS_START = 1;
    for(i = 0; i < n; i++) 
    {
        while(NRF_RNG->EVENTS_VALRDY==0)
        {}
        dst[i] = (uint8_t)NRF_RNG->VALUE;
        NRF_RNG->EVENTS_VALRDY=0;
    }
    NRF_RNG->TASKS_STOP = 1;
}


/******************************************************************************/
/** @name Implementation of nRF51 specific GZP functions
 *  @{ */
/******************************************************************************/

/**
* @brief Function for setting the Primask variable. Only necessary if ARMCC
* compiler skips __set_PRIMASK at high optimization levels. 
*
* @param primask The primask value. 1 to disable interrupts, 0 otherwise.
*/
static void nrf_gzp_set_primask(uint32_t primask)
{
    #if defined(__CC_ARM)
    //lint -save -e10 -e618 -e438 -e550 -e526 -e628 -e526
    volatile register uint32_t __regPriMask         __ASM("primask");
    __regPriMask = (primask);
    #else
    __set_PRIMASK(primask);
    #endif
    //lint -restore
}

void nrf_gzp_flush_rx_fifo(uint32_t pipe)
{
    static uint8_t dummy_packet[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];
    uint32_t length;

    nrf_gzp_set_primask(1);
    while(nrf_gzll_get_rx_fifo_packet_count(pipe) >0)
    {
        length = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH;
        (void)nrf_gzll_fetch_packet_from_rx_fifo(pipe,dummy_packet,&length);
    }
    nrf_gzp_set_primask(0);
}
/** @} */



/******************************************************************************/
/** @name Implementation of debug functions
 *  @{ */
/******************************************************************************/


/** @} */

/** @} */
#endif
