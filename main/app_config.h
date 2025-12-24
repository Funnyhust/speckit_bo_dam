#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

// Audio Settings
#define SAMPLE_Rate (16000)
#define SAMPLE_BITS (16)
#define I2S_DMA_BUF_COUNT (4)
#define I2S_DMA_BUF_LEN (120) // Samples per buffer (Mono) -> 240 Bytes

// ESP-NOW Protocol defined in subsequent tasks

// ============================================================================
// Software Gain Configuration (Tăng âm lượng loa)
// ============================================================================
// Để BẬT software gain: Đổi 0 thành 1
// Để TĂNG âm lượng: Tăng giá trị SOFTWARE_GAIN_DB
#define SOFTWARE_GAIN_ENABLE 0 // 0 = TẮT, 1 = BẬT
#define SOFTWARE_GAIN_DB 6.0f  // 0=không đổi, 6=2x, 12=4x, 18=8x

// Lưu ý:
// - Gain quá cao → méo tiếng
// - Nên dùng Hardware gain (MAX98357A jumper) trước
// - Chỉ dùng Software gain nếu Hardware gain chưa đủ

// ============================================================================
// Phase 3: Audio Streaming - RingBuffer Configuration
// ============================================================================
// Buffer size for audio streaming (TX and RX)
// 4KB = ~125ms of audio at 16kHz, 16-bit mono
#define AUDIO_BUFFER_SIZE 4096 // bytes

// ============================================================================
// ESP-NOW Protocol - Audio Packet Structure
// ============================================================================
// Packet structure for Over-The-Air transmission via ESP-NOW
// Total size: 244 bytes (fits within ESP-NOW 250 byte limit)

typedef struct __attribute__((packed)) {
  uint16_t magic_word; // 0xA55A - Sync word to filter noise
  uint16_t
      seq_num; // Rolling sequence number (0-65535) for ordering/loss detection
  int16_t
      payload[120]; // PCM Audio Data: 120 samples * 2 bytes = 240 bytes
                    // Time duration: 120 samples / 16000 Hz = 7.5ms per packet
} audio_packet_t;

// Magic word constant
#define AUDIO_PACKET_MAGIC 0xA55A

// Broadcast MAC address (send to all devices)
#define ESP_NOW_BROADCAST_MAC {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

#endif // APP_CONFIG_H
