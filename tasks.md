# Tasks: ESP32 ESP-NOW Walkie-Talkie

## Phase 1: Foundation & Audio Loopback Test
**Goal:** Verify hardware connections and audio drivers without RF complexity.
- [x] T001 [P] Initialize Project Skeleton with CMake and ESP-IDF (C11 standard) in `CMakeLists.txt` <!-- id: 10 -->
- [x] T002 Implement GPIO and LED/Button drivers in `main/board_pinout.h` and `main/main.c` <!-- id: 11 -->
- [x] T003 Implement I2S Driver (Master, 16kHz, 16-bit, DMA) in `main/audio_driver.c` <!-- id: 12 -->
- [x] T004 Implement Loopback Logic (Read Mic -> Buffer -> Write Amp) in `main/main.c` <!-- id: 13 -->
- [x] T005 Verify Audio: Speak into Mic, hear delay/echo on Speaker (Manual Test) <!-- id: 14 -->

## Phase 2: ESP-NOW Communication
**Goal:** Establish reliable wireless link between two units.
- [x] T006 Initialize WiFi in Station Mode & ESP-NOW in `main/wifi_transport.c` <!-- id: 15 -->
- [x] T007 Define `audio_packet_t` struct (Header + SeqNum + Payload) in `main/app_config.h` <!-- id: 16 -->
- [x] T008 Implement TX Task: Send dummy packets with counter in `main/wifi_transport.c` <!-- id: 17 -->
- [x] T009 Implement RX Callback: Receive packets & log Sequence Numbers in `main/wifi_transport.c` <!-- id: 18 -->
- [ ] T010 Verify RF Link: Check RSSI and Packet Loss Rate (Manual Test) <!-- id: 19 -->

## Phase 3: Audio Transport Integration
**Goal:** Combine Audio and WiFi for One-way streaming.
- [x] T011 Implement RingBuffer for Audio-WiFi decoupling in `main/ring_buffer_lib.c` <!-- id: 20 -->
- [x] T012 Connect Audio Task to RingBuffer (Push Mic Data) in `main/main.c` <!-- id: 21 -->
- [x] T013 Connect WiFi Task to RingBuffer (Pop Data -> Packetize -> Send) in `main/wifi_transport.c` <!-- id: 22 -->
- [x] T014 Implement RX Assembly (Receive -> Jitter Buffer/Ring Buffer -> I2S Write) in `main/wifi_transport.c` <!-- id: 23 -->

## Phase 4: PTT Logic & Optimization
**Goal:** Full Walkie-Talkie functionality.
- [ ] T015 Implement State Machine (RX_IDLE <-> TX_ACTIVE) using PTT Button in `main/main.c` <!-- id: 24 -->
- [ ] T016 Implement Mute Logic: Disable I2S Amp during TX to prevent feedback in `main/audio_driver.c` <!-- id: 25 -->
- [ ] T017 Optimize Latency: Tune RingBuffer size vs Dropouts <!-- id: 26 -->
- [ ] T018 Verify Full Duplex (Half-Duplex logic): Test conversation between 2 units <!-- id: 27 -->
