# SPECKIT GUIDE: Quy trình Phát triển Phần mềm với AI

Tài liệu hướng dẫn quy trình sử dụng Speckit để chuẩn hóa cách làm việc với AI Assistant (Cursor, Copilot, Gemini...), giúp kiểm soát chất lượng code và kiến trúc dự án ngay từ đầu.

---

## MỤC LỤC
1. [Giới thiệu chung](#1-giới-thiệu-chung)
2. [Cấu trúc lõi của Speckit](#2-cấu-trúc-lõi-của-speckit)
3. [Quy trình sử dụng (Workflow)](#3-quy-trình-sử-dụng-workflow)
4. [Case Study Mẫu: ESP32 Walkie-Talkie](#4-case-study-mẫu-esp32-walkie-talkie-chi-tiết)
5. [Tổng kết & Đánh giá](#5-tổng-kết--đánh-giá)

---

## 1. Giới thiệu chung
[cite_start]**Speckit** là một công cụ và phương pháp luận giúp lập trình viên làm việc với AI một cách hiệu quả, có quy trình và quy tắc chung[cite: 3].

### Vấn đề giải quyết:
Bình thường, khi hỏi AI rời rạc, code sinh ra thường:
* Sai kiến trúc hoặc thư viện mong muốn.
* Thiếu nhất quán giữa các file.
* Mất ngữ cảnh khi đoạn chat quá dài.

### Giải pháp:
[cite_start]Speckit yêu cầu tạo ra các file ngữ cảnh ("Bộ não dự án") trước khi code[cite: 4]. Mỗi khi AI thực hiện hành động, nó sẽ đọc qua các file này để đảm bảo tuân thủ tuyệt đối ý đồ của lập trình viên.

---

## 2. Cấu trúc lõi của Speckit
[cite_start]Một dự án Speckit tiêu chuẩn bao gồm 4 file ngữ cảnh quan trọng cần được tạo ra theo thứ tự[cite: 5]:

### [cite_start]1. `constitution.md` (Hiến pháp dự án) [cite: 6]
* **Chức năng:** Chứa các nguyên tắc kỹ thuật "Bất khả xâm phạm" (Non-negotiable rules).
* **Ví dụ:** Tech Stack (ESP-IDF, React, Python...), Coding Convention, các thư viện bị cấm sử dụng.

### [cite_start]2. `spec.md` (Đặc tả yêu cầu) [cite: 7]
* **Chức năng:** Mô tả chi tiết yêu cầu nghiệp vụ, Input/Output, User Stories.
* **Mục tiêu:** Giúp AI hiểu "Sản phẩm này làm cái gì, hoạt động ra sao".

### [cite_start]3. `plan.md` (Kế hoạch kỹ thuật) [cite: 8, 9]
* **Chức năng:** Chuyển đổi yêu cầu thành giải pháp kỹ thuật.
* **Nội dung:** Kiến trúc hệ thống, sơ đồ luồng dữ liệu (Data Flow), cấu trúc thư mục, định nghĩa các Module.

### [cite_start]4. `tasks.md` (Danh sách nhiệm vụ) [cite: 10]
* **Chức năng:** Chia nhỏ `plan.md` thành một Checklist công việc cụ thể (To-do list).
* **Mục tiêu:** AI sẽ code lần lượt từng dòng trong checklist này để tránh bỏ sót hoặc gây lỗi dây chuyền.

---

## 3. Quy trình sử dụng (Workflow)

### [cite_start]Bước 1: Khởi tạo dự án [cite: 14, 15]
Cài đặt/Clone repo speckit về dự án (Yêu cầu đã cài đặt `uv` hoặc `python`):

```bash
# Cách 1: Tạo dự án mới
uvx --from git+https://github.com/github/spec-kit.git specify init <PROJECT_NAME>

# Cách 2: Tích hợp vào thư mục hiện tại
uvx --from git+https://github.com/github/spec-kit.git specify init --here
```

### Bước 2: Lựa chọn Assistant

Lựa chọn Assistant bạn đang dùng: Cursor, Copilot, hoặc Gemini.

> **Lưu ý:** Việc chọn đúng Assistant rất quan trọng vì mỗi con AI có cách quản lý ngữ cảnh (Context) và thao tác file khác nhau.

### Bước 3: Phát triển tài liệu đặc tả (Prompting)

Sử dụng cửa sổ chat của IDE, gõ lệnh theo thứ tự:

1. `/constitution` + [Yêu cầu Tech Stack/Luật] → AI tạo file `constitution.md`
2. `/specify` + [Yêu cầu chức năng] → AI tạo file `spec.md`
3. `/plan` + [Yêu cầu kiến trúc] → AI tạo file `plan.md`
4. `/tasks` + [Yêu cầu lộ trình] → AI tạo file `tasks.md`

### Bước 4: Thực thi code (Implementation)

Gõ lệnh: `/implement` + "Thực hiện Task số X"

AI sẽ tự động đọc `tasks.md`, viết code, và yêu cầu bạn xác nhận (tick vào checkbox) khi hoàn thành.

---

## 4. Case Study Mẫu: ESP32 Walkie-Talkie (Chi tiết)

Dưới đây là toàn bộ prompt mẫu đã sử dụng để build thành công firmware bộ đàm ESP32.

### Giai đoạn 1: Thiết lập Hiến pháp (`/constitution`)

**Input Prompt:**

```
Chúng ta sẽ bắt đầu dự án firmware: ESP32 ESP-NOW Walkie-Talkie. Tôi muốn bạn đóng vai trò là Lead Embedded Software Engineer. Trước tiên, hãy thiết lập Constitution (Hiến pháp dự án) với các quy tắc bất di bất dịch sau:

1. Tech Stack
   - MCU: ESP32-WROOM-32
   - Audio Hardware: INMP441 (Mic - I2S Master), MAX98357A (Amp - I2S Master)
   - Protocol: ESP-NOW (Broadcast mode) - vì cần độ trễ thấp < 100ms
   - SDK: ESP-IDF v5.1+ (Sử dụng CMake)
   - Language: C (C11 Standard)

2. Non-negotiable Engineering Standards (Quy tắc bắt buộc)
   - Audio Pipeline: Bắt buộc sử dụng DMA cho I2S. Tuyệt đối không dùng CPU polling để đọc/ghi âm thanh
   - Concurrency: Sử dụng FreeRTOS (Task, Queue, Event Group). Tách biệt Task Audio và Task Wifi
   - GPIO Management: Không hard-code số chân trong file logic. Phải định nghĩa toàn bộ trong board_pinout.h
   - Memory: Hạn chế malloc trong vòng lặp chính. Sử dụng Static Allocation hoặc khởi tạo buffer một lần duy nhất
   - Error Handling: Mọi hàm ESP-IDF trả về esp_err_t phải được kiểm tra bằng macro ESP_ERROR_CHECK()

Hãy xác nhận bạn đã hiểu Hiến pháp này.
```

### Giai đoạn 2: Đặc tả kỹ thuật (`/specify`)

**Input Prompt:**

```
Dựa trên Hiến pháp đã duyệt, hãy viết Đặc tả kỹ thuật (Spec) cho hệ thống.

1. Sơ đồ phần cứng (Pin Mapping): Chúng ta sử dụng I2S Port 0 ở chế độ Full-Duplex (hoặc Simplex nếu cần thiết, hãy đề xuất tối ưu nhất cho ESP32 WROOM)
   - Common Clock: I2S_BCLK (GPIO 14), I2S_LRCK (GPIO 15)
   - Mic INMP441 (Input): I2S_SD_IN (GPIO 32)
   - Amp MAX98357A (Output): I2S_SD_OUT (GPIO 22)
   - Controls: PTT Button (Push-to-Talk): GPIO 4 (Active Low), Status LED: GPIO 2

2. Audio Configuration:
   - Sample Rate: 16,000 Hz
   - Resolution: 16-bit
   - Channel: Mono

3. User Stories & Logic:
   - Chế độ RX (Mặc định): Luôn lắng nghe gói tin ESP-NOW → Giải mã → Đẩy vào DMA buffer → Phát ra Loa
   - Chế độ TX (Khi nhấn giữ nút PTT):
     * Ngắt Loa (Mute Speaker) để chống hú
     * Đọc Mic liên tục → Nén dữ liệu đơn giản (nếu cần) → Chia nhỏ thành gói tin < 250 bytes
     * Gửi Broadcast ESP-NOW
     * Bật đèn LED báo hiệu đang nói
   - Khi nhả nút PTT: Quay lại chế độ RX ngay lập tức
```

### Giai đoạn 3: Lập kế hoạch (`/plan`)

**Input Prompt:**

```
Hãy lập Kế hoạch triển khai (Technical Plan) chi tiết.

1. Kiến trúc Firmware:
   - Vẽ sơ đồ luồng dữ liệu (Data Flow) giữa: Mic → I2S Read Task → Ring Buffer → ESP-NOW Task
   - Định nghĩa cấu trúc gói tin (Packet Structure) sẽ gửi qua mạng (bao gồm Header để đánh dấu thứ tự gói tin, tránh Jitter)

2. Cấu trúc Thư mục (File Structure):
   - Tôi muốn tách module rõ ràng: main.c, audio_driver.c/h (xử lý I2S), wifi_transport.c/h (xử lý ESP-NOW), board_pinout.h

3. Chiến lược bộ nhớ:
   - Tính toán kích thước Buffer DMA và Ring Buffer cần thiết để đảm bảo độ trễ thấp nhưng không bị đứt quãng âm thanh
```

### Giai đoạn 4: Chia task (`/tasks`)

**Input Prompt:**

```
Hãy chia dự án thành danh sách các Nhiệm vụ (Tasks checklist) để thực hiện lần lượt. Tôi đề xuất lộ trình sau, hãy bổ sung chi tiết kỹ thuật cho từng bước:

- Task 1: Audio Loopback Test (Quan trọng nhất). Chưa cần Wifi. Đọc Mic → DMA → Phát thẳng ra Loa. Mục tiêu: Kiểm tra nhiễu, volume và kết nối phần cứng
- Task 2: ESP-NOW Communication. Viết driver gửi/nhận struct dữ liệu giả lập. Test kết nối giữa 2 mạch
- Task 3: Audio Transport Integration. Kết nối Audio vào Wifi. Xử lý việc chia cắt (fragmentation) và ghép lại gói tin audio
- Task 4: PTT Logic & Optimization. Thêm nút bấm, LED và tinh chỉnh Ring Buffer để mượt tiếng
```

### Giai đoạn 5: Thực thi (`/implement`)

Sau khi có file `tasks.md`, lần lượt ra lệnh: `/implement Task 1`, `/implement Task 2`...

**Kết quả:**
- Các phase tiến hành mượt mà, lỗi biên dịch được fix nhanh
- Thiết bị hoạt động đúng spec: Nhấn PTT là nói, thả ra là nghe, độ trễ thấp

---

## 5. Tổng kết & Đánh giá

### Ưu điểm:

- **Hiểu dự án nhanh:** Người mới chỉ cần đọc 4 file Speckit là nắm được toàn bộ hệ thống (Tech stack, Pinout, Logic) mà không cần đọc từng dòng code
- **Code chất lượng:** Do có "Hiến pháp", code sinh ra tuân thủ chuẩn mực (ví dụ: luôn dùng DMA, check lỗi) thay vì code "rác"
- **Dễ debug:** Chia nhỏ Tasks giúp khoanh vùng lỗi (lỗi do Wifi hay do Audio) dễ dàng hơn

### Nhược điểm:

- **Phụ thuộc AI:** Code hoàn toàn do AI viết, nếu AI sai logic tầng thấp (Driver), người dùng cần kiến thức chuyên sâu để fix
- **Cần Model mạnh:** Yêu cầu các model AI tốt (Claude 3.5 Sonnet, GPT-4o) để xử lý ngữ cảnh dài