# 🔌 Sơ Đồ Kết Nối Phần Cứng

## Sơ Đồ Đơn Giản - Hardware Connections

```mermaid
graph TB
    subgraph "Thiết Bị Walkie-Talkie"
        ESP32[ESP32-WROOM-32<br/>Vi điều khiển chính]
        
        MIC[🎤 INMP441<br/>Microphone I2S]
        SPK[🔊 MAX98357A<br/>Amplifier I2S]
        BTN[🔘 Nút PTT<br/>Push Button]
        LED[💡 LED Trạng Thái<br/>Status LED]
        
        SPEAKER[Loa 3W-8Ω<br/>Speaker]
    end
    
    BTN -->|GPIO 4| ESP32
    LED -->|GPIO 2| ESP32
    
    MIC -->|I2S Data| ESP32
    ESP32 -->|I2S Data| SPK
    SPK -->|Audio| SPEAKER
    
    ESP32 -.Wireless<br/>ESP-NOW.-> ESP32
    
    style ESP32 fill:#42b983
    style MIC fill:#4ecdc4
    style SPK fill:#ff6b6b
    style BTN fill:#ffd93d
    style LED fill:#ffd93d
```

---

## Sơ Đồ Chi Tiết - Pinout Connections

```mermaid
graph LR
    subgraph "ESP32 Pins"
        GPIO2[GPIO 2]
        GPIO4[GPIO 4]
        GPIO14[GPIO 14]
        GPIO15[GPIO 15]
        GPIO22[GPIO 22]
        GPIO32[GPIO 32]
        VCC[3.3V]
        GND[GND]
    end
    
    subgraph "Nút PTT"
        BTN_PIN[Pin 1]
        BTN_GND[Pin 2]
    end
    
    subgraph "LED"
        LED_ANODE[+ Anode]
        LED_RESISTOR[220Ω]
        LED_CATHODE[- Cathode]
    end
    
    subgraph "INMP441 Mic"
        MIC_SCK[SCK - Clock]
        MIC_WS[WS - Word Select]
        MIC_SD[SD - Data]
        MIC_VDD[VDD - Power]
        MIC_GND[GND]
    end
    
    subgraph "MAX98357A Amp"
        AMP_BCLK[BCLK - Clock]
        AMP_LRC[LRC - Word Select]
        AMP_DIN[DIN - Data]
        AMP_VDD[VDD - Power 5V]
        AMP_GND[GND]
        AMP_OUT_P[OUT+]
        AMP_OUT_N[OUT-]
    end
    
    subgraph "Loa"
        SPK_P[+ Terminal]
        SPK_N[- Terminal]
    end
    
    GPIO4 --> BTN_PIN
    BTN_GND --> GND
    
    GPIO2 --> LED_RESISTOR
    LED_RESISTOR --> LED_ANODE
    LED_CATHODE --> GND
    
    GPIO14 --> MIC_SCK
    GPIO14 --> AMP_BCLK
    GPIO15 --> MIC_WS
    GPIO15 --> AMP_LRC
    GPIO32 --> MIC_SD
    GPIO22 --> AMP_DIN
    
    VCC --> MIC_VDD
    GND --> MIC_GND
    GND --> AMP_GND
    
    AMP_OUT_P --> SPK_P
    AMP_OUT_N --> SPK_N
    
    style GPIO2 fill:#ffd93d
    style GPIO4 fill:#ffd93d
    style GPIO14 fill:#4ecdc4
    style GPIO15 fill:#4ecdc4
    style GPIO22 fill:#ff6b6b
    style GPIO32 fill:#4ecdc4
```

---

## Sơ Đồ Hoạt Động - Truyền Âm Thanh

```mermaid
sequenceDiagram
    actor User as 👤 Người Dùng
    participant BTN as 🔘 Nút PTT
    participant LED as 💡 LED
    participant MIC as 🎤 Mic
    participant ESP32 as 🔲 ESP32
    participant AIR as 📡 Wireless
    participant ESP32_B as 🔲 ESP32 (Nhận)
    participant SPK as 🔊 Loa
    
    User->>BTN: Nhấn và giữ
    BTN->>ESP32: Signal LOW
    ESP32->>LED: Bật LED
    LED-->>User: Sáng (đang truyền)
    
    User->>MIC: Nói vào mic
    MIC->>ESP32: Audio data (I2S)
    ESP32->>ESP32: Xử lý audio
    ESP32->>AIR: Phát sóng (ESP-NOW)
    
    AIR->>ESP32_B: Nhận tín hiệu
    ESP32_B->>SPK: Audio data (I2S)
    SPK-->>User: Phát ra âm thanh
    
    User->>BTN: Thả nút
    BTN->>ESP32: Signal HIGH
    ESP32->>LED: Tắt LED
    LED-->>User: Tắt (chế độ nhận)
```

