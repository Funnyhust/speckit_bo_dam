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
**Speckit** là một công cụ và phương pháp luận giúp lập trình viên làm việc với AI một cách hiệu quả, có quy trình và quy tắc chung.

### Vấn đề giải quyết:
Bình thường, khi hỏi AI rời rạc, code sinh ra thường:
* Sai kiến trúc hoặc thư viện mong muốn.
* Thiếu nhất quán giữa các file.
* Mất ngữ cảnh khi đoạn chat quá dài.

### Giải pháp:
Speckit yêu cầu tạo ra các file ngữ cảnh ("Bộ não dự án") trước khi code. Mỗi khi AI thực hiện hành động, nó sẽ đọc qua các file này để đảm bảo tuân thủ tuyệt đối ý đồ của lập trình viên.

---

## 2. Cấu trúc lõi của Speckit
Một dự án Speckit tiêu chuẩn bao gồm 4 file ngữ cảnh quan trọng cần được tạo ra theo thứ tự:

### 1. `constitution.md` (Hiến pháp dự án)
* **Chức năng:** Chứa các nguyên tắc kỹ thuật "Bất khả xâm phạm" (Non-negotiable rules).
* **Ví dụ:** Tech Stack (ESP-IDF, React, Python...), Coding Convention, các thư viện bị cấm sử dụng.

### 2. `spec.md` (Đặc tả yêu cầu)
* **Chức năng:** Mô tả chi tiết yêu cầu nghiệp vụ, Input/Output, User Stories.
* **Mục tiêu:** Giúp AI hiểu "Sản phẩm này làm cái gì, hoạt động ra sao".

### 3. `plan.md` (Kế hoạch kỹ thuật)
* **Chức năng:** Chuyển đổi yêu cầu thành giải pháp kỹ thuật.
* **Nội dung:** Kiến trúc hệ thống, sơ đồ luồng dữ liệu (Data Flow), cấu trúc thư mục, định nghĩa các Module.

### 4. `tasks.md` (Danh sách nhiệm vụ)
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

### 💡 Ý Tưởng

Thiết kế một bộ đàm không dây sử dụng ESP32 và ESP-NOW để hai người có thể nói chuyện với nhau giống như bộ đàm cầm tay truyền thống. 

**Cách hoạt động đơn giản:**
- Nhấn giữ nút → Nói vào micro → Giọng nói được truyền qua sóng Wi-Fi đến bộ đàm bên kia → Phát ra loa
- Thả nút → Nghe người kia nói

**Tại sao chọn ESP-NOW?**
- Truyền trực tiếp giữa 2 thiết bị, không cần router Wi-Fi
- Rất nhanh (độ trễ thấp), phù hợp cho giao tiếp giọng nói thời gian thực
- Đơn giản, dễ lập trình

**Ứng dụng thực tế:**
- Liên lạc trong nhà xưởng, công trường
- Bộ đàm cho trẻ em chơi
- Hệ thống intercom tự làm

---

### 🎯 Bài Toán Kỹ Thuật

**Yêu cầu:** Xây dựng firmware cho bộ đàm không dây sử dụng ESP32, cho phép 2 thiết bị giao tiếp giọng nói với nhau trong phạm vi Wi-Fi.

**Thách thức kỹ thuật:**
- ⚡ **Độ trễ thấp:** Phải đạt < 100ms end-to-end để đảm bảo trải nghiệm tự nhiên
- 🔊 **Chất lượng âm thanh:** Giọng nói rõ ràng, không bị giật, không bị hú (feedback)
- 🎛️ **Giao diện đơn giản:** Chỉ cần 1 nút PTT (Push-to-Talk) để điều khiển
- 📡 **Không cần router:** Sử dụng ESP-NOW để truyền trực tiếp giữa 2 thiết bị

**Ràng buộc:**
- Hardware: ESP32-WROOM-32, INMP441 (Mic), MAX98357A (Amp)
- Giới hạn gói tin ESP-NOW: 250 bytes
- Phải sử dụng DMA để tránh mất mẫu audio

---

### Giai đoạn 0: Lên Ý Tưởng (Brainstorming với Gemini)

Trước khi bắt đầu viết code, cần có một ý tưởng rõ ràng và các prompt chuẩn cho SpecKit. Giai đoạn này sử dụng Gemini để brainstorm và hoàn thiện ý tưởng.

**Bước 1: Nghiên cứu SpecKit**

Import link GitHub SpecKit vào Gemini để AI hiểu rõ quy trình:

