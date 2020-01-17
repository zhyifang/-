#include "ep_warn.h"

#include "nrf_gpio.h"

void Warn_Init(void)
{
	nrf_gpio_cfg_output(BEEP_PIN);
	nrf_gpio_cfg_output(LED_PIN);
}

void Warn_LED(void)
{

}
void Warn_Beep(void)
{

}

