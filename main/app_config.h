#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

// Audio Settings
#define SAMPLE_Rate       (16000)
#define SAMPLE_BITS       (16)
#define I2S_DMA_BUF_COUNT (4)
#define I2S_DMA_BUF_LEN   (120) // Samples per buffer (Mono) -> 240 Bytes

// ESP-NOW Protocol defined in subsequent tasks

#endif // APP_CONFIG_H
