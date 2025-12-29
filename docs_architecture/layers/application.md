# 🎯 Application Layer - Task Orchestration

> **Module:** `main.c`  
> **Trách nhiệm:** Task management, PTT control, buffer coordination

---

## Architecture Overview

```mermaid
graph TB
    subgraph "main.c - Application Layer"
        MAIN[app_main<br/>Entry Point]
        PTT_LOOP[PTT Control Loop<br/>Main Thread]
        
        subgraph "FreeRTOS Tasks"
            CAP[audio_capture_task<br/>Priority 5]
            PLAY[audio_playback_task<br/>Priority 5]
            WIFI_TX[wifi_tx_task<br/>Priority 5]
        end
        
        subgraph "Shared Resources"
            TX_BUF[TX StreamBuffer<br/>4KB]
            RX_BUF[RX StreamBuffer<br/>4KB]
            PTT_FLAG[ptt_pressed<br/>volatile bool]
            RX_COUNT[rx_packet_count<br/>volatile uint8_t]
        end
    end
    
    MAIN --> PTT_LOOP
    MAIN --> CAP
    MAIN --> PLAY
    MAIN --> WIFI_TX
    
    PTT_LOOP -.controls.-> PTT_FLAG
    PTT_LOOP -.resets.-> TX_BUF
    PTT_LOOP -.resets.-> RX_BUF
    
    CAP -.reads.-> PTT_FLAG
    CAP --> TX_BUF
    
    TX_BUF --> WIFI_TX
    
    RX_BUF --> PLAY
    PLAY -.reads.-> RX_COUNT
    
    style MAIN fill:#42b983
    style PTT_FLAG fill:#ffd93d
```

---

## Task Details

### 1. audio_capture_task (Lines 39-77)

**Purpose:** Read from microphone, push to TX buffer when PTT pressed

**Priority:** 5 (Medium)  
**Stack Size:** 4096 bytes  
**Core Affinity:** Any (not pinned)

**State Machine:**
```mermaid
stateDiagram-v2
    [*] --> Init
    Init --> Idle: audio_driver_init OK
    
    Idle --> Capturing: ptt_pressed == true
    Capturing --> Idle: ptt_pressed == false
    
    state Capturing {
        [*] --> ReadMic
        ReadMic --> PushBuffer: bytes_read > 0
        PushBuffer --> ReadMic
    }
    
    state Idle {
        [*] --> Sleep
        Sleep --> Sleep: vTaskDelay 50ms
    }
```

**Key Code:**
```c
// Line 61-67
if (ptt_pressed) {
  esp_err_t ret = audio_driver_read(buffer, buffer_size, &bytes_read);
  if (ret == ESP_OK && bytes_read > 0) {
    xStreamBufferSend(tx_audio_buffer, buffer, bytes_read, 0);
  }
}
```

---

### 2. audio_playback_task (Lines 80-103)

**Purpose:** Pop from RX buffer, write to speaker (with pre-buffering)

**Priority:** 5 (Medium)  
**Stack Size:** 4096 bytes

**Flow:**
```mermaid
flowchart TD
    START([audio_playback_task])
    
    START --> LOOP{Loop Forever}
    
    LOOP --> WAIT[xStreamBufferReceive<br/>rx_audio_buffer, 240B<br/>BLOCK portMAX_DELAY]
    
    WAIT --> CHECK_SIZE{received == 240?}
    CHECK_SIZE -->|No| LOOP
    CHECK_SIZE -->|Yes| CHECK_COUNT{rx_packet_count<br/>>= 10?}
    
    CHECK_COUNT -->|Yes| WRITE[audio_driver_write<br/>buffer, 240]
    CHECK_COUNT -->|No| SKIP[Discard packet]
    
    WRITE --> INC[rx_packet_count++<br/>saturate at 11]
    SKIP --> INC
    
    INC --> LOOP
    
    style WRITE fill:#42b983
    style SKIP fill:#ff6b6b
```

---

### 3. wifi_tx_task (External)

**Purpose:** Pop from TX buffer, send via ESP-NOW

**Defined in:** `wifi_transport.c`  
**Priority:** 5 (Medium)  
**Stack Size:** 4096 bytes

See [Transport Layer](../layers/transport.md) for details.

---

## PTT Control Loop (Lines 145-173)

**Purpose:** Monitor PTT button, control LED, manage state transitions

**Polling Rate:** 50ms (20 Hz)

