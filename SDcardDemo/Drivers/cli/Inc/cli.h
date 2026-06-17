#ifndef CLI_H
#define CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"
#include <stdint.h>

typedef void (*CLI_CommandHandler)(int argc, char **argv);

typedef struct
{
    const char *name;
    CLI_CommandHandler handler;
    const char *help;
} CLI_Command;

void CLI_Init(UART_HandleTypeDef *huart);
void CLI_StartReception(void);
void CLI_Process(void);
void CLI_RxCpltCallback(UART_HandleTypeDef *huart);
void CLI_TxCpltCallback(UART_HandleTypeDef *huart);
void CLI_RegisterCommands(const CLI_Command *cmds, uint16_t count);
void CLI_Print(const char *text);
void CLI_PrintPrompt(void);

#ifdef __cplusplus
}
#endif

#endif /* CLI_H */
