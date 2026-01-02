# Hiến Chương Bộ Đàm ESP32 ESP-NOW

## Tech Stack (KHÔNG THỂ THƯƠNG LƯỢNG)

### I. Nền Tảng Phần Cứng
- **MCU:** ESP32-WROOM-32
- **Đầu Vào Audio:** INMP441 (I2S MEMS Microphone - I2S Master Mode)
- **Đầu Ra Audio:** MAX98357A (I2S Class-D Amplifier - I2S Master Mode)
- **Giao Thức Truyền Thông:** ESP-NOW (Broadcast Mode)
  - **Yêu Cầu Độ Trễ:** < 100ms đầu cuối
  - **Lý Do:** Chế độ WiFi AP/STA gây ra độ trễ không chấp nhận được cho giọng nói thời gian thực

### II. Ngăn Xếp Phần Mềm
- **SDK:** ESP-IDF v5.1 hoặc mới hơn
- **Hệ Thống Build:** CMake (ESP-IDF native)
- **Ngôn Ngữ Lập Trình:** C (C11 Standard)
- **RTOS:** FreeRTOS (tích hợp sẵn trong ESP-IDF)

## Tiêu Chuẩn Kỹ Thuật Không Thể Thương Lượng

### III. Kiến Trúc Audio Pipeline (QUAN TRỌNG)
> [!CAUTION]
> Vi phạm các quy tắc này sẽ dẫn đến lỗi audio, tràn buffer, hoặc mất ổn định hệ thống.

- **DMA Bắt Buộc:** Tất cả truyền audio I2S PHẢI sử dụng DMA. Nghiêm cấm polling CPU.
- **Quản Lý Buffer:** 
  - Sử dụng double-buffering hoặc ring buffer cho streaming audio liên tục
  - Kích thước buffer phải là lũy thừa của 2 để tối ưu hiệu suất DMA
  - Kích thước buffer tối thiểu: 512 mẫu (có thể cấu hình dựa trên yêu cầu độ trễ)
- **Tốc Độ Lấy Mẫu:** 16 kHz (có thể cấu hình, nhưng phải khớp ở cả phía TX/RX)
- **Độ Sâu Bit:** 16-bit signed PCM

### IV. Mô Hình Đồng Thời (BẮT BUỘC)
- **Phân Tách Task:**
  - `audio_rx_task`: I2S microphone DMA → truyền ESP-NOW
  - `audio_tx_task`: nhận ESP-NOW → I2S speaker DMA
  - `wifi_task`: khởi tạo ESP-NOW, quản lý peer, sự kiện mạng
- **Giao Tiếp Giữa Các Task:**
  - Sử dụng FreeRTOS Queues để truyền dữ liệu audio giữa các task
  - Sử dụng Event Groups để đồng bộ hóa (ví dụ: "sẵn sàng truyền", "peer đã kết nối")
- **Phân Bổ Ưu Tiên:**
  - Audio tasks: Ưu tiên cao (có thể cấu hình, thường là `configMAX_PRIORITIES - 2`)
  - WiFi task: Ưu tiên trung bình

### V. Quản Lý GPIO (NGHIÊM NGẶT)
- **Định Nghĩa Pin Tập Trung:** Tất cả số pin GPIO PHẢI được định nghĩa trong `board_pinout.h`
- **Không Số Ma Thuật:** Không bao giờ hard-code số pin trong file `.c`
- **Cấu Trúc Ví Dụ:**
  ```c
  // board_pinout.h
  #define I2S_MIC_SCK_PIN    GPIO_NUM_26
  #define I2S_MIC_WS_PIN     GPIO_NUM_25
  #define I2S_MIC_SD_PIN     GPIO_NUM_33
  ```

### VI. Quản Lý Bộ Nhớ (QUAN TRỌNG)
> [!WARNING]
> Cấp phát bộ nhớ động trong đường dẫn audio thời gian thực sẽ gây ra jitter và dropout.

- **Ưu Tiên Cấp Phát Tĩnh:** Sử dụng buffer tĩnh cho dữ liệu audio bất cứ khi nào có thể
- **Quy Tắc Cấp Phát Heap:**
  - Cho phép: Khởi tạo một lần (ví dụ: `malloc` trong `app_main`)
  - Cấm: `malloc`/`free` trong vòng lặp xử lý audio hoặc ISR
