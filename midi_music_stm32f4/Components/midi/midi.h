#ifndef __MIID_H__
#define __MIID_H__


#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#include "i2c_slave/i2c_slave.h"
#include "delay/delay.h"
#include "tone/tone.h"
#include "timer/timer.h"
#include "status/status.h"

#define MIDI_FILE_MAX_SIZE                  7680            // Total 97.5K Byte Mem

#define MIDI_PLAY_OFF                       0
#define MIDI_PLAY_ON                        1


#define MIDI_FILE_DEFAULT_TEMPO             50000
#define MIDI_FILE_DEFAULT_TICK              480

#define MIDI_ACTION_END                     0
#define MIDI_ACTION_UNKNOWN                 1
#define MIDI_ACTION_RELEASE                 2
#define MIDI_ACTION_PRESS                   3
#define MIDI_ACTION_AF_TOUCH                4
#define MIDI_ACTION_SET_TEMPO               5



typedef struct
{
    uint32_t midi_delay_tick;

    uint8_t midi_action;

    uint32_t midi_content;
} MIDI_COMMAND;     // 9 bytes


extern uint8_t midi_play_status;
extern uint32_t midi_file_size;
extern uint16_t midi_file_tick;

extern MIDI_COMMAND MIDI_ACTION_SRAM[MIDI_FILE_MAX_SIZE];


void MIDI_SRAM_Init(void);

void MIDI_Load_File(uint32_t *f_len);
void MIDI_Load_Tick(uint16_t *f_tick);

void MIDI_Handler(void);
void MIDI_Play(void);


#endif