```
Hãy phân tích repository này: https://github.com/github/spec-kit

Tôi muốn hiểu:
1. SpecKit hoạt động như thế nào?
2. Các file cần tạo là gì (constitution, spec, plan, tasks)?
3. Quy trình sử dụng SpecKit với AI Assistant?
4. Cách viết prompt hiệu quả cho từng giai đoạn?

Hãy tóm tắt ngắn gọn để tôi có thể áp dụng ngay.
```

**Bước 2: Đề xuất ý tưởng ban đầu**

```
Tôi đang lên ý tưởng thực hiện một bộ đàm sử dụng ESP32 và ESP-NOW, 
mic INMP441 và module MAX98357A. Hãy giúp tôi lên ý tưởng thực hiện nó với SpecKit.

Yêu cầu:
- Độ trễ thấp (< 100ms)
- Không cần router Wi-Fi
- Giao diện đơn giản (1 nút PTT)
- Chất lượng âm thanh tốt
```

**Bước 3: Trao đổi và hoàn thiện**

Thảo luận với Gemini về các khía cạnh:
- ✅ **Tech Stack:** ESP-IDF hay Arduino? → Chọn ESP-IDF vì hiệu năng cao
- ✅ **Audio Pipeline:** DMA hay Polling? → Chọn DMA để tránh mất mẫu
- ✅ **Protocol:** ESP-NOW hay MQTT? → Chọn ESP-NOW vì độ trễ thấp
- ✅ **Buffer Strategy:** Kích thước bao nhiêu? → Tính toán dựa trên sample rate
- ✅ **Pin Mapping:** GPIO nào cho I2S? → Tham khảo datasheet ESP32

**Bước 4: Yêu cầu Gemini tạo prompts cho SpecKit**

```
Dựa trên ý tưởng đã thống nhất, hãy tạo cho tôi các prompt chuẩn 
để sử dụng với SpecKit theo thứ tự:

1. Prompt cho /constitution (Hiến pháp dự án)
2. Prompt cho /specify (Đặc tả kỹ thuật)
3. Prompt cho /plan (Kế hoạch triển khai)
4. Prompt cho /tasks (Danh sách nhiệm vụ)

Mỗi prompt cần:
- Rõ ràng, chi tiết
- Bao gồm tất cả thông tin kỹ thuật đã thống nhất
- Dễ copy-paste để sử dụng ngay
```

**Output Result:**

Gemini đã tạo ra bộ 4 prompts chuẩn (như các giai đoạn 1-4 bên dưới), bao gồm:
- ✅ Tech Stack rõ ràng (ESP32, ESP-IDF, I2S hardware)
- ✅ Engineering Standards cụ thể (DMA mandatory, FreeRTOS, GPIO management)
- ✅ Pin mapping chi tiết
- ✅ Audio configuration (16kHz, 16-bit, Mono)
- ✅ Kiến trúc hệ thống với sơ đồ luồng dữ liệu
- ✅ Packet structure tối ưu (244 bytes)
- ✅ Task breakdown theo 4 phases

> **💡 Tip:** Giai đoạn 0 này rất quan trọng! Nó giúp bạn tránh được việc phải sửa đổi nhiều lần sau này vì ý tưởng chưa rõ ràng. Hãy dành thời gian brainstorm kỹ với AI trước khi bắt đầu code.

---

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

**Output Result:**

AI đã tạo file [`.specify/memory/constitution.md`](specs_vi/constitution.md) với nội dung:
- ✅ Định nghĩa rõ ràng Tech Stack (ESP32, ESP-IDF, I2S hardware)
- ✅ Quy tắc bắt buộc về Audio Pipeline (DMA mandatory)
- ✅ Concurrency model (FreeRTOS tasks)
- ✅ Memory management rules (no malloc in loops)
- ✅ Code review checklist

---

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

**Output Result:**

AI đã tạo file [`specs/spec.md`](specs/spec.md) với nội dung:
- ✅ Pin mapping chi tiết cho I2S và GPIO
- ✅ Functional Requirements (FR.HW.01-04, FR.AUDIO.01-04, FR.RX.01-04, FR.TX.01-07)
- ✅ Non-Functional Requirements (Latency < 100ms, Stability, Quality)
- ✅ User Scenarios (Successful Transmission, Receive-Only Operation)
- ✅ Success Criteria và Assumptions

---

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

**Output Result:**

