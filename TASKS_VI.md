# Danh Sách Công Việc: ESP32 ESP-NOW Walkie-Talkie

## 📋 Tổng Quan Dự Án

**Mục tiêu**: Xây dựng bộ đàm không dây sử dụng ESP32 với giao thức ESP-NOW

**Công nghệ**:
- Vi điều khiển: ESP32-WROOM-32
- Giao thức: ESP-NOW (broadcast mode)
- Audio: I2S với INMP441 (Mic) và MAX98357A (Amp)
- Sample rate: 16kHz, 16-bit, Mono

---

## ✅ Phase 1: Nền Tảng & Kiểm Tra Audio Loopback

**Mục tiêu**: Xác minh kết nối phần cứng và driver audio hoạt động đúng trước khi thêm phức tạp của RF.

### Các Công Việc:

- [x] **T001** [P] Khởi tạo Project Skeleton với CMake và ESP-IDF (chuẩn C11) trong `CMakeLists.txt`
- [x] **T002** Implement GPIO và LED/Button drivers trong `main/board_pinout.h` và `main/main.c`
- [x] **T003** Implement I2S Driver (Master, 16kHz, 16-bit, DMA) trong `main/audio_driver.c`
- [x] **T004** Implement Loopback Logic (Đọc Mic → Buffer → Ghi Amp) trong `main/main.c`
- [x] **T005** Kiểm tra Audio: Nói vào Mic, nghe delay/echo từ Speaker (Test thủ công)

---

## 🧪 Hướng Dẫn Kiểm Thử Phase 1

### **Chuẩn Bị**:

#### **Phần cứng cần thiết**:
- [x] 1x ESP32-WROOM-32 Development Board
- [x] 1x INMP441 I2S Microphone
- [x] 1x MAX98357A I2S Amplifier
- [x] 1x Speaker 4Ω hoặc 8Ω
- [x] 1x LED (bất kỳ màu)
- [x] 1x Nút nhấn (Push Button)
- [x] Dây nối Dupont
- [x] Cáp USB để flash ESP32

#### **Kết nối phần cứng** (xem WIRING_GUIDE.md):
```
INMP441 → ESP32:
  SCK  → GPIO 14
  WS   → GPIO 25
  SD   → GPIO 32
  L/R  → GND
  VDD  → 3.3V
  GND  → GND

MAX98357A → ESP32:
  BCLK → GPIO 14 (CHUNG với INMP441 SCK)
  LRC  → GPIO 25 (CHUNG với INMP441 WS)
  DIN  → GPIO 22
  VIN  → 5V
  GND  → GND
  OUT+ → Speaker (+)
  OUT- → Speaker (-)

LED → ESP32:
  Anode (+) → GPIO 2 (qua điện trở 220Ω)
  Cathode (-) → GND

Button → ESP32:
  Pin 1 → GPIO 4
  Pin 2 → GND
```

### **Bước 1: Build và Flash Firmware**

```bash
cd /home/iot/Lumi/Speckit/speckit_bo_dam

# Build project
idf.py build

# Flash vào ESP32 (thay /dev/ttyUSB0 bằng port của bạn)
idf.py -p /dev/ttyUSB0 flash

# Mở monitor để xem log
idf.py -p /dev/ttyUSB0 monitor
```

**Kết quả mong đợi**:
```
I (xxx) MAIN: === Doppler Walkie-Talkie - Phase 2: ESP-NOW Test ===
I (xxx) MAIN: GPIO initialized
I (xxx) AUDIO_DRIVER: Initializing I2S Driver (16kHz, 16-bit, Mono)...
I (xxx) AUDIO_DRIVER: I2S Driver initialized successfully
I (xxx) MAIN: Audio Loopback Task started
I (xxx) MAIN: Starting audio loopback (512 bytes per iteration)...
```

✅ **PASS**: Không có lỗi, log hiển thị đầy đủ  
❌ **FAIL**: Có lỗi I2S hoặc GPIO → Kiểm tra kết nối phần cứng

---

### **Bước 2: Test LED và Button (T002)**

**Cách test**:
1. Nhấn và giữ nút PTT (GPIO 4)
2. Quan sát LED (GPIO 2)

**Kết quả mong đợi**:
- ✅ LED **sáng** khi nhấn nút
- ✅ LED **tắt** khi thả nút
- ✅ Phản ứng **ngay lập tức** (<100ms)

**Nếu FAIL**:
- Kiểm tra LED có nối đúng cực không
- Kiểm tra điện trở 220Ω
- Kiểm tra nút nhấn có hoạt động không (dùng đồng hồ vạn năng)

---

### **Bước 3: Test I2S Driver (T003)**

**Cách test**: Quan sát log

**Kết quả mong đợi**:
```
I (xxx) AUDIO_DRIVER: I2S Driver initialized successfully
```

