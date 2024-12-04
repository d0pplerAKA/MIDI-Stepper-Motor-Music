#include "timer/timer.h"


void TIM_Status_Check(TONE_TRACK track[TIM_CHANNEL_NUM])
{
    printf("<Info> Active Channels: ");

    for(uint8_t i = 0; i < TIM_CHANNEL_NUM; i++)
    {
        if(track[i].is_active) printf("%d ", i);
    }

    printf("\n");
}

void TIM_Freqency_onLoop(TONE_TRACK track[TIM_CHANNEL_NUM])
{
    TIM_TypeDef *TIMx[TIM_CHANNEL_NUM] = {
        TIM1, TIM2, TIM3, TIM4, TIM5, TIM9, TIM10, TIM11
    };

    for(uint8_t i = 0; i < TIM_CHANNEL_NUM; i++)
    {
        if(track[i].is_active) TIM_Freqency_Set(TIMx[i], track[i].timer_freqency, 1);
        else TIM_Freqency_Set(TIMx[i], 1000000, 0);
    }
#ifdef TIM_DEBUG
    TIM_Status_Check(track);
#endif
}


void TIM_Freqency_Set(TIM_TypeDef *TIMx, float freq, uint8_t active)
{
    uint16_t psc_arr_value = 0;
    TIM_Freqency_Generate(freq, &psc_arr_value);

    if(active)
    {
        TIMx->ARR = (uint32_t) psc_arr_value;
        TIMx->PSC = (uint32_t) psc_arr_value;

        if(TIMx == TIM5) TIM_SetCompare2(TIMx, psc_arr_value / 2);
        else TIM_SetCompare1(TIMx, psc_arr_value / 2);
    }
    else
    {
        if(TIMx == TIM5) TIM_SetCompare2(TIMx, 0);
        else TIM_SetCompare1(TIMx, 0);
    }
}


void TIM_Freqency_Generate(float freq, uint16_t *value)
{
    *value = sqrtf((float) TIM_CLK_VALUE / freq) - 1.0f;
}

void TIM_Freqency_Clear(TONE_TRACK track[TIM_CHANNEL_NUM])
{
    TIM_TypeDef *TIMx[TIM_CHANNEL_NUM] = {
        TIM1, TIM2, TIM3, TIM4, TIM5, TIM9, TIM10, TIM11
    };
    
    for(uint8_t i = 0; i < TIM_CHANNEL_NUM; i++)
    {
        track[i].is_active = 0;
        tone_track_active_num = 0;

        TIM_Freqency_Set(TIMx[i], 1000000, 0);
    }
}

void TIM_GPIO_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_8 | GPIO_Pin_15;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM5);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM9);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_TIM2);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_TIM10);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_TIM11);

    

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;              // STEPPER MOTOR DIR PIN
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_ResetBits(GPIOB, GPIO_Pin_7);
}

void TIM_Channel_Init(void)
{
    // APB1 SYSCLK / 2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4 | RCC_APB1Periph_TIM5, ENABLE);

    // APB1 SYSCLK / 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_TIM9 | RCC_APB2Periph_TIM10 | RCC_APB2Periph_TIM11, ENABLE);
    

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;

    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_Prescaler = 24999;
    TIM_TimeBaseStruct.TIM_Period = 3999;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStruct);
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStruct);
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStruct);

    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStruct);
    TIM_TimeBaseInit(TIM10, &TIM_TimeBaseStruct);
    TIM_TimeBaseInit(TIM11, &TIM_TimeBaseStruct);

    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStruct);


    TIM_OCInitTypeDef TIM_OCStruct;

    TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCStruct.TIM_Pulse = 0;
    TIM_OC1Init(TIM2, &TIM_OCStruct);
    TIM_OC1Init(TIM3, &TIM_OCStruct);
    TIM_OC1Init(TIM4, &TIM_OCStruct);
    TIM_OC2Init(TIM5, &TIM_OCStruct);
    TIM_OC1Init(TIM1, &TIM_OCStruct);
    TIM_OC1Init(TIM9, &TIM_OCStruct);
    TIM_OC1Init(TIM10, &TIM_OCStruct);
    TIM_OC1Init(TIM11, &TIM_OCStruct);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM5, TIM_OCPreload_Enable);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC1PreloadConfig(TIM9, TIM_OCPreload_Enable);
    TIM_OC1PreloadConfig(TIM10, TIM_OCPreload_Enable);
    TIM_OC1PreloadConfig(TIM11, TIM_OCPreload_Enable);

    
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_ARRPreloadConfig(TIM5, ENABLE);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_ARRPreloadConfig(TIM9, ENABLE);
    TIM_ARRPreloadConfig(TIM10, ENABLE);
    TIM_ARRPreloadConfig(TIM11, ENABLE);

    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
    TIM_Cmd(TIM5, ENABLE);
    TIM_Cmd(TIM9, ENABLE);
    TIM_Cmd(TIM10, ENABLE);
    TIM_Cmd(TIM11, ENABLE);

    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    TIM_SetCompare1(TIM2, 0);
    TIM_SetCompare1(TIM3, 0);
    TIM_SetCompare1(TIM4, 0);
    TIM_SetCompare2(TIM5, 0);
    TIM_SetCompare1(TIM1, 0);
    TIM_SetCompare1(TIM9, 0);
    TIM_SetCompare1(TIM10, 0);
    TIM_SetCompare1(TIM11, 0);
}

