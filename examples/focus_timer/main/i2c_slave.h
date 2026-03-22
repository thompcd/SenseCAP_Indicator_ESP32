/**
 * I2C Slave for Focus Timer
 * 
 * Exposes timer status via I2C on Grove connector:
 * - Address: 0x42
 * - Register 0x00: Status (0=stopped, 1=running)
 * - Register 0x01: Minutes remaining
 * - Register 0x02: Seconds remaining (0-59)
 */

#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_SLAVE_ADDR  0x42

// Registers
#define REG_STATUS      0x00
#define REG_MINUTES     0x01
#define REG_SECONDS     0x02

/**
 * @brief Initialize I2C slave
 * @return ESP_OK on success
 */
esp_err_t i2c_slave_init(void);

/**
 * @brief Deinitialize I2C slave
 * @return ESP_OK on success
 */
esp_err_t i2c_slave_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // I2C_SLAVE_H