✅ **PASS**: Log hiển thị "I2S Driver initialized successfully"  
❌ **FAIL**: Lỗi I2S init

**Nếu FAIL**:
- Kiểm tra GPIO 14, 25, 32, 22 có nối đúng không
- Kiểm tra INMP441 và MAX98357A có nguồn 3.3V/5V không
- Kiểm tra GND có nối chung không

---

### **Bước 4: Test Audio Loopback (T004, T005)**

**Cách test**:
1. Đảm bảo Mic và Speaker **cách nhau >50cm** (tránh feedback)
2. Nói vào Mic với âm lượng bình thường: "Testing, one, two, three"
3. Lắng nghe từ Speaker

**Kết quả mong đợi**:

#### **A. Nghe thấy giọng nói**:
- ✅ Nghe rõ ràng giọng mình từ speaker
- ✅ Có độ trễ nhẹ (~16-32ms, như echo)
- ✅ Không bị méo tiếng
- ✅ Không có tiếng hú (howling/feedback)

#### **B. Log RMS thay đổi**:
```
I (xxx) MAIN: Audio RMS: 0.0012 (bytes: 512)  ← Im lặng
I (xxx) MAIN: Audio RMS: 0.0856 (bytes: 512)  ← Nói bình thường
I (xxx) MAIN: Audio RMS: 0.2341 (bytes: 512)  ← Nói to
I (xxx) MAIN: Audio RMS: 0.0018 (bytes: 512)  ← Im lặng lại
```

**Bảng RMS chuẩn**:
| Trạng thái | RMS Value | Ý nghĩa |
|------------|-----------|---------|
| Im lặng | 0.0001 - 0.01 | ✅ Bình thường |
| Nói nhỏ | 0.02 - 0.05 | ✅ Mic hoạt động |
| Nói bình thường | 0.05 - 0.15 | ✅ Tốt |
| Nói to / La hét | 0.15 - 0.5 | ✅ Tốt |
| Luôn = 0 | 0.0000 | ❌ Mic không hoạt động |
| Luôn cao | >0.5 | ⚠️ Quá lớn, có thể méo |

---

### **Bước 5: Troubleshooting Phase 1**

#### **Vấn đề 1: Không nghe thấy gì từ Speaker**

**Nguyên nhân có thể**:
- MAX98357A không có nguồn 5V
- Kết nối DIN (GPIO 22) sai
- Speaker không kết nối OUT+ và OUT-
- GAIN trên MAX98357A quá thấp

**Cách fix**:
```bash
# Kiểm tra log có lỗi I2S TX không
idf.py monitor | grep "I2S"

# Kiểm tra kết nối:
- MAX98357A VIN có 5V không? (đo bằng đồng hồ)
- GPIO 22 có nối đúng DIN không?
- Speaker có kết nối OUT+ và OUT- không?
```

**Nếu vẫn không được**: Hàn jumper GAIN trên MAX98357A (12dB)

---

#### **Vấn đề 2: RMS luôn bằng 0**

**Nguyên nhân**: Mic không hoạt động

**Cách fix**:
```bash
# Kiểm tra kết nối INMP441:
- VDD có 3.3V không? (⚠️ KHÔNG dùng 5V!)
- SCK (GPIO 14) có nối đúng không?
- WS (GPIO 25) có nối đúng không?
- SD (GPIO 32) có nối đúng không?
- L/R có nối GND không? (QUAN TRỌNG!)
```

**Test Mic**: Thổi nhẹ vào Mic, RMS phải tăng lên

---

#### **Vấn đề 3: Tiếng hú (Feedback/Howling)**

**Nguyên nhân**: Mic và Speaker quá gần, âm thanh từ Speaker quay lại Mic

**Cách fix**:
1. Đặt Mic và Speaker **xa nhau** (>50cm)
2. Hướng Mic và Speaker **ngược chiều**
3. Giảm GAIN trên MAX98357A (bỏ jumper)
4. Giảm volume (nếu có potentiometer)

**Lưu ý**: Feedback là bình thường nếu Mic và Speaker quá gần!

---

#### **Vấn đề 4: Âm thanh bị méo**

**Nguyên nhân**:
- GAIN quá cao
- Speaker không đúng trở kháng (cần 4Ω hoặc 8Ω)
- Nguồn 5V không đủ dòng

**Cách fix**:
1. Giảm GAIN trên MAX98357A
2. Kiểm tra speaker có đúng 4Ω/8Ω không
3. Dùng nguồn 5V ít nhất 500mA

---

### **✅ Checklist Hoàn Thành Phase 1**

Đánh dấu ✅ khi hoàn thành:

