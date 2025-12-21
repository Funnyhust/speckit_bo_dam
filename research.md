# Research: ESP32 Walkie-Talkie

## Decisions & Rationale

### 1. Protocol: ESP-NOW vs UDP Broadcast
- **Decision:** **ESP-NOW**.
- **Rationale:** 
    - Connectionless: No AP association time (which takes seconds).
    - Low Overhead: Smaller headers than IP/UDP.
    - Lower Latency: Direct MAC-to-MAC communication.
- **Alternatives:** 
    - **UDP Broadcast:** Requires WiFi connection, higher latency, depends on router.
    - **BT Classic (A2DP):** High latency (>100ms), complex stack.

### 2. Audio Codec: Raw PCM vs ADPCM vs SBC
- **Decision:** **Raw PCM (16-bit, 16kHz)**.
- **Rationale:** 
    - Simplicity: No CPU overhead for compression.
    - Bandwidth: 32KBps is effectively ~256kbps. ESP-NOW theoretical max is 1-2Mbps. It fits comfortably (approx 12-25% airtime utilization).
    - Quality: Lossless (within 16kHz band).
- **Alternatives:**
    - **ADPCM (4-bit):** Reduces bandwidth by 4x. Consider **ONLY IF** ESP-NOW packet loss is high due to congestion.

### 3. Buffer Management: FreeRTOS StreamBuffer vs RingBuffer
- **Decision:** **FreeRTOS StreamBuffer**.
- **Rationale:** 
    - Built-in to ESP-IDF/FreeRTOS.
    - Thread-safe for ISR to Task (DMA -> Task) and Task to Task.
    - Optimized for stream data.

## Open Questions Resolved
- **Q:** Can ESP-NOW handle 32KBps continuous stream?
- **A:** Yes, practical throughput is often >500kbps. 256kbps is safe if RF environment isn't saturated.
