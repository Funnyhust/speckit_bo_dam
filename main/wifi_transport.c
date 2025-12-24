#include "wifi_transport.h"
#include "app_config.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "WIFI_TRANSPORT";

// Broadcast MAC address
static uint8_t broadcast_mac[6] = ESP_NOW_BROADCAST_MAC;

// Sequence number counter
static uint16_t seq_num_counter = 0;

// Phase 3: StreamBuffer handles for audio streaming
static StreamBufferHandle_t tx_audio_buffer = NULL; // Mic → WiFi
static StreamBufferHandle_t rx_audio_buffer = NULL; // WiFi → Speaker

// ESP-NOW receive callback
static void esp_now_recv_cb(const esp_now_recv_info_t *recv_info,
                            const uint8_t *data, int len) {
  if (len != sizeof(audio_packet_t)) {
    ESP_LOGW(TAG, "Received packet with wrong size: %d bytes (expected %d)",
             len, sizeof(audio_packet_t));
    return;
  }

  const audio_packet_t *packet = (const audio_packet_t *)data;

  // Validate magic word
  if (packet->magic_word != AUDIO_PACKET_MAGIC) {
    ESP_LOGW(TAG, "Invalid magic word: 0x%04X (expected 0x%04X)",
             packet->magic_word, AUDIO_PACKET_MAGIC);
    return;
  }

  // Log received packet info
  if (packet->seq_num % 1000 == 0) {
    ESP_LOGI(TAG, "RX: SeqNum=%u, RSSI=%d", packet->seq_num,
             recv_info->rx_ctrl->rssi);
  }

  // Phase 3: Push audio payload to RX buffer for playback
  if (rx_audio_buffer != NULL) {
    xStreamBufferSend(rx_audio_buffer, packet->payload, 240, 0);
  }
}

// ESP-NOW send callback
static void esp_now_send_cb(const esp_now_send_info_t *tx_info,
                            esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    ESP_LOGD(TAG, "TX Success to " MACSTR, MAC2STR(tx_info->des_addr));
  } else {
    ESP_LOGW(TAG, "TX Failed to " MACSTR, MAC2STR(tx_info->des_addr));
  }
}

esp_err_t wifi_transport_init(void) {
  ESP_LOGI(TAG, "Initializing WiFi and ESP-NOW...");

  // Initialize NVS (required for WiFi)
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Initialize TCP/IP stack
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // Initialize WiFi in Station mode
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "WiFi initialized in Station mode");

  // Initialize ESP-NOW
  ESP_ERROR_CHECK(esp_now_init());
  ESP_LOGI(TAG, "ESP-NOW initialized");

  // Register callbacks
  ESP_ERROR_CHECK(esp_now_register_send_cb(esp_now_send_cb));
  ESP_ERROR_CHECK(esp_now_register_recv_cb(esp_now_recv_cb));

  // Add broadcast peer
  esp_now_peer_info_t peer_info = {0};
  memcpy(peer_info.peer_addr, broadcast_mac, 6);
  peer_info.channel = 0; // Use current channel
  peer_info.ifidx = WIFI_IF_STA;
  peer_info.encrypt = false; // No encryption for broadcast

  ret = esp_now_add_peer(&peer_info);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add broadcast peer: %s", esp_err_to_name(ret));
    return ret;
  }

  ESP_LOGI(TAG, "Broadcast peer added: " MACSTR, MAC2STR(broadcast_mac));
  ESP_LOGI(TAG, "WiFi Transport initialized successfully");

  return ESP_OK;
}

esp_err_t wifi_transport_send(const audio_packet_t *packet) {
  if (packet == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t ret = esp_now_send(broadcast_mac, (const uint8_t *)packet,
                               sizeof(audio_packet_t));
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "ESP-NOW send failed: %s", esp_err_to_name(ret));
  }

  return ret;
}

uint16_t wifi_transport_get_seq_num(void) { return seq_num_counter++; }

// Phase 3: Buffer management functions
void wifi_transport_set_tx_buffer(void *buffer) {
  tx_audio_buffer = (StreamBufferHandle_t)buffer;
  ESP_LOGI(TAG, "TX audio buffer set");
}

void wifi_transport_set_rx_buffer(void *buffer) {
  rx_audio_buffer = (StreamBufferHandle_t)buffer;
  ESP_LOGI(TAG, "RX audio buffer set");
}

// Phase 3: WiFi TX Task
void wifi_tx_task(void *pvParameters) {
  ESP_LOGI(TAG, "WiFi TX Task started");

  if (tx_audio_buffer == NULL) {
    ESP_LOGE(TAG, "TX buffer not set! Task exiting.");
    vTaskDelete(NULL);
    return;
  }

  audio_packet_t packet;
  packet.magic_word = AUDIO_PACKET_MAGIC;

  uint32_t packet_count = 0;
  uint32_t error_count = 0;

  while (1) {
    // Wait for EXACTLY 240 bytes (1 full packet) - blocking
    // This naturally rate-limits to audio capture speed
    size_t received = xStreamBufferReceive(tx_audio_buffer, packet.payload, 240,
                                           portMAX_DELAY);

    if (received >= 240) {

      // Assign sequence number
      packet.seq_num = wifi_transport_get_seq_num();

      // Send via ESP-NOW
      esp_err_t ret = wifi_transport_send(&packet);

      if (ret == ESP_OK) {
        packet_count++;
        if (packet_count % 500 == 0) { // Log every 500 packets (~3.75s)
          ESP_LOGI(TAG, "Sent %lu packets, Errors: %lu", packet_count,
                   error_count);
        }
      } else {
        // If send fails, just count error and continue
        // Don't retry to avoid further congestion
        error_count++;
        if (error_count % 100 == 0) {
          ESP_LOGW(TAG, "TX errors: %lu (queue full)", error_count);
        }
      }

      // CRITICAL: Add delay to let ESP-NOW queue drain
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
}
