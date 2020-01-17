/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
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

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "twi_master_int.h"
#include "ep_tick.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
//#include "ep_gps.h"
//#include "ep_pstorage.h"

/* Max cycles approximately to wait on RXDREADY and TXDREADY event, this is optimum way instead of using timers, this is not power aware, negetive side is this is not power aware */
#define MAX_TIMEOUT_LOOPS             (1000UL)        /*!< MAX while loops to wait for RXD/TXD event */

static uint8_t twi_pinselect_scl, twi_pinselect_sda; 
volatile uint8_t tx_bytes_to_send, rx_bytes_to_receive;
volatile uint8_t *tx_data_ptr, *rx_data_ptr;
volatile bool twi_operation_complete, twi_ack_received, twi_blocking_enabled;
twi_config_t cfg;
//extern GPS_Handle_t Gps_Handle;
//extern PSTORAGE_Handle_t PSTORAGE_Handle;

static bool twi_master_clear_bus(void);
void TWI_INTERRUPT()
{   
    if(TWI->EVENTS_TXDSENT)
    {
        TWI->EVENTS_TXDSENT = 0;
        if(tx_bytes_to_send)
        {
            TWI->TXD = *tx_data_ptr++;  
            tx_bytes_to_send--;
        }
        else 
        {
            if(rx_bytes_to_receive == 0)
            {
                TWI->TASKS_STOP = 1; 
            }
            else
            {
                NRF_PPI->CHENSET = (1 << TWI_PPI_CH0);
                TWI->TASKS_STARTRX = 1;
            }
        }
    }
    
    if(TWI->EVENTS_STOPPED)
    {
        TWI->EVENTS_STOPPED = 0;
        twi_operation_complete = true;
        twi_ack_received = true;
    }
    
    if(TWI->EVENTS_RXDREADY)
    {
        TWI->EVENTS_RXDREADY = 0;
        *rx_data_ptr++ = TWI->RXD;
//		Gps_Handle.DataPtr++;
        
        if (--rx_bytes_to_receive == 1)
        {
            NRF_PPI->CH[TWI_PPI_CH0].TEP = (uint32_t)&TWI->TASKS_STOP;
        }
        
        if(rx_bytes_to_receive > 0)
        {
            TWI->TASKS_RESUME = 1;
        }

    }
    
    if(TWI->EVENTS_ERROR)
    {
        TWI->EVENTS_ERROR = 0;
        twi_operation_complete = true;
        twi_ack_received = false;
    }
}

bool twi_master_write(uint8_t address, uint8_t *tx_data, uint8_t tx_data_length)
{
    if (tx_data_length == 0)
    {    
        return false;
    }

    if(!twi_blocking_enabled)
    {
        // If an operation is already underway, delay operation
        twi_wait_for_completion();
    }
 
    NRF_PPI->CHENCLR = (1 << TWI_PPI_CH0);    
    TWI->ADDRESS = address;
    tx_data_ptr = tx_data;
    tx_bytes_to_send = tx_data_length - 1;
    rx_bytes_to_receive = 0;
    TWI->TXD = *tx_data_ptr++;
    TWI->TASKS_STARTTX = 1;
    twi_operation_complete = false;
    twi_wait_for_completion();
    return true;
}

bool twi_master_write_read(uint8_t address, uint8_t *tx_data, uint8_t tx_data_length, uint8_t *rx_data, uint8_t rx_data_length)
{
    if (tx_data_length == 0 || rx_data_length == 0)
    {    
        return false;
    }

    if(!twi_blocking_enabled)
    {
        // If an operation is already underway, delay operation
        twi_wait_for_completion();
    }
    
    TWI->ADDRESS = address;
    tx_data_ptr = tx_data;
    tx_bytes_to_send = tx_data_length - 1;
    rx_data_ptr = rx_data;
    rx_bytes_to_receive = rx_data_length;
	
       
    if (rx_bytes_to_receive == 1)
    {
        NRF_PPI->CH[TWI_PPI_CH0].TEP = (uint32_t)&TWI->TASKS_STOP;
    }
    else
    {
        NRF_PPI->CH[TWI_PPI_CH0].TEP = (uint32_t)&TWI->TASKS_SUSPEND;
    }
    NRF_PPI->CHENCLR = (1 << TWI_PPI_CH0);
    
    TWI->TXD = *tx_data_ptr++;
    TWI->TASKS_STARTTX = 1;
    twi_operation_complete = false;
    return true;
}

