#ifndef __EP_SPI_SLAVE__H
#define __EP_SPI_SLAVE__H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "nrf.h"
#include "nrf_error.h"
#include "app_error.h"

#define MOTOR_COMM_OVER_TIME 1000

#define SPIS_MISO_PIN		24
#define SPIS_MOSI_PIN		22
#define SPIS_SCK_PIN		25
#define SPIS_CSN_PIN		23

#define CMD_HEADER				0XEF

#define RET_SET_MOTOR_POWER    				0X70
#define RET_SET_MOTOR_CALIBRATION 			0X71
#define RET_DRIVE_ZERO_POSITION       0X72
#define RET_SET_MOTOR_VOLTAGE			0X73
#define RET_MOTOR_FAULT_STOP          0X74
#define RET_MOTOR_HANDLE_SIDE          0X75
#define RET_MOTOR_SYS_ON          0X76
#define RET_MOTOR_SYS_OFF          0X77

#define CMD_MOTOR_CAL_BACK                 0X50
#define CMD_MOTOR_STATE             0X51
#define CMD_MOTOR_OPEN_COMMUNICATE         0X52
#define CMD_MOTOR_ZERO_ACKNOWLEGE     0X53
#define CMD_MOTOR_VOLTAGE_ACKNOWLEGE   0X54
#define CMD_MOTOR_STOP_ACKNOWLEGE       0x55
#define CMD_MOTOR_HANDLE_SIDE_SET_ACKNOWLEGE       0x56
#define CMD_MOTOR_ON_ACKNOWLEGE       0x57
#define CMD_MOTOR_OFF_ACKNOWLEGE       0x59
#define CMD_MOTOR_POSITION_ACKNOWLEGE       0x58

extern uint8_t ucLeftShortFlag;
extern uint8_t ucRightShortFlag;
extern uint8_t MotorFualts;

extern uint8_t ucAngle;
extern uint8_t ucSteeringDrection;
extern uint8_t ucVoltage;

extern uint8_t DriverCalibrationStatus;
extern uint8_t VoltageStatus;
extern uint8_t SetBoatFrontNeeded;
extern uint8_t SetBatteryVoltageNeeded;
extern uint8_t UserSetBatteryVoltageNeeded;
extern uint8_t ucClearFault;

extern uint8_t UserHandleSideSetFlag;
extern uint8_t UserHandleSideValue;
extern uint8_t UserHandleSideSetStatus;

typedef enum
{
	MOTOR_Status_UVWarn              =(1 << 0),
	MOTOR_Status_MotOV               =(1 << 1),
	MOTOR_Status_MotUV               =(1 << 2),
	MOTOR_Status_MotOC               =(1 << 3),
	MOTOR_Status_Stall               =(1 << 4),
	MOTOR_Status_MotOT               =(1 << 5),
	MOTOR_Status_OTWarn              =(1 << 6),
	MOTOR_Status_SENSORFault         =(1 << 7)
}MOTOR_Status_e;

/**@brief SPI transaction bit order definitions. */
typedef enum
{
    SPIM_LSB_FIRST,                         /**< Least significant bit shifted out first. */
    SPIM_MSB_FIRST                          /**< Most significant bit shifted out first. */
} spi_slave_endian_t;

/**@brief SPI mode definitions for clock polarity and phase. */
typedef enum
{
    SPI_MODE_0,                             /**< (CPOL = 0, CPHA = 0). */
    SPI_MODE_1,                             /**< (CPOL = 0, CPHA = 1). */
    SPI_MODE_2,                             /**< (CPOL = 1, CPHA = 0). */
    SPI_MODE_3                              /**< (CPOL = 1, CPHA = 1). */
} spi_slave_mode_t;

/**@brief SPI peripheral device configuration data. */
typedef struct 
{
    uint32_t           pin_miso;            /**< SPI MISO pin. */
    uint32_t           pin_mosi;            /**< SPI MOSI pin. */
    uint32_t           pin_sck;             /**< SPI SCK pin. */
    uint32_t           pin_csn;             /**< SPI CSN pin. */
    spi_slave_mode_t   mode;                /**< SPI mode. */
    spi_slave_endian_t bit_order;           /**< SPI transaction bit order. */    
    uint8_t            def_tx_character;    /**< Device configuration mode default character (DEF). Character clocked out in case of an ignored transaction. */    
    uint8_t            orc_tx_character;    /**< Device configuration mode over-read character. Character clocked out after an over-read of the transmit buffer. */        
} spi_slave_config_t;

