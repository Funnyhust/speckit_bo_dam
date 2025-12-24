#ifndef WIFI_TRANSPORT_H
#define WIFI_TRANSPORT_H

#include "app_config.h"
#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Initialize WiFi in Station Mode and ESP-NOW
 * @return ESP_OK on success
 */
esp_err_t wifi_transport_init(void);

/**
 * @brief Send audio packet via ESP-NOW broadcast
 * @param packet Pointer to audio packet to send
 * @return ESP_OK on success
 */
esp_err_t wifi_transport_send(const audio_packet_t *packet);

/**
 * @brief Get current sequence number for TX
 * @return Current sequence number
 */
uint16_t wifi_transport_get_seq_num(void);

/**
 * @brief Set TX audio buffer handle (Phase 3)
 * @param buffer StreamBuffer handle for audio TX
 */
void wifi_transport_set_tx_buffer(void *buffer);

/**
 * @brief Set RX audio buffer handle (Phase 3)
 * @param buffer StreamBuffer handle for audio RX
 */
void wifi_transport_set_rx_buffer(void *buffer);

/**
 * @brief WiFi TX task - Pop audio from buffer and send via ESP-NOW (Phase 3)
 * @param pvParameters Task parameters (unused)
 */
void wifi_tx_task(void *pvParameters);

#endif // WIFI_TRANSPORT_H
