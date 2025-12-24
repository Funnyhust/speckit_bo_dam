# Hướng Dẫn Hàn Mạch - ESP32 Walkie-Talkie

## 📋 Danh Sách Linh Kiện

- **1x** ESP32-WROOM-32 Development Board
- **1x** INMP441 I2S Microphone Module
- **1x** MAX98357A I2S Amplifier Module
- **1x** Speaker 4Ω hoặc 8Ω (3-5W)
- **1x** Nút nhấn (Push Button)
- **1x** LED (bất kỳ màu nào)
- **1x** Điện trở 220Ω-330Ω (cho LED)
- Dây nối Dupont hoặc dây hàn

---

## 🔌 Sơ Đồ Kết Nối Chi Tiết

### **1. INMP441 Microphone → ESP32**

| INMP441 Module | Tên Chân Trên Module | ESP32 GPIO | Chức Năng |
|----------------|----------------------|------------|-----------|
| **SCK** | SCK (Serial Clock) | **GPIO 14** | BCLK (Bit Clock) - CHUNG với Amp |
| **WS** | WS (Word Select) | **GPIO 25** | LRCK (Word Select) - CHUNG với Amp |
| **SD** | SD (Serial Data) | **GPIO 32** | Data từ Mic → ESP32 |
| **L/R** | L/R (Left/Right) | **GND** | Chọn kênh Left (nối GND) hoặc Right (nối VDD) |
| **VDD** | VDD (Power) | **3.3V** | Nguồn 3.3V |
| **GND** | GND (Ground) | **GND** | Đất |

**Lưu ý**: 
- Chân **L/R** nối **GND** để chọn kênh Left (khuyến nghị)
- Hoặc nối **VDD** để chọn kênh Right (tùy chọn)

---

### **2. MAX98357A Amplifier → ESP32**

| MAX98357A Module | Tên Chân Trên Module | ESP32 GPIO | Chức Năng |
|------------------|----------------------|------------|-----------|
| **BCLK** | BCLK (Bit Clock) | **GPIO 14** | BCLK - CHUNG với Mic |
| **LRC** | LRC (Left/Right Clock) | **GPIO 25** | LRCK - CHUNG với Mic |
| **DIN** | DIN (Data Input) | **GPIO 22** | Data từ ESP32 → Amp |
| **SD** | SD (Shutdown) | **Không nối** | Để hở (hoặc nối VIN để luôn bật) |
| **GAIN** | GAIN (Jumper) | **Xem bảng dưới** | Điều chỉnh gain |
| **VIN** | VIN (Power) | **5V hoặc 3.3V** | Nguồn (khuyến nghị 5V) |
| **GND** | GND (Ground) | **GND** | Đất |

**Cài đặt GAIN** (jumper trên module):
| Jumper | Gain | Khuyến nghị |
|--------|------|-------------|
| Không hàn | 9dB | ✅ Thử đầu tiên (âm lượng vừa) |
| Hàn 1 chấm | 12dB | Nếu quá nhỏ |
| Hàn 2 chấm | 15dB | Nếu vẫn nhỏ |

**Kết nối Speaker**:
- Speaker (+) → MAX98357A chân **OUT+**
- Speaker (-) → MAX98357A chân **OUT-**

---

### **3. Nút PTT (Push-to-Talk) → ESP32**

| Nút Nhấn | ESP32 GPIO | Chức Năng |
|----------|------------|-----------|
| Chân 1 | **GPIO 4** | PTT Button |
| Chân 2 | **GND** | Đất |

**Lưu ý**: 
- ESP32 đã bật **pull-up nội** (GPIO_PULLUP_ONLY)
- Khi nhấn nút: GPIO 4 → GND (Active Low)
- Khi thả nút: GPIO 4 → 3.3V (pull-up)

---

### **4. LED Trạng Thái → ESP32**

| LED | ESP32 GPIO | Chức Năng |
|-----|------------|-----------|
| Anode (+) | **GPIO 2** qua điện trở 220Ω | Status LED |
| Cathode (-) | **GND** | Đất |

**Sơ đồ**:
```
GPIO 2 ──[220Ω]──[LED]──GND
```

---

## 🎨 Sơ Đồ Tổng Quan

