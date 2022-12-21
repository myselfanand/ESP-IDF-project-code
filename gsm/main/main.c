#include<stdio.h>
#include<string.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
//#include "sdkconfig.h"

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define UART UART_NUM_2
//int num=0;

#define BUF_SIZE (1024)
#define RX_BUF_SIZE (2024)

// static int len;
// const char a[] = {0x0D};
// len = strlen(a);

//static const char *TAG = "GSM_EXAMPLE";
void init(void){
	const uart_config_t uart_config = {
		.baud_rate=9600,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
		};
	uart_driver_install(UART,BUF_SIZE*2,0,0,NULL,0);
	uart_param_config(UART,&uart_config);
	uart_set_pin(UART,TXD_PIN,RXD_PIN,UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE);
	ESP_ERROR_CHECK(uart_wait_tx_done(UART, 100)); // wait timeout is 100 RTOS ticks (TickType_t)
}

static void tx_task()
{
	int len;
	const char a = 0x0D;
	const char b[] = "AT";
	const char c[] = "AT+CMGS=\"+918503829785\"";
	len = strlen(b);
		printf("sending data..\n");
        uart_write_bytes(UART, b, len);
		uart_write_bytes(UART,&a,1);
		len=strlen(c);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		uart_write_bytes(UART, c, len);
		uart_write_bytes(UART,&a,1);

		vTaskDelay(2000 / portTICK_PERIOD_MS);
		const char m[]="\n HY I am neeraj ";
		len=strlen(m);
		uart_write_bytes(UART, m, len);
		vTaskDelay(20 / portTICK_PERIOD_MS);
		const char z = 0x1A;
		uart_write_bytes(UART,&z,1);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		printf("AT sent\n");
       
}


static void rx_task(void *arg){
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*)malloc(RX_BUF_SIZE+1);
    while (1){
        const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 100 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            data[rxBytes] = '\0';
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: %s", rxBytes, data);
        }
		vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    free(data);
}



void app_main(void)
{
    init();
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	tx_task();
    //xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
	xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
	vTaskDelay(4000 / portTICK_PERIOD_MS);
	printf("task done..\n");
}


	