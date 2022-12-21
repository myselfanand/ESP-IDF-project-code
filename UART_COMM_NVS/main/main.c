#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN 17
#define RXD_PIN 16
#define UART UART_NUM_2

    uint8_t* data;
    char data2[32];

    nvs_handle handle;
    static const char *NVS_CUSTOMER = "customer data"; 
    static const char *data1 = "param 1";
    uint32_t str_length = 32;
    char str_data[32] = {0};
 

void init(void) 
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    // We won't use a buffer for sending data.
    uart_driver_install(UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}


static void tx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
	char* Txdata = (char*) malloc(100);
    while (1) {
        uart_write_bytes(UART, Txdata, strlen(Txdata));
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}


static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
      const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 500 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = '\0';
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
             
            // write data to nvs...
            nvs_open("NVS_CUSTOMER", NVS_READWRITE, &handle);
            for(int i = 0; i < rxBytes; i++)
            {
              data2[i] = data[i];
            }
            nvs_set_str( handle, data1, data2);
            nvs_commit(handle) ;
            nvs_close(handle);
        }
    }
    free(data);
}

void nvs_data(void)
{
    // Initialize the NVS partition
    // nvs_flash_init();
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    
    // Wait for memory initialization
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Open storage with a given namespace
    // storage namespace - "NVS_CUSTOMER"
    
    nvs_open("NVS_CUSTOMER", NVS_READWRITE, &handle);  
    nvs_get_str(handle, data1, str_data, &str_length);
    printf(" Data : %s len:%u\r\n", str_data, str_length);
    nvs_commit(handle) ;
    nvs_close(handle);
}
 
void app_main(void)
{
    nvs_data();
    init();
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-2, NULL);
}
