# Hướng dẫn Tích hợp Speckit vào Antigravity

Để sử dụng các tính năng của Speckit trong Antigravity, chúng ta cần cấu hình thủ công các workflow.

## Yêu cầu

Cần tạo các file workflow trong thư mục `.agent/workflows/` tương ứng với các lệnh Speckit.

## Các bước thực hiện thủ công

Với mỗi lệnh Speckit bạn muốn sử dụng (nằm trong `.gemini/commands/`):

1. **Xem định nghĩa lệnh**: Mở file cấu hình TOML tương ứng trong `.gemini/commands/` (ví dụ: `speckit.specify.toml`).
2. **Lấy nội dung Prompt**: Tìm trường `prompt = """ ... """`. Copy toàn bộ nội dung văn bản nằm giữa dấu ngoặc kép ba.
   - **Lưu ý**: Nội dung này thường bắt đầu bằng block YAML front-matter (được bao bởi `---`).
3. **Tạo file Workflow**: 
   - Tạo file mới trong `.agent/workflows/` với tên giống tên lệnh nhưng đuôi là `.md`.
   - Ví dụ: `.gemini/commands/speckit.specify.toml` -> `.agent/workflows/speckit.specify.md`.
4. **Dán nội dung**: Paste nội dung đã copy vào file `.md` này.

## Ví dụ minh họa

Để kích hoạt lệnh `/speckit.plan`:

1. Đọc file `.gemini/commands/speckit.plan.toml`.
2. Copy nội dung trong `prompt`.
3. Tạo file `.agent/workflows/speckit.plan.md` và paste nội dung vào.

Sau khi hoàn tất, bạn có thể sử dụng các workflow này trong Antigravity.
