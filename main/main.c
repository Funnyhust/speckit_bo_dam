#include "app_config.h"
#include "audio_driver.h"
#include "board_pinout.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"
#include "wifi_transport.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "MAIN";

// Phase 3: StreamBuffer handles
static StreamBufferHandle_t tx_audio_buffer = NULL; // Mic → WiFi
static StreamBufferHandle_t rx_audio_buffer = NULL; // WiFi → Speaker

static void gpio_init(void) {
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

// Phase 3: Audio Capture Task (Mic → TX Buffer)
static void audio_capture_task(void *pvParameters) {
  ESP_LOGI(TAG, "Audio Capture Task started");

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

  ESP_LOGI(TAG, "Starting audio capture (pushing to TX buffer)...");

  while (1) {
    // Read from Microphone
    esp_err_t ret = audio_driver_read(buffer, buffer_size, &bytes_read);

    if (ret == ESP_OK && bytes_read > 0) {
      // Push to TX buffer (non-blocking)
      xStreamBufferSend(tx_audio_buffer, buffer, bytes_read, 0);
    } else {
      ESP_LOGW(TAG, "Read failed or no data");
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
}

// Phase 3: Audio Playback Task (RX Buffer → Speaker)
static void audio_playback_task(void *pvParameters) {
  ESP_LOGI(TAG, "Audio Playback Task started");

  int16_t buffer[120]; // 240 bytes
  size_t bytes_written = 0;

  while (1) {
    // Pop from RX buffer (blocking)
    size_t received =
        xStreamBufferReceive(rx_audio_buffer, buffer, 240, portMAX_DELAY);

    if (received == 240) {
      // Write to Speaker
      audio_driver_write(buffer, 240, &bytes_written);
    }
  }
}

void app_main(void) {
  ESP_LOGI(TAG, "=== Doppler Walkie-Talkie - Phase 3: Audio Streaming ===");

  // Initialize GPIO
  gpio_init();

  // Phase 3: Create StreamBuffers
  tx_audio_buffer = xStreamBufferCreate(AUDIO_BUFFER_SIZE, 240);
  if (tx_audio_buffer == NULL) {
    ESP_LOGE(TAG, "Failed to create TX audio buffer");
    return;
  }
  ESP_LOGI(TAG, "TX audio buffer created: %d bytes", AUDIO_BUFFER_SIZE);

  rx_audio_buffer = xStreamBufferCreate(AUDIO_BUFFER_SIZE, 240);
  if (rx_audio_buffer == NULL) {
    ESP_LOGE(TAG, "Failed to create RX audio buffer");
    return;
  }
  ESP_LOGI(TAG, "RX audio buffer created: %d bytes", AUDIO_BUFFER_SIZE);

  // Initialize WiFi and ESP-NOW
  ESP_ERROR_CHECK(wifi_transport_init());

  // Set buffer handles in wifi_transport
  wifi_transport_set_tx_buffer(tx_audio_buffer);
  wifi_transport_set_rx_buffer(rx_audio_buffer);

  // Create Audio Capture Task (Phase 3)
  xTaskCreate(audio_capture_task, "audio_capture", 4096, NULL, 5, NULL);

  // Create Audio Playback Task (Phase 3)
  xTaskCreate(audio_playback_task, "audio_playback", 4096, NULL, 5, NULL);

  // Create WiFi TX Task (Phase 3)
  xTaskCreate(wifi_tx_task, "wifi_tx", 4096, NULL, 5, NULL);

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