```
┌──────────────────────────────────────────────────────────────┐
│                      ESP32-WROOM-32                          │
│                                                              │
│  GPIO 14 (BCLK)  ──────┬────────────────────────┬───────────│
│                        │                        │           │
│  GPIO 25 (LRCK)  ──────┼────────┬───────────────┼───────────│
│                        │        │               │           │
│  GPIO 32 (DI)    ──────┼────────┼───────┐       │           │
│                        │        │       │       │           │
│  GPIO 22 (DO)    ──────┼────────┼───────┼───────┼──┐        │
│                        │        │       │       │  │        │
│  GPIO 4  (PTT)   ──────┼────────┼───────┼───────┼──┼────┐   │
│                        │        │       │       │  │    │   │
│  GPIO 2  (LED)   ──────┼────────┼───────┼───────┼──┼────┼───│
│                        │        │       │       │  │    │   │
│  3.3V ─────────────────┼────────┼───────┼───────┼──┼────┼───│
│  5V   ─────────────────┼────────┼───────┼───────┼──┼────┼───│
│  GND  ─────────────────┼────────┼───────┼───────┼──┼────┼───│
└────────────────────────┼────────┼───────┼───────┼──┼────┼───┘
                         │        │       │       │  │    │
                         ↓        ↓       ↓       ↓  ↓    ↓
        ┌────────────────────────────┐   ┌──────────────────┐
        │   INMP441 Microphone       │   │  MAX98357A Amp   │
        ├────────────────────────────┤   ├──────────────────┤
        │ SCK  ← GPIO 14 (BCLK)      │   │ BCLK ← GPIO 14   │
        │ WS   ← GPIO 25 (LRCK)      │   │ LRC  ← GPIO 25   │
        │ SD   → GPIO 32 (DI)        │   │ DIN  ← GPIO 22   │
        │ L/R  ← GND                 │   │ SD   - Không nối │
        │ VDD  ← 3.3V                │   │ VIN  ← 5V        │
        │ GND  ← GND                 │   │ GND  ← GND       │
        └────────────────────────────┘   └────────┬─────────┘
                                                  │
                                                  ↓
                                         ┌─────────────────┐
                                         │  Speaker 4Ω/8Ω │
                                         │  OUT+ ← OUT+   │
                                         │  OUT- ← OUT-   │
                                         └─────────────────┘

        ┌──────────────┐                 ┌──────────────┐
        │ PTT Button   │                 │  Status LED  │
        ├──────────────┤                 ├──────────────┤
        │ Pin1 → GPIO4 │                 │ (+) ← GPIO2  │
        │ Pin2 → GND   │                 │ (-) ← GND    │
        └──────────────┘                 └──────────────┘
                                         (qua R 220Ω)
```

---

## ✅ Checklist Trước Khi Bật Nguồn

### **Kiểm tra kết nối**:
- [ ] INMP441 SCK → ESP32 GPIO 14
- [ ] INMP441 WS → ESP32 GPIO 25
- [ ] INMP441 SD → ESP32 GPIO 32
- [ ] INMP441 L/R → GND
- [ ] INMP441 VDD → 3.3V
- [ ] INMP441 GND → GND

- [ ] MAX98357A BCLK → ESP32 GPIO 14 (CHUNG với INMP441 SCK)
- [ ] MAX98357A LRC → ESP32 GPIO 25 (CHUNG với INMP441 WS)
- [ ] MAX98357A DIN → ESP32 GPIO 22
- [ ] MAX98357A VIN → 5V (hoặc 3.3V)
- [ ] MAX98357A GND → GND
- [ ] Speaker → OUT+ và OUT-

- [ ] PTT Button → GPIO 4 và GND
- [ ] LED (+) → GPIO 2 qua điện trở 220Ω
- [ ] LED (-) → GND

### **Kiểm tra nguồn**:
- [ ] Tất cả GND nối chung
- [ ] INMP441 dùng 3.3V (KHÔNG dùng 5V!)
- [ ] MAX98357A dùng 5V (khuyến nghị) hoặc 3.3V

---

## 🔧 Lưu Ý Quan Trọng

### **1. Điện áp nguồn**
- ⚠️ **INMP441**: Chỉ dùng **3.3V**, KHÔNG nối 5V (sẽ hỏng!)
- ✅ **MAX98357A**: Khuyến nghị **5V** (âm lượng lớn hơn), có thể dùng 3.3V (âm lượng nhỏ hơn)

### **2. Nối chung BCLK và WS**
- ✅ GPIO 14 (BCLK) nối chung cho cả INMP441 (SCK) và MAX98357A (BCLK)
- ✅ GPIO 25 (LRCK) nối chung cho cả INMP441 (WS) và MAX98357A (LRC)
- ⚠️ Đây là thiết kế ĐÚNG, tiết kiệm GPIO

