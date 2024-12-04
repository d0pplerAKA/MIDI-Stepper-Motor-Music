#ifndef __I2C_SLAVE_H__
#define __I2C_SLAVE_H__

#include "stdio.h"

#include "stm32f4xx.h"

#include "midi/midi.h"
#include "status/status.h"
#include "led/led.h"


#define I2C_SRAM_SIZE                           9


#define I2C_SRAM_BLOCK_BASIC                    0
#define I2C_SRAM_BLOCK_INVALID                  1

#define I2C_SLAVE_ADDRESS                       0xCE

#define I2C_SLAVE_ADDR_STATUS                   0       // System Status
#define I2C_SLAVE_ADDR_MIDI_PLAY_STATUS         1       // MIDI PLAY
#define I2C_SLAVE_ADDR_MIDI_CLR                 2       // MIDI File Clear
#define I2C_SLAVE_ADDR_MIDI_SIZE                3       // MIDI File Size
#define I2C_SLAVE_ADDR_MIDI_TICK                7       // MIDI File Tick

#define I2C_SLAVE_ADDR_WHO_READ_ONLY            9      // Who am I
#define I2C_SLAVE_ADDR_RST                      10       // System Reset


extern uint8_t I2C_SRAM[I2C_SRAM_SIZE];
extern uint8_t *i2c2_midi_ptr;


void I2C_SRAM_Init(void);
void I2C2_Slave_Init(void);
void I2C2_Clear_STOPF(void);
void I2C2_Clear_ADDR(void);


#endif
