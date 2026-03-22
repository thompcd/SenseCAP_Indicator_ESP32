/**
 * RP2040 Communication for Focus Timer
 * 
 * Sends timer status to RP2040 via internal UART.
 * RP2040 then exposes this via I2C on Grove connector.
 */

#include "rp2040_comm.h"
#include "cobs.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "rp2040_comm";

// UART pins for ESP32↔RP2040 communication
#define ESP32_RP2040_TXD    (19)
#define ESP32_RP2040_RXD    (20)
#define UART_PORT_NUM       (2)
#define UART_BAUD_RATE      (115200)

esp_err_t rp2040_comm_init(void)
{
    ESP_LOGI(TAG, "Initializing RP2040 communication");
    
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, 512, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, ESP32_RP2040_TXD, ESP32_RP2040_RXD, 
                                  UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    
    ESP_LOGI(TAG, "RP2040 UART ready (TX:%d, RX:%d)", ESP32_RP2040_TXD, ESP32_RP2040_RXD);
    return ESP_OK;
}

esp_err_t rp2040_send_timer_status(timer_status_t *status)
{
    uint8_t data[8] = {0};
    uint8_t encoded[16] = {0};
    
    // Build packet: [type][running][minutes][seconds]
    data[0] = PKT_TYPE_TIMER_STATUS;
    data[1] = status->running;
    data[2] = status->minutes;
    data[3] = status->seconds;
    
    // COBS encode
    cobs_encode_result result = cobs_encode(encoded, sizeof(encoded), data, 4);
    
    if (result.status != COBS_ENCODE_OK) {
        ESP_LOGE(TAG, "COBS encode failed: %d", result.status);
        return ESP_FAIL;
    }
    
    // Send with null terminator
    uart_write_bytes(UART_PORT_NUM, encoded, result.out_len + 1);
    
    return ESP_OK;
}
