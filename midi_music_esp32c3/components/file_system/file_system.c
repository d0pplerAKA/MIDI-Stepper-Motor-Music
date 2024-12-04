#include "include/file_system.h"


uint8_t crc_table[256];
static wl_handle_t wl_handle = WL_INVALID_HANDLE;

uint8_t flash_mount(void)
{
    const esp_vfs_fat_mount_config_t mount_config = {
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE,
        .max_files = 3,
        .format_if_mount_failed = true
    };

    esp_err_t err = esp_vfs_fat_spiflash_mount(MOUNT_PATH, "storage", &mount_config, &wl_handle);
    return err;
}


uint8_t flash_unmount(void)
{
    esp_err_t err = esp_vfs_fat_spiflash_unmount(MOUNT_PATH, wl_handle);

    return err;
}


void flash_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}


void flash_list_files(void)
{
    if(flash_mount() == ESP_OK)
    {
        struct dirent *entry;
        DIR *dir = opendir(MOUNT_PATH);

        if(dir == NULL)
        {
            printf("Error opening directory '%s'\n", MOUNT_PATH);
            return;
        }

        printf("Listing files in directory: %s\n", MOUNT_PATH);
        while((entry = readdir(dir)) != NULL)
        {
            printf("Found file: %s\n", entry->d_name);
        }

        closedir(dir);

        flash_unmount();
    }
    else printf("flash mounting error\n");
}

uint8_t file_is_valid(char *valid_name)
{
    if(strstr(valid_name, ".mid") == NULL) return 1;
    else return 0;
}

uint8_t file_delete(const char *fileName)
{
    if(flash_mount() == ESP_OK)
    {
        DIR *dir;
        struct dirent *entry;
        int fileFound = 0;
    
        dir = opendir(MOUNT_PATH);
        if(dir == NULL) return 1;

        while((entry = readdir(dir)) != NULL)
        {
            if(strcmp(entry->d_name, fileName) == 0)
            {
                fileFound = 1;
                break;
            }
        }

        closedir(dir);

        if(fileFound)
        {
            char fullPath[64];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", MOUNT_PATH, fileName);

            return unlink(fullPath);
        }
        else return 2;  // 文件未找到
    }
    else return 1;
}

void midi_file_delete(void *pt)
{
    const char *file_name = (char *) pt;

    uint8_t rst = file_delete(file_name);
    if(rst == 2) printf("File Delete Failed, File Doesnt Exist\n");
    else if(rst == 1) printf("File System Mount Fail\n");
    else printf("File Delete Success\n");

    vTaskDelete(NULL);
}


/**
 * @brief system console
 * 
 *  midi
 *  midi play []
 *  midi delete [] 
 *  midi list
 *  midi upload
 * 
 * @param rx_sbuf command string
 * @param rx_len command string length
 * @return uint32_t command code
 */
uint32_t system_console(char *rx_sbuf, uint8_t rx_len)
{
    printf("-> input: %s\n", rx_sbuf);

    if(strncmp("midi", rx_sbuf, sizeof(uint8_t) * 4) == 0)
    {
        if(strncmp("play", rx_sbuf + 5, sizeof(uint8_t) * 4) == 0)
        {

            return 1;
        }
        else if(strncmp("delete", rx_sbuf + 5, sizeof(uint8_t) * 6) == 0)
        {
           
            return 2;
        }
        else if(strncmp("list", rx_sbuf + 5, sizeof(uint8_t) * 4) == 0)
        {

            return 3;
        }
        else if(strncmp("upload", rx_sbuf + 5, sizeof(uint8_t) * 6) == 0)
        {
            if(*(rx_sbuf + 12) == '0') return 5;
            else return 4;
        }
        else
        {
            printf("midi system console\n");

            return 0;
        }
    }
    else return 0;
}
