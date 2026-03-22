/**
 * I2C Slave Implementation for Focus Timer
 * 
 * Uses ESP-IDF I2C driver to expose timer status.
 * Connected to Grove I2C connector.
 * 
 * Registers:
 * - 0x00: Status (0=stopped, 1=running)
 * - 0x01: Minutes remaining
 * - 0x02: Seconds remaining (0-59)
 */

#include "i2c_slave.h"
#include "focus_timer_ui.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "i2c_slave";

// I2C Slave Configuration
// Using GPIO 19 (SDA) and GPIO 20 (SCL) for Grove connector
#define I2C_SLAVE_SDA_IO    GPIO_NUM_39
#define I2C_SLAVE_SCL_IO    GPIO_NUM_40
#define I2C_SLAVE_PORT      I2C_NUM_1
#define I2C_SLAVE_RX_BUF    256
#define I2C_SLAVE_TX_BUF    256

static volatile bool slave_initialized = false;

// Task to handle I2C slave requests
static void i2c_slave_task(void *arg)
{
    uint8_t rx_data[4];
    uint8_t tx_data[4];
    int len;
    
    ESP_LOGI(TAG, "I2C slave task started");
    
    while (slave_initialized) {
        // Prepare current timer state
        tx_data[REG_STATUS] = focus_timer_is_running() ? 1 : 0;
        tx_data[REG_MINUTES] = focus_timer_get_remaining_minutes();
        tx_data[REG_SECONDS] = focus_timer_get_remaining_secs();
        tx_data[3] = 0;
        
        // Try to read register address from master
        len = i2c_slave_read_buffer(I2C_SLAVE_PORT, rx_data, 1, pdMS_TO_TICKS(50));
        
        if (len > 0) {
            uint8_t reg = rx_data[0] % 3;
            
            // Write response to TX buffer for master to read
            i2c_slave_write_buffer(I2C_SLAVE_PORT, &tx_data[reg], 1, pdMS_TO_TICKS(50));
            
            ESP_LOGD(TAG, "Reg request: 0x%02X, value: %d", reg, tx_data[reg]);
        }
        
        // Also proactively fill TX buffer with status for reads without register address
        i2c_slave_write_buffer(I2C_SLAVE_PORT, tx_data, 3, 0);
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    vTaskDelete(NULL);
}

esp_err_t i2c_slave_init(void)
{
    ESP_LOGI(TAG, "Initializing I2C slave on addr 0x%02X", I2C_SLAVE_ADDR);
    ESP_LOGI(TAG, "SDA: GPIO%d, SCL: GPIO%d", I2C_SLAVE_SDA_IO, I2C_SLAVE_SCL_IO);
    
    i2c_config_t conf_slave = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = I2C_SLAVE_SDA_IO,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave.addr_10bit_en = 0,
        .slave.slave_addr = I2C_SLAVE_ADDR,
        .slave.maximum_speed = 400000,
        .clk_flags = 0,
    };
    
    esp_err_t ret = i2c_param_config(I2C_SLAVE_PORT, &conf_slave);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = i2c_driver_install(I2C_SLAVE_PORT, I2C_MODE_SLAVE, 
                             I2C_SLAVE_RX_BUF, I2C_SLAVE_TX_BUF, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    slave_initialized = true;
    
    // Create slave handler task
    xTaskCreate(i2c_slave_task, "i2c_slave", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "I2C slave initialized successfully");
    return ESP_OK;
}

esp_err_t i2c_slave_deinit(void)
{
    if (slave_initialized) {
        slave_initialized = false;
        vTaskDelay(pdMS_TO_TICKS(100)); // Let task exit
        
        return i2c_driver_delete(I2C_SLAVE_PORT);
    }
    return ESP_OK;
}
