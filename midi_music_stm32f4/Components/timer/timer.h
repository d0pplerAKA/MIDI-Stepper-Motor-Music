#ifndef __TIMER_H__
#define __TIMER_H__


#include "stdio.h"
#include "math.h"

#include "stm32f4xx.h"

#include "delay/delay.h"


//#define TIM_DEBUG

#define TIM_CHANNEL_NUM                     8
#include "tone/tone.h"

#define TIM_APB_NUM_1                       0
#define TIM_APB_NUM_2                       1

#define TIM_CLK_VALUE                       100000000   // 100MHz


void TIM_GPIO_Init(void);
void TIM_Channel_Init(void);

void TIM_Freqency_onLoop(TONE_TRACK track[TIM_CHANNEL_NUM]);
void TIM_Freqency_Set(TIM_TypeDef *TIMx, float freq, uint8_t active);
void TIM_Freqency_Generate(float freq, uint16_t *value);
void TIM_Freqency_Clear(TONE_TRACK track[TIM_CHANNEL_NUM]);
void TIM_Status_Check(TONE_TRACK track[TIM_CHANNEL_NUM]);





#endif
