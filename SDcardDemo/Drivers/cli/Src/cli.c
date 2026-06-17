#include "cli.h"
#include "cli_cfg.h"
#include "cli_port.h"

#include <stddef.h>
#include <string.h>

#include "LCD_CLI.h"

#define CLI_CR                      '\r'
#define CLI_LF                      '\n'
#define CLI_BS                      0x08U
#define CLI_DEL                     0x7FU
#define CLI_ESC                     0x1BU
#define CLI_CSI_START               '['
#define CLI_ARROW_UP                'A'
#define CLI_ARROW_DOWN              'B'
#define CLI_SPACE                   ' '

static UART_HandleTypeDef *s_cli_uart = NULL;

static volatile uint8_t s_rx_byte = 0U;
static volatile uint16_t s_line_index = 0U;
static volatile uint8_t s_line_overflow = 0U;
static volatile uint8_t s_command_ready = 0U;
static volatile uint8_t s_escape_state = 0U;

static char s_line_buffer[CLI_BUFFER_SIZE];
static char s_command_buffer[CLI_BUFFER_SIZE];

static uint8_t s_tx_ring[CLI_TX_BUFFER_SIZE];
static volatile uint16_t s_tx_head = 0U;
static volatile uint16_t s_tx_tail = 0U;
static volatile uint16_t s_tx_inflight_len = 0U;
static volatile uint8_t s_tx_busy = 0U;
static volatile uint8_t s_tx_overflow = 0U;
static volatile uint8_t s_tx_kick_pending = 0U;

static const CLI_Command *s_user_cmds = NULL;
static uint16_t s_user_cmd_count = 0U;

#if (CLI_HISTORY_SIZE > 0U)
static char s_history[CLI_HISTORY_SIZE][CLI_BUFFER_SIZE];
static uint16_t s_history_count = 0U;
static uint16_t s_history_next = 0U;
static int16_t s_history_nav = -1;
#endif

#if (CLI_ENABLE_CURSOR_BLINK == 1U)
static uint32_t s_cursor_last_tick = 0U;
static uint8_t s_cursor_visible = 0U;
#endif

static int CLI_StrCmp(const char *a, const char *b)
{
    while ((*a != '\0') && (*b != '\0'))
    {
        if (*a != *b)
        {
            return (int)((unsigned char)*a) - (int)((unsigned char)*b);
        }
        a++;
        b++;
    }

    return (int)((unsigned char)*a) - (int)((unsigned char)*b);
}

static size_t CLI_StrLen(const char *s)
{
    size_t len = 0U;
    while (s[len] != '\0')
    {
        len++;
    }
    return len;
}

static uint8_t CLI_IsInIsr(void)
{
    return ((__get_IPSR() != 0U) ? 1U : 0U);
}

static void CLI_TxKick(void)
{
    uint16_t tail;
    uint16_t head;
    uint16_t len;

    if (s_cli_uart == NULL)
    {
        return;
    }

    if (s_tx_busy != 0U)
    {
        return;
    }

    __disable_irq();
    tail = s_tx_tail;
    head = s_tx_head;
    if (tail == head)
    {
        __enable_irq();
        return;
    }

    if (head > tail)
    {
        len = (uint16_t)(head - tail);
    }
    else
    {
        len = (uint16_t)(CLI_TX_BUFFER_SIZE - tail);
    }

    s_tx_busy = 1U;
    s_tx_inflight_len = len;
    __enable_irq();

    if (HAL_UART_Transmit_IT(s_cli_uart, &s_tx_ring[tail], len) != HAL_OK)
    {
        __disable_irq();
        s_tx_busy = 0U;
        s_tx_inflight_len = 0U;
        __enable_irq();
    }
}

static void CLI_TxTriggerFromContext(void)
{
    if (CLI_IsInIsr() != 0U)
    {
        s_tx_kick_pending = 1U;
        return;
    }

    CLI_TxKick();
}

static void CLI_TxEnqueueByte(uint8_t byte)
{
    uint16_t next;
    LCD_CLI_PutChar((char)byte, LCD_COLOR_WHITE);

    __disable_irq();
    next = (uint16_t)((s_tx_head + 1U) % CLI_TX_BUFFER_SIZE);
    if (next == s_tx_tail)
    {
        s_tx_overflow = 1U;
        __enable_irq();
        return;
    }

    s_tx_ring[s_tx_head] = byte;
    s_tx_head = next;
    __enable_irq();

    CLI_TxTriggerFromContext();
}

static void CLI_TxEnqueueBuffer(const uint8_t *data, uint16_t len)
{
    uint16_t i;
    for (i = 0U; i < len; i++)
    {
        CLI_TxEnqueueByte(data[i]);
    }
}