AI đã tạo file [`specs/plan.md`](specs/plan.md) với nội dung:
- ✅ Sơ đồ Data Flow (Mermaid diagram) từ Mic → DMA → Task → ESP-NOW
- ✅ Packet Structure: 244 bytes (2B magic + 2B seq_num + 240B payload)
- ✅ Memory Strategy: DMA buffer 240 bytes x 4, Ring buffer 4KB
- ✅ Latency Budget: Tổng < 50ms (trong target 100ms)
- ✅ File Structure: Tách module `audio_driver`, `wifi_transport`, `board_pinout`
- ✅ Implementation Phases: 4 giai đoạn từ Foundation → Loopback → Wireless → Integration

---

### Giai đoạn 4: Chia task (`/tasks`)

**Input Prompt:**

```
Hãy chia dự án thành danh sách các Nhiệm vụ (Tasks checklist) để thực hiện lần lượt. Tôi đề xuất lộ trình sau, hãy bổ sung chi tiết kỹ thuật cho từng bước:

- Task 1: Audio Loopback Test (Quan trọng nhất). Chưa cần Wifi. Đọc Mic → DMA → Phát thẳng ra Loa. Mục tiêu: Kiểm tra nhiễu, volume và kết nối phần cứng
- Task 2: ESP-NOW Communication. Viết driver gửi/nhận struct dữ liệu giả lập. Test kết nối giữa 2 mạch
- Task 3: Audio Transport Integration. Kết nối Audio vào Wifi. Xử lý việc chia cắt (fragmentation) và ghép lại gói tin audio
- Task 4: PTT Logic & Optimization. Thêm nút bấm, LED và tinh chỉnh Ring Buffer để mượt tiếng
```

**Output Result:**

AI đã tạo file [`specs/tasks.md`](specs/tasks.md) với nội dung:
- ✅ Phase 1: 5 tasks (T001-T005) - Foundation & Audio Loopback
- ✅ Phase 2: 5 tasks (T006-T010) - ESP-NOW Communication
- ✅ Phase 3: 4 tasks (T011-T014) - Audio Transport Integration
- ✅ Phase 4: 9 tasks (T015-T023) - PTT Logic & Optimization
- ✅ Mỗi task có mô tả chi tiết, file cần sửa, và phương pháp verify

---

### Giai đoạn 5: Thực thi (`/implement`)

Sau khi có file `tasks.md`, lần lượt ra lệnh: `/implement Task 1`, `/implement Task 2`...

**Output Result:**

AI đã tạo các file source code:
- ✅ [`main/main.c`](main/main.c) - Entry point, task creation, state machine
- ✅ [`main/board_pinout.h`](main/board_pinout.h) - GPIO definitions
- ✅ [`main/app_config.h`](main/app_config.h) - Audio/WiFi configuration
- ✅ [`main/audio_driver.c`](main/audio_driver.c) - I2S driver implementation
- ✅ [`main/wifi_transport.c`](main/wifi_transport.c) - ESP-NOW implementation

**Kết quả thực tế:**
- ✅ Phase 1-2 hoàn thành 100%: Audio loopback hoạt động, ESP-NOW gửi/nhận thành công
- ✅ Phase 4 hoàn thành 50%: Đã tăng DMA buffer lên 8, thêm logic buffer 3 packets
- ⏳ Phase 3 đang triển khai: RingBuffer và PTT State Machine
- 🎯 Thiết bị đã test thành công: Độ trễ ~60ms, âm thanh rõ ràng

---

## 5. Tổng kết & Đánh giá

### Ưu điểm:

- **Hiểu dự án nhanh:** Người mới chỉ cần đọc 4 file Speckit là nắm được toàn bộ hệ thống (Tech stack, Pinout, Logic) mà không cần đọc từng dòng code
- **Code chất lượng:** Do có "Hiến pháp", code sinh ra tuân thủ chuẩn mực (ví dụ: luôn dùng DMA, check lỗi) thay vì code "rác"
- **Dễ debug:** Chia nhỏ Tasks giúp khoanh vùng lỗi (lỗi do Wifi hay do Audio) dễ dàng hơn

### Nhược điểm:

- **Phụ thuộc AI:** Code hoàn toàn do AI viết, nếu AI sai logic tầng thấp (Driver), người dùng cần kiến thức chuyên sâu để fix
- **Cần Model mạnh:** Yêu cầu các model AI tốt (Claude 3.5 Sonnet, GPT-4o) để xử lý ngữ cảnh dài