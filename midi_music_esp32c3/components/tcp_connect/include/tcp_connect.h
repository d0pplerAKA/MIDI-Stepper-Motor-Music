#ifndef __TCP_CONNECT_H__
#define __TCP_CONNECT_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include "stdio.h"
#include "string.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdkconfig.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"

#include "midi_process.h"
#include "file_system.h"
#include "midi_play.h"

#define TCP_HOST_PORT                       22
#define TCP_ASSIGNED_IP_PORT1               "192.168.4.2"

#define TCP_WIFI_SSID                       "eduroam plus c3"
#define TCP_WIFI_PASS                       "10075386"
#define TCP_WIFI_CHANNEL                    6
#define TCP_WIFI_MAX_CONNECT                4

#define TCP_CONNECT_TIMEOUT                 100
#define TCP_CRC_ERROR_TOLERANCE             10

#define ACK_CODE_LEN                        4
#define ACK_CODE_FAIL                       "fail"
#define ACK_CODE_RECV                       "recv"
#define ACK_CODE_STRT                       "strt"
#define ACK_CODE_RTRY                       "rtry"
#define ACK_CODE_NTRY                       "ntry"
#define ACK_CODE_CNTU                       "cntu"
#define ACK_CODE_WAIT                       "wait"



typedef struct
{
    u32_t ip_addr;
    uint16_t port;
    TaskHandle_t task_handle;
} tcp_server_t;



uint32_t tcp_error_code(uint8_t *code);

void tcp_init_softap(void *pt);
void tcp_deinit_softap(void *pt);
void tcp_start_task(void *pt);
void tcp_task(void *pt);

void tcp_recv_toString(uint16_t port, char *file_name, uint32_t file_size);


#ifdef __cplusplus
    }
#endif

#endif
