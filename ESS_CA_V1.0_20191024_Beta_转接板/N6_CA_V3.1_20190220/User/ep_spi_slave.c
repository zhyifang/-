#include "ep_spi_slave.h"
#include "app_util.h"
#include "ep_motor.h"

#define SPI1_TWI1_IRQ_PRI 3
#define TX_BUF_SIZE    5
#define RX_BUF_SIZE    5
#define DEF_CHARACTER 0xAA             /**< SPI default character. Character clocked out in case of an ignored transaction. */      
#define ORC_CHARACTER 0x55             /**< SPI over-read character. Character clocked out after an over-read of the transmit buffer. */    

uint8_t m_tx_buf[TX_BUF_SIZE];
uint8_t m_rx_buf[RX_BUF_SIZE];

/**************电机反馈数据**************/
uint8_t calibrationState = 0;
uint8_t MotorStatus = 0;
uint8_t SetBatteryVoltageNeeded = 0;
uint8_t UserSetBatteryVoltageNeeded = 0; 
uint8_t SetBoatFrontNeeded = 0;
uint8_t MotorFaults = 0;
uint8_t DriverCalibrationStatus = 0;
uint8_t VoltageStatus = 0;

uint8_t UserHandleSideSetFlag = 0;
uint8_t UserHandleSideValue = 0;
uint8_t UserHandleSideSetStatus = 0;

//uint8_t MotorUnderVoltageWarning = 0;
//uint8_t MotorOverTemWarning = 0;
/**************油门方向信息**************/
uint8_t ucAngle = 0;
uint8_t ucSteeringDrection = 3;
uint8_t ucVoltage = 0;

/**************丝杠短标志位**************/
uint8_t ucLeftShortFlag = 0;
uint8_t ucRightShortFlag = 0;

uint8_t ucClearFault = 0;
uint16_t cmdOverTimeCnt = 0;

/**@brief States of the SPI transaction state machine. */
typedef enum
{
    SPI_STATE_INIT,                                 /**< Initialization state. In this state the module waits for a call to @ref spi_slave_buffers_set. */                                                                                             
    SPI_BUFFER_RESOURCE_REQUESTED,                  /**< State where the configuration of the memory buffers, which are to be used in SPI transaction, has started. */
    SPI_BUFFER_RESOURCE_CONFIGURED,                 /**< State where the configuration of the memory buffers, which are to be used in SPI transaction, has completed. */
    SPI_XFER_COMPLETED                              /**< State where SPI transaction has been completed. */
} spi_state_t;

volatile uint8_t *   mp_spi_tx_buf;          /**< SPI slave TX buffer. */
volatile uint8_t *   mp_spi_rx_buf;          /**< SPI slave RX buffer. */
volatile uint32_t    m_spi_tx_buf_size;      /**< SPI slave TX buffer size in bytes. */
volatile uint32_t    m_spi_rx_buf_size;      /**< SPI slave RX buffer size in bytes. */
volatile spi_state_t m_spi_state;            /**< SPI slave state. */

static spi_slave_event_handler_t m_event_callback;  /**< SPI slave event callback function. */