- **DMA Buffers:** Phải được cấp phát trong bộ nhớ hỗ trợ DMA sử dụng `heap_caps_malloc(size, MALLOC_CAP_DMA)`

### VII. Xử Lý Lỗi (BẮT BUỘC)
- **Mã Trả Về ESP-IDF:** Tất cả hàm trả về `esp_err_t` PHẢI được kiểm tra với `ESP_ERROR_CHECK()` hoặc xử lý lỗi tường minh
- **Ví Dụ:**
  ```c
  esp_err_t ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  ESP_ERROR_CHECK(ret);  // Hệ thống sẽ abort khi lỗi với thông báo chẩn đoán
  ```
- **Logging:** Sử dụng macro logging ESP-IDF (`ESP_LOGI`, `ESP_LOGE`, v.v.) với mức log phù hợp

## Tổ Chức Code

### VIII. Cấu Trúc Dự Án
```
doppler/
├── main/
│   ├── main.c                 # Entry point, task creation
│   ├── board_pinout.h         # GPIO pin definitions
│   ├── audio/
│   │   ├── i2s_mic.c/h       # Microphone driver
│   │   ├── i2s_speaker.c/h   # Speaker driver
│   │   └── audio_pipeline.c/h # Audio processing
│   ├── network/
│   │   ├── espnow_tx.c/h     # ESP-NOW transmitter
│   │   └── espnow_rx.c/h     # ESP-NOW receiver
│   └── config/
│       └── app_config.h       # Application-wide constants
├── CMakeLists.txt
└── sdkconfig                  # ESP-IDF configuration
```

### IX. Quy Ước Đặt Tên
- **Files:** `snake_case.c`, `snake_case.h`
- **Functions:** `module_action()` (ví dụ: `i2s_mic_init()`, `espnow_send_audio()`)
- **Macros/Constants:** `UPPER_SNAKE_CASE`
- **Types:** `snake_case_t` (ví dụ: `audio_buffer_t`)

## Yêu Cầu Hiệu Suất

### X. Ngân Sách Độ Trễ
| Thành Phần | Độ Trễ Tối Đa | Ghi Chú |
|-----------|-------------|----------|
| I2S Mic Capture | 20ms | Phụ thuộc kích thước DMA buffer |
| ESP-NOW TX | 10ms | Thường 2-5ms |
| ESP-NOW RX | 10ms | Bao gồm xử lý callback |
| I2S Speaker Playback | 20ms | Phụ thuộc kích thước DMA buffer |
| **Tổng Đầu Cuối** | **< 100ms** | **Yêu cầu cứng** |

### XI. Chất Lượng Audio
- **SNR Tối Thiểu:** 40 dB (giới hạn bởi phần cứng INMP441)
- **Xử Lý Mất Gói:** Triển khai chèn im lặng cho các gói bị mất (không có tiếng nổ)
- **Jitter Buffer:** Tùy chọn buffer 1-2 gói để bù jitter mạng

## Quản Trị

### XII. Thẩm Quyền Hiến Chương
- Hiến chương này thay thế tất cả các thực hành hoặc sở thích lập trình khác
- Bất kỳ sai lệch nào PHẢI được ghi chép với lý do kỹ thuật và được phê duyệt
- Sửa đổi yêu cầu:
  1. Đề xuất bằng văn bản với lý do
  2. Phân tích tác động lên code hiện có
  3. Kế hoạch di chuyển (nếu có)

### XIII. Danh Sách Kiểm Tra Code Review
Tất cả code gửi lên phải xác minh:
- [ ] Không có GPIO pin hard-coded (kiểm tra `board_pinout.h`)
- [ ] Không có `malloc` trong vòng lặp audio
- [ ] Tất cả giá trị trả về `esp_err_t` được kiểm tra
- [ ] DMA được sử dụng cho tất cả truyền I2S
- [ ] FreeRTOS tasks được ưu tiên hóa đúng cách
- [ ] Logging sử dụng macro ESP-IDF (không dùng `printf`)

**Phiên Bản**: 1.0.0 | **Phê Chuẩn**: 2025-12-21 | **Sửa Đổi Lần Cuối**: 2025-12-21
