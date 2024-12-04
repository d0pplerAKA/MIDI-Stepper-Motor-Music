#ifndef __STATUS_H__
#define __STATUS_H__


#include "stm32f4xx.h"

#include "led/led.h"


#define System_Status_Waiting                   0
#define System_Status_MIDI_Playing              1

void LED_STATUS(uint8_t status);


#endif
