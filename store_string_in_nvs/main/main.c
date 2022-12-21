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

void app_main(void)
{
    // Initialize the NVS partition
    // nvs_flash_init();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    
    // Wait for memory initialization
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    nvs_handle handle;
    static const char *NVS_CUSTOMER = "customer data";
    static const char *data1 = "param 1";
    uint32_t str_length = 32;
    char str_data[32] = {0};

    // Open storage with a given namespace
    // storage namespace - "storage"
    nvs_open("NVS_CUSTOMER", NVS_READWRITE, &handle);
    
    // Read data
    // key - "data"
    // value - "read_data"
     
    nvs_get_str(handle, data1, str_data, &str_length);
    printf(" Data : %s len:%u\r\n", str_data, str_length);
   
    nvs_set_str( handle, data1, "embedded garage");

    nvs_commit(handle) ;
    nvs_close(handle);
}
