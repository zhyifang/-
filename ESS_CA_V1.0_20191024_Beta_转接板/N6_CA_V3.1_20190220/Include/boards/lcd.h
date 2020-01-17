/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef LCD_H__
#define LCD_H__

#include "nrf_gpio.h"

#define BUTTON_START   	8
#define BUTTON_0       	8
#define BUTTON_1       	9
#define BUTTON_STOP    	9
#define BUTTON_PULL    	NRF_GPIO_PIN_PULLUP

#define LCD_DATA	   		10
#define LCD_WR		   		11
#define LCD_RD         	12
#define LCD_cs		   		13
#define HALL		   		 	7

#define RX_PIN_NUMBER  	2
#define TX_PIN_NUMBER  	0
#define HWFC           	false

#endif
