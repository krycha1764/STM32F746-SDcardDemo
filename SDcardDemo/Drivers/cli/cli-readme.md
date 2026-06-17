# CLI Library for STM32 HAL

Modul CLI to lekka biblioteka do obslugi konsoli tekstowej po UART dla STM32 (HAL).
Projekt jest przygotowany jako biblioteka wielokrotnego uzytku, bez zaleznosci od logiki aplikacyjnej.

## Najwazniejsze cechy

- UART RX oparty o przerwania (`HAL_UART_Receive_IT`)
- Brak dynamicznej alokacji pamieci (tylko bufory statyczne)
- Edycja linii:
  - echo znakow
  - Backspace (`0x08`) i DEL (`0x7F`)
- Parsowanie komend w stylu `argc/argv`
- Rejestracja komend aplikacji przez tablice `CLI_Command`
- Wbudowane `help` i obsluga `Unknown command`
- Prompt (`>`) i opcjonalny migajacy kursor
- Historia 10 ostatnich komend przywolywana strzalka w gore
- Nieblokujacy TX ring buffer oparty o `HAL_UART_Transmit_IT`
- Kod podzielony na API publiczne, konfiguracje i implementacje

## Struktura plikow

- `.../Inc/cli.h` - publiczne API
- `.../Inc/cli_cfg.h` - konfiguracja makrami
- `.../Src/cli.c` - implementacja biblioteki
- `.../Inc/cli_port.h` - warstwa portowania HAL
- `.../Src/cli_port.c` - domyslna implementacja portu

## Publiczne API

Biblioteka udostepnia:

- `void CLI_Init(UART_HandleTypeDef *huart);`
- `void CLI_StartReception(void);`
- `void CLI_Process(void);`
- `void CLI_RxCpltCallback(UART_HandleTypeDef *huart);`
- `void CLI_TxCpltCallback(UART_HandleTypeDef *huart);`
- `void CLI_RegisterCommands(const CLI_Command *cmds, uint16_t count);`
- `void CLI_Print(const char *text);`
- `void CLI_PrintPrompt(void);`

Typy:

- `typedef void (*CLI_CommandHandler)(int argc, char **argv);`
- `typedef struct { const char *name; CLI_CommandHandler handler; const char *help; } CLI_Command;`

## Konfiguracja (`cli_cfg.h`)

Dostepne makra:

- `CLI_BUFFER_SIZE`
- `CLI_MAX_ARGS`
- `CLI_ENABLE_ECHO`
- `CLI_ENABLE_BACKSPACE`
- `CLI_ENABLE_PROMPT`
- `CLI_HISTORY_SIZE`
- `CLI_TX_BUFFER_SIZE`
- `CLI_ENABLE_CURSOR_BLINK`
- `CLI_CURSOR_BLINK_MS`
- `CLI_PROMPT_TEXT`
- `CLI_TX_TIMEOUT_MS`

## Jak uzyc w aplikacji

### Opcja A: Kopiowanie plikow do Core (szybki start)

1. Skopiuj pliki:
   - `cli.h`, `cli_cfg.h`, `cli_port.h` → do `Core/Inc/`
   - `cli.c`, `cli_port.c` → do `Core/Src/`

2. W `main.c` dodaj include:

```c
#include "cli.h"
```

1. Zainicjalizuj CLI po inicjalizacji UART (przed petla `while(1)`):

```c
CLI_Init(&huart3);
CLI_RegisterCommands(app_commands, (uint16_t)(sizeof(app_commands) / sizeof(app_commands[0])));
CLI_StartReception();
CLI_Print("\r\nSTM32 CLI ready. Type help.\r\n");
CLI_PrintPrompt();
```

1. W petli glownej wywoluj:

```c
while (1)
{
  CLI_Process();
  // Twoja logika aplikacji
}
```

1. Przekaz callback HAL do CLI (w sekcji `USER CODE 4`):

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    CLI_RxCpltCallback(huart);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  CLI_TxCpltCallback(huart);
}
```

1. Upewnij sie, ze IRQ UART jest wlaczone w NVIC i handler wywoluje HAL:

```c
void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart3);
}
```

### Opcja B: Dodanie jako katalog w Drivers (zalecane)

1. Skopiuj caly katalog `cli` (z podkatalogami `Inc` i `Src`) do:

```
Drivers/cli/
```

1. W STM32CubeIDE dodaj sciezke include:
   - Projekt → Properties → C/C++ Build → Settings → MCU GCC Compiler → Includes
   - Dodaj: `Drivers/cli/Inc`

2. Dodaj pliki zrodlowe do projektu:
   - `Drivers/cli/Src/cli.c`
   - `Drivers/cli/Src/cli_port.c`

3. Postepuj dalej jak w **Opcji A** od punktu 2 (inicjalizacja, callbacki, petla).

**Uwaga:** Opcja B jest zalecana – ulatwia aktualizacje biblioteki i utrzymanie porzadku w projekcie.

## Przyklad komend aplikacji

Komendy aplikacyjne sa poza biblioteka CLI. Przyklad z projektu:

- `led on|off`
- `blink on|off`
- `blinktime <ms>` (ustawia okres migania LED2)
- `Version`
- `Help`
- `Reset`

## Przeplyw danych

1. Przerwanie UART odbiera 1 bajt i dokleja do bufora linii.
2. Po `\r`/`\n` ustawia flage gotowosci komendy.
3. `CLI_Process()`:
   - parsuje bufor na `argc/argv`
   - uruchamia handler komendy
   - wypisuje prompt
4. Obsluga bledow obejmuje m.in. zbyt dluga linie i nieznana komende.
5. Strzalka gora (`ESC [ A`) przywoluje ostatnie komendy z historii.

## Ograniczenia i dalszy rozwoj

- Nadawanie korzysta z TX ring buffera i `HAL_UART_Transmit_IT`.
- W przyszlosci mozna dodac:
  - DMA dla TX/RX
  - historie komend i auto-uzupelnianie

## Wskazowki integracyjne dla CubeMX

- Wlasny kod umieszczaj w sekcjach `USER CODE`, aby przetrwal regeneracje.
- Utrzymuj rozdzial: biblioteka CLI nie powinna zawierac logiki urzadzenia (LED, silniki, itp.).
