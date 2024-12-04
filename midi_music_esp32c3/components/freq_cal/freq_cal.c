#include "include/freq_cal.h"


void freq_get_rtc_clk(uint32_t *clk)
{
    *clk = periph_rtc_dig_clk8m_get_freq();
}


void freq_ledc_timer_init(void)
{
    ledc_timer_config_t timer_config_0 = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_6_BIT,
            .timer_num = LEDC_TIMER_0,
            .freq_hz = 5000,
            .clk_cfg = LEDC_USE_RTC8M_CLK
    };
/*
    ledc_timer_config_t timer_config_1 = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_10_BIT,
            .timer_num = LEDC_TIMER_1,
            .freq_hz = 10,
            .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config_t timer_config_2 = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_10_BIT,
            .timer_num = LEDC_TIMER_2,
            .freq_hz = 10,
            .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config_t timer_config_3 = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_10_BIT,
            .timer_num = LEDC_TIMER_3,
            .freq_hz = 10,
            .clk_cfg = LEDC_AUTO_CLK
    };
*/
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config_0));
    //ESP_ERROR_CHECK(ledc_timer_config(&timer_config_1));
    //ESP_ERROR_CHECK(ledc_timer_config(&timer_config_2));
    //ESP_ERROR_CHECK(ledc_timer_config(&timer_config_3));
}

void freq_ledc_channel_init(void)
{
    ledc_channel_config_t channel_config_0 = {
        .gpio_num = GPIO_BUILTIN_LED,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = 0
    };

    ledc_channel_config(&channel_config_0);
}


void freq_init(void)
{
    gpio_config_t io_config = {
        .pin_bit_mask = GPIO_BUILTIN_LED,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_config);
    freq_ledc_timer_init();
    freq_ledc_channel_init();
}


void freq_task(void *pt)
{
    freq_init();
    
    while(1)
    {
        
        for(int32_t i = 0; i < 64; i++)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(5);
        }

        vTaskDelay(500);

        for(int32_t i = 63; i >= 0; i--)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(5);
        }

        vTaskDelay(500);
        

       gpio_set_level(GPIO_BUILTIN_LED, 0);
    }
}