static uint8_t CLI_IsSpace(char c)
{
    return (uint8_t)((c == ' ') || (c == '\t'));
}

#if (CLI_HISTORY_SIZE > 0U)
static void CLI_HistoryResetNavigation(void)
{
    s_history_nav = -1;
}

static void CLI_HistoryPush(const char *line)
{
    size_t len;

    if ((line == NULL) || (line[0] == '\0'))
    {
        return;
    }

    len = CLI_StrLen(line);
    if (len >= CLI_BUFFER_SIZE)
    {
        len = CLI_BUFFER_SIZE - 1U;
    }

    (void)memcpy(s_history[s_history_next], line, len);
    s_history[s_history_next][len] = '\0';

    s_history_next = (uint16_t)((s_history_next + 1U) % CLI_HISTORY_SIZE);
    if (s_history_count < CLI_HISTORY_SIZE)
    {
        s_history_count++;
    }

    CLI_HistoryResetNavigation();
}

static const char *CLI_HistoryGetByOffset(uint16_t offset_from_latest)
{
    uint16_t latest;
    uint16_t index;

    if ((s_history_count == 0U) || (offset_from_latest >= s_history_count))
    {
        return NULL;
    }

    latest = (uint16_t)((s_history_next + CLI_HISTORY_SIZE - 1U) % CLI_HISTORY_SIZE);
    index = (uint16_t)((latest + CLI_HISTORY_SIZE - offset_from_latest) % CLI_HISTORY_SIZE);
    return s_history[index];
}

static void CLI_EchoEraseCurrentLine(uint16_t current_len)
{
#if (CLI_ENABLE_ECHO == 1U)
    uint16_t i;
    static const uint8_t erase_seq[3] = {CLI_BS, CLI_SPACE, CLI_BS};

    for (i = 0U; i < current_len; i++)
    {
        CLI_TxEnqueueBuffer(erase_seq, 3U);
    }
#else
    (void)current_len;
#endif
}

static void CLI_HistoryRecallUp(void)
{
    const char *cmd;
    size_t len;
    uint16_t old_len;

    if (s_history_count == 0U)
    {
        return;
    }

    if (s_history_nav < 0)
    {
        s_history_nav = 0;
    }
    else if ((uint16_t)s_history_nav < (s_history_count - 1U))
    {
        s_history_nav++;
    }

    cmd = CLI_HistoryGetByOffset((uint16_t)s_history_nav);
    if (cmd == NULL)
    {
        return;
    }

    old_len = s_line_index;
    len = CLI_StrLen(cmd);
    if (len >= CLI_BUFFER_SIZE)
    {
        len = CLI_BUFFER_SIZE - 1U;
    }

    (void)memcpy(s_line_buffer, cmd, len);
    s_line_buffer[len] = '\0';
    s_line_index = (uint16_t)len;

    CLI_EchoEraseCurrentLine(old_len);
#if (CLI_ENABLE_ECHO == 1U)
    CLI_TxEnqueueBuffer((const uint8_t *)s_line_buffer, (uint16_t)len);
#endif
}

static void CLI_HistoryRecallDown(void)
{
    const char *cmd;
    size_t len;
    uint16_t old_len;

    if (s_history_count == 0U)
    {
        return;
    }

    old_len = s_line_index;

    if (s_history_nav <= 0)
    {
        /* Already at the newest entry or no navigation active – clear the line */
        s_history_nav = -1;
        s_line_index = 0U;
        s_line_buffer[0] = '\0';
        CLI_EchoEraseCurrentLine(old_len);
        return;
    }

    s_history_nav--;

    cmd = CLI_HistoryGetByOffset((uint16_t)s_history_nav);
    if (cmd == NULL)
    {
        s_history_nav = -1;
        s_line_index = 0U;
        s_line_buffer[0] = '\0';
        CLI_EchoEraseCurrentLine(old_len);
        return;
    }

    len = CLI_StrLen(cmd);
    if (len >= CLI_BUFFER_SIZE)
    {
        len = CLI_BUFFER_SIZE - 1U;
    }

    (void)memcpy(s_line_buffer, cmd, len);
    s_line_buffer[len] = '\0';
    s_line_index = (uint16_t)len;

    CLI_EchoEraseCurrentLine(old_len);
#if (CLI_ENABLE_ECHO == 1U)
    CLI_TxEnqueueBuffer((const uint8_t *)s_line_buffer, (uint16_t)len);
#endif
}
#endif

static int CLI_ParseArgs(char *line, char **argv, int max_args)
{
    int argc = 0;

    while ((*line != '\0') && (argc < max_args))
    {
        while (CLI_IsSpace(*line) != 0U)
        {
            line++;
        }

        if (*line == '\0')
        {
            break;
        }

        argv[argc++] = line;

        while ((*line != '\0') && (CLI_IsSpace(*line) == 0U))
        {
            line++;
        }

        if (*line == '\0')
        {
            break;
        }

        *line = '\0';
        line++;
    }

    return argc;
}