- [ ] Build và flash firmware thành công
- [ ] LED sáng/tắt khi nhấn/thả nút PTT
- [ ] Log hiển thị "I2S Driver initialized successfully"
- [ ] Nghe thấy giọng mình từ speaker (có echo nhẹ)
- [ ] RMS thay đổi khi nói (0.05-0.15)
- [ ] Không có tiếng hú (feedback)
- [ ] Không có lỗi trong log

**Nếu tất cả đều ✅**: Phase 1 hoàn thành! Chuyển sang Phase 2.

---

## ✅ Phase 2: Giao Tiếp ESP-NOW

**Mục tiêu**: Thiết lập kết nối wireless đáng tin cậy giữa 2 thiết bị.

### Các Công Việc:

- [x] **T006** Khởi tạo WiFi ở chế độ Station & ESP-NOW trong `main/wifi_transport.c`
- [x] **T007** Định nghĩa struct `audio_packet_t` (Header + SeqNum + Payload) trong `main/app_config.h`
- [x] **T008** Implement TX Task: Gửi dummy packets với counter trong `main/wifi_transport.c`
- [x] **T009** Implement RX Callback: Nhận packets & log Sequence Numbers trong `main/wifi_transport.c`
- [ ] **T010** Kiểm tra RF Link: Kiểm tra RSSI và Packet Loss Rate (Test thủ công)

---

## 🧪 Hướng Dẫn Kiểm Thử Phase 2

### **Chuẩn Bị**:

#### **Phần cứng cần thiết**:
- [x] **2x ESP32-WROOM-32** (bắt buộc - cần 2 thiết bị để test wireless)
- [x] 2x Cáp USB
- [x] 2x Nguồn USB (có thể dùng 2 cổng USB trên máy tính)
- [x] (Optional) 2x INMP441 + MAX98357A nếu muốn test cả audio

**Lưu ý**: Phase 2 có thể test **KHÔNG CẦN** Mic và Speaker, chỉ cần 2 ESP32!

---

### **Bước 1: Flash Firmware Lên 2 ESP32**

```bash
cd /home/iot/Lumi/Speckit/speckit_bo_dam

# Flash ESP32 #1
idf.py -p /dev/ttyUSB0 flash

# Flash ESP32 #2 (thay port nếu khác)
idf.py -p /dev/ttyUSB1 flash
```

**Kết quả mong đợi**: Cả 2 ESP32 đều flash thành công

---

### **Bước 2: Mở 2 Terminal Monitor**

**Terminal 1 - ESP32 #1**:
```bash
idf.py -p /dev/ttyUSB0 monitor
```

**Terminal 2 - ESP32 #2**:
```bash
idf.py -p /dev/ttyUSB1 monitor
```

**Kết quả mong đợi**:

**ESP32 #1 log**:
```
I (xxx) WIFI_TRANSPORT: Initializing WiFi and ESP-NOW...
I (xxx) WIFI_TRANSPORT: WiFi initialized in Station mode
I (xxx) WIFI_TRANSPORT: ESP-NOW initialized
I (xxx) WIFI_TRANSPORT: Broadcast peer added: ff:ff:ff:ff:ff:ff
I (xxx) WIFI_TRANSPORT: WiFi Transport initialized successfully
I (xxx) MAIN: ESP-NOW Test Task started
I (xxx) MAIN: Sent 10 packets, SeqNum=9
I (xxx) WIFI_TRANSPORT: RX: SeqNum=5, RSSI=-45, From=a0:dd:6c:95:26:64
```

**ESP32 #2 log**: Tương tự, nhưng MAC address khác

✅ **PASS**: Cả 2 ESP32 đều init ESP-NOW thành công  
❌ **FAIL**: Có lỗi WiFi hoặc ESP-NOW init

---

### **Bước 3: Kiểm Tra TX - Gửi Packets (T008)**

**Cách test**: Quan sát log của **1 ESP32 bất kỳ**

**Kết quả mong đợi**:
```
I (1234) MAIN: Sent 10 packets, SeqNum=9
I (2234) MAIN: Sent 20 packets, SeqNum=19
I (3234) MAIN: Sent 30 packets, SeqNum=29
```

**Phân tích**:
- ✅ Log hiển thị "Sent X packets" mỗi 10 giây
- ✅ SeqNum tăng dần (0, 1, 2, 3, ...)
- ✅ Không có lỗi "ESP-NOW send failed"

**Nếu FAIL**:
- Kiểm tra WiFi có init thành công không
- Kiểm tra broadcast peer có được thêm không

---

### **Bước 4: Kiểm Tra RX - Nhận Packets (T009)**

**Cách test**: Đặt 2 ESP32 **gần nhau** (1-2m), quan sát log

**Kết quả mong đợi**:

**ESP32 #1 nhận từ ESP32 #2**:
```
I (xxx) WIFI_TRANSPORT: RX: SeqNum=0, RSSI=-42, From=a0:dd:6c:95:26:64
I (xxx) WIFI_TRANSPORT: RX: SeqNum=1, RSSI=-43, From=a0:dd:6c:95:26:64
I (xxx) WIFI_TRANSPORT: RX: SeqNum=2, RSSI=-41, From=a0:dd:6c:95:26:64
```

