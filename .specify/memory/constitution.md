# ESP32 ESP-NOW Walkie-Talkie Constitution

## Tech Stack (NON-NEGOTIABLE)

### I. Hardware Platform
- **MCU:** ESP32-WROOM-32
- **Audio Input:** INMP441 (I2S MEMS Microphone - I2S Master Mode)
- **Audio Output:** MAX98357A (I2S Class-D Amplifier - I2S Master Mode)
- **Communication Protocol:** ESP-NOW (Broadcast Mode)
  - **Latency Requirement:** < 100ms end-to-end
  - **Rationale:** WiFi AP/STA mode introduces unacceptable latency for real-time voice

### II. Software Stack
- **SDK:** ESP-IDF v5.1 or later
- **Build System:** CMake (ESP-IDF native)
- **Programming Language:** C (C11 Standard)
- **RTOS:** FreeRTOS (built into ESP-IDF)

## Non-Negotiable Engineering Standards

### III. Audio Pipeline Architecture (CRITICAL)
> [!CAUTION]
> Violation of these rules will result in audio glitches, buffer overruns, or system instability.

- **DMA Mandatory:** All I2S audio transfers MUST use DMA. CPU polling is strictly forbidden.
- **Buffer Management:** 
  - Use double-buffering or ring buffers for continuous audio streaming
  - Buffer sizes must be power-of-2 for optimal DMA performance
  - Minimum buffer size: 512 samples (configurable based on latency requirements)
- **Sample Rate:** 16 kHz (configurable, but must match on both TX/RX sides)
- **Bit Depth:** 16-bit signed PCM

### IV. Concurrency Model (MANDATORY)
- **Task Separation:**
  - `audio_rx_task`: I2S microphone DMA → ESP-NOW transmission
  - `audio_tx_task`: ESP-NOW reception → I2S speaker DMA
  - `wifi_task`: ESP-NOW initialization, peer management, network events
- **Inter-Task Communication:**
  - Use FreeRTOS Queues for audio data transfer between tasks
  - Use Event Groups for synchronization (e.g., "ready to transmit", "peer connected")
- **Priority Assignment:**
  - Audio tasks: High priority (configurable, typically `configMAX_PRIORITIES - 2`)
  - WiFi task: Medium priority

### V. GPIO Management (STRICT)
- **Centralized Pin Definitions:** All GPIO pin numbers MUST be defined in `board_pinout.h`
- **No Magic Numbers:** Never hard-code pin numbers in `.c` files
- **Example Structure:**
  ```c
  // board_pinout.h
  #define I2S_MIC_SCK_PIN    GPIO_NUM_26
  #define I2S_MIC_WS_PIN     GPIO_NUM_25
  #define I2S_MIC_SD_PIN     GPIO_NUM_33
  ```

### VI. Memory Management (CRITICAL)
> [!WARNING]
> Dynamic memory allocation in real-time audio paths will cause jitter and dropouts.

- **Static Allocation Preferred:** Use static buffers for audio data whenever possible
- **Heap Allocation Rules:**
  - Allowed: One-time initialization (e.g., `malloc` in `app_main`)
  - Forbidden: `malloc`/`free` in audio processing loops or ISRs
- **DMA Buffers:** Must be allocated in DMA-capable memory using `heap_caps_malloc(size, MALLOC_CAP_DMA)`

### VII. Error Handling (MANDATORY)
- **ESP-IDF Return Codes:** All functions returning `esp_err_t` MUST be checked with `ESP_ERROR_CHECK()` or explicit error handling
- **Example:**
  ```c
  esp_err_t ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  ESP_ERROR_CHECK(ret);  // System will abort on error with diagnostic message
  ```
- **Logging:** Use ESP-IDF logging macros (`ESP_LOGI`, `ESP_LOGE`, etc.) with appropriate log levels

## Code Organization

### VIII. Project Structure
```
doppler/
├── main/
│   ├── main.c                 # Entry point, task creation
│   ├── board_pinout.h         # GPIO pin definitions
│   ├── audio/
│   │   ├── i2s_mic.c/h       # Microphone driver
│   │   ├── i2s_speaker.c/h   # Speaker driver
│   │   └── audio_pipeline.c/h # Audio processing
│   ├── network/
│   │   ├── espnow_tx.c/h     # ESP-NOW transmitter
│   │   └── espnow_rx.c/h     # ESP-NOW receiver
│   └── config/
│       └── app_config.h       # Application-wide constants
├── CMakeLists.txt
└── sdkconfig                  # ESP-IDF configuration
```

### IX. Naming Conventions
- **Files:** `snake_case.c`, `snake_case.h`
- **Functions:** `module_action()` (e.g., `i2s_mic_init()`, `espnow_send_audio()`)
- **Macros/Constants:** `UPPER_SNAKE_CASE`
- **Types:** `snake_case_t` (e.g., `audio_buffer_t`)

## Performance Requirements

### X. Latency Budget
| Component | Max Latency | Notes |
|-----------|-------------|-------|
| I2S Mic Capture | 20ms | DMA buffer size dependent |
| ESP-NOW TX | 10ms | Typical 2-5ms |
| ESP-NOW RX | 10ms | Includes callback processing |
| I2S Speaker Playback | 20ms | DMA buffer size dependent |
| **Total End-to-End** | **< 100ms** | **Hard requirement** |

### XI. Audio Quality
- **Minimum SNR:** 40 dB (limited by INMP441 hardware)
- **Packet Loss Handling:** Implement silence insertion for lost packets (no crackling)
- **Jitter Buffer:** Optional 1-2 packet buffer for network jitter compensation

## Governance

### XII. Constitution Authority
- This constitution supersedes all other coding practices or preferences
- Any deviation MUST be documented with technical justification and approved
- Amendments require:
  1. Written proposal with rationale
  2. Impact analysis on existing code
  3. Migration plan (if applicable)

### XIII. Code Review Checklist
All code submissions must verify:
- [ ] No hard-coded GPIO pins (check `board_pinout.h`)
- [ ] No `malloc` in audio loops
- [ ] All `esp_err_t` returns are checked
- [ ] DMA used for all I2S transfers
- [ ] FreeRTOS tasks properly prioritized
- [ ] Logging uses ESP-IDF macros (no `printf`)

**Version**: 1.0.0 | **Ratified**: 2025-12-21 | **Last Amended**: 2025-12-21
