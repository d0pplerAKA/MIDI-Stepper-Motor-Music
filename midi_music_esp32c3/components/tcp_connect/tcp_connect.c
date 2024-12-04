#include "include/tcp_connect.h"


tcp_server_t tcp_server;
esp_netif_t *tcp_netif;


static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        //wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;

    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        //wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
    }
}


void tcp_init_softap(void *pt)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    tcp_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = TCP_WIFI_SSID,
            .ssid_len = strlen(TCP_WIFI_SSID),
            .channel = TCP_WIFI_CHANNEL,
            .password = TCP_WIFI_PASS,
            .max_connection = TCP_WIFI_MAX_CONNECT,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    if (strlen(TCP_WIFI_PASS) == 0)
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(CONFIG_ESP_PHY_MAX_WIFI_TX_POWER));

    vTaskDelete(NULL);
}

void tcp_deinit_softap(void *pt)
{
    esp_netif_destroy_default_wifi(tcp_netif);
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    
    //ESP_ERROR_CHECK(esp_netif_deinit());

    vTaskDelete(NULL);
}

void tcp_start_task(void *pt)
{   
    tcp_server.ip_addr = inet_addr(TCP_ASSIGNED_IP_PORT1);
    tcp_server.port = TCP_HOST_PORT;

    xTaskCreate(tcp_task, "tcp server", 20480, (void *) &tcp_server, 5, &tcp_server.task_handle);

    vTaskDelete(NULL);
}

void tcp_task(void *pt)
{
    tcp_server_t *_tcp = (tcp_server_t *) pt;

    struct sockaddr_in server_addr;
    int server_socket = -1;

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_socket < 0)
    {
        printf("[port %d] Unable to create socket instance.\n", _tcp->port);
        vTaskDelete(NULL);
    }


    memset(&server_addr, 0, sizeof(server_addr));                                       //初始化
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(_tcp->port);