---

## Bảng Kết Nối Nhanh

### ESP32 → Nút PTT
| ESP32 Pin | Nút PTT | Chức năng |
|-----------|---------|-----------|
| GPIO 4 | Pin 1 | Input (Pull-up) |
| GND | Pin 2 | Ground |

### ESP32 → LED
| ESP32 Pin | LED | Chức năng |
|-----------|-----|-----------|
| GPIO 2 | Anode (+) qua 220Ω | Output |
| GND | Cathode (-) | Ground |

### ESP32 → INMP441 Microphone
| ESP32 Pin | INMP441 | Chức năng |
|-----------|---------|-----------|
| GPIO 14 | SCK | I2S Clock |
| GPIO 15 | WS | Word Select |
| GPIO 32 | SD | Data Input |
| 3.3V | VDD | Power |
| GND | GND | Ground |
| - | L/R | GND (Left channel) |

### ESP32 → MAX98357A Amplifier
| ESP32 Pin | MAX98357A | Chức năng |
|-----------|-----------|-----------|
| GPIO 14 | BCLK | I2S Clock |
| GPIO 15 | LRC | Word Select |
| GPIO 22 | DIN | Data Output |
| 5V | VIN | Power (hoặc 3.3V) |
| GND | GND | Ground |

### MAX98357A → Loa
| MAX98357A | Loa | Chức năng |
|-----------|-----|-----------|
| OUT+ | + Terminal | Positive |
| OUT- | - Terminal | Negative |

---

## Sơ Đồ Vật Lý - Layout Đơn Giản

```mermaid
graph TB
    subgraph "Mặt Trước Thiết Bị"
        MIC_HOLE[🎤 Lỗ Mic<br/>INMP441]
        BTN_TOP[🔘 Nút PTT<br/>Nhấn để nói]
        LED_TOP[💡 LED<br/>Trạng thái]
        SPK_HOLE[🔊 Lỗ Loa<br/>Speaker]
    end
    
    subgraph "Bên Trong"
        ESP32_BOARD[ESP32<br/>Development Board]
        AMP_BOARD[MAX98357A<br/>Amplifier Module]
        SPEAKER_UNIT[Loa 3W 8Ω]
    end
    
    MIC_HOLE -.-> ESP32_BOARD
    BTN_TOP -.-> ESP32_BOARD
    LED_TOP -.-> ESP32_BOARD
    ESP32_BOARD -.-> AMP_BOARD
    AMP_BOARD -.-> SPEAKER_UNIT
    SPEAKER_UNIT -.-> SPK_HOLE
    
    style MIC_HOLE fill:#4ecdc4
    style BTN_TOP fill:#ffd93d
    style LED_TOP fill:#ffd93d
    style SPK_HOLE fill:#ff6b6b
    style ESP32_BOARD fill:#42b983
```

---

## Lưu Ý Quan Trọng

### ⚡ Nguồn Điện
- **ESP32:** 3.3V (từ USB hoặc pin)
- **INMP441:** 3.3V
- **MAX98357A:** 5V (hoặc 3.3V, nhưng 5V cho âm lượng lớn hơn)
- **Loa:** 3W 8Ω (hoặc 4Ω)

### 🔧 Lắp Ráp
1. Kết nối INMP441 vào ESP32 (I2S RX)
2. Kết nối MAX98357A vào ESP32 (I2S TX)
3. Kết nối loa vào MAX98357A
4. Kết nối nút PTT vào GPIO 4
5. Kết nối LED vào GPIO 2 (qua điện trở 220Ω)

### ⚠️ Chú Ý
- **I2S Clock chung:** GPIO 14 và GPIO 15 dùng chung cho cả mic và loa
- **Pull-up:** Nút PTT cần pull-up (đã enable internal pull-up)
- **Điện trở LED:** Bắt buộc dùng 220Ω để bảo vệ LED
- **Gain:** MAX98357A có jumper để điều chỉnh gain (9dB hoặc 15dB)

---

**Xem thêm:**
- [Sơ Đồ Phức Tạp →](system.md)
- [Wiring Guide →](../../WIRING_GUIDE.md)
