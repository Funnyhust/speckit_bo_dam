# 📊 Sơ Đồ Khối Hệ Thống

## 1. Sơ Đồ Tổng Quan Hệ Thống

```mermaid
graph TB
    subgraph "Device A - Transmitter"
        A_BTN[PTT Button<br/>GPIO 4]
        A_MIC[INMP441<br/>Microphone]
        A_LED[Status LED<br/>GPIO 2]
        
        subgraph "ESP32-A"
            A_I2S_RX[I2S RX<br/>DMA Buffers]
            A_CAP[Capture Task]
            A_TX_BUF[TX Buffer<br/>4KB]
            A_WIFI_TX[WiFi TX Task]
            A_ESP_NOW[ESP-NOW<br/>Protocol]
        end
    end
    
    subgraph "Wireless Medium"
        AIR[802.11 Wi-Fi<br/>2.4 GHz]
    end
    
    subgraph "Device B - Receiver"
        subgraph "ESP32-B"
            B_ESP_NOW[ESP-NOW<br/>Protocol]
            B_RX_BUF[RX Buffer<br/>4KB]
            B_PLAY[Playback Task]
            B_I2S_TX[I2S TX<br/>DMA Buffers]
        end
        
        B_SPK[MAX98357A<br/>Amplifier]
        B_LED[Status LED<br/>GPIO 2]
    end
    
    A_BTN -->|Press| A_LED
    A_MIC -->|Analog Audio| A_I2S_RX
    A_I2S_RX -->|512B/64ms| A_CAP
    A_CAP -->|Stream| A_TX_BUF
    A_TX_BUF -->|240B/15ms| A_WIFI_TX
    A_WIFI_TX -->|244B Packets| A_ESP_NOW
    
    A_ESP_NOW -->|Broadcast| AIR
    AIR -->|Receive| B_ESP_NOW
    
    B_ESP_NOW -->|240B Payload| B_RX_BUF
    B_RX_BUF -->|Pre-buffer 10 pkts| B_PLAY
    B_PLAY -->|240B/15ms| B_I2S_TX
    B_I2S_TX -->|I2S Data| B_SPK
    B_SPK -->|Sound| B_LED
    
    style A_ESP_NOW fill:#ff6b6b
    style B_ESP_NOW fill:#ff6b6b
    style AIR fill:#ffd93d
    style A_LED fill:#42b983
    style B_LED fill:#42b983
```

---

## 2. Sơ Đồ Kiến Trúc Phân Tầng

```mermaid
graph TB
    subgraph "Layer 4: Hardware"
        HW1[PTT Button<br/>GPIO 4<br/>Active Low]
        HW2[Status LED<br/>GPIO 2<br/>Active High]
        HW3[INMP441 Mic<br/>I2S Interface]
        HW4[MAX98357A Amp<br/>I2S Interface]
    end
    
    subgraph "Layer 3: Driver Layer"
        DRV1[audio_driver.c]
        DRV2[I2S Driver<br/>ESP-IDF]
        DRV3[GPIO Driver<br/>ESP-IDF]
        DRV4[Software Gain<br/>6dB default]
    end
    
    subgraph "Layer 2: Application Layer"
        APP1[main.c<br/>Entry Point]
        APP2[Audio Capture Task<br/>Priority 5]
        APP3[Audio Playback Task<br/>Priority 5]
        APP4[PTT Control Loop<br/>Main Thread]
        APP5[TX StreamBuffer<br/>4KB]
        APP6[RX StreamBuffer<br/>4KB]
    end
    
    subgraph "Layer 1: Transport Layer"
        TRN1[wifi_transport.c]
        TRN2[WiFi TX Task<br/>Priority 5]
        TRN3[ESP-NOW Driver<br/>ESP-IDF]
        TRN4[WiFi Driver<br/>STA Mode]
    end
    
    HW1 --> DRV3
    HW2 --> DRV3
    HW3 --> DRV2
    HW4 --> DRV2
    
    DRV3 --> APP4
    DRV2 --> DRV1
    DRV1 --> APP2
    DRV1 --> APP3
    DRV4 --> APP3
    
    APP4 -.controls.-> APP2
    APP4 -.controls.-> APP5
    APP4 -.controls.-> APP6
    
    APP2 --> APP5
    APP5 --> TRN2
    TRN2 --> TRN3
    TRN3 --> TRN4
    
    TRN4 -.wireless.-> TRN4
    
    TRN3 --> APP6
    APP6 --> APP3
    
    style APP1 fill:#42b983
    style TRN3 fill:#ff6b6b
    style DRV1 fill:#4ecdc4
    style APP4 fill:#ffd93d
```

