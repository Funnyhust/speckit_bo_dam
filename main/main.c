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
#include <stdio.h>
#include <string.h>
#include "driver/uart.h"

static const char *TAG = "MAIN";

// Phase 3: StreamBuffer handles
static StreamBufferHandle_t tx_audio_buffer = NULL; // Mic → WiFi
static StreamBufferHandle_t rx_audio_buffer = NULL; // WiFi → Speaker

// Phase 4: PTT state and pre-buffering
static volatile bool ptt_pressed = false;
static volatile uint8_t rx_packet_count = 0; // Count packets received
#define MIN_PACKETS_BEFORE_PLAY 10           // Wait for 3 packets (~720 bytes)

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
    // Phase 4: Only capture when PTT is pressed
    if (ptt_pressed) {
      // Read from Microphone
      esp_err_t ret = audio_driver_read(buffer, buffer_size, &bytes_read);

      if (ret == ESP_OK && bytes_read > 0) {
        // Push to TX buffer (non-blocking)
        xStreamBufferSend(tx_audio_buffer, buffer, bytes_read, 0);
      } else {
        ESP_LOGW(TAG, "Read failed or no data");
        vTaskDelay(pdMS_TO_TICKS(10));
      }
    } else {
      // PTT not pressed, just wait
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}

// Phase 3: Audio Playback Task (RX Buffer → Speaker)
static void audio_playback_task(void *pvParameters) {
  ESP_LOGI(TAG, "Audio Playback Task started");

#if TEST_MODE_STREAMING
  // Test Mode: Read Test Packet -> UART
  test_streaming_packet_t test_pkt;
  const size_t item_size = sizeof(test_streaming_packet_t);
  
  ESP_LOGW(TAG, "TEST MODE ENABLED: Speaker DISABLED. Streaming to UART.");
#else
  // Normal Mode: Read Raw PCM -> Speaker
  int16_t buffer[120]; // 240 bytes
  const size_t item_size = 240;
  size_t bytes_written = 0;
#endif

  while (1) {
#if TEST_MODE_STREAMING
    // Read test packet (245 bytes)
    size_t received = xStreamBufferReceive(rx_audio_buffer, &test_pkt, item_size, portMAX_DELAY);
    if (received == item_size) {
        // Write entire packet to UART
        uart_write_bytes(UART_NUM_0, (const char*)&test_pkt, item_size);
    }
#else
    // Pop from RX buffer (blocking)
    size_t received =
        xStreamBufferReceive(rx_audio_buffer, buffer, item_size, portMAX_DELAY);

    if (received == item_size) {
      // Phase 4: Only play after receiving at least 3 packets
      if (rx_packet_count >= MIN_PACKETS_BEFORE_PLAY) {
        // Write to Speaker
        audio_driver_write(buffer, 240, &bytes_written);
      }
      // Increment packet count (saturate at MIN_PACKETS_BEFORE_PLAY + 1)
      if (rx_packet_count < MIN_PACKETS_BEFORE_PLAY + 1) {
        rx_packet_count++;
      }
    }
#endif
  }
}

void app_main(void) {
  ESP_LOGI(TAG, "=== Doppler Walkie-Talkie - Phase 3: Audio Streaming ===");

  // Initialize GPIO
  gpio_init();

#if TEST_MODE_STREAMING
  // Init UART High Speed
  uart_config_t uart_config = {
      .baud_rate = 2000000,
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 2048, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
  ESP_LOGW(TAG, "UART initialized at 2,000,000 Baud for Streaming");
#endif

  // Phase 3: Create StreamBuffers
  tx_audio_buffer = xStreamBufferCreate(AUDIO_BUFFER_SIZE, 240);
  if (tx_audio_buffer == NULL) {
    ESP_LOGE(TAG, "Failed to create TX audio buffer");
    return;
  }
  ESP_LOGI(TAG, "TX audio buffer created: %d bytes", AUDIO_BUFFER_SIZE);

#if TEST_MODE_STREAMING
  rx_audio_buffer = xStreamBufferCreate(AUDIO_BUFFER_SIZE, sizeof(test_streaming_packet_t));
#else
  rx_audio_buffer = xStreamBufferCreate(AUDIO_BUFFER_SIZE, 240);
#endif
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
  bool last_ptt_state = false;

  while (1) {
    int ptt_state = gpio_get_level(GPIO_PTT_BUTTON);
    bool ptt_active = (ptt_state == PTT_ACTIVE_LEVEL);

    // Phase 4: PTT state change detection
    if (ptt_active != last_ptt_state) {
      if (ptt_active) {
        // PTT pressed: Start transmission
        ESP_LOGI(TAG, "PTT PRESSED - TX Mode");
        ptt_pressed = true;
        gpio_set_level(GPIO_STATUS_LED, LED_ACTIVE_LEVEL);

        // Reset RX buffer and packet count (prepare for next reception)
        xStreamBufferReset(rx_audio_buffer);
        rx_packet_count = 0;
      } else {
        // PTT released: Return to RX mode
        ESP_LOGI(TAG, "PTT RELEASED - RX Mode");
        ptt_pressed = false;
        gpio_set_level(GPIO_STATUS_LED, !LED_ACTIVE_LEVEL);

        // Clear TX buffer (stop any pending transmission)
        xStreamBufferReset(tx_audio_buffer);
      }
      last_ptt_state = ptt_active;
    }

    vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
  }
}
