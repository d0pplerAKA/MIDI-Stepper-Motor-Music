#include "file_system.h"
#include "tcp_connect.h"
#include "midi_process.h"
#include "midi_play.h"
#include "freq_cal.h"
#include "midi_command.h"

#include "hal/usb_serial_jtag_ll.h"


void app_main_task(void * ignore);

extern "C" void app_main()
{
    //initArduino();

    printf("\n");

    flash_init();
    midi_command_uart_init();
    midi_command_i2c_init();

    vTaskDelay(1000);

    xTaskCreate(tcp_init_softap, "wifi init", 8192, NULL, 5, NULL);
    vTaskDelay(1000);
    xTaskCreate(tcp_start_task, "tcp prepare", 4096, NULL, 5, NULL);

    printf("midi wifi socket opened\n");

    //xTaskCreate(app_main_task, "main", 8192, NULL, 8, NULL);

    // Do your own thing
}

void app_main_task(void * ignore)
{
    TickType_t tick = xTaskGetTickCount();

    uint8_t rx_sbuf[64];
    uint8_t rx_cnt = 0;

    while(1)
    {
        vTaskDelayUntil(&tick, 100);

        if(usb_serial_jtag_ll_rxfifo_data_available())
        {
            memset(rx_sbuf, 0, sizeof(uint8_t) * 64);
            rx_cnt = usb_serial_jtag_ll_read_rxfifo(rx_sbuf, 64);

            char *console_cmd = (char *) malloc(sizeof(char) * (rx_cnt + 1));
            memset(console_cmd, 0, sizeof(char) * (rx_cnt + 1));
            memcpy(console_cmd, rx_sbuf, sizeof(char) * rx_cnt);

            uint32_t console_result = system_console(console_cmd, rx_cnt);

            switch(console_result)
            {
                case 0:
                {
                    printf("--------------------------------\n");
                    printf("{      midi system console     }\n");
                    printf("--------------------------------\n");
                    printf("- <midi>\n");
                    printf("- <midi play> []\n");
                    printf("- <midi delete> []\n");
                    printf("- <midi list>\n");
                    printf("- <midi upload 1/0>\n");
                    printf("--------------------------------\n\n");
                    
                    break;
                }

                case 1:
                {
                    char *file_name_ptr = (console_cmd + 10);

                    char midi_file_name[64];
                    memset(midi_file_name, 0, sizeof(midi_file_name));
                    memcpy(midi_file_name, file_name_ptr, sizeof(uint8_t) * strlen(file_name_ptr));

                    xTaskCreate(midi_play_task, 
                                "midi play", 
                                6144, 
                                (void *) midi_file_name, 
                                8, 
                                NULL);

                    break;
                }

                case 2:
                {
                    const char *midi_file_name = (console_cmd + 12);
                    
                    //xTaskCreate(midi_file_delete, "midi delete", 8192, (void *)midi_file_name, 8, NULL);

                    uint8_t rst = file_delete(midi_file_name);
                    if(rst == 2) printf("File Delete Failed, File Doesnt Exist\n");
                    else if(rst == 1) printf("File System Mount Fail\n");
                    else printf("File Delete Success\n");

                    break;
                }

                case 3:
                {
                    flash_list_files();

                    break;
                }

                case 4:
                {
                    xTaskCreate(tcp_init_softap, "wifi init", 8192, NULL, 5, NULL);
                    vTaskDelay(1000);
                    xTaskCreate(tcp_start_task, "tcp prepare", 4096, NULL, 5, NULL);

                    printf("midi wifi socket opened\n");

                    break;
                }

                case 5:
                {                    
                    xTaskCreate(tcp_deinit_softap, "tcp deinit", 1024, NULL, 5, NULL);
                    
                    printf("midi wifi socket closed\n");

                    break;
                }
            }

            free(console_cmd);
        }
    }
}
