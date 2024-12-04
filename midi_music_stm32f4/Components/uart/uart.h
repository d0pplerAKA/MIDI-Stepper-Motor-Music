#ifndef __UART1_H__
#define __UART1_H__


#include "stdio.h"

#include "stm32f4xx.h"

#include "midi/midi.h"



extern uint8_t UART2_DMA_FLAG;

void UART1_Init(uint32_t baudrate);
void USART1_IRQhandler(void);

void UART2_Init(uint32_t baudrate);
void UART2_DMA_Init(uint8_t *buffer_ptr, uint32_t buffer_size);           // [Channel 4] [Stream 5]


#endif
