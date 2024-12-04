#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__


#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "dirent.h"

#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"


#define MOUNT_PATH                  "/flash"
#define MOUNT_PATH_LEN              7


#ifdef __cplusplus
    extern "C" {
#endif

uint8_t flash_mount(void);
uint8_t flash_unmount(void);

void flash_init(void);

void flash_list_files(void);
uint8_t file_is_valid(char *valid_name);
uint8_t file_delete(const char *fileName);
void midi_file_delete(void *pt);

uint32_t system_console(char *rx_sbuf, uint8_t rx_len);

#ifdef __cplusplus
    }
#endif

#endif
