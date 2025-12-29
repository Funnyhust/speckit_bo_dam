# 🚀 Initialization Sequence

> **Workflow:** Power-on → System ready for PTT operation

---

## Complete Initialization Flow

```mermaid
sequenceDiagram
    participant BOOT as Bootloader
    participant APP as app_main
    participant GPIO as GPIO Init
    participant WIFI as WiFi Transport
    participant AUDIO as Audio Driver
    participant TASKS as FreeRTOS Tasks
    
    BOOT->>APP: Entry point
    
    APP->>APP: ESP_LOGI: Phase 3 Audio Streaming
    
    APP->>GPIO: gpio_init
    GPIO->>GPIO: Configure LED (GPIO 2)
    GPIO->>GPIO: Configure PTT (GPIO 4, Pull-up)
    GPIO-->>APP: GPIO initialized
    
    APP->>APP: xStreamBufferCreate TX (4KB)
    APP->>APP: xStreamBufferCreate RX (4KB)
    
    APP->>WIFI: wifi_transport_init
    WIFI->>WIFI: nvs_flash_init
    WIFI->>WIFI: esp_wifi_init (STA mode)
    WIFI->>WIFI: esp_now_init
    WIFI->>WIFI: Register callbacks
    WIFI->>WIFI: Add broadcast peer
    WIFI-->>APP: ESP_OK
    
    APP->>WIFI: wifi_transport_set_tx_buffer
    APP->>WIFI: wifi_transport_set_rx_buffer
    
    APP->>TASKS: xTaskCreate audio_capture_task
    TASKS->>AUDIO: audio_driver_init
    AUDIO->>AUDIO: i2s_new_channel (TX + RX)
    AUDIO->>AUDIO: i2s_channel_enable
    AUDIO-->>TASKS: I2S ready
    
    APP->>TASKS: xTaskCreate audio_playback_task
    APP->>TASKS: xTaskCreate wifi_tx_task
    
    APP->>APP: Enter main loop (PTT monitoring)
    
    Note over APP: System ready for operation
```

---

## Initialization Checklist

### Phase 1: GPIO Setup (main.c:24-36)
- ✅ LED configured as output
- ✅ PTT configured as input with pull-up
- ✅ Initial LED state: OFF

### Phase 2: Buffer Creation (main.c:112-124)
- ✅ TX StreamBuffer: 4KB
- ✅ RX StreamBuffer: 4KB
- ✅ Trigger level: 240 bytes

### Phase 3: WiFi & ESP-NOW (wifi_transport.c:63-113)
- ✅ NVS flash initialized
- ✅ WiFi STA mode started
- ✅ ESP-NOW initialized
- ✅ Broadcast peer added

### Phase 4: Audio Driver (audio_driver.c:23-88)
- ✅ I2S channels created
- ✅ TX/RX configured (8kHz, 16-bit, Mono)
- ✅ DMA buffers allocated (16 × 256 samples)
- ✅ Channels enabled

### Phase 5: Task Creation (main.c:134-140)
- ✅ audio_capture_task (Priority 5)
- ✅ audio_playback_task (Priority 5)
- ✅ wifi_tx_task (Priority 5)

---

## Boot Time Estimate

| Stage | Duration | Notes |
|-------|----------|-------|
| Bootloader | ~100ms | ESP32 ROM |
| app_main start | ~50ms | FreeRTOS init |
| GPIO init | < 1ms | Fast |
| WiFi init | ~200ms | Driver load |
| ESP-NOW init | ~50ms | Protocol stack |
| I2S init | ~10ms | DMA setup |
| Task creation | ~5ms | Minimal |
| **Total** | **~416ms** | Ready to use |

---

## Related Documents
- [System Overview →](../layers/overview.md)
