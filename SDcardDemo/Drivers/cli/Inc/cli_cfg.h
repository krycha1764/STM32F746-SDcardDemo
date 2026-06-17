#ifndef CLI_CFG_H
#define CLI_CFG_H

/* CLI input line buffer length including terminating null byte. */
#ifndef CLI_BUFFER_SIZE
#define CLI_BUFFER_SIZE              128U
#endif

/* Maximum number of parsed arguments including command token. */
#ifndef CLI_MAX_ARGS
#define CLI_MAX_ARGS                 8U
#endif

/* TX ring buffer size in bytes for non-blocking UART transmit. */
#ifndef CLI_TX_BUFFER_SIZE
#define CLI_TX_BUFFER_SIZE           1024U
#endif

/* Number of commands stored in history (Up arrow recall). */
#ifndef CLI_HISTORY_SIZE
#define CLI_HISTORY_SIZE             10U
#endif

/* Enable character echo on RX. */
#ifndef CLI_ENABLE_ECHO
#define CLI_ENABLE_ECHO              1U
#endif

/* Enable handling of Backspace (0x08) and DEL (0x7F). */
#ifndef CLI_ENABLE_BACKSPACE
#define CLI_ENABLE_BACKSPACE         1U
#endif

/* Print prompt after command processing. */
#ifndef CLI_ENABLE_PROMPT
#define CLI_ENABLE_PROMPT            1U
#endif

/* Optional blinking cursor support in CLI_Process(). */
#ifndef CLI_ENABLE_CURSOR_BLINK
#define CLI_ENABLE_CURSOR_BLINK      0U
#endif

#ifndef CLI_CURSOR_BLINK_MS
#define CLI_CURSOR_BLINK_MS          500U
#endif

#ifndef CLI_PROMPT_TEXT
#define CLI_PROMPT_TEXT              "> "
#endif

/* Kept for compatibility when replacing TX backend with blocking transmit. */
#ifndef CLI_TX_TIMEOUT_MS
#define CLI_TX_TIMEOUT_MS            50U
#endif

#endif /* CLI_CFG_H */
