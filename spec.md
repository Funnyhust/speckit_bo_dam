# Feature Specification: ESP32 ESP-NOW Walkie-Talkie

## 1. Overview
This feature implements a Walkie-Talkie system using ESP32-WROOM-32 via ESP-NOW protocol (Broadcast mode) for low-latency (<100ms) audio communication. It leverages I2S microphones (INMP441) and amplifiers (MAX98357A) with DMA transfer to ensure smooth, real-time half-duplex voice transmission.

### 1.1 Trigger
- User wants to communicate over short distances without Wi-Fi infrastructure.
- User presses the PTT (Push-to-Talk) button to transmit voice.

### 1.2 Goal
- Provide clear, low-latency audio communication between two or more ESP32 units.
- Operate reliably in connectionless mode using ESP-NOW.

## 2. Functional Requirements

### 2.1 Hardware Interface
- **FR.HW.01:** The system **MUST** initialize I2S Port 0 in Full-Duplex or Master mode.
- **FR.HW.02:** The system **MUST** use GPIO 14 (BCLK), 15 (LRCK), 32 (SD_IN), 22 (SD_OUT) for I2S communication.
- **FR.HW.03:** The system **MUST** detect PTT button press on GPIO 4 (Active Low, Internal Pull-up).
- **FR.HW.04:** The system **MUST** control the Status LED on GPIO 2.

### 2.2 Audio Processing
- **FR.AUDIO.01:** The system **MUST** capture and playback audio at 16,000 Hz sample rate.
- **FR.AUDIO.02:** The system **MUST** use 16-bit resolution for audio samples.
- **FR.AUDIO.03:** The system **MUST** handle Mono audio channel configuration.
- **FR.AUDIO.04:** The system **SHOULD** implement a basic compression algorithm if bandwidth exceeds ESP-NOW limits (optional but recommended).

### 2.3 Receive Mode (RX - Default)
- **FR.RX.01:** The system **MUST** default to RX mode on startup or when PTT is released.
- **FR.RX.02:** In RX mode, the system **MUST** continuously listen for ESP-NOW broadcast packets.
- **FR.RX.03:** Upon receiving valid audio packets, the system **MUST** decode and write data to the I2S DMA buffer for playback.
- **FR.RX.04:** The Status LED **SHOULD** remain OFF (or indicate Standby/RX status as defined).

### 2.4 Transmit Mode (TX)
- **FR.TX.01:** The system **MUST** enter TX mode immediately when PTT is pressed.
- **FR.TX.02:** In TX mode, the system **MUST** mute the Speaker output (software mute or disable I2S TX) to prevent feedback loops.
- **FR.TX.03:** The Status LED **MUST** turn ON continuously to indicate transmission.
- **FR.TX.04:** The system **MUST** read audio from the Microphone via I2S.
- **FR.TX.05:** The system **MUST** packetize audio data into chunks smaller than 250 bytes (ESP-NOW payload limit).
- **FR.TX.06:** The system **MUST** broadcast packets via ESP-NOW to the specific peer (or broadcast address).
- **FR.TX.07:** Upon releasing PTT, the system **MUST** immediately stop transmission, unmute Speaker, and return to RX mode.

## 3. Non-Functional Requirements
- **NFR.LATENCY:** End-to-end audio latency **MUST** be less than 100ms.
- **NFR.STABILITY:** The system **MUST NOT** crash or hang during rapid PTT toggling.
- **NFR.QUALITY:** Audio output **MUST** be intelligible (mean opinion score > 3.0 equivalent).
- **NFR.POWER:** The system **SHOULD** optimize power consumption in RX mode where possible (e.g., using light sleep if feasible without losing packets, though difficult with ESP-NOW).

## 4. User Scenarios

### 4.1 Successful Transmission
1. User A holds device, PTT is released (RX mode).
2. User A presses and holds PTT button.
3. User A's Status LED turns ON.
4. User A speaks into the microphone.
5. User B (in range) hears User A's voice from their speaker.
6. User A releases PTT button.
7. User A's Status LED turns OFF, device returns to listening mode.

### 4.2 Receive-Only Operation
1. User B leaves device on table (RX mode).
2. Device receives stream from User A.
3. Audio plays smoothly without stuttering.

## 5. Success Criteria
- **SC.01:** Audio can be transmitted and received between two devices with < 100ms latency.
- **SC.02:** Voice is clearly audible and recognizable.
- **SC.03:** PTT operation is responsive (LED reacts instantly).
- **SC.04:** No audio feedback (howling) occurs when transmitting.

## 6. Assumptions & Constraints
- **Assump.01:** Devices are within effective Wi-Fi range.
- **Const.01:** ESP-NOW payload is limited to 250 bytes, requiring efficient packetization.
- **Const.02:** ESP32 WROOM-32 single-core or dual-core usage must respect FreeRTOS task pinning for Wi-Fi stability.

## 7. Phase 4 – PTT & Buffer Logic

- Khi không nhấn nút PTT, RX StreamBuffer phải rỗng.
- Khi thiết bị A nhấn PTT, thiết bị B chỉ bắt đầu phát sau khi RX buffer đã nhận đủ 3 packet (≈ 720 B).
- Tăng số DMA buffer cho speaker lên 8.
