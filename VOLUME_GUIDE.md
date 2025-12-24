# Hướng Dẫn Tăng Âm Lượng Loa

## 🔊 3 Cách Tăng Âm Lượng

### **1. Hardware Gain (MAX98357A Module)** ⭐ Khuyến nghị

**Ưu điểm**: Không cần sửa code, tăng ngay lập tức  
**Nhược điểm**: Cần hàn lại module

#### **Bảng GAIN**:

| Jumper | Gain | Âm lượng | Méo tiếng? | Khuyến nghị |
|--------|------|----------|------------|-------------|
| Không hàn | 9dB | Vừa phải | ❌ Không | ✅ Mặc định |
| Hàn 1 chấm | 12dB | Lớn (+50%) | ❌ Không | ✅ Tốt |
| Hàn 2 chấm | 15dB | Rất lớn (+100%) | ⚠️ Có thể | ⚠️ Cẩn thận |
| Hàn 3 chấm | 18dB | Cực lớn (+150%) | ✅ Có | ❌ Không nên |

#### **Vị trí jumper trên MAX98357A**:

```
Mặt trên module:
┌─────────────────────┐
│  MAX98357A          │
│                     │
│  [GAIN Jumpers]     │
│   ● ● ●             │  ← 3 chấm hàn
│   1 2 3             │
│                     │
│  VIN  GND  DIN      │
│  BCLK LRC  SD       │
└─────────────────────┘

Cách hàn:
- 9dB (mặc định):   ● ● ●    (không hàn gì)
- 12dB (+3dB):      ●─● ●    (hàn chấm 1-2)
- 15dB (+6dB):      ● ●─●    (hàn chấm 2-3)
- 18dB (+9dB):      ●─●─●    (hàn cả 3 chấm)
```

#### **Cách hàn**:

1. **Xác định chấm cần hàn** (xem bảng trên)
2. **Dùng thiếc nhỏ** nối 2 chấm lại với nhau
3. **Kiểm tra**: Dùng đồng hồ vạn năng đo continuity
4. **Test**: Flash lại firmware và nghe thử

---

### **2. Software Gain (Trong Code)** 🎚️ Linh hoạt

**Ưu điểm**: Linh hoạt, điều chỉnh dễ dàng  
**Nhược điểm**: Tốn CPU, có thể méo nếu gain quá cao

#### **Cách bật Software Gain**:

Mở file `main/app_config.h` và thay đổi 2 dòng sau:

```c
// Tìm dòng này (gần cuối file):
#define SOFTWARE_GAIN_ENABLE  0     // 0 = TẮT, 1 = BẬT
#define SOFTWARE_GAIN_DB      6.0f  // 0=không đổi, 6=2x, 12=4x, 18=8x

// Đổi thành:
#define SOFTWARE_GAIN_ENABLE  1     // ← Đổi 0 thành 1 để BẬT
#define SOFTWARE_GAIN_DB      6.0f  // ← Thay đổi giá trị này để điều chỉnh
```

**Ví dụ**:
```c
// Tăng gấp đôi (2x):
#define SOFTWARE_GAIN_ENABLE  1
#define SOFTWARE_GAIN_DB      6.0f

// Tăng gấp 4 lần (4x):
#define SOFTWARE_GAIN_ENABLE  1
#define SOFTWARE_GAIN_DB      12.0f

// Tắt software gain:
#define SOFTWARE_GAIN_ENABLE  0
#define SOFTWARE_GAIN_DB      6.0f  // Giá trị này không quan trọng khi TẮT
```

#### **Bảng Giá Trị GAIN_DB**:

| GAIN_DB | Âm lượng | Méo tiếng? | Khuyến nghị |
|---------|----------|------------|-------------|
| 0.0f | Không đổi | ❌ Không | Mặc định |
| 3.0f | +40% | ❌ Không | ✅ Tốt |
| 6.0f | +100% (2x) | ❌ Không | ✅ Tốt |
| 9.0f | +180% (2.8x) | ⚠️ Có thể | ⚠️ Cẩn thận |
| 12.0f | +300% (4x) | ✅ Có | ❌ Không nên |

#### **Công thức**:

```
Gain Linear = 10^(GAIN_DB / 20)

Ví dụ:
- GAIN_DB = 6  → Gain = 10^(6/20) = 2.0x (gấp đôi)
- GAIN_DB = 12 → Gain = 10^(12/20) = 4.0x (gấp 4)
```

#### **Sau khi sửa**:

```bash
# Rebuild và flash
cd /home/iot/Lumi/Speckit/speckit_bo_dam
idf.py build flash monitor
```

---

### **3. Kết Hợp Hardware + Software** 🚀 Mạnh nhất

**Ví dụ**: Hardware 12dB + Software 6dB = **18dB tổng cộng**

**Lưu ý**: 
- ⚠️ Dễ bị méo tiếng nếu gain quá cao
- ✅ Nên bắt đầu với Hardware gain trước
- ✅ Chỉ dùng Software gain nếu vẫn chưa đủ

---

## 📊 So Sánh 3 Cách

