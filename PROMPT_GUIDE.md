# SPECKIT PROMPT GUIDE: Hướng dẫn Input chuẩn

Tài liệu này hướng dẫn cách viết Prompt (Input đầu vào) cho từng lệnh của Speckit. Chất lượng của file đầu ra phụ thuộc hoàn toàn vào chất lượng thông tin bạn nạp vào ở bước này.

---

## 1. Giai đoạn Khởi tạo: `/speckit.constitution`

**Mục tiêu:** Thiết lập "Hiến pháp" dự án. File TOML yêu cầu AI tìm và thay thế các `[PLACEHOLDERS]` trong template gốc.

**Nguyên tắc:** Nếu bạn không cung cấp thông tin, AI sẽ tự suy đoán (có thể sai). Hãy cung cấp các luật "bất di bất dịch".

### Cấu trúc Input chuẩn:

```
/speckit.constitution
Tôi muốn thiết lập Hiến pháp cho dự án [TÊN_DỰ_ÁN].

1. Context:
- Vai trò AI: [Ví dụ: Senior Embedded Engineer, Lead Architect...]
- Mục tiêu: [Mô tả ngắn gọn]

2. Tech Stack (Bắt buộc):
- Phần cứng/OS: [...]
- Ngôn ngữ & Version: [...]
- Framework/SDK: [...]

3. Non-negotiable Rules (Luật cấm/Luật cứng):
- Architecture: [Ví dụ: Microservices, Hexagonal...]
- Coding Style: [Ví dụ: Google Style Guide, NO Global variables...]
- Performance/Security: [Ví dụ: Latency < 100ms, Zero-trust...]

4. Governance:
- Version khởi tạo: v1.0.0
```

### Ví dụ mẫu (Bộ đàm ESP32):

```
/speckit.constitution 
Dự án ESP32 Walkie-Talkie. 

Tech Stack: ESP32, ESP-IDF v5.1, C11, ESP-NOW. 

Luật cứng: 
- Bắt buộc dùng DMA cho Audio (không CPU polling)
- Tách biệt Task Audio và Wifi bằng FreeRTOS
- Không hard-code GPIO number
```

---

## 2. Giai đoạn Đặc tả: `/speckit.specify`

**Mục tiêu:** Tạo file `spec.md` mô tả **WHAT** (Cái gì) và **WHY** (Tại sao).

**Lưu ý quan trọng từ TOML:**
- **KHÔNG** đưa chi tiết kỹ thuật (Tech stack, API, Library) vào đây
- Tập trung vào giá trị người dùng (User Value) và Tiêu chí thành công (Success Criteria)
- AI sẽ tự động tạo checklist chất lượng, nếu bạn viết quá sơ sài, AI sẽ báo lỗi hoặc yêu cầu Clarify

### Cấu trúc Input chuẩn:

```
/speckit.specify [Mô tả tính năng tự nhiên]

1. Tính năng mong muốn:
- [Mô tả hành vi người dùng: Khi user làm A thì hệ thống làm B]

2. Phạm vi (Scope):
- Bao gồm: [...]
- KHÔNG bao gồm: [...]

3. Tiêu chí thành công (Bắt buộc phải đo đếm được & Phi kỹ thuật):
- [Ví dụ: Người dùng hoàn thành tác vụ < 3 click]
- [Ví dụ: Hệ thống phản hồi < 1s]
- [Lưu ý: Đừng viết "Dùng Redis cache" -> Sai. Hãy viết "Tải trang tức thì" -> Đúng]
```

### Ví dụ mẫu:

```
/speckit.specify 
Tính năng "Push-to-Talk" cơ bản. 

Khi người dùng nhấn giữ nút PTT, thiết bị chuyển sang chế độ phát, đèn LED sáng đỏ. 
Khi nhả nút, thiết bị về chế độ thu ngay lập tức. 

Tiêu chí thành công: 
- Độ trễ từ lúc bấm nút đến lúc thu âm < 50ms
- Âm thanh không bị méo khi chuyển đổi trạng thái
```

