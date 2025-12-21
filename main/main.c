#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "board_pinout.h"
#include "audio_driver.h"

static const char *TAG = "MAIN";

// Calculate RMS (Root Mean Square) for audio level monitoring
static float calculate_rms(const int16_t *samples, size_t sample_count)
{
    float sum = 0.0f;
    for (size_t i = 0; i < sample_count; i++) {
        float normalized = (float)samples[i] / 32768.0f;
        sum += normalized * normalized;
    }
    return sqrtf(sum / sample_count);
}

static void gpio_init(void)
{
    // Configure LED
    gpio_reset_pin(GPIO_STATUS_LED);
    gpio_set_direction(GPIO_STATUS_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_STATUS_LED, !LED_ACTIVE_LEVEL);

    // Configure PTT Button
    gpio_reset_pin(GPIO_PTT_BUTTON);
    gpio_set_direction(GPIO_PTT_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_PTT_BUTTON, GPIO_PULLUP_ONLY);
    
    ESP_LOGI(TAG, "GPIO initialized");
}

static void audio_loopback_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Audio Loopback Task started");
    
    // Initialize Audio Driver
    ESP_ERROR_CHECK(audio_driver_init());

    // Allocate buffer: 512 bytes = 256 samples (16-bit mono)
    const size_t buffer_size = 512;
    int16_t *buffer = (int16_t *)malloc(buffer_size);
    
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate audio buffer");
        vTaskDelete(NULL);
        return;
    }

    size_t bytes_read = 0;
    size_t bytes_written = 0;
    uint32_t loop_count = 0;
    
    ESP_LOGI(TAG, "Starting audio loopback (512 bytes per iteration)...");

    while (1) {
        // Read from Microphone
        esp_err_t ret = audio_driver_read(buffer, buffer_size, &bytes_read);
        
        if (ret == ESP_OK && bytes_read > 0) {
            // Write to Speaker immediately
            audio_driver_write(buffer, bytes_read, &bytes_written);
            
            // Debug: Calculate and log RMS every 1 second
            loop_count++;
            if (loop_count % 31 == 0) {  // ~31 iterations/sec at 16kHz with 512-byte buffers
                float rms = calculate_rms(buffer, bytes_read / 2);
                ESP_LOGI(TAG, "Audio RMS: %.4f (bytes: %d)", rms, bytes_read);
            }
        } else {
            ESP_LOGW(TAG, "Read failed or no data");
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== Doppler Walkie-Talkie - Audio Loopback Test ===");
    
    // Initialize GPIO
    gpio_init();
    
    // Create Audio Loopback Task
    xTaskCreate(audio_loopback_task, "audio_loopback", 4096, NULL, 5, NULL);
    
    // Main loop: Monitor PTT button and control LED
    while (1) {
        int ptt_state = gpio_get_level(GPIO_PTT_BUTTON);
        
        if (ptt_state == PTT_ACTIVE_LEVEL) {
            gpio_set_level(GPIO_STATUS_LED, LED_ACTIVE_LEVEL);
        } else {
            gpio_set_level(GPIO_STATUS_LED, !LED_ACTIVE_LEVEL);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