uint32_t spi_slave_init(const spi_slave_config_t * p_spi_slave_config)
{    
    uint32_t err_code;
    uint32_t spi_mode_mask;
    
    if (p_spi_slave_config == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Configure the SPI pins for input.
    NRF_GPIO->PIN_CNF[p_spi_slave_config->pin_miso] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)     |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)   |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)  |
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);

    NRF_GPIO->PIN_CNF[p_spi_slave_config->pin_csn] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)     |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)   |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)  |
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);

    NRF_GPIO->PIN_CNF[p_spi_slave_config->pin_mosi] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)     |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)   |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)  |
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);

    NRF_GPIO->PIN_CNF[p_spi_slave_config->pin_sck] = 
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)     |
        (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)   |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)  |
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);

    NRF_SPIS1->PSELCSN  = p_spi_slave_config->pin_csn;
    NRF_SPIS1->PSELSCK  = p_spi_slave_config->pin_sck;
    NRF_SPIS1->PSELMOSI = p_spi_slave_config->pin_mosi;
    NRF_SPIS1->PSELMISO = p_spi_slave_config->pin_miso;
    NRF_SPIS1->MAXRX    = 0;
    NRF_SPIS1->MAXTX    = 0;
    
    // Configure SPI mode.
    spi_mode_mask = 0;
    err_code      = NRF_SUCCESS;
    switch (p_spi_slave_config->mode)
    {
        case SPI_MODE_0:
            spi_mode_mask = ((SPIS_CONFIG_CPOL_ActiveHigh << SPIS_CONFIG_CPOL_Pos) |
                             (SPIS_CONFIG_CPHA_Leading << SPIS_CONFIG_CPHA_Pos));
            break;

        case SPI_MODE_1:
            spi_mode_mask = ((SPIS_CONFIG_CPOL_ActiveHigh << SPIS_CONFIG_CPOL_Pos) |
                             (SPIS_CONFIG_CPHA_Trailing << SPIS_CONFIG_CPHA_Pos));
            break;

        case SPI_MODE_2:
            spi_mode_mask = ((SPIS_CONFIG_CPOL_ActiveLow << SPIS_CONFIG_CPOL_Pos)  |
                             (SPIS_CONFIG_CPHA_Leading << SPIS_CONFIG_CPHA_Pos));
            break;

        case SPI_MODE_3:
            spi_mode_mask = ((SPIS_CONFIG_CPOL_ActiveLow << SPIS_CONFIG_CPOL_Pos)  |
                             (SPIS_CONFIG_CPHA_Trailing << SPIS_CONFIG_CPHA_Pos));
            break;

        default:
            err_code = NRF_ERROR_INVALID_PARAM;
            break;
    }

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Configure bit order.    
    if (p_spi_slave_config->bit_order == SPIM_LSB_FIRST)
    {
        NRF_SPIS1->CONFIG = (spi_mode_mask | (SPIS_CONFIG_ORDER_LsbFirst << SPIS_CONFIG_ORDER_Pos));
        NRF_SPIS1->DEF    = p_spi_slave_config->def_tx_character;
        NRF_SPIS1->ORC    = p_spi_slave_config->orc_tx_character;
    }
    else
    {
        NRF_SPIS1->CONFIG = (spi_mode_mask | (SPIS_CONFIG_ORDER_MsbFirst << SPIS_CONFIG_ORDER_Pos));
        NRF_SPIS1->DEF    = p_spi_slave_config->def_tx_character;
        NRF_SPIS1->ORC    = p_spi_slave_config->orc_tx_character;
    }

    // Clear possible pending events.
    NRF_SPIS1->EVENTS_END      = 0;
    NRF_SPIS1->EVENTS_ACQUIRED = 0;
    
    // Enable END_ACQUIRE shortcut.        
    NRF_SPIS1->SHORTS = (SPIS_SHORTS_END_ACQUIRE_Enabled << SPIS_SHORTS_END_ACQUIRE_Pos);
    
    m_spi_state = SPI_STATE_INIT; 

    // Set correct IRQ priority and clear any possible pending interrupt.
    NVIC_SetPriority(SPI1_TWI1_IRQn, SPI1_TWI1_IRQ_PRI);    
    NVIC_ClearPendingIRQ(SPI1_TWI1_IRQn);
    
    // Enable IRQ.    
    NRF_SPIS1->INTENSET = (SPIS_INTENSET_ACQUIRED_Enabled << SPIS_INTENSET_ACQUIRED_Pos) |
                          (SPIS_INTENSET_END_Enabled << SPIS_INTENSET_END_Pos);
    NVIC_EnableIRQ(SPI1_TWI1_IRQn);
    
    // Enable SPI slave device.        
    NRF_SPIS1->ENABLE = (SPIS_ENABLE_ENABLE_Enabled << SPIS_ENABLE_ENABLE_Pos);        
    
    return err_code;
}

/**@brief Function for executing the state entry action.
 */
