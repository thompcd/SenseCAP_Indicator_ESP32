/**
 * Focus Timer UI Implementation
 * 
 * Clean dark UI with timer display and control buttons
 */

#include "focus_timer_ui.h"
#include "lv_port.h"
#include "lvgl.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "rp2040_comm.h"

static const char *TAG = "focus_ui";

// Timer state
static volatile bool timer_running = false;
static volatile uint16_t remaining_seconds = 0;
static TimerHandle_t countdown_timer = NULL;

// UI elements
static lv_obj_t *screen = NULL;
static lv_obj_t *label_message = NULL;
static lv_obj_t *label_time = NULL;
static lv_obj_t *btn_5min = NULL;
static lv_obj_t *btn_15min = NULL;
static lv_obj_t *btn_30min = NULL;
static lv_obj_t *btn_start_stop = NULL;
static lv_obj_t *label_start_stop = NULL;

// Colors
#define COLOR_BG        lv_color_hex(0x1a1a2e)  // Dark navy
#define COLOR_TEXT      lv_color_hex(0xe0e0e0)  // Light gray
#define COLOR_ACCENT    lv_color_hex(0x16213e)  // Darker accent
#define COLOR_BTN       lv_color_hex(0x0f3460)  // Button blue
#define COLOR_BTN_ACTIVE lv_color_hex(0xe94560) // Active/Stop red
#define COLOR_TIME      lv_color_hex(0x00ff88)  // Bright green for time

// Forward declarations
static void update_display(void);
static void timer_callback(TimerHandle_t xTimer);

// Button callbacks
static void btn_5min_cb(lv_event_t *e)
{
    if (!timer_running) {
        remaining_seconds = 5 * 60;
        update_display();
        ESP_LOGI(TAG, "Set timer to 5 minutes");
    }
}

static void btn_15min_cb(lv_event_t *e)
{
    if (!timer_running) {
        remaining_seconds = 15 * 60;
        update_display();
        ESP_LOGI(TAG, "Set timer to 15 minutes");
    }
}

static void btn_30min_cb(lv_event_t *e)
{
    if (!timer_running) {
        remaining_seconds = 30 * 60;
        update_display();
        ESP_LOGI(TAG, "Set timer to 30 minutes");
    }
}

static void btn_start_stop_cb(lv_event_t *e)
{
    if (timer_running) {
        // Stop timer
        timer_running = false;
        if (countdown_timer) {
            xTimerStop(countdown_timer, 0);
        }
        lv_label_set_text(label_start_stop, "START");
        lv_obj_set_style_bg_color(btn_start_stop, COLOR_BTN, LV_PART_MAIN);
        ESP_LOGI(TAG, "Timer stopped");
    } else if (remaining_seconds > 0) {
        // Start timer
        timer_running = true;
        if (countdown_timer) {
            xTimerStart(countdown_timer, 0);
        }
        lv_label_set_text(label_start_stop, "STOP");
        lv_obj_set_style_bg_color(btn_start_stop, COLOR_BTN_ACTIVE, LV_PART_MAIN);
        ESP_LOGI(TAG, "Timer started: %d seconds", remaining_seconds);
    }
    update_display();
}

static void send_status_to_rp2040(void)
{
    timer_status_t status = {
        .running = timer_running ? 1 : 0,
        .minutes = remaining_seconds / 60,
        .seconds = remaining_seconds % 60
    };
    rp2040_send_timer_status(&status);
}

