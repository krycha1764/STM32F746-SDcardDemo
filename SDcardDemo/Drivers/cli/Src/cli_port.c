#include "cli_port.h"

HAL_StatusTypeDef CLI_Port_Write(UART_HandleTypeDef *huart, const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if ((huart == NULL) || (data == NULL) || (len == 0U))
    {
        return HAL_ERROR;
    }
    return HAL_UART_Transmit(huart, (uint8_t *)data, (uint16_t)len, timeout_ms);
}

uint32_t CLI_Port_GetTick(void)
{
    return HAL_GetTick();
}
