/*
 * LCD_CLI.h
 *
 *  Created on: 16 cze 2026
 *      Author: mikolaj
 */

#ifndef INC_LCD_CLI_H_
#define INC_LCD_CLI_H_

#include <inttypes.h>

void LCD_CLI_Clear();
void LCD_CLI_NL();
void LCD_CLI_PutChar(char c, uint32_t color);
void LCD_CLI_Print(const char* c, uint32_t color);

#endif /* INC_LCD_CLI_H_ */
