# 🎧 Audio Reception Flow

> **Workflow:** ESP-NOW packet received → Audio played through speaker

---

## Complete Reception Sequence

```mermaid
sequenceDiagram
    participant AIR as Wireless
    participant ESP_NOW as ESP-NOW
    participant RX_CB as esp_now_recv_cb
    participant RX_BUF as RX StreamBuffer
    participant PLAY as Playback Task
    participant GAIN as Software Gain
    participant I2S as I2S TX DMA
    participant SPK as MAX98357A
    actor User as User B
    
    Note over AIR,User: PHASE 1: PACKET RECEPTION
    
    AIR->>ESP_NOW: 802.11 Broadcast packet
    ESP_NOW->>RX_CB: Interrupt callback
    
    RX_CB->>RX_CB: Check len == 244?
    RX_CB->>RX_CB: Check magic == 0xA55A?
    
    alt Valid Packet
        RX_CB->>RX_BUF: xStreamBufferSend<br/>240 bytes, timeout=0
        Note over RX_BUF: Packet count++
    else Invalid
        RX_CB->>RX_CB: ESP_LOGW: Drop packet
    end
    
    Note over RX_BUF,PLAY: PHASE 2: PRE-BUFFERING
    
    loop Until 10 packets received
        PLAY->>RX_BUF: xStreamBufferReceive<br/>240 bytes, BLOCK
        RX_BUF-->>PLAY: 240 bytes
        PLAY->>PLAY: Check rx_packet_count >= 10?
        alt Not ready
            PLAY->>PLAY: Discard (don't play yet)
        end
    end
    
    Note over PLAY,SPK: PHASE 3: PLAYBACK
    
    loop Continuous playback
        PLAY->>RX_BUF: xStreamBufferReceive 240B
        RX_BUF-->>PLAY: 240 bytes
        
        PLAY->>GAIN: apply_gain(samples, 6dB)
        GAIN-->>PLAY: Amplified samples
        
        PLAY->>I2S: audio_driver_write(240B)
        I2S->>SPK: DMA transfer
        SPK-->>User: Audio output
    end
```

---

## Pre-Buffering Logic

### Why Pre-Buffer?
```mermaid
graph TB
    PROBLEM[Problem: Jitter]
    JITTER[Packets arrive at irregular intervals]
    UNDERRUN[Speaker buffer underrun]
    DROPOUT[Audio dropouts]
    
    SOLUTION[Solution: Pre-buffering]
    WAIT[Wait for 10 packets ~2.4KB]
    SMOOTH[Smooth playback]
    
    PROBLEM --> JITTER
    JITTER --> UNDERRUN
    UNDERRUN --> DROPOUT
    
    SOLUTION --> WAIT
    WAIT --> SMOOTH
    
    style PROBLEM fill:#ff6b6b
    style SOLUTION fill:#42b983
```

### Implementation (main.c:92-100)
```c
if (received == 240) {
  // Phase 4: Only play after receiving at least 10 packets
  if (rx_packet_count >= MIN_PACKETS_BEFORE_PLAY) {
    // Write to Speaker
    audio_driver_write(buffer, 240, &bytes_written);
  }
  // Increment packet count (saturate at MIN_PACKETS_BEFORE_PLAY + 1)
  if (rx_packet_count < MIN_PACKETS_BEFORE_PLAY + 1) {
    rx_packet_count++;
  }
}
```

**MIN_PACKETS_BEFORE_PLAY = 10** (line 22)  
**Buffering time:** 10 packets × 15ms = **150ms**

---

## Packet Validation Flow

```mermaid
flowchart TD
    START([esp_now_recv_cb])
    
    START --> SIZE{len == 244?}
    SIZE -->|No| WARN_SIZE[ESP_LOGW: Wrong size]
    SIZE -->|Yes| CAST[Cast to audio_packet_t]
    
    WARN_SIZE --> RETURN[Return]
    
    CAST --> MAGIC{magic_word<br/>== 0xA55A?}
    MAGIC -->|No| WARN_MAGIC[ESP_LOGW: Invalid magic]
    MAGIC -->|Yes| LOG{seq_num % 1000 == 0?}
    
    WARN_MAGIC --> RETURN
    
    LOG -->|Yes| PRINT[ESP_LOGI: SeqNum, RSSI]
    LOG -->|No| PUSH
    
    PRINT --> PUSH{rx_audio_buffer != NULL?}
    PUSH -->|Yes| SEND[xStreamBufferSend<br/>payload, 240, timeout=0]
    PUSH -->|No| RETURN
    
    SEND --> RETURN
    
    style MAGIC fill:#ffd93d
    style SEND fill:#42b983
```

**Location:** `wifi_transport.c` lines 24-51

---

## Software Gain Application

### Gain Processing (audio_driver.c:128-133)
```c
#if SOFTWARE_GAIN_ENABLE
  // Apply software gain (cấu hình trong app_config.h)
  int16_t *samples = (int16_t *)buffer;
  size_t sample_count = len / sizeof(int16_t);
  apply_gain(samples, sample_count, SOFTWARE_GAIN_DB);
#endif
```

**Default Gain:** 6 dB = 2× amplification

---

## Timing Analysis

### Reception Timeline
```mermaid
gantt
    title Packet Reception to Playback
    dateFormat X
    axisFormat %L ms
    
    section Receive
    Wireless RX              :a1, 0, 5
    ESP-NOW Callback         :a2, 5, 1
    
    section Buffer
    Push to RX Buffer        :a3, 6, 1
    Wait for 10 packets      :a4, 7, 143
    
    section Playback
    Pop from Buffer          :a5, 150, 1
    Software Gain            :a6, 151, 1
    I2S DMA Write            :a7, 152, 15
    
    section Total
    First Audio Out          :milestone, 167, 0
```

**First packet latency:** ~167ms (includes 150ms pre-buffering)  
**Steady-state latency:** ~17ms per packet

---

## Expert Review

### ✅ Strengths
1. **Robust validation** - Magic word + size check
2. **Pre-buffering** - Prevents dropouts from jitter
3. **Software gain** - Adjustable volume
4. **Non-blocking RX** - Callback-driven

### ⚠️ Issues
1. **High pre-buffer latency** - 150ms exceeds target
2. **No jitter buffer** - Fixed 10-packet wait
3. **No packet loss concealment** - Gaps in audio
4. **No adaptive buffering** - Cannot adjust to network

### 💡 Recommendations
1. **Reduce to 3 packets** - ~45ms buffering
2. **Add adaptive buffer** - Adjust based on jitter
3. **Implement PLC** - Packet Loss Concealment
4. **Add comfort noise** - Fill silent gaps

---

## Related Documents
- [← PTT Transmission Flow](ptt-transmission.md)
- [Audio Layer →](../layers/audio.md)
