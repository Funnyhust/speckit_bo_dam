#ifndef BOARD_PINOUT_H
#define BOARD_PINOUT_H

// I2S Configuration
#define I2S_PORT_NUM      (0)
#define I2S_BCLK_IO       (14)
#define I2S_LRCK_IO       (15) // WS
#define I2S_DI_IO         (32) // SD_IN (Mic INMP441)
#define I2S_DO_IO         (22) // SD_OUT (Amp MAX98357A)

// GPIO Configuration
#define GPIO_PTT_BUTTON   (4)
#define GPIO_STATUS_LED   (2)

// Active Levels
#define PTT_ACTIVE_LEVEL  (0)  // Active Low
#define LED_ACTIVE_LEVEL  (1)  // Active High

#endif // BOARD_PINOUT_H
