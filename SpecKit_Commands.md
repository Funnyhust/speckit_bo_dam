# SpecKit Command Reference

Tài liệu này giải thích ý nghĩa và cách sử dụng của 9 lệnh trong bộ công cụ SpecKit (`.gemini/commands`).

## 1. Core Workflow (Quy trình cốt lõi)
Các lệnh này điều phối vòng đời phát triển phần mềm từ ý tưởng đến triển khai.

| Lệnh | Ý nghĩa | Vai trò tương đương |
| :--- | :--- | :--- |
| **`/speckit.specify`** | **Khởi tạo & Đặc tả**. Phỏng vấn người dùng hoặc đọc yêu cầu thô để tạo ra file `spec.md` (Tài liệu đặc tả yêu cầu). | Product Manager |
| **`/speckit.plan`** | **Lập kế hoạch**. Dựa trên `spec.md` để thiết kế kiến trúc, chọn công nghệ và tạo file `plan.md` (Kế hoạch kỹ thuật). | Tech Lead / Architect |
| **`/speckit.tasks`** | **Chia việc**. Chia nhỏ `plan.md` thành danh sách công việc cụ thể trong `tasks.md`. | Project Manager |
| **`/speckit.implement`** | **Thực thi**. Đọc `tasks.md` và thực hiện code từng task. Đây là lệnh trực tiếp viết code. | Software Engineer |
| **`/speckit.analyze`** | **Kiểm tra nhất quán**. Quét toàn bộ 3 file (`spec`, `plan`, `tasks`) để tìm điểm mâu thuẫn, thiếu sót hoặc trùng lặp. | Auditor / QA Lead |

## 2. Quality & Support (Chất lượng & Hỗ trợ)
Các lệnh bổ trợ để đảm bảo dự án tuân thủ chuẩn mực và rõ ràng.

| Lệnh | Ý nghĩa | Mục đích |
| :--- | :--- | :--- |
| **`/speckit.checklist`** | **Rà soát thủ công**. Kiểm tra dự án dựa trên các bảng kiểm có sẵn (UX, Security, Performance...). | Đảm bảo không bỏ sót các tiêu chuẩn phi chức năng. |
| **`/speckit.clarify`** | **Làm rõ**. "Soi" tài liệu `spec/plan` để tìm các câu từ mơ hồ và đặt câu hỏi ngược lại cho người dùng. | Xóa bỏ sự mơ hồ trước khi code. |
| **`/speckit.constitution`** | **Hiến pháp**. Kiểm tra xem các đề xuất/code có vi phạm các nguyên tắc cốt lõi ("Hiến pháp") của dự án không. | Giữ vững định hướng và nguyên tắc team. |
| **`/speckit.taskstoissues`** | **Xuất Issue**. Chuyen đổi nội dung trong `tasks.md` thành định dạng để post lên GitHub Issues hoặc Jira. | Đồng bộ tiến độ ra hệ thống quản lý bên ngoài. |

## Cách sử dụng
Gõ lệnh trực tiếp trong khung chat với Agent:
> `/speckit.so_ten_lenh`

Ví dụ: `/speckit.analyze`, `/speckit.specify`