---

## 3. Sơ Đồ Data Flow - TX Path

```mermaid
flowchart LR
    subgraph "Hardware Input"
        MIC[🎤 INMP441<br/>Microphone]
    end
    
    subgraph "I2S Layer"
        I2S_RX[I2S RX Channel<br/>8kHz, 16-bit, Mono]
        DMA_RX[DMA Buffer<br/>16 × 256 samples]
    end
    
    subgraph "Application Layer"
        CAP_TASK[Capture Task<br/>512 bytes/read]
        TX_BUF[TX StreamBuffer<br/>4096 bytes]
    end
    
    subgraph "Transport Layer"
        WIFI_TASK[WiFi TX Task<br/>240 bytes/packet]
        PACKET[Packet Creation<br/>magic + seq + payload]
        ESP_NOW[ESP-NOW Send<br/>244 bytes]
    end
    
    subgraph "Wireless"
        AIR[📡 Broadcast<br/>FF:FF:FF:FF:FF:FF]
    end
    
    MIC -->|Analog| I2S_RX
    I2S_RX -->|DMA| DMA_RX
    DMA_RX -->|512B/64ms| CAP_TASK
    CAP_TASK -->|xStreamBufferSend| TX_BUF
    TX_BUF -->|xStreamBufferReceive| WIFI_TASK
    WIFI_TASK -->|Serialize| PACKET
    PACKET -->|esp_now_send| ESP_NOW
    ESP_NOW -->|Wireless| AIR
    
    style MIC fill:#42b983
    style AIR fill:#ff6b6b
    style TX_BUF fill:#4ecdc4
```

---

## 4. Sơ Đồ Data Flow - RX Path

```mermaid
flowchart LR
    subgraph "Wireless"
        AIR[📡 Broadcast<br/>Received]
    end
    
    subgraph "Transport Layer"
        ESP_NOW[ESP-NOW Callback<br/>Interrupt Context]
        VALIDATE[Validate<br/>magic + size]
    end
    
    subgraph "Application Layer"
        RX_BUF[RX StreamBuffer<br/>4096 bytes]
        PRE_BUF[Pre-buffering<br/>Wait 10 packets]
        PLAY_TASK[Playback Task<br/>240 bytes/read]
    end
    
    subgraph "I2S Layer"
        GAIN[Software Gain<br/>6dB amplification]
        DMA_TX[DMA Buffer<br/>16 × 256 samples]
        I2S_TX[I2S TX Channel<br/>8kHz, 16-bit, Mono]
    end
    
    subgraph "Hardware Output"
        SPK[🔊 MAX98357A<br/>Amplifier]
    end
    
    AIR -->|Packet| ESP_NOW
    ESP_NOW -->|Validate| VALIDATE
    VALIDATE -->|240B payload| RX_BUF
    RX_BUF -->|Buffer| PRE_BUF
    PRE_BUF -->|xStreamBufferReceive| PLAY_TASK
    PLAY_TASK -->|Apply gain| GAIN
    GAIN -->|240B/15ms| DMA_TX
    DMA_TX -->|DMA| I2S_TX
    I2S_TX -->|I2S Data| SPK
    
    style AIR fill:#ff6b6b
    style SPK fill:#42b983
    style RX_BUF fill:#4ecdc4
    style GAIN fill:#ffd93d
```

---

## 5. Sơ Đồ PTT State Machine

