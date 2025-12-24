# Danh Sách Công Việc: ESP32 ESP-NOW Walkie-Talkie

## ✅ Phase 1: Nền Tảng & Kiểm Tra Audio Loopback

- [x] T001: Khởi tạo Project Skeleton với CMake và ESP-IDF (C11) trong `CMakeLists.txt`
- [x] T002: Implement GPIO và LED/Button drivers trong `main/board_pinout.h` và `main/main.c`
- [x] T003: Implement I2S Driver (Master, 16kHz, 16-bit, DMA) trong `main/audio_driver.c`
- [x] T004: Implement Loopback Logic (Đọc Mic → Buffer → Ghi Amp) trong `main/main.c`
- [x] T005: Kiểm tra Audio: Nói vào Mic, nghe echo từ Speaker (Test thủ công)

## ✅ Phase 2: Giao Tiếp ESP-NOW

- [x] T006: Khởi tạo WiFi ở chế độ Station & ESP-NOW trong `main/wifi_transport.c`
- [x] T007: Định nghĩa struct `audio_packet_t` (Header + SeqNum + Payload) trong `main/app_config.h`
- [x] T008: Implement TX Task: Gửi dummy packets với counter trong `main/wifi_transport.c`
- [x] T009: Implement RX Callback: Nhận packets & log Sequence Numbers trong `main/wifi_transport.c`
- [ ] T010: Kiểm tra RF Link: Kiểm tra RSSI và Packet Loss Rate (Test thủ công)

## ✅ Phase 3: Tích Hợp Audio Transport

- [ ] T011: Implement RingBuffer để tách Audio-WiFi trong `main/ring_buffer_lib.c`
- [ ] T012: Kết nối Audio Task với RingBuffer (Push Mic Data) trong `main/main.c`
- [ ] T013: Kết nối WiFi Task với RingBuffer (Pop Data → Packetize → Send) trong `main/wifi_transport.c`
- [ ] T014: Implement RX Assembly (Receive → Jitter Buffer/Ring Buffer → I2S Write) trong `main/wifi_transport.c`

## 🛠️ Phase 4: PTT Logic & Tối Ưu Hóa

- **T015**: Implement State Machine (RX_IDLE ↔ TX_ACTIVE) sử dụng PTT Button trong `main/main.c`
- **T016**: Implement Mute Logic: Disable I2S Amp khi TX để tránh feedback trong `main/audio_driver.c`
- **T017**: Tối ưu Latency: Tune RingBuffer size vs Dropouts
- **T018**: Kiểm tra Full Duplex (Half-Duplex logic): Test conversation giữa 2 thiết bị
- **T019**: Khi không nhấn nút PTT, RX StreamBuffer phải rỗng (reset buffer on idle).
- **T020**: Khi thiết bị A nhấn PTT, thiết bị B chỉ bắt đầu phát sau khi RX buffer đã nhận đủ 3 packet (≈720 B).
- **T021**: Tăng số DMA buffer cho speaker lên 8 (update `I2S_DMA_BUF_COUNT` in `audio_driver.c`).
- **T022**: Update configuration constants in `app_config.h` for new DMA count.
- **T023**: Add documentation updates in `README.md` và `spec.md` cho PTT flow và buffer logic.

---
*All tasks are atomic, can be implemented independently, and follow the project's coding standards.*