static void update_display(void)
{
    // Send status to RP2040 (for I2C exposure)
    send_status_to_rp2040();
    
    lv_port_sem_take();
    
    uint8_t mins = remaining_seconds / 60;
    uint8_t secs = remaining_seconds % 60;
    
    // Update time display
    static char time_buf[16];
    snprintf(time_buf, sizeof(time_buf), "%d:%02d", mins, secs);
    lv_label_set_text(label_time, time_buf);
    
    // Update message
    if (timer_running && remaining_seconds > 0) {
        lv_label_set_text(label_message, "Heads down. Be back in");
        lv_obj_set_style_text_color(label_time, COLOR_TIME, LV_PART_MAIN);
    } else if (remaining_seconds == 0) {
        lv_label_set_text(label_message, "Focus time complete!");
        lv_obj_set_style_text_color(label_time, COLOR_TEXT, LV_PART_MAIN);
    } else {
        lv_label_set_text(label_message, "Select duration");
        lv_obj_set_style_text_color(label_time, COLOR_TEXT, LV_PART_MAIN);
    }
    
    lv_port_sem_give();
}

static void timer_callback(TimerHandle_t xTimer)
{
    if (timer_running && remaining_seconds > 0) {
        remaining_seconds--;
        update_display();
        
        if (remaining_seconds == 0) {
            timer_running = false;
            xTimerStop(countdown_timer, 0);
            lv_port_sem_take();
            lv_label_set_text(label_start_stop, "START");
            lv_obj_set_style_bg_color(btn_start_stop, COLOR_BTN, LV_PART_MAIN);
            lv_port_sem_give();
            ESP_LOGI(TAG, "Timer complete!");
        }
    }
}

static lv_obj_t* create_preset_button(lv_obj_t *parent, const char *text, 
                                       lv_coord_t x, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 100, 50);
    lv_obj_set_pos(btn, x, 280);
    lv_obj_set_style_bg_color(btn, COLOR_BTN, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_center(label);
    
    return btn;
}

void focus_timer_ui_init(void)
{
    ESP_LOGI(TAG, "Initializing Focus Timer UI");
    
    // Create main screen with dark background
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, COLOR_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    
    // Message label
    label_message = lv_label_create(screen);
    lv_label_set_text(label_message, "Select duration");
    lv_obj_set_style_text_color(label_message, COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(label_message, &lv_font_montserrat_26, LV_PART_MAIN);
    lv_obj_align(label_message, LV_ALIGN_TOP_MID, 0, 80);
    
    // Time display (large)
    label_time = lv_label_create(screen);
    lv_label_set_text(label_time, "0:00");
    lv_obj_set_style_text_color(label_time, COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_align(label_time, LV_ALIGN_CENTER, 0, -30);
    
    // Preset buttons (5min, 15min, 30min)
    btn_5min = create_preset_button(screen, "5 min", 70, btn_5min_cb);
    btn_15min = create_preset_button(screen, "15 min", 190, btn_15min_cb);
    btn_30min = create_preset_button(screen, "30 min", 310, btn_30min_cb);
    
    // START/STOP button
    btn_start_stop = lv_btn_create(screen);
    lv_obj_set_size(btn_start_stop, 200, 60);
    lv_obj_align(btn_start_stop, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_bg_color(btn_start_stop, COLOR_BTN, LV_PART_MAIN);
    lv_obj_set_style_radius(btn_start_stop, 15, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_start_stop, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn_start_stop, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn_start_stop, btn_start_stop_cb, LV_EVENT_CLICKED, NULL);
    
    label_start_stop = lv_label_create(btn_start_stop);
    lv_label_set_text(label_start_stop, "START");
    lv_obj_set_style_text_color(label_start_stop, COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(label_start_stop, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_center(label_start_stop);
    
    // Create countdown timer (1 second period)
    countdown_timer = xTimerCreate("countdown", pdMS_TO_TICKS(1000), pdTRUE, 
                                    NULL, timer_callback);
    
    // Load the screen
    lv_scr_load(screen);
    
    ESP_LOGI(TAG, "Focus Timer UI initialized");
}

bool focus_timer_is_running(void)
{
    return timer_running;
}

uint16_t focus_timer_get_remaining_seconds(void)
{
    return remaining_seconds;
}

uint8_t focus_timer_get_remaining_minutes(void)
{
    return remaining_seconds / 60;
}

uint8_t focus_timer_get_remaining_secs(void)
{
    return remaining_seconds % 60;
}
