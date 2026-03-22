#ifndef RP2040_COMM_H
#define RP2040_COMM_H

#include <stdint.h>
#include "esp_err.h"

// Packet types for ESP32↔RP2040 communication
#define PKT_TYPE_ACK                0x00
#define PKT_TYPE_TIMER_STATUS       0xC0  // Custom: timer state

// Timer status structure
typedef struct {
    uint8_t running;     // 0=stopped, 1=running
    uint8_t minutes;     // minutes remaining
    uint8_t seconds;     // seconds remaining
} timer_status_t;

/**
 * Initialize RP2040 communication (UART)
 */
esp_err_t rp2040_comm_init(void);

/**
 * Send timer status to RP2040
 */
esp_err_t rp2040_send_timer_status(timer_status_t *status);

#endif // RP2040_COMM_H
