#ifndef __MIDI_PROCESS_H__
#define __MIDI_PROCESS_H__

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"

#include "midi_command.h"
#include "file_system.h"
#include "esp_err.h"

#define MOUNT_PATH                  "/flash"
#define MOUNT_PATH_LEN              7


#define MIDI_FILE_DEFAULT_TEMPO             50000
#define MIDI_FILE_NAME_MAX_LEN              64


#define MIDI_HEADER_CHUNK                   0x4D546864
#define MIDI_HEADER_CHUNK_END               0x00000006
#define MIDI_TRACK_CHUNK                    0x4D54726B
#define MIDI_TRACK_CHUNKEND                 0x00FF2F00


#define MIDI_TRACK_READ_SUCCESS             0
#define MIDI_TRACK_READ_END                 1
#define MIDI_TRACK_READ_FAIL                2


#define MIDI_STATUS_BYTE                    0x80
#define MIDI_STATUS_BYTE_RELEASE            0x80
#define MIDI_STATUS_BYTE_PRESS              0x90
#define MIDI_STATUS_BYTE_AF_TOUCH           0xA0
#define MIDI_STATUS_BYTE_CTRL_CHANGE        0xB0
#define MIDI_STATUS_BYTE_INST_CHANGE        0xC0
#define MIDI_STATUS_BYTE_AF_PRESSURE        0xD0
#define MIDI_STATUS_BYTE_WEEL_CHANGE        0xE0

#define MIDI_STATUS_BYTE_META               0xF0
#define MIDI_STATUS_BYTE_META_TEMPO         0x51
//#define MIDI_STATUS_BYTE_META_GROOVE        0x58
#define MIDI_STATUS_BYTE_META_TRACK_END     0x2F


#define MIDI_ACTION_END                     0
#define MIDI_ACTION_UNKNOWN                 1
#define MIDI_ACTION_RELEASE                 2
#define MIDI_ACTION_PRESS                   3
#define MIDI_ACTION_AF_TOUCH                4
#define MIDI_ACTION_SET_TEMPO               5



typedef struct
{
    uint8_t midi_action;
    uint8_t midi_tone;
    uint8_t midi_velocity;
    uint8_t midi_channel;
} MIDI_ACTION_NOTE;                             // 4 bytes

typedef struct
{
    uint32_t midi_delay_tick;
    uint32_t midi_track_tempo;

    MIDI_ACTION_NOTE midi_note_info;
} MIDI_ACTION;                                  // 12 bytes


typedef struct
{
    uint8_t *midi_track_ptr;
    uint32_t midi_track_size;

    uint8_t *midi_cnt_track;
} MIDI_TRACK;

typedef struct
{
    uint8_t midi_file_name[MIDI_FILE_NAME_MAX_LEN];
    uint32_t midi_file_size;
    uint8_t *midi_file_ptr;

    uint8_t midi_file_format;

    uint16_t midi_time_unit_tick;
    
    uint16_t midi_track_num;
    MIDI_TRACK *midi_file_track;
} MIDI_FILE;

typedef struct
{
    uint32_t midi_delay_tick;

    uint8_t midi_action;

    uint32_t midi_content;
} MIDI_COMMAND;     // 13 bytes


typedef struct
{
    MIDI_COMMAND *array;
    size_t used;
    size_t size;
    
    MIDI_COMMAND *head_array;
} ARRAY_LIST_MIDI_ACTION;




#ifdef __cplusplus
extern "C" {
#endif

uint8_t midi_file_verify(uint8_t *midi_file_ptr, uint8_t *midi_file_format, uint16_t *midi_track_num, uint16_t *midi_time_unit_tick);
uint8_t midi_file_init(char *file_name, MIDI_FILE *midi_file);
uint8_t midi_file_load_track(MIDI_FILE *midi_file);


uint32_t midi_read_dynamimc_byte(uint8_t* ptr, uint32_t* length);
uint8_t midi_read_delay(MIDI_FILE *midi_file, uint32_t *delay_time_in_us, uint16_t track_to_read);
uint8_t midi_read_command(MIDI_FILE *midi_file, MIDI_ACTION *midi_action, uint32_t track_to_read);


void midi_array_init(ARRAY_LIST_MIDI_ACTION *array_list, size_t allocate_size);
void midi_array_add(ARRAY_LIST_MIDI_ACTION *array_list, MIDI_COMMAND midi_cmd);
void midi_array_free(ARRAY_LIST_MIDI_ACTION *array_list);


void midi_tick_bubble_sort(MIDI_COMMAND *arr, uint32_t n);


uint8_t midi_file_compile(MIDI_FILE *midi_file, MIDI_COMMAND **midi_cmd, uint32_t *midi_cmd_num);
uint8_t midi_file_play(MIDI_FILE *midi_file, MIDI_COMMAND *midi_cmd, uint32_t midi_cmd_num);


#ifdef __cplusplus
}
#endif

#endif
