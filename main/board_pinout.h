#ifndef BOARD_PINOUT_H
#define BOARD_PINOUT_H

// ============================================================================
// I2S Configuration - Shared Clock Signals
// ============================================================================
// BCLK và WS được nối CHUNG cho cả Mic và Amp (tiết kiệm GPIO)
// Chỉ có SD (Serial Data) là riêng biệt cho từng thiết bị

#define I2S_PORT_NUM (0)

// --- Shared Clock Signals (Nối chung cho cả INMP441 và MAX98357A) ---
#define I2S_BCLK_IO                                                            \
  (14) // Bit Clock
       // → INMP441: chân "SCK"
       // → MAX98357A: chân "BCLK"

#define I2S_LRCK_IO                                                            \
  (25) // Word Select (Left/Right Clock) - GPIO 25 an toàn hơn GPIO 15
       // → INMP441: chân "WS"
       // → MAX98357A: chân "LRC"
       // Lưu ý: Đổi từ GPIO 15 sang GPIO 25 để tránh vấn đề boot

// --- Data Signals (Riêng biệt cho từng thiết bị) ---
#define I2S_DI_IO                                                              \
  (32) // Data Input (RX - từ Mic vào ESP32)
       // → INMP441: chân "SD" (Serial Data OUT)

#define I2S_DO_IO                                                              \
  (22) // Data Output (TX - từ ESP32 ra MAX98357A)
       // → MAX98357A: chân "DIN" (Data IN)

// GPIO Configuration
#define GPIO_PTT_BUTTON (4)
#define GPIO_STATUS_LED (2)

// Active Levels
#define PTT_ACTIVE_LEVEL (0) // Active Low
#define LED_ACTIVE_LEVEL (1) // Active High

#endif // BOARD_PINOUT_H