**ESP32 #2 nhận từ ESP32 #1**:
```
I (xxx) WIFI_TRANSPORT: RX: SeqNum=0, RSSI=-45, From=b4:e6:2d:8a:1c:3f
I (xxx) WIFI_TRANSPORT: RX: SeqNum=1, RSSI=-44, From=b4:e6:2d:8a:1c:3f
```

**Phân tích**:
- ✅ Cả 2 ESP32 đều nhận được packets của nhau
- ✅ SeqNum tăng dần, không bị nhảy số
- ✅ RSSI trong khoảng -30 đến -70 dBm
- ✅ MAC address khác nhau giữa 2 ESP32

**Nếu FAIL**:
- Không nhận được packets → Kiểm tra 2 ESP32 có cùng WiFi channel không
- RSSI quá thấp (<-80) → Đặt 2 ESP32 gần nhau hơn

---

### **Bước 5: Kiểm Tra RF Link Quality (T010)**

#### **Test 1: RSSI (Cường độ tín hiệu)**

**Cách test**: Di chuyển 2 ESP32 ra xa nhau, quan sát RSSI

| Khoảng cách | RSSI mong đợi | Chất lượng |
|-------------|---------------|------------|
| 0.5 - 1m | -30 đến -40 dBm | ✅ Xuất sắc |
| 1 - 3m | -40 đến -55 dBm | ✅ Tốt |
| 3 - 10m | -55 đến -70 dBm | ✅ Chấp nhận được |
| 10 - 20m | -70 đến -80 dBm | ⚠️ Yếu |
| >20m | <-80 dBm | ❌ Kém, dễ mất kết nối |

**Kết quả mong đợi**:
- ✅ RSSI > -70 dBm ở khoảng cách sử dụng thực tế (1-5m)
- ✅ RSSI thay đổi khi di chuyển xa/gần

---

#### **Test 2: Packet Loss (Tỷ lệ mất gói tin)**

**Cách test**: Đếm số packets nhận được

**Công thức**:
```
Packet Loss % = (Packets gửi - Packets nhận) / Packets gửi × 100%
```

**Ví dụ**:
```
ESP32 #1 gửi: SeqNum từ 0 đến 99 (100 packets)
ESP32 #2 nhận: SeqNum 0,1,2,3,5,6,7,... (thiếu SeqNum=4)
→ Packet Loss = 1/100 = 1%
```

**Kết quả mong đợi**:
- ✅ Packet Loss < 1% (khoảng cách 1-5m)
- ✅ Packet Loss < 5% (khoảng cách 5-10m)
- ⚠️ Packet Loss > 10% → Quá xa hoặc có vật cản

**Cách kiểm tra**: Quan sát SeqNum có bị nhảy số không
```
✅ Tốt: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 (liên tục)
❌ Kém: 0, 1, 2, 5, 6, 9, 10, 15 (thiếu 3,4,7,8,11-14)
```

---

#### **Test 3: Throughput (Băng thông)**

**Cách test**: Tính số packets/giây

**Hiện tại**: 1 packet/giây (test mode)

**Phase 3 sẽ cần**: ~133 packets/giây (16000 Hz / 120 samples)

**Kết quả mong đợi**:
- ✅ ESP-NOW có thể gửi >200 packets/giây
- ✅ Băng thông đủ cho audio streaming

---

### **Bước 6: Test Ở Các Môi Trường Khác Nhau**

#### **Test A: Trong nhà (Indoor)**
```
Khoảng cách: 1-5m
Vật cản: Tường gỗ, bàn ghế
RSSI mong đợi: -40 đến -60 dBm
Packet Loss: <2%
```

#### **Test B: Ngoài trời (Outdoor)**
```
Khoảng cách: 5-20m
Vật cản: Không có
RSSI mong đợi: -50 đến -70 dBm
Packet Loss: <1%
```

#### **Test C: Có vật cản (Obstructed)**
```
Khoảng cách: 1-3m
Vật cản: Tường bê tông, tủ sắt
RSSI mong đợi: -60 đến -80 dBm
Packet Loss: <5%
```

---

### **Bước 7: Troubleshooting Phase 2**

#### **Vấn đề 1: ESP-NOW init failed**

**Log lỗi**:
```
E (xxx) WIFI_TRANSPORT: Failed to add broadcast peer: ESP_ERR_ESPNOW_NOT_INIT
```

**Nguyên nhân**: WiFi chưa init hoặc ESP-NOW chưa init

**Cách fix**:
```bash
# Kiểm tra log có dòng này không:
I (xxx) WIFI_TRANSPORT: WiFi initialized in Station mode
I (xxx) WIFI_TRANSPORT: ESP-NOW initialized

# Nếu không có → Kiểm tra code wifi_transport_init()
```

