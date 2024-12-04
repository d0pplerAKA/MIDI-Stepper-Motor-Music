#include "tone.h"


float tone_table[TONE_NOTE_MAX];

uint8_t tone_track_active_num = 0;
TONE_TRACK tone_track[TONE_TRACK_MAX];


void Tone_Table_Init(void)
{
    memset(tone_track, 0, sizeof(TONE_TRACK) * TONE_TRACK_MAX);

    for(uint16_t i = 0; i < 128; i++)
        tone_table[i] = powf(TONE_NOTE_COE, (float) i) * TONE_NOTE_C0 * TONE_NOTE_FACTOR;
}

uint8_t Tone_Alloc(float freq)
{
    if(tone_track_active_num < TONE_TRACK_MAX)
    {
        for(uint8_t i = 0; i < TONE_TRACK_MAX; i++)
        {
            if(tone_track[i].is_active == 0)
            {
                tone_track[i].is_active = 1;
                tone_track[i].timer_freqency = freq;
                tone_track_active_num++;

                return 0;
            }
        }

        return 2;   // Should not be returned
    }
    else return 1;
}

uint8_t Tone_Free(float freq)
{
    for(uint8_t i = 0; i < TONE_TRACK_MAX; i++)
    {
        if(tone_track[i].is_active && tone_track[i].timer_freqency == freq)
        {
            tone_track[i].is_active = 0;
            tone_track[i].timer_freqency = 0;
            tone_track_active_num--;

            return 0;
        }
    }

    return 1;
}