/*
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
*/
    int err;
    err = bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if(err != 0)
    {
        close(server_socket);
        printf("[port %d] Unable to bine server socket.\n", _tcp->port);
        vTaskDelete(NULL);
    }


    while(1)
    {
        uint32_t listening_timeout = TCP_CONNECT_TIMEOUT;

        err = listen(server_socket, 1);
        if(err != 0)
        {
            shutdown(server_socket, SHUT_RDWR);
            close(server_socket);
            printf("[port %d] Unable to listen on server socket.\n", _tcp->port);
            vTaskDelete(NULL);
        }
        printf("[port %d] Tcp Server start listening.\n", _tcp->port);

        /***************************************************************/

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if(client_socket < 0)
        {
            shutdown(server_socket, SHUT_RDWR);
            close(server_socket);
            printf("[port %d] Unable to accept client socket.\n", _tcp->port);
            vTaskDelete(NULL);
        }
        printf("[port %d] Accepted new client connection.\n", _tcp->port);


        bool end = false;
        while (!end)
        {
            char tcp_pack[1024];

            memset(tcp_pack, 0, sizeof(tcp_pack));                              // 初始化数组
            int len = recv(client_socket, tcp_pack, sizeof(tcp_pack), 0);

            if(len > 0)
            {
                if(strncmp(tcp_pack, "disc", 4) == 0)
                {
                    end = true;

                    printf("[port %d] %s\n", _tcp->port, "Client disconnect");
                }
                else if(strncmp(tcp_pack, "midi", 4) == 0)
                {
                    send(client_socket, ACK_CODE_RECV, ACK_CODE_LEN, 0);

                    memset(tcp_pack, 0, sizeof(tcp_pack));
                    // VLA file size + VLA pack num + file name
                    recv(client_socket, tcp_pack, sizeof(tcp_pack), 0);
                    uint8_t *temp_ptr = (uint8_t *)tcp_pack;

                    uint32_t ptr_len = 0;
                    uint32_t file_size, pack_num, pack_temp_num;
                    file_size = midi_read_dynamimc_byte((uint8_t *)tcp_pack, &ptr_len);
                    for(uint32_t i = 0 ; i < ptr_len; i++)
                        temp_ptr++;


                    pack_num = midi_read_dynamimc_byte(temp_ptr, &ptr_len);
                    for(uint32_t i = 0 ; i < ptr_len; i++)
                        temp_ptr++;


                    uint8_t *file_name_ptr = temp_ptr;
                    uint32_t name_len = strlen((const char *)file_name_ptr);
                    printf("[port %d] File name len: %d\n", _tcp->port, name_len);

                    char file_name[name_len];
                    strcpy(file_name, (const char *)temp_ptr);

                    tcp_recv_toString(_tcp->port, file_name, file_size);

                    /*
                        insert waiting process for hardware
                    */

                    char file_absolute_path[MOUNT_PATH_LEN + name_len];
                    strcat(file_absolute_path, MOUNT_PATH"/");
                    strcat(file_absolute_path, file_name);
                    vTaskDelay(10);

                    pack_temp_num = 0;
                    if(flash_mount() == ESP_OK)                    
                    {
                        printf("[port %d] File System Mounted\n", _tcp->port);

                        FILE *fp = NULL;
                        fp = fopen(file_absolute_path, "w+");

                        if(fp != NULL) printf("[port %d] File <%s> created\n", _tcp->port, file_absolute_path);
                        else
                        {
                            printf("[port %d] File <%s> Unable to create\n", _tcp->port, file_absolute_path);
                            send(client_socket, ACK_CODE_FAIL, ACK_CODE_LEN, 0);

                            end=false;
                            break;
                        }

                        send(client_socket, ACK_CODE_STRT, ACK_CODE_LEN, 0);

                        for(pack_temp_num = 0; pack_temp_num < pack_num; pack_temp_num++)
                        {
                            vTaskDelay(3);

                            uint32_t recv_len = 0;
                            uint32_t temp_file_size = (pack_temp_num + 1) * 1024;

                            if(temp_file_size > file_size) recv_len = 1024 - (temp_file_size - file_size);
                            else recv_len = 1024;

                            memset(tcp_pack, 0, sizeof(tcp_pack));
                            recv(client_socket, tcp_pack, sizeof(uint8_t) * recv_len, 0);
                            printf("[port %d] Pack Received (%d / %d)\n", _tcp->port, pack_temp_num+1, pack_num);

                            for(uint32_t i = 0; i < recv_len; i++)
                                fprintf(fp, "%c", tcp_pack[i]);

                            send(client_socket, ACK_CODE_CNTU, ACK_CODE_LEN, 0);
                            printf("[port %d] Pack Flashed\n", _tcp->port);
                        }

                        /*
                        close file, commit process
                        */
                        fclose(fp);
                        flash_unmount();
                    }
                    else
                    {
                        send(client_socket, ACK_CODE_FAIL, ACK_CODE_LEN, 0);

                        printf("[port %d] File System Mount Failed\n", _tcp->port);

                        end = true;
                    }
                }
                else if(strncmp(tcp_pack, "list", 4) == 0)
                {
                    if(flash_mount() == ESP_OK)
                    {                        
                        printf("[port %d] List Loaded\n", _tcp->port);

                        struct dirent *entry;
                        DIR *dir = opendir(MOUNT_PATH);

                        if(dir == NULL) send(client_socket, ACK_CODE_FAIL, ACK_CODE_LEN, 0);
                        else
                        {
                            while((entry = readdir(dir)) != NULL)
                            {
                                memset(tcp_pack, 0, sizeof(uint8_t) * 1024);
                                memcpy(tcp_pack, entry->d_name, sizeof(uint8_t) * 256);
                                send(client_socket, tcp_pack, sizeof(tcp_pack), 0);
                            }

                            closedir(dir);
                        }

                        flash_unmount();
                    }
                    else send(client_socket, ACK_CODE_FAIL, ACK_CODE_LEN, 0);

                    end = true;
                }
                else
                {
                    printf("[port %d] Received data: %s\n", _tcp->port, tcp_pack);
                    send(client_socket, tcp_pack, sizeof(tcp_pack), 0);
                }
            }

            listening_timeout--;

            if(listening_timeout == 0)
            {
                printf("[port %d] Listening timout.\n", _tcp->port);
                end = true;
            }
        }

        shutdown(server_socket, SHUT_RDWR);
        close(client_socket);
        printf("[port %d] Host closed the connection.\n", _tcp->port);
    }

    vTaskDelete(NULL);
}

uint32_t tcp_error_code(uint8_t *code)
{
    return (uint32_t) (code[0] << 24 | code[1] << 16 | code[2] << 8 | code[3]);
}


void tcp_recv_toString(uint16_t port, char *file_name, uint32_t file_size)
{
    printf("[port %d] Midi Music info: %s with %.2fKB.\n", port, file_name, (float) (file_size / 1024.0f));
}