```mermaid
stateDiagram-v2
    [*] --> RX_Mode: Power On
    
    state RX_Mode {
        [*] --> Idle
        Idle --> Listening: Packet Received
        Listening --> Buffering: Valid Packet
        Buffering --> Playing: 10 Packets Ready
        Playing --> Listening: Continue
        Playing --> Idle: Buffer Empty
    }
    
    state TX_Mode {
        [*] --> Capturing
        Capturing --> Buffering: Audio Data
        Buffering --> Transmitting: 240B Ready
        Transmitting --> Buffering: Packet Sent
    }
    
    RX_Mode --> TX_Mode: PTT Pressed
    TX_Mode --> RX_Mode: PTT Released
    
    note right of TX_Mode
        Actions on Enter:
        - LED ON
        - ptt_pressed = true
        - Reset RX buffer
        - rx_packet_count = 0
    end note
    
    note right of RX_Mode
        Actions on Enter:
        - LED OFF
        - ptt_pressed = false
        - Reset TX buffer
    end note
```

---

## 6. Sơ Đồ Packet Structure

```mermaid
graph LR
    subgraph "audio_packet_t - 244 bytes"
        MAGIC[magic_word<br/>2 bytes<br/>0xA55A]
        SEQ[seq_num<br/>2 bytes<br/>0-65535]
        PAYLOAD[payload<br/>240 bytes<br/>120 samples × 2B]
    end
    
    MAGIC --> SEQ
    SEQ --> PAYLOAD
    
    style MAGIC fill:#ffd93d
    style SEQ fill:#4ecdc4
    style PAYLOAD fill:#42b983
```

**Packet Details:**
- **Total Size:** 244 bytes (fits ESP-NOW 250B limit)
- **Overhead:** 4 bytes (1.6%)
- **Payload Efficiency:** 98.4%
- **Duration:** 120 samples / 8000 Hz = 15ms
- **Rate:** ~67 packets/second

---

## 7. Sơ Đồ GPIO Pinout

```mermaid
graph TB
    subgraph "ESP32-WROOM-32"
        GPIO2[GPIO 2<br/>Status LED]
        GPIO4[GPIO 4<br/>PTT Button]
        GPIO14[GPIO 14<br/>I2S BCLK]
        GPIO15[GPIO 15<br/>I2S LRCK]
        GPIO22[GPIO 22<br/>I2S DO]
        GPIO32[GPIO 32<br/>I2S DI]
        GND1[GND]
        VCC1[3.3V]
    end
    
    subgraph "INMP441 Microphone"
        MIC_SCK[SCK]
        MIC_WS[WS]
        MIC_SD[SD]
        MIC_L_R[L/R → GND]
        MIC_VDD[VDD → 3.3V]
        MIC_GND[GND]
    end
    
    subgraph "MAX98357A Amplifier"
        AMP_BCLK[BCLK]
        AMP_LRC[LRC]
        AMP_DIN[DIN]
        AMP_GAIN[GAIN → GND/VDD]
        AMP_VDD[VDD → 5V]
        AMP_GND[GND]
    end
    
    subgraph "PTT Button"
        BTN_1[Pin 1]
        BTN_2[Pin 2 → GND]
    end
    
    subgraph "LED"
        LED_A[Anode]
        LED_K[Cathode → GND]
        LED_R[Resistor 220Ω]
    end
    
    GPIO14 --> MIC_SCK
    GPIO14 --> AMP_BCLK
    GPIO15 --> MIC_WS
    GPIO15 --> AMP_LRC
    GPIO32 --> MIC_SD
    GPIO22 --> AMP_DIN
    
    GPIO4 --> BTN_1
    GPIO2 --> LED_R
    LED_R --> LED_A
    
    VCC1 --> MIC_VDD
    GND1 --> MIC_GND
    GND1 --> AMP_GND
    GND1 --> BTN_2
    GND1 --> LED_K
    
    style GPIO2 fill:#42b983
    style GPIO4 fill:#ffd93d
    style GPIO14 fill:#4ecdc4
    style GPIO15 fill:#4ecdc4
```

