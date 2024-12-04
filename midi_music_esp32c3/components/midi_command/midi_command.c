#include "include/midi_command.h"


void midi_command_uart_init(void)
{
    gpio_config_t tx_config = {
        .pin_bit_mask = GPIO_NUM_5,
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&tx_config);

    const uart_config_t midi_uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &midi_uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, GPIO_NUM_5, -1, -1, -1));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_set_mode(UART_NUM_1, UART_MODE_UART));

    /*
    gpio_config_t uart_console_gpio_config = {
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&uart_console_gpio_config);

    const uart_config_t uart_console_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_console_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, GPIO_NUM_21, GPIO_NUM_20, -1, -1));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_set_mode(UART_NUM_0, UART_MODE_UART));
    */
}


void midi_command_i2c_init(void)
{
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .scl_io_num = GPIO_NUM_3,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .sda_io_num = GPIO_NUM_4,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = 400000
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_cfg));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, i2c_cfg.mode, 0, 0, 0));    
}

void midi_command_i2c_write_byte(uint8_t addr, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_SLAVE_STM32_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, addr, 1);
    i2c_master_write_byte(cmd, data, 1);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 200);
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) printf("ERROR: %s\n", esp_err_to_name(err));
}

void midi_command_i2c_write_bytes(uint8_t addr, uint8_t *data, size_t data_len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_SLAVE_STM32_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, addr, 1);
    i2c_master_write(cmd, data, data_len, 1);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 200);
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) printf("ERROR: %s\n", esp_err_to_name(err));
}

void midi_command_i2c_read_byte(uint8_t addr, uint8_t *data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_SLAVE_STM32_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, addr, 1);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_SLAVE_STM32_ADDR << 1) | I2C_MASTER_READ, 1);
    i2c_master_read_byte(cmd, data, 1);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 200);
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) printf("ERROR: %s\n", esp_err_to_name(err));
}

void midi_command_i2c_read_bytes(uint8_t addr, uint8_t *data, size_t data_len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_SLAVE_STM32_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, addr, 1);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_SLAVE_STM32_ADDR << 1) | I2C_MASTER_READ, 1);
    i2c_master_read(cmd, data, data_len - 1, 1);
    i2c_master_read_byte(cmd, data, 0);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 200);
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) printf("ERROR: %s\n", esp_err_to_name(err));
}

