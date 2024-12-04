#include "uart.h"

uint32_t UART2_ReceiveSize = 0;

uint8_t UART2_DMA_FLAG = 0;


void UART2_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitStruct.USART_BaudRate = baudrate;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStruct);

    USART_Cmd(USART2, ENABLE);
}

void UART2_DMA_Init(uint8_t *buffer_ptr, uint32_t buffer_size)           // [Channel 4] [Stream 5]
{
	UART2_DMA_FLAG = 0;
	
    DMA_InitTypeDef DMA_InitStruct;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Stream5);

    while(DMA_GetCmdStatus(DMA1_Stream5) != DISABLE);

    DMA_InitStruct.DMA_BufferSize = buffer_size;
    DMA_InitStruct.DMA_Channel = DMA_Channel_4;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t) buffer_ptr;
    DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) (&(USART2->DR));
    DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_Init(DMA1_Stream5, &DMA_InitStruct);

    NVIC_InitTypeDef NVIC_Struct;
    NVIC_Struct.NVIC_IRQChannel = DMA1_Stream5_IRQn;
    NVIC_Struct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Struct.NVIC_IRQChannelSubPriority = 2;
    NVIC_Struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_Struct);

    DMA_Cmd(DMA1_Stream5, ENABLE);

    DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
	
	DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);

    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
}


void DMA1_Stream5_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5))
    {
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
        DMA_Cmd(DMA1_Stream5, DISABLE);
		
		UART2_DMA_FLAG = 1;
        //printf("BUFF FILLED\n");
    }
}

