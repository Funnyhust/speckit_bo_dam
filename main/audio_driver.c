#include "audio_driver.h"
#include "board_pinout.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <string.h>

static const char *TAG = "AUDIO_DRIVER";

// I2S channel handles
static i2s_chan_handle_t tx_handle = NULL;
static i2s_chan_handle_t rx_handle = NULL;

// Audio configuration
#define SAMPLE_RATE (16000)
#define SAMPLE_BITS (16)
#define I2S_DMA_BUF_COUNT (4)
#define I2S_DMA_BUF_LEN (256) // 256 samples = 512 bytes

esp_err_t audio_driver_init(void) {
  ESP_LOGI(TAG, "Initializing I2S Driver (16kHz, 16-bit, Mono)...");

  // Channel configuration
  i2s_chan_config_t chan_cfg =
      I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT_NUM, I2S_ROLE_MASTER);
  chan_cfg.auto_clear = true;
  chan_cfg.dma_desc_num = I2S_DMA_BUF_COUNT;
  chan_cfg.dma_frame_num = I2S_DMA_BUF_LEN;

  // Allocate TX and RX channels
  ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));

  // TX (Speaker) configuration
  i2s_std_config_t tx_std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                      I2S_SLOT_MODE_MONO),
      .gpio_cfg =
          {
              .mclk = I2S_GPIO_UNUSED,
              .bclk = I2S_BCLK_IO,
              .ws = I2S_LRCK_IO,
              .dout = I2S_DO_IO,
              .din = I2S_GPIO_UNUSED,
              .invert_flags =
                  {
                      .mclk_inv = false,
                      .bclk_inv = false,
                      .ws_inv = false,
                  },
          },
  };

  // RX (Microphone) configuration
  i2s_std_config_t rx_std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                      I2S_SLOT_MODE_MONO),
      .gpio_cfg =
          {
              .mclk = I2S_GPIO_UNUSED,
              .bclk = I2S_BCLK_IO,
              .ws = I2S_LRCK_IO,
              .dout = I2S_GPIO_UNUSED,
              .din = I2S_DI_IO,
              .invert_flags =
                  {
                      .mclk_inv = false,
                      .bclk_inv = false,
                      .ws_inv = false,
                  },
          },
  };

  // Initialize channels
  ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &tx_std_cfg));
  ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &rx_std_cfg));

  // Enable channels
  ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
  ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

  ESP_LOGI(TAG, "I2S Driver initialized successfully");
  return ESP_OK;
}

esp_err_t audio_driver_read(void *buffer, size_t len, size_t *bytes_read) {
  if (rx_handle == NULL) {
    return ESP_FAIL;
  }
  return i2s_channel_read(rx_handle, buffer, len, bytes_read, portMAX_DELAY);
}

/**
 * @brief Apply software gain to audio buffer
 * @param buffer Pointer to audio buffer (int16_t samples)
 * @param sample_count Number of samples in buffer
 * @param gain_db Gain in dB (0 = no change, 6 = 2x louder, 12 = 4x louder)
 */
static void apply_gain(int16_t *buffer, size_t sample_count, float gain_db) {
  // Convert dB to linear gain: gain_linear = 10^(gain_db/20)
  float gain_linear = powf(10.0f, gain_db / 20.0f);

  for (size_t i = 0; i < sample_count; i++) {
    // Apply gain
    int32_t sample = (int32_t)(buffer[i] * gain_linear);

    // Clipping protection (prevent overflow)
    if (sample > 32767) {
      sample = 32767;
    } else if (sample < -32768) {
      sample = -32768;
    }

    buffer[i] = (int16_t)sample;
  }
}

esp_err_t audio_driver_write(const void *buffer, size_t len,
                             size_t *bytes_written) {
  if (tx_handle == NULL) {
    return ESP_FAIL;
  }

#if SOFTWARE_GAIN_ENABLE
  // Apply software gain (cấu hình trong app_config.h)
  int16_t *samples = (int16_t *)buffer;
  size_t sample_count = len / sizeof(int16_t);
  apply_gain(samples, sample_count, SOFTWARE_GAIN_DB);
#endif

  return i2s_channel_write(tx_handle, buffer, len, bytes_written,
                           portMAX_DELAY);
}