static __INLINE void state_entry_action_execute(void)
{
    spi_slave_evt_t event;
    
    switch (m_spi_state)
    {                             
        case SPI_BUFFER_RESOURCE_REQUESTED:
            NRF_SPIS1->TASKS_ACQUIRE = 1u;                                  
            break;            
     
        case SPI_BUFFER_RESOURCE_CONFIGURED:
            event.evt_type  = SPI_SLAVE_BUFFERS_SET_DONE;
            event.rx_amount = 0;
            event.tx_amount = 0;     
            
            APP_ERROR_CHECK_BOOL(m_event_callback != NULL);
            m_event_callback(event);         
            break;
            
        case SPI_XFER_COMPLETED:        
            event.evt_type  = SPI_SLAVE_XFER_DONE;
            event.rx_amount = NRF_SPIS1->AMOUNTRX;
            event.tx_amount = NRF_SPIS1->AMOUNTTX;    
            
            APP_ERROR_CHECK_BOOL(m_event_callback != NULL);
            m_event_callback(event);
            break;
            
        default:
            // No implementation required.            
            break;
    }
}

/**@brief Function for changing the state of the SPI state machine.
 *
 * @param[in] new_state State where the state machine transits to.
 */
static void sm_state_change(spi_state_t new_state)
{
    m_spi_state = new_state;
    state_entry_action_execute();
}


uint32_t spi_slave_buffers_set(uint8_t * p_tx_buf, 
                               uint8_t * p_rx_buf, 
                               uint8_t   tx_buf_length, 
                               uint8_t   rx_buf_length)
{
    uint32_t err_code;

    if ((p_tx_buf == NULL) || (p_rx_buf == NULL))
    {
        return NRF_ERROR_NULL;
    }
    
    switch (m_spi_state)
    {
        case SPI_STATE_INIT:
        case SPI_XFER_COMPLETED:
        case SPI_BUFFER_RESOURCE_CONFIGURED:        
            mp_spi_tx_buf     = p_tx_buf;
            mp_spi_rx_buf     = p_rx_buf;
            m_spi_tx_buf_size = tx_buf_length;
            m_spi_rx_buf_size = rx_buf_length;        
            err_code          = NRF_SUCCESS;            
                        
            sm_state_change(SPI_BUFFER_RESOURCE_REQUESTED);             
            break;

        case SPI_BUFFER_RESOURCE_REQUESTED:
            err_code = NRF_ERROR_INVALID_STATE; 
            break;
                        
        default:
            // @note: execution of this code path would imply internal error in the design.
            err_code = NRF_ERROR_INTERNAL;             
            break;
    }
    
    return err_code;
}

/**@brief SPI slave interrupt handler.
 *
 * SPI slave interrupt handler, which processes events generated by the SPI device.
 */
void SPI1_TWI1_IRQHandler(void)
{        
    // @note: as multiple events can be pending for processing, the correct event processing order 
    // is as follows:
    // - SPI semaphore acquired event.
    // - SPI transaction complete event.
    
    // Check for SPI semaphore acquired event.
    if (NRF_SPIS1->EVENTS_ACQUIRED != 0)
    {            
        NRF_SPIS1->EVENTS_ACQUIRED = 0;                     
        
        switch (m_spi_state)
        {                
            case SPI_BUFFER_RESOURCE_REQUESTED: 
				NRF_SPIS1->RXDPTR = (uint32_t)mp_spi_rx_buf;				                
				HandleCommand();
                NRF_SPIS1->TXDPTR = (uint32_t)mp_spi_tx_buf;
                NRF_SPIS1->MAXRX  = m_spi_rx_buf_size;
                NRF_SPIS1->MAXTX  = m_spi_tx_buf_size;
                
                NRF_SPIS1->TASKS_RELEASE = 1u;
                
                sm_state_change(SPI_BUFFER_RESOURCE_CONFIGURED);                                                                         
                break;
                
            default:
                // No implementation required.                    
                break;
        }
    }

    // Check for SPI transaction complete event.
    if (NRF_SPIS1->EVENTS_END != 0)
    {
        NRF_SPIS1->EVENTS_END = 0;            
        
        switch (m_spi_state)
        {
            case SPI_BUFFER_RESOURCE_CONFIGURED:                                  
                sm_state_change(SPI_XFER_COMPLETED);                                                             
                break;

            default:
                // No implementation required.                    
                break;                
        }    
    }    
}