---

## 3. Giai đoạn Lập kế hoạch: `/speckit.plan`

**Mục tiêu:** Tạo file `plan.md` chuyển đổi từ Spec sang **HOW** (Kỹ thuật).

**Logic TOML:** AI sẽ đọc `spec.md` (Yêu cầu) + `constitution.md` (Luật) để tạo ra giải pháp. Input của bạn ở đây dùng để cung cấp bối cảnh kỹ thuật cụ thể cho tính năng này.

### Cấu trúc Input chuẩn:

```
/speckit.plan [Bối cảnh kỹ thuật bổ sung]

1. Hướng tiếp cận (Technical Direction):
- [Ví dụ: Sử dụng thư viện X cho việc xử lý ảnh]
- [Ví dụ: Áp dụng Pattern Observer cho tính năng này]

2. Những điều chưa rõ (Unknowns) cần Research:
- [Ví dụ: Cần tìm hiểu giới hạn packet size của ESP-NOW]
- [Ví dụ: So sánh hiệu năng giữa JSON và Protobuf]

3. Ràng buộc cụ thể cho tính năng này (nếu khác Constitution):
- [Ví dụ: Module này cần tối ưu bộ nhớ cực đoan do chạy trên Core 0]
```

### Ví dụ mẫu:

```
/speckit.plan 
Sử dụng Ring Buffer để decouple việc đọc Mic và gửi Wifi. 

Cần research kỹ về kích thước payload tối đa của ESP-NOW để phân mảnh gói tin audio hợp lý. 

Kiến trúc module cần tách biệt driver I2S và logic mạng.
```

---

## 4. Giai đoạn Nhiệm vụ: `/speckit.tasks`

**Mục tiêu:** Tạo file `tasks.md` chứa checklist thực thi code.

**Logic TOML:** AI sẽ nhóm task theo User Story (P1, P2...). Input của bạn sẽ điều chỉnh chiến lược thực thi.

### Cấu trúc Input chuẩn:

```
/speckit.tasks [Chiến lược thực thi]

1. Chiến lược Phasing (Phân giai đoạn):
- [Ví dụ: Ưu tiên làm MVP chạy được luồng chính trước, UI làm sau]
- [Ví dụ: Làm theo hướng TDD (Test Driven Development) - Viết test trước]

2. Độ mịn của Task:
- [Ví dụ: Chia task đủ nhỏ để mỗi commit < 100 dòng code]
- [Ví dụ: Tạo task riêng cho việc viết Unit Test]
```

### Ví dụ mẫu:

```
/speckit.tasks 
Hãy chia phase theo hướng: 
- Phase 1 làm Loopback test (Mic->Loa) để chốt phần cứng
- Phase 2 mới làm truyền nhận ESP-NOW

Yêu cầu task phải có bước verify (kiểm thử) thủ công cho từng phần cứng.
```

---

## 5. Tóm tắt quy trình tư duy (Cheat Sheet)

| Lệnh | Hỏi gì? | Tư duy | Từ khóa cấm kỵ |
|------|---------|--------|----------------|
| `/constitution` | Luật chơi là gì? | Vai trò, Stack, Cấm kỵ | "Linh hoạt", "Tùy ý" |
| `/speckit.specify` | Sản phẩm làm gì? | User Story, Input/Output | Tên thư viện, Code, API |
| `/speckit.plan` | Làm thế nào? | Kiến trúc, Thuật toán | "Người dùng thấy..." |
| `/speckit.tasks` | Thứ tự làm? | Lộ trình, Checklist | "Thiết kế hệ thống" |

---

## 💡 Tips quan trọng

1. **Constitution = Luật cứng:** Một khi đã set, không nên thay đổi giữa chừng
2. **Specify = Phi kỹ thuật:** Viết như đang nói chuyện với Product Manager
3. **Plan = Kỹ thuật:** Viết như đang nói chuyện với Senior Engineer
4. **Tasks = Hành động:** Mỗi task phải có động từ rõ ràng (Implement, Test, Deploy...)