```mermaid
flowchart TD
    START([Main Loop])
    
    START --> READ[gpio_get_level<br/>GPIO_PTT_BUTTON]
    READ --> COMPARE[ptt_active =<br/>ptt_state == PTT_ACTIVE_LEVEL]
    
    COMPARE --> STATE_CHG{ptt_active !=<br/>last_ptt_state?}
    
    STATE_CHG -->|No| DELAY[vTaskDelay 50ms]
    STATE_CHG -->|Yes| PRESSED{ptt_active?}
    
    PRESSED -->|Yes| TX_MODE[TX Mode]
    PRESSED -->|No| RX_MODE[RX Mode]
    
    TX_MODE --> SET_PTT[ptt_pressed = true]
    SET_PTT --> LED_ON[LED ON]
    LED_ON --> RESET_RX[Reset RX buffer<br/>rx_packet_count = 0]
    RESET_RX --> UPDATE
    
    RX_MODE --> CLEAR_PTT[ptt_pressed = false]
    CLEAR_PTT --> LED_OFF[LED OFF]
    LED_OFF --> RESET_TX[Reset TX buffer]
    RESET_TX --> UPDATE
    
    UPDATE[last_ptt_state = ptt_active]
    UPDATE --> DELAY
    
    DELAY --> START
    
    style TX_MODE fill:#42b983
    style RX_MODE fill:#4ecdc4
```

---

## Shared Variables

### Volatile Flags
```c
// Line 20-21
static volatile bool ptt_pressed = false;
static volatile uint8_t rx_packet_count = 0;
```

**Why volatile?**
- Accessed from multiple tasks/contexts
- Prevents compiler optimization
- Ensures memory visibility

**Thread Safety:**
- `ptt_pressed`: Written by main loop, read by capture task
- `rx_packet_count`: Written by RX callback, read by playback task
- **No mutex needed** - Single reader/writer pattern

---

## StreamBuffer Configuration

### TX Buffer (Line 112)
```c
tx_audio_buffer = xStreamBufferCreate(AUDIO_BUFFER_SIZE, 240);
```
- **Size:** 4096 bytes
- **Trigger:** 240 bytes
- **Purpose:** Mic → WiFi

### RX Buffer (Line 119)
```c
rx_audio_buffer = xStreamBufferCreate(AUDIO_BUFFER_SIZE, 240);
```
- **Size:** 4096 bytes
- **Trigger:** 240 bytes
- **Purpose:** WiFi → Speaker

**Why 240 bytes trigger?**
- Matches packet payload size
- Atomic read/write operations
- Prevents partial packets

---

## Memory Layout

```mermaid
graph LR
    subgraph "Heap Memory"
        TX[TX StreamBuffer<br/>4096 bytes]
        RX[RX StreamBuffer<br/>4096 bytes]
        CAP_STACK[Capture Task Stack<br/>4096 bytes]
        PLAY_STACK[Playback Task Stack<br/>4096 bytes]
        WIFI_STACK[WiFi TX Task Stack<br/>4096 bytes]
    end
    
    subgraph "BSS Segment"
        PTT[ptt_pressed<br/>1 byte]
        COUNT[rx_packet_count<br/>1 byte]
        LAST[last_ptt_state<br/>1 byte]
    end
    
    style TX fill:#42b983
    style RX fill:#42b983
```

**Total Heap Usage:** ~20 KB (buffers + stacks)

---

## Expert Review

### ✅ Strengths
1. **Clean task separation** - Each task has single responsibility
2. **Lock-free communication** - StreamBuffers are thread-safe
3. **Simple state machine** - Easy to understand PTT logic
4. **Low polling overhead** - 50ms is acceptable

### ⚠️ Issues
1. **No task pinning** - Tasks may migrate between cores
2. **Fixed priorities** - All tasks at priority 5
3. **No watchdog** - Hung tasks not detected
4. **No error recovery** - Task failures not handled

### 💡 Recommendations
1. **Pin WiFi task to core 0** - WiFi driver runs on core 0
2. **Increase playback priority** - Prevent speaker underruns
3. **Add task watchdogs** - Detect and recover from hangs
4. **Implement error handling** - Restart tasks on failure
5. **Add telemetry** - Monitor task CPU usage

---

## Related Documents
- [← Transport Layer](transport.md)
- [PTT Transmission Flow →](../workflows/ptt-transmission.md)
- [Deep Dive: main.c →](../deepdive/main.md)
