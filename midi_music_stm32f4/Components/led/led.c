#include "led.h"

void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_Pin = LED_PIN;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = LED_PIN_ERROR | LED_PIN_OK;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_SetBits(GPIOC, LED_PIN_OK | LED_PIN_ERROR);

	LED_OFF();
}

void LED_OFF(void)
{
    GPIO_SetBits(GPIOC, LED_PIN);
}

void LED_ON(void)
{
    GPIO_ResetBits(GPIOC, LED_PIN);
}

void LED_blink(void)
{
	for(uint8_t i = 0; i < 5; i++)
	{
		LED_ON();
		delay_ms(100);
		LED_OFF();
		delay_ms(100);
	}
}

void LED_Toggle(void)
{
    if(GPIO_ReadInputDataBit(GPIOC, LED_PIN) == ENABLE)
        LED_ON();
    else
        LED_OFF();
}
