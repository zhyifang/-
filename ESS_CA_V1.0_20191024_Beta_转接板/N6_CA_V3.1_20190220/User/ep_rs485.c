#include "ep_rs485.h"
#include "ep_comm.h"
#include "nrf_gpio.h"
#include "ep_tick.h"
#include "ep_battery.h"

extern Comm_Handle_t Comm_Handle;
extern Battery_Handle_t Battery_Handle;

RS485_Mode_e RS485_Mode = RS485_IDLE;

uint8_t RS485_LockMode(RS485_Mode_e Mode) 
{
    if(RS485_Mode == RS485_IDLE)
        RS485_Mode = Mode;
    else
        return 0;
    return 1;
}

uint8_t RS485_ReleaseMode(RS485_Mode_e Mode) 
{
    if(RS485_Mode == RS485_IDLE)
        return 1;
    else if(RS485_Mode == Mode)
        RS485_Mode = RS485_IDLE;
    else
        return 0;
    return 1;
}

RS485_Mode_e RS485_GetMode(void) 
{
    return RS485_Mode;
}
    
void RS485_SetRx(void) 
{
	nrf_gpio_pin_write(RS485_RE_DE_PIN, 0);	
	NRF_UART0->TASKS_STOPTX = 1;
	NRF_UART0->TASKS_STARTRX = 1;
}

void RS485_SetTx(void) 
{
	NRF_UART0->TASKS_STARTTX = 1;
	NRF_UART0->TASKS_STOPRX = 1;
	nrf_gpio_pin_write(RS485_RE_DE_PIN, 1);
}

void RS485_Init(void)
{	
	nrf_gpio_cfg_output(RS485_TX_PIN);
	nrf_gpio_cfg_input(RS485_RX_PIN, NRF_GPIO_PIN_PULLUP);
	nrf_gpio_cfg_output(RS485_RE_DE_PIN);

	NRF_UART0->PSELTXD = RS485_TX_PIN;
	NRF_UART0->PSELRXD = RS485_RX_PIN;

	NRF_UART0->INTENSET = UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos;
	NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos;
	
	NRF_UART0->CONFIG = 0;
	NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud38400 << UART_BAUDRATE_BAUDRATE_Pos;
	NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos;
	NRF_UART0->TASKS_STARTTX = 1;
	NRF_UART0->TASKS_STARTRX = 1;
	NRF_UART0->EVENTS_RXDRDY = 0;	
	
	NVIC_SetPriority(UART0_IRQn, 1);
	NVIC_EnableIRQ(UART0_IRQn);
	RS485_SetRx();
}

void UART0_IRQHandler(void)
{
	uint8_t rx_data;
	
    
	if (NRF_UART0->EVENTS_ERROR)
	{
		NRF_UART0->EVENTS_ERROR = 0;
		NRF_UART0->ERRORSRC = 0x0F;
	}
	
	if (NRF_UART0->EVENTS_RXDRDY)  /*Is the receiver's interrupt flag set?*/ 
	{        
		NRF_UART0->EVENTS_RXDRDY = 0;
		rx_data =  NRF_UART0->RXD;
        
       // if(RS485_Mode == RS485_REMOTE)
        {
			Battery_Handle.RxPtr = 0;
			//nrf_gpio_pin_toggle(29);
            //Comm_Handle.Mode = Comm_Mode_RS485;
            if((Comm_Handle.RxPtr == 0) && (rx_data != ADDR_BRIDGESYSTEM))
            {
                Comm_Handle.RxPtr = 0; 
            }
            else if((Comm_Mode_RS485 == Comm_Handle.Mode))
            {            
                Comm_Handle.Rec.Raw[Comm_Handle.RxPtr++] = rx_data;
            }
            if((Comm_Handle.RxPtr == MAINSYSTEM_RET_LEN) && (Comm_Mode_RS485 == Comm_Handle.Mode))
            {
                Comm_Decode(&Comm_Handle);
                Comm_Handle.RxPtr = 0;
				Comm_Handle.Remote485CommFailCount = 0;
                
            }
        }

		return;
	}
	 
	if (NRF_UART0->EVENTS_TXDRDY)
	{
		NRF_UART0->EVENTS_TXDRDY = 0;
		
        //if(RS485_Mode == RS485_REMOTE)
        {
            if(Comm_Handle.TxPtr < MAINSYSTEM_CMD_LEN)                              //send out the return messages
            {
                NRF_UART0->TXD = Comm_Handle.Ret.Raw[Comm_Handle.TxPtr++];
            }
			#if 1
            else			                                                        //transmission finished 
            {
                //if(Battery_YD_Trigger(&Battery_Handle) == 0)
                {
                    RS485_SetRx();
                }
            }
			#endif
        }
		return;
	}
}
