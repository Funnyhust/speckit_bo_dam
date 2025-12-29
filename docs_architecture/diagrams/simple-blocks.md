# 📐 Sơ Đồ Khối Đơn Giản - ESP32 Walkie-Talkie

## Sơ Đồ Khối Hệ Thống

```mermaid
graph TD
    A[Nút PTT<br/>Push-to-Talk] --> B[Khối VDK<br/>ESP32-WROOM-32]
    C[Khối Nguồn<br/>5V/3.3V] --> B
    D[Khối Cảm Biến<br/>INMP441 Microphone] --> B
    
    B --> E[Khối Công Suất<br/>MAX98357A Amplifier]
    B --> F[Khối Đồng Cơ<br/>ESP-NOW Wireless]
    B --> G[Khối Hiển Thị<br/>LED Status]
    
    E --> H[Loa 3W 8Ω<br/>Speaker Output]
    
    style B fill:#42b983,stroke:#333,stroke-width:3px
    style F fill:#ff6b6b,stroke:#333,stroke-width:2px
    style D fill:#4ecdc4,stroke:#333,stroke-width:2px
    style E fill:#ffd93d,stroke:#333,stroke-width:2px
```

---

## Sơ Đồ Khối Chi Tiết - Thiết Bị Phát (TX)

```mermaid
graph LR
    A[Nút PTT] --> B[ESP32<br/>GPIO 4]
    C[INMP441<br/>Microphone] --> D[ESP32<br/>I2S RX]
    
    D --> E[ESP32<br/>Audio Processing]
    E --> F[ESP32<br/>ESP-NOW TX]
    
    F --> G[Phát sóng<br/>2.4GHz WiFi]
    
    B --> H[ESP32<br/>PTT Control]
    H --> I[LED Status<br/>GPIO 2]
    
    style B fill:#42b983
    style D fill:#42b983
    style E fill:#42b983
    style F fill:#ff6b6b
    style H fill:#42b983
```

---

## Sơ Đồ Khối Chi Tiết - Thiết Bị Nhận (RX)

```mermaid
graph LR
    A[Nhận sóng<br/>2.4GHz WiFi] --> B[ESP32<br/>ESP-NOW RX]
    
    B --> C[ESP32<br/>Audio Processing]
    C --> D[ESP32<br/>I2S TX]
    
    D --> E[MAX98357A<br/>Amplifier]
    E --> F[Loa 3W<br/>Speaker]
    
    B --> G[ESP32<br/>Status Control]
    G --> H[LED Status<br/>GPIO 2]
    
    style B fill:#ff6b6b
    style C fill:#42b983
    style D fill:#42b983
    style E fill:#ffd93d
```

---

## Sơ Đồ Khối Đầy Đủ - Cả 2 Thiết Bị

```mermaid
graph TB
    subgraph "Thiết Bị A - Người Nói"
        A1[Nút PTT] --> A2[ESP32]
        A3[Mic INMP441] --> A2
        A2 --> A4[LED]
        A2 --> A5[ESP-NOW TX]
    end
    
    A5 -.Wireless<br/>2.4GHz.-> B5
    
    subgraph "Thiết Bị B - Người Nghe"
        B5[ESP-NOW RX] --> B2[ESP32]
        B2 --> B3[MAX98357A]
        B3 --> B4[Loa]
        B2 --> B6[LED]
    end
    
    style A2 fill:#42b983
    style B2 fill:#42b983
    style A5 fill:#ff6b6b
    style B5 fill:#ff6b6b
```

---

## Sơ Đồ Khối Theo Chức Năng

```mermaid
graph TD
    subgraph "Khối Đầu Vào"
        IN1[Nút PTT<br/>GPIO Input]
        IN2[Microphone<br/>I2S Input]
    end
    
    subgraph "Khối Xử Lý Trung Tâm"
        CPU[ESP32<br/>Vi Điều Khiển]
        PROC[Audio Processing<br/>Buffer + Gain]
    end
    
    subgraph "Khối Truyền Thông"
        WIFI[ESP-NOW<br/>Wireless Protocol]
    end
    
    subgraph "Khối Đầu Ra"
        OUT1[LED Status<br/>GPIO Output]
        OUT2[Amplifier<br/>I2S Output]
        OUT3[Speaker<br/>Audio Output]
    end
    
    IN1 --> CPU
    IN2 --> CPU
    CPU --> PROC
    PROC --> WIFI
    WIFI -.Wireless.-> WIFI
    WIFI --> PROC
    PROC --> CPU
    CPU --> OUT1
    CPU --> OUT2
    OUT2 --> OUT3
    
    style CPU fill:#42b983
    style WIFI fill:#ff6b6b
    style PROC fill:#4ecdc4
```

---

## Bảng Mô Tả Các Khối

| Khối | Tên | Chức Năng | Linh Kiện |
|------|-----|-----------|-----------|
| **Khối VDK** | Vi Điều Khiển | Xử lý trung tâm, điều khiển toàn bộ hệ thống | ESP32-WROOM-32 |
| **Khối Nguồn** | Nguồn Điện | Cấp nguồn 5V và 3.3V | USB hoặc Pin |
| **Khối Cảm Biến** | Microphone | Thu âm thanh, chuyển thành tín hiệu số | INMP441 I2S |
| **Khối Công Suất** | Amplifier | Khuếch đại tín hiệu âm thanh | MAX98357A |
| **Khối Đồng Cơ** | Wireless | Truyền/nhận dữ liệu không dây | ESP-NOW Protocol |
| **Khối Hiển Thị** | LED Status | Hiển thị trạng thái hoạt động | LED đơn |
| **Khối Đầu Vào** | PTT Button | Điều khiển chế độ phát/nhận | Nút nhấn |
| **Khối Đầu Ra** | Speaker | Phát âm thanh ra ngoài | Loa 3W 8Ω |

---

## Sơ Đồ Luồng Dữ Liệu

```mermaid
graph LR
    A[Mic] -->|I2S Audio| B[ESP32]
    B -->|Packet 244B| C[ESP-NOW]
    C -->|Wireless| D[ESP-NOW]
    D -->|Packet 244B| E[ESP32]
    E -->|I2S Audio| F[Amp]
    F -->|Audio| G[Loa]
    
    style B fill:#42b983
    style E fill:#42b983
    style C fill:#ff6b6b
    style D fill:#ff6b6b
```

---

**Lưu ý:** Các sơ đồ trên được vẽ theo phong cách đơn giản, dễ hiểu, phù hợp để trình bày hoặc làm tài liệu kỹ thuật.