---

#### **Vấn đề 2: Không nhận được packets**

**Triệu chứng**: ESP32 #1 gửi, nhưng ESP32 #2 không nhận

**Nguyên nhân có thể**:
1. 2 ESP32 khác WiFi channel
2. Quá xa (RSSI < -80)
3. RX callback không được register

**Cách fix**:
```bash
# Kiểm tra cả 2 ESP32 có log này không:
I (xxx) WIFI_TRANSPORT: Broadcast peer added: ff:ff:ff:ff:ff:ff

# Kiểm tra RSSI:
- Nếu không có log RX → Đặt 2 ESP32 gần nhau (1m)
- Nếu vẫn không nhận → Kiểm tra esp_now_register_recv_cb()
```

---

#### **Vấn đề 3: Packet loss cao (>10%)**

**Nguyên nhân**:
- Quá xa
- Có vật cản (tường bê tông, tủ sắt)
- Nhiễu WiFi (nhiều mạng WiFi xung quanh)

**Cách fix**:
1. Giảm khoảng cách
2. Loại bỏ vật cản
3. Đổi WiFi channel (hiện tại dùng channel 0 = auto)

---

### **✅ Checklist Hoàn Thành Phase 2**

Đánh dấu ✅ khi hoàn thành:

- [ ] 2 ESP32 đều init WiFi và ESP-NOW thành công
- [ ] ESP32 #1 gửi packets (log "Sent X packets")
- [ ] ESP32 #2 nhận packets từ ESP32 #1 (log "RX: SeqNum=X")
- [ ] ESP32 #1 nhận packets từ ESP32 #2
- [ ] RSSI trong khoảng -30 đến -70 dBm (khoảng cách 1-5m)
- [ ] Packet loss < 5%
- [ ] SeqNum tăng dần, không bị nhảy số nhiều

**Nếu tất cả đều ✅**: Phase 2 hoàn thành! Chuyển sang Phase 3.

---

## ⏳ Phase 3: Tích Hợp Audio Transport

**Mục tiêu**: Kết hợp Audio và WiFi để streaming audio một chiều.

### Các Công Việc:

- [ ] **T011** Implement RingBuffer để tách Audio-WiFi trong `main/ring_buffer_lib.c`
- [ ] **T012** Kết nối Audio Task với RingBuffer (Push Mic Data) trong `main/main.c`
- [ ] **T013** Kết nối WiFi Task với RingBuffer (Pop Data → Packetize → Send) trong `main/wifi_transport.c`
- [ ] **T014** Implement RX Assembly (Receive → Jitter Buffer/Ring Buffer → I2S Write) trong `main/wifi_transport.c`

---

## 🧪 Hướng Dẫn Kiểm Thử Phase 3

### **Chuẩn Bị**:

#### **Phần cứng cần thiết**:
- [x] **2x ESP32-WROOM-32** (bắt buộc)
- [x] **2x INMP441 Microphone** (bắt buộc)
- [x] **2x MAX98357A Amplifier** (bắt buộc)
- [x] **2x Speaker 4Ω/8Ω** (bắt buộc)
- [x] Tất cả kết nối như Phase 1

**Lưu ý**: Phase 3 cần **ĐẦY ĐỦ** phần cứng audio!

---

### **Bước 1: Flash Firmware Phase 3**

```bash
# Flash lên 2 ESP32
idf.py -p /dev/ttyUSB0 flash  # ESP32 #1
idf.py -p /dev/ttyUSB1 flash  # ESP32 #2
```

---

### **Bước 2: Test RingBuffer (T011)**

**Cách test**: Quan sát log

**Kết quả mong đợi**:
```
I (xxx) MAIN: RingBuffer created: 4096 bytes
I (xxx) MAIN: RingBuffer test: Push OK, Pop OK
```

✅ **PASS**: RingBuffer hoạt động  
❌ **FAIL**: Lỗi malloc hoặc push/pop

---

### **Bước 3: Test Audio → RingBuffer (T012)**

**Cách test**: Nói vào Mic, kiểm tra RingBuffer có data không

**Kết quả mong đợi**:
```
I (xxx) MAIN: RingBuffer usage: 512/4096 bytes (12%)
I (xxx) MAIN: RingBuffer usage: 1024/4096 bytes (25%)
```

**Phân tích**:
- ✅ RingBuffer usage tăng khi nói vào Mic
- ✅ RingBuffer usage giảm khi WiFi task đọc data
- ⚠️ RingBuffer full (100%) → Tăng buffer size hoặc tăng tốc độ WiFi send

---

### **Bước 4: Test WiFi TX với Audio Thật (T013)**

