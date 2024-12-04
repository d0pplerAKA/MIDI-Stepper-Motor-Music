#ifndef __MIDI_COMMAND_H__
#define __MIDI_COMMAND_H__


#include "string.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "hal/uart_types.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define I2C_SLAVE_STM32_ADDR                  0x67


#define I2C_SLAVE_ADDR_STATUS               0       // System Status
#define I2C_SLAVE_ADDR_MIDI_PLAY_STATUS     1       // MIDI PLAY
#define I2C_SLAVE_ADDR_MIDI_CLR             2       // MIDI File Clear
#define I2C_SLAVE_ADDR_MIDI_SIZE            3       // MIDI File Size
#define I2C_SLAVE_ADDR_MIDI_TICK            7       // MIDI File Tick


typedef struct
{
    uint8_t motor_channel;              // Motor (TIM) Number
    uint8_t motor_status;               // Motor status (on / off)
    uint8_t motor_tone;                 // note tone value (0 ~ 127)
} midi_command_t;


#ifdef __cplusplus
extern "C" {
#endif


void midi_command_uart_init(void);

void midi_command_i2c_init(void);
void midi_command_i2c_write_byte(uint8_t addr, uint8_t data);
void midi_command_i2c_write_bytes(uint8_t addr, uint8_t *data, size_t data_len);
void midi_command_i2c_read_byte(uint8_t addr, uint8_t *data);
void midi_command_i2c_read_bytes(uint8_t addr, uint8_t *data, size_t data_len);



#ifdef __cplusplus
}
#endif

#endif