static void CLI_PrintHelp(void)
{
    uint16_t i;

    CLI_Print("Commands:\r\n");
    CLI_Print("  help - Show command list\r\n");

    for (i = 0U; i < s_user_cmd_count; i++)
    {
        if ((s_user_cmds[i].name != NULL) && (s_user_cmds[i].handler != NULL))
        {
            CLI_Print("  ");
            CLI_Print(s_user_cmds[i].name);
            CLI_Print(" - ");
            if (s_user_cmds[i].help != NULL)
            {
                CLI_Print(s_user_cmds[i].help);
            }
            else
            {
                CLI_Print("(no description)");
            }
            CLI_Print("\r\n");
        }
    }
}

static void CLI_ExecuteCommand(char *line)
{
    char *argv[CLI_MAX_ARGS];
    int argc;
    uint16_t i;

    argc = CLI_ParseArgs(line, argv, (int)CLI_MAX_ARGS);
    if (argc <= 0)
    {
        return;
    }

    for (i = 0U; i < s_user_cmd_count; i++)
    {
        if ((s_user_cmds[i].name != NULL) &&
            (s_user_cmds[i].handler != NULL) &&
            (CLI_StrCmp(argv[0], s_user_cmds[i].name) == 0))
        {
            s_user_cmds[i].handler(argc, argv);
            return;
        }
    }

    if ((CLI_StrCmp(argv[0], "help") == 0) || (CLI_StrCmp(argv[0], "?") == 0))
    {
        CLI_PrintHelp();
        return;
    }

    CLI_Print("Unknown command: ");
    CLI_Print(argv[0]);
    CLI_Print("\r\n");
}

void CLI_Init(UART_HandleTypeDef *huart)
{
    __disable_irq();
    s_cli_uart = huart;
    s_rx_byte = 0U;
    s_line_index = 0U;
    s_line_overflow = 0U;
    s_command_ready = 0U;
    s_escape_state = 0U;
    s_tx_head = 0U;
    s_tx_tail = 0U;
    s_tx_inflight_len = 0U;
    s_tx_busy = 0U;
    s_tx_overflow = 0U;
    s_tx_kick_pending = 0U;
    s_user_cmds = NULL;
    s_user_cmd_count = 0U;
    s_line_buffer[0] = '\0';
    s_command_buffer[0] = '\0';
#if (CLI_HISTORY_SIZE > 0U)
    s_history_count = 0U;
    s_history_next = 0U;
    s_history_nav = -1;
#endif
#if (CLI_ENABLE_CURSOR_BLINK == 1U)
    s_cursor_last_tick = CLI_Port_GetTick();
    s_cursor_visible = 0U;
#endif
    __enable_irq();
}

void CLI_StartReception(void)
{
    if (s_cli_uart != NULL)
    {
        (void)HAL_UART_Receive_IT(s_cli_uart, (uint8_t *)&s_rx_byte, 1U);
    }
}

void CLI_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t ch;

    if ((s_cli_uart == NULL) || (huart != s_cli_uart))
    {
        return;
    }

    ch = s_rx_byte;

    if (s_escape_state == 1U)
    {
        s_escape_state = (ch == (uint8_t)CLI_CSI_START) ? 2U : 0U;
        (void)HAL_UART_Receive_IT(s_cli_uart, (uint8_t *)&s_rx_byte, 1U);
        return;
    }

    if (s_escape_state == 2U)
    {
        s_escape_state = 0U;
#if (CLI_HISTORY_SIZE > 0U)
        if (ch == (uint8_t)CLI_ARROW_UP)
        {
            CLI_HistoryRecallUp();
        }
        else if (ch == (uint8_t)CLI_ARROW_DOWN)
        {
            CLI_HistoryRecallDown();
        }
#endif
        (void)HAL_UART_Receive_IT(s_cli_uart, (uint8_t *)&s_rx_byte, 1U);
        return;
    }

    if (ch == CLI_ESC)
    {
        s_escape_state = 1U;
        (void)HAL_UART_Receive_IT(s_cli_uart, (uint8_t *)&s_rx_byte, 1U);
        return;
    }

    if ((ch == (uint8_t)CLI_CR) || (ch == (uint8_t)CLI_LF))
    {
        if (s_command_ready == 0U)
        {
            s_line_buffer[s_line_index] = '\0';
            (void)strncpy(s_command_buffer, s_line_buffer, CLI_BUFFER_SIZE - 1U);
            s_command_buffer[CLI_BUFFER_SIZE - 1U] = '\0';
            s_command_ready = 1U;
        }

#if (CLI_ENABLE_ECHO == 1U)
        {
            static const uint8_t newline_seq[2] = {CLI_CR, CLI_LF};
            CLI_TxEnqueueBuffer(newline_seq, 2U);
        }
#endif

        s_line_index = 0U;
        s_line_buffer[0] = '\0';
#if (CLI_HISTORY_SIZE > 0U)
        CLI_HistoryResetNavigation();
#endif
    }
