/*
 * LCD_CLI.c
 *
 *  Created on: 16 cze 2026
 *      Author: mikolaj
 */

#include "LCD_CLI.h"

#include "stm32746g_discovery_lcd.h"

void LCD_CLI_Clear() {
	BSP_LCD_Clear(LCD_COLOR_BLACK);
}
