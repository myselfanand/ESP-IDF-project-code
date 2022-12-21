
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "OLEDDisplay.h"
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_system.h"


#define BUF_SIZE (1024)
uint8_t* data;
uint8_t* data2;

char lcd_data[3][20]; 

int rec_data = 0;
int len;
static const char *TAG = "oled-example";

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)
#define I2C_MASTER_SCL_IO 22               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_MASTER_PORT_NUM) /*!< I2C port number for master dev */

SemaphoreHandle_t print_mux = NULL;


static void echo_task()
{
    OLEDDisplay_t *oled = OLEDDisplay_init(I2C_MASTER_NUM,0x78,I2C_MASTER_SDA_IO,I2C_MASTER_SCL_IO);
            // int z;
    const int uart_num0 = UART_NUM_0;
    uart_config_t uart_config0 = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,    //UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 122,
    };   

    // We won't use a buffer for sending data.
    uart_driver_install(uart_num0, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(uart_num0, &uart_config0);
    uart_set_pin(uart_num0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
   
    data = (uint8_t*) malloc(BUF_SIZE);
    data2 = (uint8_t*) malloc(BUF_SIZE);


  while (1) {
       len = uart_read_bytes(uart_num0, data, BUF_SIZE, 500 / portTICK_RATE_MS);
        if (len > 0) {                   
            for(int i = 0; i < len; i++)
            {
              data2[i] = data[i];
            //   lcd_data[rec_data][i] = data[i];
            }
            data2[len] = '\0';  
            
          strcpy(lcd_data[rec_data],(const char*) data2);

          uart_write_bytes(uart_num0, (const char*) data2, len);
          OLEDDisplay_clear(oled);
          OLEDDisplay_setTextAlignment(oled,TEXT_ALIGN_CENTER);
          OLEDDisplay_setFont(oled,ArialMT_Plain_16);
        //   OLEDDisplay_drawString(oled,64, 00,(const char*)lcd_data[0]);
          
        // for (int z = 0; z < rec_data; z++)
        // {
        //     OLEDDisplay_drawString(oled,64, 25,(const char*)lcd_data[z]);
        // }

        if(rec_data == 0)
            OLEDDisplay_drawString(oled,64, 00,(const char*)lcd_data[0]);
            // OLEDDisplay_drawString(oled,64, 25,(const char*)lcd_data[1]);

		else if(rec_data == 1)
        {
            OLEDDisplay_drawString(oled,64, 00,(const char*)lcd_data[0]);
            OLEDDisplay_drawString(oled,64, 20,(const char*)lcd_data[1]);
        }

        else
        {
            OLEDDisplay_drawString(oled,64, 00,(const char*)lcd_data[0]);
            OLEDDisplay_drawString(oled,64, 20,(const char*)lcd_data[1]);
            OLEDDisplay_drawString(oled,64, 40,(const char*)lcd_data[2]);
         
            strcpy(lcd_data[0],lcd_data[1]);
         
            strcpy(lcd_data[1],lcd_data[2]);
        }

        // rec_data += 1;
        if(rec_data < 2)
         rec_data += 1;
        
        OLEDDisplay_display(oled);
		 vTaskDelay(500 / portTICK_PERIOD_MS);        
        }
    }
     vSemaphoreDelete(print_mux);
     vTaskDelete(NULL);  
}
    
void app_main(void)
{
    print_mux = xSemaphoreCreateMutex();
	ESP_LOGI(TAG,"Running");
    
    xTaskCreate(echo_task, "uart_echo_task", 1024*2, NULL, 10, NULL);
}