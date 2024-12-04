#ifndef __LED_H__
#define __LED_H__

#include "stm32f4xx.h"
#include "delay/delay.h"
#include "uart/uart.h"

#define LED_PIN             GPIO_Pin_13
#define LED_PIN_OK          GPIO_Pin_14
#define LED_PIN_ERROR       GPIO_Pin_15


void LED_Init(void);
void LED_ON(void);
void LED_OFF(void);

void LED_blink(void);
void LED_Toggle(void);


#endif
