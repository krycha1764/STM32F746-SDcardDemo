#ifndef CLI_PORT_H
#define CLI_PORT_H

#include "stm32f7xx_hal.h"
#include <stddef.h>
#include <stdint.h>

HAL_StatusTypeDef CLI_Port_Write(UART_HandleTypeDef *huart, const uint8_t *data, size_t len, uint32_t timeout_ms);
uint32_t CLI_Port_GetTick(void);

#endif /* CLI_PORT_H */
