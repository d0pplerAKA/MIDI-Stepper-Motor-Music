#include "status.h"

void LED_STATUS(uint8_t status)
{
    if(status)
    {
        GPIO_ResetBits(GPIOC, LED_PIN_ERROR);
        GPIO_SetBits(GPIOC, LED_PIN_OK);
    }
    else
    {
        GPIO_ResetBits(GPIOC, LED_PIN_OK);
        GPIO_SetBits(GPIOC, LED_PIN_ERROR);
    }
}