---

## 8. Sơ Đồ Timing - End-to-End Latency

```mermaid
gantt
    title Latency Budget - PTT Press to Audio Output
    dateFormat X
    axisFormat %L ms
    
    section TX Device
    I2S Capture (512B)       :a1, 0, 64
    TX Buffer Wait           :a2, 64, 10
    Packetize                :a3, 74, 1
    
    section Wireless
    ESP-NOW Send             :a4, 75, 5
    Air Propagation          :a5, 80, 5
    
    section RX Device
    ESP-NOW Receive          :a6, 85, 1
    RX Buffer (Pre-buffer)   :a7, 86, 150
    Playback                 :a8, 236, 15
    
    section Total
    First Audio Output       :milestone, 251, 0
```

**Latency Breakdown:**
| Stage | Duration | Percentage |
|-------|----------|------------|
| TX Capture | 64ms | 25.5% |
| TX Buffering | 10ms | 4.0% |
| Wireless | 10ms | 4.0% |
| **RX Pre-buffering** | **150ms** | **59.8%** |
| RX Playback | 15ms | 6.0% |
| **Total** | **251ms** | **100%** |

> ⚠️ **Critical:** Pre-buffering chiếm 60% latency. Giảm xuống 3 packets sẽ giảm latency xuống ~100ms.

---

## 9. Sơ Đồ Memory Layout

```mermaid
graph TB
    subgraph "Heap Memory - ~20 KB"
        TX_BUF[TX StreamBuffer<br/>4096 bytes]
        RX_BUF[RX StreamBuffer<br/>4096 bytes]
        CAP_STACK[Capture Task Stack<br/>4096 bytes]
        PLAY_STACK[Playback Task Stack<br/>4096 bytes]
        WIFI_STACK[WiFi TX Task Stack<br/>4096 bytes]
    end
    
    subgraph "DMA Memory - ~16 KB"
        I2S_RX_DMA[I2S RX DMA<br/>16 buffers × 512B = 8KB]
        I2S_TX_DMA[I2S TX DMA<br/>16 buffers × 512B = 8KB]
    end
    
    subgraph "BSS Segment - ~100 bytes"
        HANDLES[I2S Handles<br/>~32 bytes]
        FLAGS[Volatile Flags<br/>~4 bytes]
        COUNTERS[Counters<br/>~8 bytes]
    end
    
    style TX_BUF fill:#42b983
    style RX_BUF fill:#42b983
    style I2S_RX_DMA fill:#4ecdc4
    style I2S_TX_DMA fill:#4ecdc4
```

**Total Memory Usage:** ~36 KB
- Heap: ~20 KB
- DMA: ~16 KB
- Static: ~100 bytes

---

## 10. Sơ Đồ Task Scheduling

```mermaid
gantt
    title FreeRTOS Task Timeline (100ms window)
    dateFormat X
    axisFormat %L ms
    
    section Main Loop
    PTT Monitoring           :a1, 0, 5
    Idle                     :a2, 5, 45
    PTT Monitoring           :a3, 50, 5
    Idle                     :a4, 55, 45
    
    section Capture Task
    I2S Read (Blocked)       :b1, 0, 64
    Buffer Push              :b2, 64, 1
    I2S Read (Blocked)       :b3, 65, 35
    
    section WiFi TX Task
    Buffer Pop               :c1, 10, 1
    Send Packet              :c2, 11, 5
    Delay                    :c3, 16, 6
    Buffer Pop               :c4, 22, 1
    Send Packet              :c5, 23, 5
    Delay                    :c6, 28, 6
    
    section Playback Task
    Buffer Pop (Blocked)     :d1, 0, 15
    I2S Write                :d2, 15, 1
    Buffer Pop (Blocked)     :d3, 16, 15
    I2S Write                :d4, 31, 1
```

---

**Xem thêm:**
- [System Overview →](../layers/overview.md)
- [PTT Transmission Flow →](../workflows/ptt-transmission.md)
