#ifndef __FREQ_CAL_H__
#define __FREQ_CAL_H__


#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "soc/clk_ctrl_os.h"

#include "driver/i2c.h"


#define GPIO_BUILTIN_LED                GPIO_NUM_8


#ifdef __cplusplus
extern "C" {
#endif


void freq_get_rtc_clk(uint32_t *clk);
void freq_ledc_timer_init(void);
void freq_ledc_channel_init(void);
void freq_init(void);
void freq_task(void *pt);


#ifdef __cplusplus
}
#endif


#endif