/**@brief Event callback function event definitions. */
typedef enum
{
    SPI_SLAVE_BUFFERS_SET_DONE,             /**< Memory buffer set event. Memory buffers have been set successfully to the SPI slave device and SPI transactions can be done. */
    SPI_SLAVE_XFER_DONE,                    /**< SPI transaction event. SPI transaction has been completed. */  
    SPI_SLAVE_EVT_TYPE_MAX                  /**< Enumeration upper bound. */      
} spi_slave_evt_type_t;

/**@brief Struct containing event context from the SPI slave driver. */
typedef struct
{
    spi_slave_evt_type_t evt_type;          /**< Type of event. */    
    uint32_t             rx_amount;         /**< Number of bytes received in last transaction (parameter is only valid upon @ref SPI_SLAVE_XFER_DONE event). */
    uint32_t             tx_amount;         /**< Number of bytes transmitted in last transaction (parameter is only valid upon @ref SPI_SLAVE_XFER_DONE event). */    
} spi_slave_evt_t;

/**@brief SPI slave event callback function type.
 *
 * @param[in] event                 SPI slave driver event.  
 */
typedef void (*spi_slave_event_handler_t)(spi_slave_evt_t event);

/**@brief Function for registering a handler for SPI slave driver event.
 *
 * @note Multiple registration requests will overwrite any possible existing registration. 
 *
 * @param[in] event_handler         The function to be called by the SPI slave driver upon event.
 *
 * @retval NRF_SUCCESS              Operation success.
 * @retval NRF_ERROR_NULL           Operation success. NULL handler registered.    
 */
uint32_t spi_slave_evt_handler_register(spi_slave_event_handler_t event_handler);

/**@brief Function for initializing the SPI slave device.
 *
 * @param[in] p_spi_slave_config    SPI peripheral device configuration data.
 *
 * @retval NRF_SUCCESS              Operation success.
 * @retval NRF_ERROR_NULL           Operation failure. NULL pointer supplied.    
 * @retval NRF_ERROR_INVALID_PARAM  Operation failure. Invalid parameter supplied.
 */
uint32_t spi_slave_init(const spi_slave_config_t * p_spi_slave_config);

/**@brief Function for preparing the SPI slave device ready for a single SPI transaction.
 * 
 * Function prepares the SPI slave device to be ready for a single SPI transaction. It configures 
 * the SPI slave device to use the memory, supplied with the function call, in SPI transactions. 
 * 
 * The @ref spi_slave_event_handler_t will be called with appropriate event @ref 
 * spi_slave_evt_type_t when either the memory buffer configuration or SPI transaction has been 
 * completed.
 *
 * @note The callback function @ref spi_slave_event_handler_t can be called before returning from 
 * this function, since it is called from the SPI slave interrupt context.
 *
 * @note This function can be called from the callback function @ref spi_slave_event_handler_t 
 * context.
 *
 * @note Client application must call this function after every @ref SPI_SLAVE_XFER_DONE event if it 
 * wants the SPI slave driver to be ready for possible new SPI transaction. 
 *
 * @param[in] p_tx_buf              Pointer to the TX buffer.
 * @param[in] p_rx_buf              Pointer to the RX buffer.
 * @param[in] tx_buf_length         Length of the TX buffer in bytes.
 * @param[in] rx_buf_length         Length of the RX buffer in bytes. 
 *
 * @retval NRF_SUCCESS              Operation success.
 * @retval NRF_ERROR_NULL           Operation failure. NULL pointer supplied.   
 * @retval NRF_ERROR_INVALID_STATE  Operation failure. SPI slave device in incorrect state.
 * @retval NRF_ERROR_INTERNAL       Operation failure. Internal error ocurred.
 */
uint32_t spi_slave_buffers_set(uint8_t * p_tx_buf, 
                               uint8_t * p_rx_buf, 
                               uint8_t   tx_buf_length, 
                               uint8_t   rx_buf_length);

uint32_t spi_slave_evt_handler_register(spi_slave_event_handler_t event_handler);
uint32_t spi_slave_example_init(void);
unsigned char HandleCommand(void);
#endif
