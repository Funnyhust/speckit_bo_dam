#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

/**
 * @brief Initialize I2S audio driver
 * @return ESP_OK on success
 */
esp_err_t audio_driver_init(void);

/**
 * @brief Read audio data from microphone
 * @param buffer Pointer to buffer to store data
 * @param len Number of bytes to read
 * @param bytes_read Pointer to store actual bytes read
 * @return ESP_OK on success
 */
esp_err_t audio_driver_read(void *buffer, size_t len, size_t *bytes_read);

/**
 * @brief Write audio data to amplifier
 * @param buffer Pointer to data to write
 * @param len Number of bytes to write
 * @param bytes_written Pointer to store actual bytes written
 * @return ESP_OK on success
 */
esp_err_t audio_driver_write(const void *buffer, size_t len, size_t *bytes_written);

#endif // AUDIO_DRIVER_H
