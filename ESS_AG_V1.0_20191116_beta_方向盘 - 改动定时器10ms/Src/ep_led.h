#ifndef __ep_led_h__
#define __ep_led_h__

#define LED_BLINK_TIME 2
#define FLICK_FAST_NUM 1
#define FLICK_SLOW_NUM 4

#define LED_R_PIN 16
#define LED_G_PIN 17
#define LED_B_PIN 18

#define LED1 3
#define LED2 4
#define LED3 5
#define LED4 12
#define LED5 13
#define LED6 14
#define LED7 16
#define LED8 17
#define LED9 18

#define For_Dir 1

typedef struct
{
	uint8_t led_flick_cnt;
	uint8_t led_dir;
	uint8_t led_connect_flag;
}LED_Handle_t;


typedef enum
{
	START_UP_SHOW,
	SETTING_MODE_SHOW,
	ERRO_SHOW,
}LED_Show_e;

#define LED_R_ON	(nrf_gpio_pin_set(LED_R_PIN))
#define LED_G_ON	(nrf_gpio_pin_set(LED_G_PIN))
#define LED_B_ON	(nrf_gpio_pin_set(LED_B_PIN))

#define LED_R_OFF	(nrf_gpio_pin_clear(LED_R_PIN))
#define LED_G_OFF	(nrf_gpio_pin_clear(LED_G_PIN))
#define LED_B_OFF	(nrf_gpio_pin_clear(LED_B_PIN))

#define IS_LED_R_ON ((nrf_gpio_pin_read(LED_R_PIN))?(0):(1))
#define IS_LED_G_ON ((nrf_gpio_pin_read(LED_G_PIN))?(0):(1))
#define IS_LED_B_ON ((nrf_gpio_pin_read(LED_B_PIN))?(0):(1))

#define LED_R_PIN_OUT (nrf_gpio_cfg_output(LED_R_PIN))
#define LED_R_PIN_IN (nrf_gpio_cfg_input(LED_R_PIN,NRF_GPIO_PIN_NOPULL))

#define LED_G_PIN_OUT (nrf_gpio_cfg_output(LED_G_PIN))
#define LED_G_PIN_IN (nrf_gpio_cfg_input(LED_G_PIN,NRF_GPIO_PIN_NOPULL))

#define LED_B_PIN_OUT (nrf_gpio_cfg_output(LED_B_PIN))
#define LED_B_PIN_IN (nrf_gpio_cfg_input(LED_B_PIN,NRF_GPIO_PIN_NOPULL))

typedef enum
{
	LED_None,
	LED_R,
	LED_G,
	LED_B,
	LED_RG,
	LED_GB,
	LED_RB,
	LED_RGB,
	LED_MAX
}LED_Color_e;

void LED_init(void);
void LED_trigger(uint8_t ColorPin);
void LED_blink(uint8_t ColorPin);
void LED_nBlink(uint8_t led_num);
void LED_nBlink_Fast(uint8_t led_num);
void LED_nBlink_Slow(uint8_t led_num);

void LED_display_color(LED_Color_e ColorNum);
void LED_AllOff(void);
void LED_Display(void);
void LED_SysON(void);
void LED_SysOff(void);


#endif