uint32_t spi_slave_evt_handler_register(spi_slave_event_handler_t event_handler)
{
    m_event_callback = event_handler;
    
    return (m_event_callback != NULL) ? NRF_SUCCESS : NRF_ERROR_NULL;
}

/**@brief Function for SPI slave event callback.
 *
 * Upon receiving an SPI transaction complete event, LED1 will blink and the buffers will be set.
 *
 * @param[in] event SPI slave driver event.  
 */
static void spi_slave_event_handle(spi_slave_evt_t event)
{
    uint32_t err_code;
    
    if (event.evt_type == SPI_SLAVE_XFER_DONE)
    { 
        err_code = spi_slave_buffers_set(m_tx_buf, m_rx_buf, sizeof(m_tx_buf), sizeof(m_rx_buf));
        APP_ERROR_CHECK(err_code);          
    }
}

uint32_t spi_slave_example_init(void)
{
    uint32_t           err_code;
    spi_slave_config_t spi_slave_config;
        
    err_code = spi_slave_evt_handler_register(spi_slave_event_handle);
    APP_ERROR_CHECK(err_code);    

    spi_slave_config.pin_miso         = SPIS_MISO_PIN;
    spi_slave_config.pin_mosi         = SPIS_MOSI_PIN;
    spi_slave_config.pin_sck          = SPIS_SCK_PIN;
    spi_slave_config.pin_csn          = SPIS_CSN_PIN;
    spi_slave_config.mode             = SPI_MODE_1;            
    spi_slave_config.bit_order        = SPIM_MSB_FIRST;            
    spi_slave_config.def_tx_character = DEF_CHARACTER;      
    spi_slave_config.orc_tx_character = ORC_CHARACTER;      
    
    err_code = spi_slave_init(&spi_slave_config);
    APP_ERROR_CHECK(err_code);
       
    err_code = spi_slave_buffers_set(m_tx_buf, m_rx_buf, sizeof(m_tx_buf), sizeof(m_rx_buf));
    APP_ERROR_CHECK(err_code);            

    return NRF_SUCCESS;
}

