#ifndef __MIDI_PLAY_H__
#define __MIDI_PLAY_H__


#include "math.h"

#include "midi_command.h"
#include "midi_process.h"
#include "file_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"


#define MIDI_MOTOR_NUM                  8
#define MIDI_MOTOR_RUN                  1
#define MIDI_MOTOR_STOP                 0

#define MIDI_COMMAND_CANDO              0
#define MIDI_COMMAND_CANTDO             1

#define MIDI_REG_DEFAULT                0xA0
#define MIDI_REG_MIDI_MESSAGE           0xB0


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    uint8_t note;
    uint8_t tone;
} midi_info_t;

typedef struct
{
    midi_info_t midi_info;
    uint8_t motor_status;
} midi_motor_t;




extern midi_motor_t midi_motor_status[MIDI_MOTOR_NUM];


void midi_motor_channel_init(midi_motor_t *motor_channel);
uint8_t midi_motor_process(midi_motor_t *motor_channel, midi_info_t midi_info, midi_command_t *cmd);

void midi_play_task(void *pt);
void midi_play_debug(uint32_t midi_tempo, uint16_t midi_unit_tempo, MIDI_COMMAND *midi_command, uint32_t midi_command_num);


#ifdef __cplusplus
}
#endif

#endif

