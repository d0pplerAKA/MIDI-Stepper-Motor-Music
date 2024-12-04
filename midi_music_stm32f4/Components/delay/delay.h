#ifndef __DELAY_H__
#define __DELAY_H__

#include "stm32f4xx.h"
#include "stdint.h"

void delay_ms(uint16_t d_ms);
void delay_i2c(uint16_t us);
void delay_1us(void);

#endif