/**
 * Detects stuck slaves (SDA = 0 and SCL = 1) and tries to clear the bus.
 *
 * @return
 * @retval false Bus is stuck.
 * @retval true Bus is clear.
 */
static bool twi_master_clear_bus(void)
{
    bool bus_clear;
    TWI_SDA_HIGH();
    TWI_SCL_HIGH();		
	TWI_DELAY();

    if (TWI_SDA_READ() == 1 && TWI_SCL_READ() == 1)
    {
        bus_clear = true;
		twi_operation_complete = true;
    }
    else
    {
        uint_fast8_t i;
        bus_clear = false;

        // Clock max 18 pulses worst case scenario(9 for master to send the rest of command and 9 for slave to respond) to SCL line and wait for SDA come high
        for (i = 18; i > 0; i--)
        {
            TWI_SCL_LOW();	
			TWI_DELAY();
            TWI_SCL_HIGH();		
			TWI_DELAY();

            if (TWI_SDA_READ() == 1)
            {
                bus_clear = true;
				twi_operation_complete = true;
                break;
            }
        }
    }	
	
	if(TWI->EVENTS_SUSPENDED)
	{
		TWI->TASKS_RESUME = 1;		
	}
    return bus_clear;
}

bool twi_wait_for_completion(void)
{
    static uint16_t twi_delay_cnt;
    twi_delay_cnt = 0;
    while(twi_operation_complete == false)
    {
        if(twi_delay_cnt++ > MAX_TIMEOUT_LOOPS)
        {
            twi_master_clear_bus();
        }
    }
    return true;    
}

bool twi_master_init(twi_config_t *cfg)
{
    NVIC_DisableIRQ(TWI_INTERRUPT_NO);
    TWI->ENABLE = 0;
    
    twi_operation_complete = true;
    twi_ack_received = true;
    
    twi_pinselect_scl = cfg->pinselect_scl;
    twi_pinselect_sda = cfg->pinselect_sda;
    
    twi_blocking_enabled = (cfg->blocking_mode == TWI_BLOCKING_ENABLED ? 1 : 0);
          
    /* To secure correct signal levels on the pins used by the TWI
       master when the system is in OFF mode, and when the TWI master is 
       disabled, these pins must be configured in the GPIO peripheral.
    */ 
    NRF_GPIO->PIN_CNF[twi_pinselect_scl] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos)
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);    

    NRF_GPIO->PIN_CNF[twi_pinselect_sda] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos)
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos); 
    
    TWI->EVENTS_RXDREADY = 0;
    TWI->EVENTS_TXDSENT = 0;
    TWI->PSELSCL = twi_pinselect_scl;
    TWI->PSELSDA = twi_pinselect_sda;
    
    switch(cfg->frequency)
    {
        case TWI_FREQ_100KHZ:
            TWI->FREQUENCY = TWI_FREQUENCY_FREQUENCY_K100 << TWI_FREQUENCY_FREQUENCY_Pos;
            break;
        case TWI_FREQ_250KHZ:
            TWI->FREQUENCY = TWI_FREQUENCY_FREQUENCY_K250 << TWI_FREQUENCY_FREQUENCY_Pos;
            break;
        case TWI_FREQ_400KHZ:
            TWI->FREQUENCY = TWI_FREQUENCY_FREQUENCY_K400 << TWI_FREQUENCY_FREQUENCY_Pos;
            break;
    }

    NRF_PPI->CH[TWI_PPI_CH0].EEP = (uint32_t)&TWI->EVENTS_BB;
    NRF_PPI->CH[TWI_PPI_CH0].TEP = (uint32_t)&TWI->TASKS_SUSPEND;
    NRF_PPI->CHENCLR = (1 << TWI_PPI_CH0);
    NVIC_SetPriority(TWI_INTERRUPT_NO, TWI_IRQ_PRIORITY_NO_SD);
    NVIC_EnableIRQ(TWI_INTERRUPT_NO);

    TWI->INTENSET = TWI_INTENSET_TXDSENT_Msk | TWI_INTENSET_STOPPED_Msk | TWI_INTENSET_ERROR_Msk | TWI_INTENSET_RXDREADY_Msk;
    TWI->ENABLE = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;
    return twi_master_clear_bus();
}