**Cách test**: 
1. Nói vào Mic của ESP32 #1
2. Quan sát log của ESP32 #1

**Kết quả mong đợi**:
```
I (xxx) WIFI_TX: Sent audio packet: SeqNum=100, Size=244 bytes
I (xxx) WIFI_TX: Sent audio packet: SeqNum=101, Size=244 bytes
```

**Phân tích**:
- ✅ Gửi ~133 packets/giây (16000 Hz / 120 samples)
- ✅ SeqNum tăng liên tục
- ✅ Không có lỗi "ESP-NOW send failed"

---

### **Bước 5: Test RX → Speaker (T014)**

**Cách test**:
1. **ESP32 #1**: Nói vào Mic "Testing, one, two, three"
2. **ESP32 #2**: Lắng nghe từ Speaker

**Kết quả mong đợi**:
- ✅ Nghe thấy "Testing, one, two, three" từ Speaker của ESP32 #2
- ✅ Độ trễ <100ms (chấp nhận được)
- ✅ Audio rõ ràng, không bị méo
- ⚠️ Có thể hơi "robot" do packet loss

**Log ESP32 #2**:
```
I (xxx) WIFI_RX: Received audio: SeqNum=100, RSSI=-45
I (xxx) WIFI_RX: Received audio: SeqNum=101, RSSI=-46
I (xxx) WIFI_RX: Written to speaker: 240 bytes
```

---

### **Bước 6: Test Full Duplex (Cả 2 chiều)**

**Cách test**:
1. **ESP32 #1**: Nói vào Mic
2. **ESP32 #2**: Nghe từ Speaker
3. **Đồng thời ESP32 #2**: Nói vào Mic
4. **ESP32 #1**: Nghe từ Speaker

**Kết quả mong đợi**:
- ✅ Cả 2 ESP32 đều nghe được nhau
- ⚠️ Có thể bị feedback nếu Mic và Speaker quá gần
- ⚠️ Âm thanh có thể bị "chồng lên nhau" (full-duplex)

**Lưu ý**: Phase 3 vẫn là **full-duplex** (cả 2 đều phát liên tục). Phase 4 sẽ thêm PTT để chỉ 1 người nói tại 1 thời điểm.

---

### **Bước 7: Đo Latency (Độ trễ)**

**Cách test**:
1. Dùng điện thoại ghi âm
2. Đặt gần Mic của ESP32 #1 và Speaker của ESP32 #2
3. Vỗ tay hoặc nói "Beep"
4. Nghe lại recording, đo khoảng cách giữa 2 âm thanh

**Kết quả mong đợi**:
- ✅ Latency < 100ms (yêu cầu spec)
- ✅ Latency ~30-60ms (tốt)
- ⚠️ Latency > 100ms → Giảm buffer size

---

### **Bước 8: Troubleshooting Phase 3**

#### **Vấn đề 1: Không nghe thấy gì từ Speaker**

**Nguyên nhân**:
- RX callback không ghi data ra speaker
- Speaker bị mute
- Packet loss 100%

**Cách fix**:
```bash
# Kiểm tra log có "Received audio" không
I (xxx) WIFI_RX: Received audio: SeqNum=X

# Nếu có nhận nhưng không nghe thấy:
- Kiểm tra audio_driver_write() có được gọi không
- Kiểm tra speaker có hoạt động không (test Phase 1)

# Nếu không nhận:
- Kiểm tra 2 ESP32 có gần nhau không (1-2m)
- Kiểm tra RSSI > -70 dBm
```

---

#### **Vấn đề 2: Audio bị "đứt đoạn" (choppy)**

**Nguyên nhân**:
- Packet loss cao
- RingBuffer quá nhỏ
- WiFi send không kịp

**Cách fix**:
1. Giảm khoảng cách giữa 2 ESP32
2. Tăng RingBuffer size (4KB → 8KB)
3. Kiểm tra packet loss < 5%

---

#### **Vấn đề 3: Latency quá cao (>200ms)**

**Nguyên nhân**:
- RingBuffer quá lớn
- Jitter buffer quá lớn
- DMA buffer quá nhiều

**Cách fix**:
1. Giảm RingBuffer size (4KB → 2KB)
2. Giảm DMA buffer count (4 → 2)
3. Bỏ Jitter buffer (nếu có)

---

### **✅ Checklist Hoàn Thành Phase 3**

Đánh dấu ✅ khi hoàn thành:

- [ ] RingBuffer hoạt động (push/pop OK)
- [ ] Audio task đẩy data vào RingBuffer
- [ ] WiFi task gửi audio packets (~133 packets/giây)
- [ ] ESP32 #2 nhận được audio packets
- [ ] Nghe thấy giọng nói từ ESP32 #1 qua Speaker của ESP32 #2
- [ ] Latency < 100ms
- [ ] Audio rõ ràng, không bị đứt đoạn nhiều
- [ ] Packet loss < 5%