/***************填充发送数组****************/
unsigned char cmdTxBuf[5];
void TxCommand(unsigned char cmdbyte)
{
	unsigned char ipos = 0;
	unsigned char cmdtxCs=0;
	unsigned char icnt=0;

	ipos = 0;
	cmdTxBuf[ipos++] = CMD_HEADER;
	switch(cmdbyte)
	{
		case CMD_MOTOR_OPEN_COMMUNICATE:
			cmdTxBuf[ipos++] = RET_SET_MOTOR_VOLTAGE;
			cmdTxBuf[ipos++] = 0X00;
			cmdTxBuf[ipos++] = ucVoltage;
			break;
#if 1
		case RET_SET_MOTOR_CALIBRATION:
			cmdTxBuf[ipos++] = RET_SET_MOTOR_CALIBRATION;
			cmdTxBuf[ipos++] = 0X00;
			cmdTxBuf[ipos++] = 0X00;	
			break;	
#endif
		case CMD_MOTOR_CAL_BACK:
			if((calibrationState&0X40)== 0x40)//校准中
			{
				cmdTxBuf[ipos++] = RET_SET_MOTOR_CALIBRATION;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = 0X00;//相当于那边还是在校准
			}
			else if((calibrationState&0X10)== 0x10)
			{
				cmdTxBuf[ipos++] = RET_SET_MOTOR_POWER;
				cmdTxBuf[ipos++] = ucSteeringDrection;
				cmdTxBuf[ipos++] = ucAngle;
			}
			else if((calibrationState&0X20)== 0x20)
			{
				cmdTxBuf[ipos++] = RET_MOTOR_FAULT_STOP;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = 0X00;//丝杠短太多
			}
			break;
		case CMD_MOTOR_STATE:
			//#if 1
			if(Motor_Handle.SetMotorOnFlag == true)//开机
			{
				cmdTxBuf[ipos++] = RET_MOTOR_SYS_ON;
				cmdTxBuf[ipos++] = Motor_Handle.MotorPosition2Motor>>8;
				cmdTxBuf[ipos++] = Motor_Handle.MotorPosition2Motor;
				cmdOverTimeCnt++;
				if(cmdOverTimeCnt>MOTOR_COMM_OVER_TIME)
				{
					Motor_Handle.SetMotorOnFlag = false;
					cmdOverTimeCnt = 0;
				}
			}
			else if(Motor_Handle.SetMotorOffFlag == true)
			{
				cmdTxBuf[ipos++] = RET_MOTOR_SYS_OFF;
				cmdTxBuf[ipos++] = 0;
				cmdTxBuf[ipos++] = false;
				cmdOverTimeCnt++;
				if(cmdOverTimeCnt>MOTOR_COMM_OVER_TIME)
				{
					Motor_Handle.SetMotorOffFlag = false;
					cmdOverTimeCnt = 0;
				}

			}
			else if((MotorStatus==0)&&(SetBoatFrontNeeded==0)&&(UserSetBatteryVoltageNeeded==0))
			{
				cmdTxBuf[ipos++] = RET_SET_MOTOR_POWER;
				cmdTxBuf[ipos++] = ucSteeringDrection;
				cmdTxBuf[ipos++] = ucAngle;			
			}
			else if((MotorStatus==0)&&(SetBoatFrontNeeded==1)&&(UserSetBatteryVoltageNeeded==0))
			{
				cmdTxBuf[ipos++] = RET_DRIVE_ZERO_POSITION;
				cmdTxBuf[ipos++] = ucSteeringDrection;
				cmdTxBuf[ipos++] = ucAngle;
			}
			else if((MotorStatus==0)&&(SetBoatFrontNeeded==0)&&(UserSetBatteryVoltageNeeded==1))
			{
				cmdTxBuf[ipos++] = RET_SET_MOTOR_VOLTAGE;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = ucVoltage;
			}
			else if((MotorStatus==0)&&(SetBoatFrontNeeded==1)&&(UserSetBatteryVoltageNeeded==1))
			{
				cmdTxBuf[ipos++] = RET_SET_MOTOR_VOLTAGE;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = ucVoltage;
			}
			else if((MotorStatus==0)&&(UserHandleSideSetFlag == 1))
			{
				cmdTxBuf[ipos++] = RET_MOTOR_HANDLE_SIDE;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = UserHandleSideValue;
			}
			else//故障停机指令
			{			
				cmdTxBuf[ipos++] = RET_MOTOR_FAULT_STOP;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = 0X00;			
			}
			ucClearFault = 0;//不清除错误，只有在故障之后清一次
			break;
		case CMD_MOTOR_ZERO_ACKNOWLEGE:
			if (DriverCalibrationStatus ==1)
			{
				SetBoatFrontNeeded = 0;
				cmdTxBuf[ipos++] = RET_SET_MOTOR_POWER;
				cmdTxBuf[ipos++] = ucSteeringDrection;
				cmdTxBuf[ipos++] = ucAngle;				
			}
			else if(DriverCalibrationStatus ==2)
			{
				cmdTxBuf[ipos++] = RET_DRIVE_ZERO_POSITION;
				cmdTxBuf[ipos++] = ucSteeringDrection;
				cmdTxBuf[ipos++] = ucAngle;			
			}
			break;
		case CMD_MOTOR_HANDLE_SIDE_SET_ACKNOWLEGE:
			if (UserHandleSideSetStatus ==1)
			{
				UserHandleSideSetFlag = 0;//设置成功消除标志
				cmdTxBuf[ipos++] = RET_SET_MOTOR_POWER;
				cmdTxBuf[ipos++] = ucSteeringDrection;
				cmdTxBuf[ipos++] = ucAngle;				
			}
			else if(UserHandleSideSetStatus ==2)
			{
				cmdTxBuf[ipos++] = RET_MOTOR_HANDLE_SIDE;
				cmdTxBuf[ipos++] = 0;
				cmdTxBuf[ipos++] = UserHandleSideValue;			
			}
			break;
		case CMD_MOTOR_VOLTAGE_ACKNOWLEGE:
			if(VoltageStatus ==1)
			{
				UserSetBatteryVoltageNeeded = 0;
				SetBatteryVoltageNeeded = 0;
				Motor_VoltageNeedSetFlag(&Motor_Handle,SetBatteryVoltageNeeded);
				if(((calibrationState&0X40)== 0x40)||(calibrationState==0))//没校准
				{
					cmdTxBuf[ipos++] = RET_SET_MOTOR_CALIBRATION;
					cmdTxBuf[ipos++] = 0X00;
					cmdTxBuf[ipos++] = 0X00;
				}
				else if(calibrationState==0x10)//校准成功
				{
					cmdTxBuf[ipos++] = RET_SET_MOTOR_POWER;
					cmdTxBuf[ipos++] = ucSteeringDrection;
					cmdTxBuf[ipos++] = ucAngle;				
				}
				else if(calibrationState==0x20)//校准失败
				{
					cmdTxBuf[ipos++] = RET_MOTOR_FAULT_STOP;
					cmdTxBuf[ipos++] = 0X00;
					cmdTxBuf[ipos++] = ucClearFault;			
				}
			}
			else if(VoltageStatus ==2)
			{
				cmdTxBuf[ipos++] = RET_SET_MOTOR_VOLTAGE;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = ucVoltage;
			}
			else if(VoltageStatus ==3)//电机有故障
			{
				cmdTxBuf[ipos++] = RET_MOTOR_FAULT_STOP;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = 0;
				SetBatteryVoltageNeeded = 0;
				UserSetBatteryVoltageNeeded = 0;
				Motor_VoltageNeedSetFlag(&Motor_Handle,SetBatteryVoltageNeeded);
			}
			break;
		case CMD_MOTOR_STOP_ACKNOWLEGE:
			if((ucLeftShortFlag==1)||(ucRightShortFlag==1)||(MotorFaults!=0))
			{
				if(UserSetBatteryVoltageNeeded==1)
				{
					cmdTxBuf[ipos++] = RET_SET_MOTOR_VOLTAGE;
					cmdTxBuf[ipos++] = 0X00;
					cmdTxBuf[ipos++] = ucVoltage;
					MotorFaults = 0;
				}
				else
				{
					cmdTxBuf[ipos++] = RET_MOTOR_FAULT_STOP;
					cmdTxBuf[ipos++] = 0X00;
					cmdTxBuf[ipos++] = ucClearFault;//100 ：清除故障 
				}
			}
			else
			{
				if(UserSetBatteryVoltageNeeded==1)
				{
					cmdTxBuf[ipos++] = RET_SET_MOTOR_VOLTAGE;
					cmdTxBuf[ipos++] = 0X00;
					cmdTxBuf[ipos++] = ucVoltage;
					MotorFaults = 0;
				}
				else
				{
					cmdTxBuf[ipos++] = RET_SET_MOTOR_POWER;
					cmdTxBuf[ipos++] = ucSteeringDrection;
					cmdTxBuf[ipos++] = ucAngle;
				}
			}
	
			break;
		case CMD_MOTOR_OFF_ACKNOWLEGE:
			//motor receive off cmd
			Comm_SetSetupType(SysOffRetType);//return to ag
			//stop the motor 
			cmdTxBuf[ipos++] = RET_MOTOR_FAULT_STOP;
			cmdTxBuf[ipos++] = 0X00;
			cmdTxBuf[ipos++] = 0X00;
			break;
		case CMD_MOTOR_ON_ACKNOWLEGE:
			//motor receive on cmd
			Comm_SetSetupType(SysOnRetType);//return to ag

			//set motor angle & dir
			cmdTxBuf[ipos++] = RET_SET_MOTOR_POWER;
			cmdTxBuf[ipos++] = ucSteeringDrection;
			cmdTxBuf[ipos++] = ucAngle;			
		
			break;
		case CMD_MOTOR_POSITION_ACKNOWLEGE://motor  position receive
		//#if 1
			if(Motor_Handle.SetMotorOnFlag == true)//开机
			{
				cmdTxBuf[ipos++] = RET_MOTOR_SYS_ON;
				cmdTxBuf[ipos++] = Motor_Handle.MotorPosition2Motor>>8;
				cmdTxBuf[ipos++] = Motor_Handle.MotorPosition2Motor;
				cmdOverTimeCnt++;
				if(cmdOverTimeCnt>MOTOR_COMM_OVER_TIME)
				{
					Motor_Handle.SetMotorOnFlag = false;
					cmdOverTimeCnt = 0;
				}
			}
			else if(Motor_Handle.SetMotorOffFlag == true)
			{
				cmdTxBuf[ipos++] = RET_MOTOR_SYS_OFF;
				cmdTxBuf[ipos++] = 0;
				cmdTxBuf[ipos++] = false;
				cmdOverTimeCnt++;
				if(cmdOverTimeCnt>MOTOR_COMM_OVER_TIME)
				{
					Motor_Handle.SetMotorOffFlag = false;
					cmdOverTimeCnt = 0;
				}
			}
			else if((MotorStatus==0)&&(SetBoatFrontNeeded==0)&&(UserSetBatteryVoltageNeeded==0))
			{
				cmdTxBuf[ipos++] = RET_SET_MOTOR_POWER;
				cmdTxBuf[ipos++] = ucSteeringDrection;
				cmdTxBuf[ipos++] = ucAngle;			
			}
			else if((MotorStatus==0)&&(SetBoatFrontNeeded==1)&&(UserSetBatteryVoltageNeeded==0))
			{
				cmdTxBuf[ipos++] = RET_DRIVE_ZERO_POSITION;
				cmdTxBuf[ipos++] = ucSteeringDrection;
				cmdTxBuf[ipos++] = ucAngle;
			}
			else if((MotorStatus==0)&&(SetBoatFrontNeeded==0)&&(UserSetBatteryVoltageNeeded==1))
			{
				cmdTxBuf[ipos++] = RET_SET_MOTOR_VOLTAGE;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = ucVoltage;
			}
			else if((MotorStatus==0)&&(SetBoatFrontNeeded==1)&&(UserSetBatteryVoltageNeeded==1))
			{
				cmdTxBuf[ipos++] = RET_SET_MOTOR_VOLTAGE;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = ucVoltage;
			}
			else if((MotorStatus==0)&&(UserHandleSideSetFlag == 1))
			{
				cmdTxBuf[ipos++] = RET_MOTOR_HANDLE_SIDE;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = UserHandleSideValue;
			}
			else//故障停机指令
			{			
				cmdTxBuf[ipos++] = RET_MOTOR_FAULT_STOP;
				cmdTxBuf[ipos++] = 0X00;
				cmdTxBuf[ipos++] = 0X00;			
			}
			ucClearFault = 0;//不清除错误，只有在故障之后清一次
			break;
		default:
			break;
	}

	for(icnt=0,cmdtxCs=0;icnt<ipos;icnt++)
	{
		cmdtxCs^=cmdTxBuf[icnt];
	}
	cmdTxBuf[ipos++] = cmdtxCs&0xff;
	for(cmdtxCs=0;cmdtxCs<ipos;cmdtxCs++)
	{
		m_tx_buf[cmdtxCs] = cmdTxBuf[cmdtxCs];
	}

}


