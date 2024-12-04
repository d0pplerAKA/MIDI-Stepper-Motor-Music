#ifndef __TONE_H__
#define __TONE_H__


#include "stdint.h"
#include "math.h"
#include "string.h"

typedef struct
{
    uint8_t is_active;
    float timer_freqency;
    uint32_t compare_value;
} TONE_TRACK;
#include "timer/timer.h"


#define TONE_TRACK_MAX              TIM_CHANNEL_NUM
#define TONE_NOTE_MAX               128

#define TONE_NOTE_COE               1.0594631f
#define TONE_NOTE_C0                8.176f
#define TONE_NOTE_C0_CUSTOM         1.0f
#define TONE_NOTE_FACTOR            2.4f



extern float tone_table[TONE_NOTE_MAX];

extern uint8_t tone_track_active_num;
extern TONE_TRACK tone_track[TONE_TRACK_MAX];



void Tone_Table_Init(void);
uint8_t Tone_Alloc(float freq);
uint8_t Tone_Free(float freq);




#endif