**Nếu tất cả đều ✅**: Phase 3 hoàn thành! Chuyển sang Phase 4.

---

## ⏳ Phase 4: PTT Logic & Tối Ưu Hóa

**Mục tiêu**: Chức năng Walkie-Talkie hoàn chỉnh với half-duplex.

### Các Công Việc:

- [ ] **T015** Implement State Machine (RX_IDLE ↔ TX_ACTIVE) sử dụng PTT Button trong `main/main.c`
- [ ] **T016** Implement Mute Logic: Disable I2S Amp khi TX để tránh feedback trong `main/audio_driver.c`
- [ ] **T017** Tối ưu Latency: Tune RingBuffer size vs Dropouts
- [ ] **T018** Kiểm tra Full Duplex (Half-Duplex logic): Test conversation giữa 2 thiết bị

---

## 🧪 Hướng Dẫn Kiểm Thử Phase 4

### **Chuẩn Bị**:
- Giống Phase 3 (2 ESP32 + đầy đủ audio hardware)

---

### **Bước 1: Test State Machine (T015)**

**Cách test**:
1. Không nhấn nút PTT → Quan sát log
2. Nhấn nút PTT → Quan sát log
3. Thả nút PTT → Quan sát log

**Kết quả mong đợi**:

**Không nhấn PTT (RX_IDLE)**:
```
I (xxx) MAIN: State: RX_IDLE
I (xxx) MAIN: Mic: OFF, Speaker: ON, LED: OFF
```

**Nhấn PTT (TX_ACTIVE)**:
```
I (xxx) MAIN: State: TX_ACTIVE
I (xxx) MAIN: Mic: ON, Speaker: MUTED, LED: ON
```

**Thả PTT (quay lại RX_IDLE)**:
```
I (xxx) MAIN: State: RX_IDLE
I (xxx) MAIN: Mic: OFF, Speaker: ON, LED: OFF
```

✅ **PASS**: State chuyển đổi đúng  
❌ **FAIL**: State không đổi hoặc LED không phản ứng

---

### **Bước 2: Test Mute Logic (T016)**

**Cách test**:
1. Nhấn PTT
2. Nói vào Mic
3. Lắng nghe Speaker của **CHÍNH thiết bị đó**

**Kết quả mong đợi**:
- ✅ **KHÔNG** nghe thấy gì từ Speaker (đã bị mute)
- ✅ ESP32 khác nghe được (qua ESP-NOW)
- ✅ Không có feedback/howling

**Nếu vẫn nghe thấy từ Speaker**:
- Mute logic chưa hoạt động
- Kiểm tra `audio_driver_mute_speaker(true)` có được gọi không

---

### **Bước 3: Test Half-Duplex Communication**

**Scenario 1: User A nói, User B nghe**

**ESP32 #1 (User A)**:
1. Nhấn và giữ PTT
2. LED sáng
3. Nói: "Hello, this is User A"
4. Thả PTT
5. LED tắt

**ESP32 #2 (User B)**:
1. Không nhấn PTT
2. LED tắt
3. Nghe: "Hello, this is User A" từ Speaker
4. Rõ ràng, không bị méo

**Kết quả mong đợi**:
- ✅ User B nghe rõ User A
- ✅ User A không nghe thấy gì từ Speaker của mình (đã mute)
- ✅ Latency < 100ms

---

**Scenario 2: User B trả lời**

**ESP32 #2 (User B)**:
1. Nhấn và giữ PTT
2. LED sáng
3. Nói: "Hi User A, I hear you loud and clear"
4. Thả PTT
5. LED tắt

**ESP32 #1 (User A)**:
1. Không nhấn PTT
2. LED tắt
3. Nghe: "Hi User A, I hear you loud and clear"

**Kết quả mong đợi**:
- ✅ User A nghe rõ User B
- ✅ Không có feedback
- ✅ Half-duplex hoạt động (chỉ 1 người nói tại 1 thời điểm)

---

### **Bước 4: Test Rapid PTT Toggle**

**Cách test**: Nhấn/thả PTT nhanh liên tục (5-10 lần)

**Kết quả mong đợi**:
- ✅ LED phản ứng ngay lập tức (<100ms)
- ✅ State chuyển đổi đúng mỗi lần
- ✅ Không bị crash hoặc hang
- ✅ Audio không bị lỗi

---

### **Bước 5: Test Latency Optimization (T017)**

**Cách test**: Thử các cấu hình buffer khác nhau

| RingBuffer Size | DMA Buffers | Latency | Dropout Rate | Đánh giá |
|-----------------|-------------|---------|--------------|----------|
| 2KB | 2 | ~30ms | 5-10% | ⚠️ Nhanh nhưng nhiều dropout |
| 4KB | 4 | ~50ms | 1-2% | ✅ Cân bằng tốt |
| 8KB | 8 | ~100ms | <1% | ⚠️ Ổn định nhưng chậm |

