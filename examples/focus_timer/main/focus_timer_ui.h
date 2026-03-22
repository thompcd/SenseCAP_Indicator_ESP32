/**
 * Focus Timer UI Header
 */

#ifndef FOCUS_TIMER_UI_H
#define FOCUS_TIMER_UI_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Focus Timer UI
 */
void focus_timer_ui_init(void);

/**
 * @brief Get timer running status
 * @return true if timer is running, false otherwise
 */
bool focus_timer_is_running(void);

/**
 * @brief Get remaining time in seconds
 * @return remaining seconds
 */
uint16_t focus_timer_get_remaining_seconds(void);

/**
 * @brief Get remaining minutes
 * @return remaining minutes
 */
uint8_t focus_timer_get_remaining_minutes(void);

/**
 * @brief Get remaining seconds (0-59)
 * @return remaining seconds within current minute
 */
uint8_t focus_timer_get_remaining_secs(void);

#ifdef __cplusplus
}
#endif

#endif // FOCUS_TIMER_UI_H