#if (CLI_ENABLE_BACKSPACE == 1U)
    else if ((ch == CLI_BS) || (ch == CLI_DEL))
    {
        if (s_line_index > 0U)
        {
            s_line_index--;
            s_line_buffer[s_line_index] = '\0';

#if (CLI_ENABLE_ECHO == 1U)
            {
                static const uint8_t erase_seq[3] = {CLI_BS, CLI_SPACE, CLI_BS};
                CLI_TxEnqueueBuffer(erase_seq, 3U);
            }
#endif
        }
#if (CLI_HISTORY_SIZE > 0U)
        CLI_HistoryResetNavigation();
#endif
    }
#endif
    else
    {
        if (s_line_index < (CLI_BUFFER_SIZE - 1U))
        {
            s_line_buffer[s_line_index] = (char)ch;
            s_line_index++;
            s_line_buffer[s_line_index] = '\0';

#if (CLI_ENABLE_ECHO == 1U)
            CLI_TxEnqueueByte(ch);
#endif
#if (CLI_HISTORY_SIZE > 0U)
        CLI_HistoryResetNavigation();
#endif
        }
        else
        {
            s_line_overflow = 1U;
        }
    }

    (void)HAL_UART_Receive_IT(s_cli_uart, (uint8_t *)&s_rx_byte, 1U);
}

void CLI_RegisterCommands(const CLI_Command *cmds, uint16_t count)
{
    __disable_irq();
    s_user_cmds = cmds;
    s_user_cmd_count = count;
    __enable_irq();
}

void CLI_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint16_t len;

    if ((s_cli_uart == NULL) || (huart != s_cli_uart))
    {
        return;
    }

    __disable_irq();
    len = s_tx_inflight_len;
    s_tx_tail = (uint16_t)((s_tx_tail + len) % CLI_TX_BUFFER_SIZE);
    s_tx_inflight_len = 0U;
    s_tx_busy = 0U;
    __enable_irq();

    CLI_TxKick();
}

void CLI_Print(const char *text)
{
    size_t len;

    if ((s_cli_uart == NULL) || (text == NULL))
    {
        return;
    }

    len = CLI_StrLen(text);
    if (len == 0U)
    {
        return;
    }

    //LCD_CLI_Print(text, LCD_COLOR_WHITE);
    CLI_TxEnqueueBuffer((const uint8_t *)text, (uint16_t)len);
}

void CLI_PrintPrompt(void)
{
#if (CLI_ENABLE_PROMPT == 1U)
    CLI_Print(CLI_PROMPT_TEXT);
#endif
}

void CLI_Process(void)
{
    uint8_t command_ready_snapshot;

#if (CLI_ENABLE_CURSOR_BLINK == 1U)
    uint32_t now = CLI_Port_GetTick();
#endif

    if (s_tx_kick_pending != 0U)
    {
        s_tx_kick_pending = 0U;
        CLI_TxKick();
    }

    command_ready_snapshot = s_command_ready;
    if (command_ready_snapshot != 0U)
    {
        __disable_irq();
        s_command_ready = 0U;
        __enable_irq();

#if (CLI_ENABLE_CURSOR_BLINK == 1U)
        s_cursor_visible = 0U;
        s_cursor_last_tick = CLI_Port_GetTick();
#endif

        if (s_line_overflow != 0U)
        {
            s_line_overflow = 0U;
            CLI_Print("Input too long\r\n");
        }
        else if (s_tx_overflow != 0U)
        {
            s_tx_overflow = 0U;
            CLI_Print("TX buffer full\r\n");
        }
        else
        {
            CLI_HistoryPush(s_command_buffer);
            CLI_ExecuteCommand(s_command_buffer);
        }

        CLI_PrintPrompt();
        return;
    }

#if (CLI_ENABLE_CURSOR_BLINK == 1U)
    if ((now - s_cursor_last_tick) >= CLI_CURSOR_BLINK_MS)
    {
        if (s_cursor_visible == 0U)
        {
            CLI_Print("_");
            s_cursor_visible = 1U;
        }
        else
        {
            CLI_Print("\b \b");
            s_cursor_visible = 0U;
        }
        s_cursor_last_tick = now;
    }
#endif
}