**Kết quả mong đợi**:
- ✅ Tìm được cấu hình tối ưu: Latency <100ms, Dropout <2%
- ✅ Khuyến nghị: RingBuffer 4KB, DMA 4 buffers

---

### **Bước 6: Test Full System (T018)**

**Scenario: Conversation thực tế**

**User A**:
1. Nhấn PTT
2. "Hello User B, do you copy?"
3. Thả PTT
4. Chờ phản hồi

**User B**:
1. Nghe "Hello User B, do you copy?"
2. Nhấn PTT
3. "Yes User A, I copy you loud and clear"
4. Thả PTT

**User A**:
1. Nghe "Yes User A, I copy you loud and clear"
2. Nhấn PTT
3. "Great! Over and out"
4. Thả PTT

**Kết quả mong đợi**:
- ✅ Conversation tự nhiên, không bị gián đoạn
- ✅ Audio rõ ràng cả 2 chiều
- ✅ PTT responsive
- ✅ Không có feedback
- ✅ Latency chấp nhận được (<100ms)

---

### **Bước 7: Test Ở Khoảng Cách Khác Nhau**

| Khoảng cách | RSSI | Audio Quality | Đánh giá |
|-------------|------|---------------|----------|
| 1m | -35 dBm | Xuất sắc | ✅ |
| 3m | -50 dBm | Tốt | ✅ |
| 5m | -60 dBm | Chấp nhận được | ✅ |
| 10m | -70 dBm | Hơi đứt đoạn | ⚠️ |
| 15m | -80 dBm | Nhiều dropout | ❌ |

**Kết quả mong đợi**:
- ✅ Hoạt động tốt ở khoảng cách 1-5m (indoor)
- ✅ Hoạt động tốt ở khoảng cách 5-15m (outdoor)

---

### **Bước 8: Troubleshooting Phase 4**

#### **Vấn đề 1: PTT không phản ứng**

**Nguyên nhân**:
- Nút nhấn hỏng
- GPIO 4 không được config đúng
- State machine không chạy

**Cách fix**:
```bash
# Test nút nhấn:
- Nhấn PTT, kiểm tra LED có sáng không
- Nếu LED không sáng → Nút hỏng hoặc GPIO sai

# Kiểm tra log:
I (xxx) MAIN: PTT pressed, state: TX_ACTIVE
```

---

#### **Vấn đề 2: Vẫn có feedback khi TX**

**Nguyên nhân**: Speaker không bị mute

**Cách fix**:
```bash
# Kiểm tra log khi nhấn PTT:
I (xxx) AUDIO_DRIVER: Speaker MUTED

# Nếu không có log này:
- Kiểm tra audio_driver_mute_speaker() có được gọi không
- Kiểm tra implementation của mute function
```

---

#### **Vấn đề 3: Audio bị "cắt" đầu câu**

**Nguyên nhân**: 
- Nhấn PTT → Mất vài packet đầu tiên
- RingBuffer chưa kịp fill

**Cách fix**:
1. Thêm delay nhỏ (~50ms) sau khi nhấn PTT trước khi bắt đầu gửi
2. Hoặc gửi vài packet "silence" trước khi gửi audio thật

---

### **✅ Checklist Hoàn Thành Phase 4**

Đánh dấu ✅ khi hoàn thành:

- [ ] State machine hoạt động (RX ↔ TX)
- [ ] LED phản ứng ngay khi nhấn/thả PTT
- [ ] Speaker bị mute khi TX (không feedback)
- [ ] Half-duplex hoạt động (chỉ 1 người nói tại 1 thời điểm)
- [ ] Conversation tự nhiên giữa 2 người
- [ ] Latency < 100ms
- [ ] Audio rõ ràng, không méo
- [ ] Hoạt động tốt ở khoảng cách 1-5m

**Nếu tất cả đều ✅**: 🎉 **DỰ ÁN HOÀN THÀNH!** 🎉

---

## 🎯 Tổng Kết

### **Tiến Độ Hiện Tại**:
- ✅ Phase 1: Hoàn thành
- ✅ Phase 2: Hoàn thành
- ⏳ Phase 3: Chưa làm
- ⏳ Phase 4: Chưa làm

### **Thời Gian Ước Tính**:
- Phase 3: 2-3 giờ
- Phase 4: 1-2 giờ
- **Tổng**: 3-5 giờ để hoàn thành dự án

### **Phần Cứng Cần Thiết**:
- Phase 1: 1 ESP32 + Audio (Mic + Amp + Speaker)
- Phase 2: 2 ESP32 (không cần audio)
- Phase 3-4: 2 ESP32 + 2 bộ Audio đầy đủ

---

**Chúc bạn thành công!** 🚀