| Tiêu chí | Hardware Gain | Software Gain | Kết hợp |
|----------|---------------|---------------|---------|
| **Dễ làm** | ⚠️ Cần hàn | ✅ Chỉ sửa code | ⚠️ Cả 2 |
| **Linh hoạt** | ❌ Cố định | ✅ Dễ điều chỉnh | ✅ Rất linh hoạt |
| **Chất lượng** | ✅ Tốt nhất | ⚠️ Có thể méo | ⚠️ Dễ méo |
| **Tốn CPU** | ✅ Không | ❌ Có | ❌ Có |
| **Khuyến nghị** | ✅ Thử đầu tiên | ✅ Nếu chưa đủ | ⚠️ Cẩn thận |

---

## 🎯 Hướng Dẫn Từng Bước

### **Bước 1: Thử Hardware Gain (12dB)**

1. Tắt nguồn ESP32
2. Hàn jumper 1-2 trên MAX98357A (12dB)
3. Bật nguồn và test
4. **Nếu đủ lớn** → Xong!
5. **Nếu vẫn nhỏ** → Sang bước 2

### **Bước 2: Thêm Software Gain (6dB)**

1. Mở `main/audio_driver.c`
2. Tìm hàm `audio_driver_write()`
3. Bỏ comment 4 dòng:
   ```c
   #define GAIN_DB 6.0f
   int16_t *samples = (int16_t *)buffer;
   size_t sample_count = len / sizeof(int16_t);
   apply_gain(samples, sample_count, GAIN_DB);
   ```
4. Rebuild và flash:
   ```bash
   idf.py build flash monitor
   ```
5. Test lại
6. **Nếu đủ lớn** → Xong!
7. **Nếu vẫn nhỏ** → Tăng `GAIN_DB` lên 9.0f hoặc 12.0f

### **Bước 3: Nếu Vẫn Nhỏ**

**Kiểm tra**:
- [ ] Speaker có đúng 4Ω hoặc 8Ω?
- [ ] MAX98357A có nguồn 5V? (không phải 3.3V)
- [ ] Kết nối OUT+ và OUT- đúng chưa?
- [ ] Mic có thu âm đủ lớn không? (xem RMS log)

**Nếu Mic thu âm quá nhỏ**:
- Đặt Mic gần nguồn âm hơn
- Kiểm tra kết nối INMP441
- Thử tăng gain trong `audio_driver_read()` (tương tự `write`)

---

## ⚠️ Lưu Ý Quan Trọng

### **1. Méo Tiếng (Clipping)**

**Triệu chứng**: Âm thanh bị rè, méo, không rõ ràng

**Nguyên nhân**: Gain quá cao → sample vượt quá giới hạn ±32767

**Giải pháp**:
- Giảm GAIN_DB xuống
- Hoặc giảm Hardware gain
- Code đã có **clipping protection** (tự động giới hạn ±32767)

### **2. Nhiễu (Noise)**

**Triệu chứng**: Tiếng xì xào khi im lặng

**Nguyên nhân**: Gain cao → khuếch đại cả nhiễu

**Giải pháp**:
- Dùng nguồn 5V ổn định cho MAX98357A
- Nối GND chung tốt
- Tránh đặt dây audio gần dây nguồn

### **3. Feedback (Hồi Âm)**

**Triệu chứng**: Tiếng hú, rít

**Nguyên nhân**: Mic thu lại âm từ Speaker

**Giải pháp**:
- Đặt Mic và Speaker xa nhau
- Giảm gain
- Mute Speaker khi TX (Phase 4)

---

## 🧪 Test Âm Lượng

### **Test 1: Kiểm tra RMS**

```bash
# Chạy firmware và quan sát log
idf.py monitor

# Nói vào mic, xem RMS:
I (xxx) MAIN: Audio RMS: 0.1234 (bytes: 512)

# RMS tốt:
- Im lặng: 0.001 - 0.01
- Nói bình thường: 0.05 - 0.15
- Nói to: 0.15 - 0.5
```

### **Test 2: Nghe Thử**

1. Nói vào Mic với âm lượng bình thường
2. Nghe từ Speaker:
   - ✅ **Rõ ràng, không méo** → Gain tốt
   - ⚠️ **Hơi nhỏ** → Tăng gain
   - ❌ **Méo tiếng** → Giảm gain

---

## 📝 Tóm Tắt

### **Khuyến nghị**:

1. **Bắt đầu**: Hardware gain 12dB (hàn jumper 1-2)
2. **Nếu chưa đủ**: Thêm Software gain 6dB
3. **Nếu vẫn chưa đủ**: Tăng Software gain lên 9dB hoặc 12dB
4. **Nếu bị méo**: Giảm gain xuống

### **Công thức vàng**:

```
Tổng Gain = Hardware Gain + Software Gain

Ví dụ tốt:
- Hardware 12dB + Software 6dB = 18dB (rất lớn, có thể méo)
- Hardware 12dB + Software 3dB = 15dB (lớn, ít méo)
- Hardware 9dB + Software 6dB = 15dB (lớn, ít méo)

Ví dụ an toàn:
- Hardware 12dB + Software 0dB = 12dB (vừa phải)
- Hardware 9dB + Software 3dB = 12dB (vừa phải)
```

---

**Chúc bạn điều chỉnh âm lượng thành công!** 🔊