### **3. Tránh feedback (hồi âm)**
- Đặt Mic và Speaker **xa nhau** (>50cm)
- Hướng Mic và Speaker **ngược chiều**
- Giảm GAIN trên MAX98357A nếu bị hú

### **4. Chân L/R của INMP441**
- Nối **GND**: Chọn kênh Left (khuyến nghị)
- Nối **VDD**: Chọn kênh Right
- **Không để hở**: Sẽ không hoạt động ổn định

---

## 🧪 Test Sau Khi Hàn

### **Test 1: Kiểm tra nguồn**
```bash
# Bật nguồn ESP32
# Quan sát:
- LED trên ESP32 sáng
- LED trên INMP441 sáng (nếu có)
- LED trên MAX98357A sáng (nếu có)
```

### **Test 2: Kiểm tra PTT Button**
```bash
# Nhấn nút PTT
# Quan sát:
- LED Status (GPIO 2) sáng khi nhấn
- LED tắt khi thả
```

### **Test 3: Kiểm tra Audio Loopback**
```bash
# Flash firmware Phase 1
idf.py -p /dev/ttyUSB0 flash monitor

# Nói vào Mic
# Quan sát:
- Nghe thấy giọng mình từ Speaker (có echo nhẹ)
- Log hiển thị RMS thay đổi
```

---

## 🐛 Troubleshooting

### **Vấn đề 1: Không nghe thấy gì**
**Kiểm tra**:
- [ ] Speaker có kết nối đúng OUT+ và OUT-?
- [ ] MAX98357A có nguồn 5V?
- [ ] GAIN có hàn chưa? (thử hàn 1 chấm = 12dB)
- [ ] Chân DIN (GPIO 22) có nối đúng?

### **Vấn đề 2: RMS luôn bằng 0**
**Kiểm tra**:
- [ ] INMP441 có nguồn 3.3V?
- [ ] Chân SD (GPIO 32) có nối đúng?
- [ ] Chân L/R có nối GND?
- [ ] SCK và WS có nối đúng GPIO 14 và 15?

### **Vấn đề 3: Tiếng hú (feedback)**
**Giải pháp**:
- Đặt Mic và Speaker xa nhau
- Giảm GAIN (bỏ jumper hoặc hàn ít hơn)
- Che Mic hoặc Speaker bằng vật cách âm

### **Vấn đề 4: Âm thanh bị méo**
**Kiểm tra**:
- [ ] Speaker có đúng trở kháng 4Ω hoặc 8Ω?
- [ ] GAIN có quá cao không? (thử giảm)
- [ ] Nguồn 5V có đủ dòng? (cần ít nhất 500mA)

---

## 📸 Hình Ảnh Tham Khảo

### **INMP441 Module**
```
     ┌─────────────┐
     │   INMP441   │
     ├─────────────┤
VDD  │ ●           │  Mic
GND  │ ●        ┌──┤
SD   │ ●        │  │
WS   │ ●        └──┤
SCK  │ ●           │
L/R  │ ●           │
     └─────────────┘
```

### **MAX98357A Module**
```
     ┌─────────────┐
     │ MAX98357A   │
     ├─────────────┤
VIN  │ ●           │
GND  │ ●   [GAIN]  │  ← Jumper
DIN  │ ●   ● ● ●   │     9/12/15dB
BCLK │ ●           │
LRC  │ ●           │
SD   │ ●           │
     ├─────────────┤
OUT+ │ ●  Speaker  │
OUT- │ ●           │
     └─────────────┘
```

---

## 🎯 Tóm Tắt Nhanh

| Thiết bị | Chân | ESP32 GPIO | Ghi chú |
|----------|------|------------|---------|
| **INMP441** | SCK | 14 | CHUNG với Amp |
| | WS | 25 | CHUNG với Amp |
| | SD | 32 | Riêng |
| | L/R | GND | Chọn Left |
| | VDD | 3.3V | ⚠️ KHÔNG 5V! |
| **MAX98357A** | BCLK | 14 | CHUNG với Mic |
| | LRC | 25 | CHUNG với Mic |
| | DIN | 22 | Riêng |
| | VIN | 5V | Khuyến nghị |
| **PTT** | Button | 4 | Active Low |
| **LED** | Anode | 2 | Qua R 220Ω |

---

**Chúc bạn hàn mạch thành công!** 🚀
