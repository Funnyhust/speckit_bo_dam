# Đặc Tả Tính Năng: Bộ Đàm ESP32 ESP-NOW

## 1. Tổng Quan
Tính năng này triển khai hệ thống Bộ Đàm sử dụng ESP32-WROOM-32 thông qua giao thức ESP-NOW (chế độ Broadcast) để truyền thông âm thanh độ trễ thấp (<100ms). Hệ thống tận dụng micro I2S (INMP441) và bộ khuếch đại (MAX98357A) với truyền DMA để đảm bảo truyền tải giọng nói bán song công mượt mà, thời gian thực.

### 1.1 Kích Hoạt
- Người dùng muốn giao tiếp trong khoảng cách ngắn mà không cần hạ tầng Wi-Fi.
- Người dùng nhấn nút PTT (Push-to-Talk) để truyền giọng nói.

### 1.2 Mục Tiêu
- Cung cấp truyền thông âm thanh rõ ràng, độ trễ thấp giữa hai hoặc nhiều thiết bị ESP32.
- Hoạt động ổn định ở chế độ không kết nối sử dụng ESP-NOW.

## 2. Yêu Cầu Chức Năng

### 2.1 Giao Diện Phần Cứng
- **FR.HW.01:** Hệ thống **PHẢI** khởi tạo I2S Port 0 ở chế độ Full-Duplex hoặc Master.
- **FR.HW.02:** Hệ thống **PHẢI** sử dụng GPIO 14 (BCLK), 15 (LRCK), 32 (SD_IN), 22 (SD_OUT) cho giao tiếp I2S.
- **FR.HW.03:** Hệ thống **PHẢI** phát hiện nhấn nút PTT trên GPIO 4 (Active Low, Internal Pull-up).
- **FR.HW.04:** Hệ thống **PHẢI** điều khiển LED trạng thái trên GPIO 2.

### 2.2 Xử Lý Âm Thanh
- **FR.AUDIO.01:** Hệ thống **PHẢI** thu và phát lại âm thanh ở tốc độ lấy mẫu 16,000 Hz.
- **FR.AUDIO.02:** Hệ thống **PHẢI** sử dụng độ phân giải 16-bit cho các mẫu âm thanh.
- **FR.AUDIO.03:** Hệ thống **PHẢI** xử lý cấu hình kênh âm thanh Mono.
- **FR.AUDIO.04:** Hệ thống **NÊN** triển khai thuật toán nén cơ bản nếu băng thông vượt quá giới hạn ESP-NOW (tùy chọn nhưng được khuyến nghị).

### 2.3 Chế Độ Nhận (RX - Mặc Định)
- **FR.RX.01:** Hệ thống **PHẢI** mặc định ở chế độ RX khi khởi động hoặc khi thả nút PTT.
- **FR.RX.02:** Ở chế độ RX, hệ thống **PHẢI** liên tục lắng nghe các gói broadcast ESP-NOW.
- **FR.RX.03:** Khi nhận được gói âm thanh hợp lệ, hệ thống **PHẢI** giải mã và ghi dữ liệu vào bộ đệm DMA I2S để phát lại.
- **FR.RX.04:** LED trạng thái **NÊN** tắt (hoặc chỉ báo trạng thái Standby/RX như đã định nghĩa).

### 2.4 Chế Độ Truyền (TX)
- **FR.TX.01:** Hệ thống **PHẢI** vào chế độ TX ngay lập tức khi nhấn PTT.
- **FR.TX.02:** Ở chế độ TX, hệ thống **PHẢI** tắt tiếng đầu ra Loa (tắt tiếng phần mềm hoặc vô hiệu hóa I2S TX) để ngăn vòng lặp phản hồi.
- **FR.TX.03:** LED trạng thái **PHẢI** bật liên tục để chỉ báo truyền.
- **FR.TX.04:** Hệ thống **PHẢI** đọc âm thanh từ Micro qua I2S.
- **FR.TX.05:** Hệ thống **PHẢI** đóng gói dữ liệu âm thanh thành các khối nhỏ hơn 250 byte (giới hạn payload ESP-NOW).
- **FR.TX.06:** Hệ thống **PHẢI** broadcast các gói qua ESP-NOW đến peer cụ thể (hoặc địa chỉ broadcast).
- **FR.TX.07:** Khi thả PTT, hệ thống **PHẢI** ngay lập tức dừng truyền, bật tiếng Loa, và quay về chế độ RX.

## 3. Yêu Cầu Phi Chức Năng
- **NFR.LATENCY:** Độ trễ âm thanh đầu cuối **PHẢI** nhỏ hơn 100ms.
- **NFR.STABILITY:** Hệ thống **KHÔNG ĐƯỢC** crash hoặc treo khi chuyển đổi PTT nhanh.
- **NFR.QUALITY:** Đầu ra âm thanh **PHẢI** rõ ràng (điểm đánh giá ý kiến trung bình > 3.0 tương đương).
- **NFR.POWER:** Hệ thống **NÊN** tối ưu hóa tiêu thụ năng lượng ở chế độ RX nếu có thể (ví dụ: sử dụng light sleep nếu khả thi mà không mất gói, mặc dù khó với ESP-NOW).

## 4. Kịch Bản Người Dùng

### 4.1 Truyền Thành Công
1. Người dùng A cầm thiết bị, PTT được thả (chế độ RX).
2. Người dùng A nhấn và giữ nút PTT.
3. LED trạng thái của Người dùng A bật.
4. Người dùng A nói vào micro.
5. Người dùng B (trong phạm vi) nghe giọng nói của Người dùng A từ loa của họ.
6. Người dùng A thả nút PTT.
7. LED trạng thái của Người dùng A tắt, thiết bị quay về chế độ lắng nghe.

### 4.2 Hoạt Động Chỉ Nhận
1. Người dùng B để thiết bị trên bàn (chế độ RX).
2. Thiết bị nhận luồng từ Người dùng A.
3. Âm thanh phát mượt mà không bị giật.

## 5. Tiêu Chí Thành Công
- **SC.01:** Âm thanh có thể được truyền và nhận giữa hai thiết bị với độ trễ < 100ms.
- **SC.02:** Giọng nói rõ ràng và có thể nhận diện.
- **SC.03:** Hoạt động PTT phản hồi nhanh (LED phản ứng ngay lập tức).
- **SC.04:** Không có phản hồi âm thanh (tiếng hú) xảy ra khi truyền.

## 6. Giả Định & Ràng Buộc
- **Assump.01:** Các thiết bị nằm trong phạm vi Wi-Fi hiệu quả.
- **Const.01:** Payload ESP-NOW giới hạn ở 250 byte, yêu cầu đóng gói hiệu quả.
- **Const.02:** ESP32 WROOM-32 sử dụng đơn lõi hoặc đa lõi phải tuân thủ task pinning FreeRTOS để ổn định Wi-Fi.

## 7. Giai Đoạn 4 – Logic PTT & Buffer

- Khi không nhấn nút PTT, RX StreamBuffer phải rỗng.
- Khi thiết bị A nhấn PTT, thiết bị B chỉ bắt đầu phát sau khi RX buffer đã nhận đủ 3 packet (≈ 720 B).
- Tăng số DMA buffer cho speaker lên 8.
