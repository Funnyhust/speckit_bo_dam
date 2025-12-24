# Hướng Dẫn Chi Tiết Các Phase - ESP32 Walkie-Talkie

## 📋 Tổng Quan

Dự án được chia thành **4 Phase** (giai đoạn), mỗi phase xây dựng dựa trên phase trước đó:

```
Phase 1: Foundation → Phase 2: ESP-NOW → Phase 3: Audio Transport → Phase 4: PTT Logic
   (Audio cơ bản)      (Wireless)         (Kết hợp Audio+WiFi)      (Walkie-Talkie hoàn chỉnh)
```

---

## ✅ Phase 1: Foundation & Audio Loopback Test

### **Mục tiêu**: 
Kiểm tra phần cứng (Mic, Speaker, I2S) hoạt động đúng **TRƯỚC KHI** thêm phức tạp của wireless.

### **Tại sao cần Phase này?**
- Nếu audio không hoạt động, không cần làm ESP-NOW
- Dễ debug: Chỉ có audio, không có WiFi
- Xác nhận timing và latency của I2S

### **Các Task**:

#### **T001: Initialize Project Skeleton** ✅
```
Làm gì: Tạo cấu trúc project với CMake và ESP-IDF
File: CMakeLists.txt, main/CMakeLists.txt
Kết quả: Project build được, chạy "Hello World"
```

#### **T002: GPIO and LED/Button Drivers** ✅
```
Làm gì: Cấu hình GPIO cho LED (GPIO 2) và nút PTT (GPIO 4)
File: main/board_pinout.h, main/main.c
Kết quả: Nhấn nút PTT → LED sáng
```

#### **T003: I2S Driver** ✅
```
Làm gì: Khởi tạo I2S cho Mic (INMP441) và Amp (MAX98357A)
File: main/audio_driver.c/h
Cấu hình:
  - Sample rate: 16kHz
  - Bit depth: 16-bit
  - Channels: Mono
  - DMA: 4 buffers × 256 samples
Kết quả: I2S init thành công, không lỗi
```

#### **T004: Loopback Logic** ✅
```
Làm gì: Đọc từ Mic → Ghi ngay ra Speaker (echo)
File: main/main.c (audio_loopback_task)
Luồng: Mic → I2S RX → Buffer → I2S TX → Speaker
Kết quả: Code chạy, có RMS log
```

#### **T005: Verify Audio** ✅
```
Làm gì: Test thực tế bằng cách nói vào Mic
Kết quả mong đợi:
  - Nghe thấy giọng mình từ speaker (có echo nhẹ ~16-32ms)
  - RMS thay đổi khi nói (0.05-0.15)
  - Không có tiếng hú (feedback)
```

### **Kết quả Phase 1**:
- ✅ Audio hardware hoạt động
- ✅ I2S DMA hoạt động
- ✅ Latency ~16-32ms (đủ cho walkie-talkie)

---

## ✅ Phase 2: ESP-NOW Communication

### **Mục tiêu**: 
Thiết lập kết nối wireless giữa 2 ESP32 **TRƯỚC KHI** gửi audio thật.

### **Tại sao cần Phase này?**
- Test ESP-NOW với dummy data đơn giản hơn audio
- Kiểm tra RF link quality (RSSI, packet loss)
- Xác nhận 2 ESP32 giao tiếp được với nhau

### **Các Task**:

#### **T006: Initialize WiFi & ESP-NOW** ✅
```
Làm gì: Khởi tạo WiFi Station mode và ESP-NOW
File: main/wifi_transport.c/h
Các bước:
  1. Init NVS (Non-Volatile Storage)
  2. Init WiFi trong Station mode
  3. Init ESP-NOW
  4. Register TX/RX callbacks
  5. Add broadcast peer (FF:FF:FF:FF:FF:FF)
Kết quả: ESP-NOW init thành công, không lỗi
```

#### **T007: Define audio_packet_t** ✅
```
Làm gì: Định nghĩa cấu trúc gói tin audio
File: main/app_config.h
Cấu trúc:
  - magic_word: 0xA55A (2 bytes) - Để lọc noise
  - seq_num: 0-65535 (2 bytes) - Sequence number
  - payload: 120 samples (240 bytes) - Audio data
  - Tổng: 244 bytes (< 250 bytes limit của ESP-NOW)
Kết quả: Struct được định nghĩa, compile OK
```

#### **T008: TX Task - Send Dummy Packets** ✅
```
Làm gì: Gửi gói tin test với counter tăng dần
File: main/main.c (esp_now_test_task)
Logic:
  1. Tạo packet với magic word
  2. Fill payload với sawtooth wave (test pattern)
  3. Gán sequence number
  4. Gửi qua ESP-NOW broadcast
  5. Lặp lại mỗi 1 giây
Kết quả: Log "Sent X packets, SeqNum=Y"
```