//取接收循环队列数据
uint8_t ucCheckSum=0;
uint8_t ucCmdData[3];
uint8_t RxCommand(void)
{
	if(m_rx_buf[0] == CMD_HEADER)
	{
		ucCheckSum = CMD_HEADER;
	}
	else
		return 0;
	
	if((m_rx_buf[1]>=CMD_MOTOR_CAL_BACK)&&(m_rx_buf[1]<=CMD_MOTOR_OFF_ACKNOWLEGE))
	{
		ucCmdData[0] = m_rx_buf[1];
		ucCheckSum^=m_rx_buf[1];
		
		ucCmdData[1] = m_rx_buf[2];
		ucCheckSum^=m_rx_buf[2];
		
		ucCmdData[2] = m_rx_buf[3];
		ucCheckSum^=m_rx_buf[3];
		
		if((ucCheckSum&0xff)==m_rx_buf[4])
			return ucCmdData[0];
		else
			return 0;
	}
	else
		return 0;
}

//处理接收数据
unsigned char HandleCommand(void)
{
	unsigned char cmdbyte;
	if(RxCommand())
	{
		cmdbyte = ucCmdData[0];
		switch(cmdbyte)
		{
			case CMD_MOTOR_CAL_BACK:
				SetBatteryVoltageNeeded = 0;
				Motor_VoltageNeedSetFlag(&Motor_Handle,SetBatteryVoltageNeeded);
				VoltageStatus = 0;
				calibrationState = ucCmdData[2];
				if(calibrationState == 0x21)
					ucLeftShortFlag = 1;
				else if(calibrationState == 0x22)
					ucRightShortFlag = 1;
				else if(calibrationState == 0x23)
				{
					ucLeftShortFlag = 1;
					ucRightShortFlag = 1;
				}
				else
				{
					ucLeftShortFlag = 0;
					ucRightShortFlag = 0;
				}
				
			break;		
			case CMD_MOTOR_STATE:
				VoltageStatus = 0;
				MotorStatus = ucCmdData[2];	
#if 0
				if((MotorStatus&0x01)==0x01)			
					MotorFaults|=MOTOR_Status_UVWarn;
				else
					MotorFaults&=~MOTOR_Status_UVWarn;
				if((MotorStatus&0x02)==0x02)
					MotorFaults	|=MOTOR_Status_MotOV;
				else
					MotorFaults	&=~MOTOR_Status_MotOV;
				if((MotorStatus&0x04)==0x04)
					MotorFaults	|=MOTOR_Status_MotUV;
				else
					MotorFaults	&=~MOTOR_Status_MotUV;
				if((MotorStatus&0x08)==0x08)
					MotorFaults	|=MOTOR_Status_MotOC;
				else
					MotorFaults	&=~MOTOR_Status_MotOC;
				if((MotorStatus&0x10)==0x10)
					MotorFaults	|=MOTOR_Status_Stall;	
				else
					MotorFaults	&=~MOTOR_Status_Stall;
				if((MotorStatus&0x20)==0x20)
					MotorFaults	|=MOTOR_Status_MotOT;	
				else
					MotorFaults	&=~MOTOR_Status_MotOT;
				if((MotorStatus&0x40)==0x40)
					MotorFaults |=MOTOR_Status_OTWarn;
				else
					MotorFaults &=~MOTOR_Status_OTWarn;
				if((MotorStatus&0x80)==0x80)			
					MotorFaults|=MOTOR_Status_SENSORFault;
				else
					MotorFaults &=~MOTOR_Status_SENSORFault;
#endif
				Motor_SetErrorFlag(&Motor_Handle, MotorStatus);
			break;
			
			case CMD_MOTOR_OPEN_COMMUNICATE:
				SetBatteryVoltageNeeded= 1;
				Motor_VoltageNeedSetFlag(&Motor_Handle,SetBatteryVoltageNeeded);
				VoltageStatus = ucCmdData[2];
			break;
			
			case CMD_MOTOR_ZERO_ACKNOWLEGE:
				DriverCalibrationStatus = ucCmdData[2];	
			break;
			
			case CMD_MOTOR_VOLTAGE_ACKNOWLEGE:
				VoltageStatus = ucCmdData[2];	
			break;
			
			case CMD_MOTOR_STOP_ACKNOWLEGE:
				break;

			case CMD_MOTOR_HANDLE_SIDE_SET_ACKNOWLEGE://左右舷设置反馈
				UserHandleSideSetStatus = ucCmdData[2];
				break;
			case CMD_MOTOR_POSITION_ACKNOWLEGE://motor  position receive
				Motor_Handle.MotorPosition2Steering = (ucCmdData[1]<<8)|ucCmdData[2];	
			break;
			case CMD_MOTOR_OFF_ACKNOWLEGE:
			 	Motor_Handle.SetMotorOffFlag = false;
				cmdOverTimeCnt = 0;
			break;

			case CMD_MOTOR_ON_ACKNOWLEGE:
				Motor_Handle.SetMotorOnFlag = false;
				cmdOverTimeCnt = 0;
			break;
			default:
				break;
		}		
		TxCommand(cmdbyte);
		Motor_ClearFailCount();
		Motor_ClearErrorFlag(&Motor_Handle, ERR_COMM_FAIL_DETECTED_BIT);
	}
	return 0;
}

