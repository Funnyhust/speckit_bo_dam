# Data Model: ESP32 Walkie-Talkie

## Network Entities

### Packet Structure
Used for OTA transmission via ESP-NOW.

```c
typedef struct __attribute__((packed)) {
    uint16_t magic_word;    // 0xA55A
    uint16_t seq_num;       // Packet Sequence for ordering
    int16_t  payload[120];  // 120 Samples * 2 bytes = 240 bytes
} audio_packet_t;
```

**Total Size:** 244 Bytes.

## Internal Entities

### Audio Config
Global configuration struct.

```c
typedef struct {
    int sample_rate;        // 16000
    int dma_buf_count;      // 4
    int dma_buf_len;        // 120 samples (240 bytes)
    i2s_port_t port_num;    // I2S_NUM_0
} audio_config_t;
```

### App State
State machine for PTT logic.

```c
typedef enum {
    STATE_RX_IDLE,      // Listening
    STATE_RX_PLAYING,   // Receiving Audio
    STATE_TX_ACTIVE     // Transmitting
} app_state_t;
```
