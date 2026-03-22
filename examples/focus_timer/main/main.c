/**
 * Focus Timer for SenseCAP Indicator
 * 
 * A simple focus timer with:
 * - Dark UI showing "Heads down. Be back in X:XX"
 * - 5min, 15min, 30min preset buttons
 * - START/STOP button
 * - I2C slave on Grove connector exposing timer status
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp_board.h"
#include "lv_port.h"
#include "focus_timer_ui.h"
#include "i2c_slave.h"
#include "rp2040_comm.h"

static const char *TAG = "focus_timer";

#define VERSION   "v1.0.0"

#define BANNER  "\n\
  _____ ___   ____ _   _ ____    _____ ___ __  __ _____ ____  \n\
 |  ___/ _ \\ / ___| | | / ___|  |_   _|_ _|  \\/  | ____|  _ \\ \n\
 | |_ | | | | |   | | | \\___ \\    | |  | || |\\/| |  _| | |_) |\n\
 |  _|| |_| | |___|  |_| |___) |   | |  | || |  | | |___|  _ < \n\
 |_|   \\___/ \\____|\\___/|____/    |_| |___|_|  |_|_____|_| \\_\\\n\
----------------------------------------------------------\n\
 Version: %s %s %s\n\
----------------------------------------------------------\n\
"

void app_main(void)
{
    ESP_LOGI("", BANNER, VERSION, __DATE__, __TIME__);

    // Initialize board (LCD, touch, etc.)
    ESP_ERROR_CHECK(bsp_board_init());
    
    // Initialize LVGL port
    lv_port_init();
    
    // NOTE: I2C slave disabled - GPIO 39/40 conflicts with touch controller
    // Instead, we communicate with RP2040 via UART, and RP2040 exposes I2C
    
    // Initialize RP2040 communication
    rp2040_comm_init();
    
    // Create the Focus Timer UI
    lv_port_sem_take();
    focus_timer_ui_init();
    lv_port_sem_give();
    
    ESP_LOGI(TAG, "Focus Timer initialized. I2C slave at 0x42");
    
    // Main loop - timer logic handled in UI task
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