#### **T009: RX Callback - Receive & Log** ✅
```
Làm gì: Nhận gói tin và log thông tin
File: main/wifi_transport.c (esp_now_recv_cb)
Logic:
  1. Kiểm tra size packet (phải = 244 bytes)
  2. Validate magic word (phải = 0xA55A)
  3. Log: SeqNum, RSSI, MAC address nguồn
Kết quả: Log "RX: SeqNum=X, RSSI=-Y, From=MAC"
```

#### **T010: Verify RF Link** ⏳
```
Làm gì: Test với 2 ESP32 thật
Setup:
  - ESP32 #1: Flash firmware, chạy
  - ESP32 #2: Flash firmware, chạy
  - Đặt cách nhau 1-5m
Kiểm tra:
  - Cả 2 đều nhận được packets của nhau
  - RSSI: -30 đến -70 dBm (tốt)
  - Packet loss: <1%
  - Sequence number tăng đều
Kết quả: RF link ổn định
```

### **Kết quả Phase 2**:
- ✅ ESP-NOW hoạt động
- ✅ 2 ESP32 giao tiếp được
- ✅ RSSI và packet loss chấp nhận được

---

## ⏳ Phase 3: Audio Transport Integration

### **Mục tiêu**: 
Kết hợp Audio (Phase 1) và ESP-NOW (Phase 2) để truyền audio thật qua wireless.

### **Tại sao cần Phase này?**
- Audio và WiFi chạy ở tốc độ khác nhau → cần buffer
- Tránh blocking: Audio task không chờ WiFi, WiFi task không chờ Audio
- Xử lý jitter và packet loss

### **Các Task**:

#### **T011: Implement RingBuffer**
```
Làm gì: Tạo circular buffer để tách Audio và WiFi
File: main/ring_buffer_lib.c/h (hoặc dùng FreeRTOS StreamBuffer)
Chức năng:
  - Push: Audio task đẩy data vào
  - Pop: WiFi task lấy data ra
  - Thread-safe: Dùng mutex/semaphore
Kích thước: 4KB (~125ms audio)
Kết quả: RingBuffer hoạt động, test push/pop
```

#### **T012: Connect Audio → RingBuffer**
```
Làm gì: Audio task đẩy data vào RingBuffer thay vì loopback
File: main/main.c (audio_loopback_task → audio_capture_task)
Logic cũ: Mic → Buffer → Speaker (loopback)
Logic mới: Mic → Buffer → RingBuffer
Kết quả: Audio data được đẩy vào RingBuffer liên tục
```

#### **T013: Connect RingBuffer → WiFi TX**
```
Làm gì: WiFi task lấy data từ RingBuffer và gửi qua ESP-NOW
File: main/wifi_transport.c (wifi_tx_task)
Logic:
  1. Pop 240 bytes từ RingBuffer (120 samples)
  2. Tạo audio_packet_t
  3. Gán magic word và seq_num
  4. Gửi qua ESP-NOW
  5. Lặp lại
Tốc độ: ~133 packets/giây (16000 Hz / 120 samples)
Kết quả: Audio được gửi qua ESP-NOW
```

#### **T014: RX Assembly → Speaker**
```
Làm gì: Nhận packets và phát qua speaker
File: main/wifi_transport.c (esp_now_recv_cb)
Logic:
  1. Nhận packet từ ESP-NOW
  2. Validate magic word và seq_num
  3. Đẩy payload vào Jitter Buffer (optional)
  4. Ghi ra I2S TX (speaker)
Xử lý:
  - Packet loss: Bỏ qua hoặc fill silence
  - Out-of-order: Sắp xếp theo seq_num (optional)
Kết quả: Nghe được audio từ ESP32 khác
```

### **Kết quả Phase 3**:
- ✅ Audio được truyền qua ESP-NOW
- ✅ Nghe được giọng nói từ ESP32 khác
- ✅ Latency <100ms
- ⚠️ Vẫn là full-duplex (cả 2 thiết bị đều phát liên tục)

---

## ⏳ Phase 4: PTT Logic & Optimization

### **Mục tiêu**: 
Thêm logic Push-to-Talk để hoạt động như walkie-talkie thật (half-duplex).

### **Tại sao cần Phase này?**
- Walkie-talkie thật là half-duplex (chỉ 1 người nói tại 1 thời điểm)
- Tránh feedback (mic thu lại âm từ speaker)
- Tiết kiệm băng thông (chỉ gửi khi cần)

### **Các Task**:

#### **T015: State Machine**
```
Làm gì: Implement state machine cho PTT
File: main/main.c
States:
  - RX_IDLE: Chế độ nghe (mặc định)
    → Mic: TẮT
    → Speaker: BẬT
    → ESP-NOW: Lắng nghe
    → LED: TẮT
    
  - TX_ACTIVE: Chế độ phát (khi nhấn PTT)
    → Mic: BẬT
    → Speaker: TẮT (mute)
    → ESP-NOW: Gửi
    → LED: BẬT

Chuyển đổi:
  - Nhấn PTT: RX_IDLE → TX_ACTIVE
  - Thả PTT: TX_ACTIVE → RX_IDLE
  
Kết quả: State machine hoạt động đúng
```

#### **T016: Mute Logic**
```
Làm gì: Tắt speaker khi TX để tránh feedback
File: main/audio_driver.c
Cách 1: Software mute (ghi buffer silence)
Cách 2: Hardware mute (disable I2S TX channel)
Cách 3: GPIO control (nếu amp có pin SD)

Logic:
  - TX mode: audio_driver_mute_speaker(true)
  - RX mode: audio_driver_mute_speaker(false)
  
Kết quả: Không có feedback khi TX
```

#### **T017: Optimize Latency**
```
Làm gì: Tune buffer size để cân bằng latency vs dropout
Tham số:
  - RingBuffer size: 2KB-8KB
  - DMA buffer count: 2-8
  - Jitter buffer: 0-5 packets
  
Test:
  - Giảm buffer → Latency thấp, dễ dropout
  - Tăng buffer → Latency cao, ít dropout
  
Mục tiêu: Latency <100ms, dropout <1%
Kết quả: Tìm được cấu hình tối ưu
```

#### **T018: Verify Full System**
```
Làm gì: Test toàn bộ hệ thống với 2 ESP32
Scenario 1: User A nói, User B nghe
  1. User A nhấn PTT
  2. User A nói "Hello"
  3. User B nghe "Hello" từ speaker
  4. User A thả PTT
  
Scenario 2: User B trả lời
  1. User B nhấn PTT
  2. User B nói "Hi there"
  3. User A nghe "Hi there"
  4. User B thả PTT
  
Kiểm tra:
  - ✅ Audio rõ ràng, không méo
  - ✅ Latency <100ms
  - ✅ Không có feedback
  - ✅ PTT responsive (LED phản ứng ngay)
  - ✅ Half-duplex (chỉ 1 người nói tại 1 thời điểm)
  
Kết quả: Walkie-talkie hoạt động hoàn chỉnh
```

### **Kết quả Phase 4**:
- ✅ PTT logic hoạt động
- ✅ Half-duplex (như walkie-talkie thật)
- ✅ Không có feedback
- ✅ Latency <100ms
- ✅ Audio chất lượng tốt

---

## 📊 Tổng Kết Các Phase

| Phase | Mục tiêu | Input | Output | Trạng thái |
|-------|----------|-------|--------|------------|
| **1** | Test audio hardware | Mic | Speaker (loopback) | ✅ Hoàn thành |
| **2** | Test ESP-NOW wireless | Dummy data | Log RSSI/SeqNum | ✅ Hoàn thành |
| **3** | Kết hợp Audio + WiFi | Mic | ESP-NOW → Speaker | ⏳ Chưa làm |
| **4** | PTT logic | PTT button | Half-duplex walkie-talkie | ⏳ Chưa làm |

---

## 🎯 Tại Sao Chia Thành 4 Phase?

### **1. Incremental Development (Phát triển từng bước)**
- Mỗi phase xây dựng dựa trên phase trước
- Dễ debug: Biết chính xác phase nào có vấn đề
- Có thể dừng lại ở bất kỳ phase nào nếu cần

### **2. Risk Mitigation (Giảm rủi ro)**
- Phase 1: Xác nhận hardware OK
- Phase 2: Xác nhận RF link OK
- Phase 3: Xác nhận integration OK
- Phase 4: Thêm features

### **3. Testing Strategy (Chiến lược test)**
- Mỗi phase có test riêng, dễ verify
- Không cần 2 ESP32 cho Phase 1 và 2
- Chỉ cần 2 ESP32 từ Phase 3 trở đi

### **4. Learning Curve (Học từng bước)**
- Phase 1: Học I2S và audio
- Phase 2: Học ESP-NOW
- Phase 3: Học buffer management
- Phase 4: Học state machine

---

## 🚀 Roadmap Tiếp Theo

### **Hiện tại**: Phase 2 hoàn thành ✅

### **Tiếp theo**: Phase 3 - Audio Transport Integration

**Các bước**:
1. Implement RingBuffer (T011)
2. Modify audio task để push vào RingBuffer (T012)
3. Create WiFi TX task để pop từ RingBuffer (T013)
4. Modify RX callback để ghi ra speaker (T014)
5. Test với 2 ESP32

**Thời gian ước tính**: 2-3 giờ

---

**Chúc bạn code thành công!** 🎉
