/*
 * LCD_CLI.c
 *
 *  Created on: 16 cze 2026
 *      Author: mikolaj
 */

#include "LCD_CLI.h"

#include "dma2d.h"
#include <string.h>
#include "stm32746g_discovery_lcd.h"

static int16_t column = 0;

void LL_FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);

void LCD_CLI_Clear() {
	BSP_LCD_Clear(LCD_COLOR_BLACK);
}

void LCD_CLI_NL() {
	HAL_DMA2D_PollForTransfer(&hdma2d, 10);
	hdma2d.Init.Mode         = DMA2D_M2M;
	hdma2d.Init.ColorMode    = DMA2D_ARGB8888;
	hdma2d.Init.OutputOffset = 0;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputAlpha = 0xFF;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_ARGB8888;
	hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.Instance = DMA2D;

	HAL_DMA2D_Init(&hdma2d);
	HAL_DMA2D_ConfigLayer(&hdma2d, 1);
	HAL_DMA2D_Start(&hdma2d, (uint32_t)0xC0000000 + (4*(480*17)), (uint32_t)0xC0000000, 480, 252);
	HAL_DMA2D_PollForTransfer(&hdma2d, 10);

	LL_FillBuffer(1,(uint32_t*)(0xC0000000 + (4*480*21*12)) , 480, 17, 0, LCD_COLOR_BLACK);
	column = 0;
}

void LCD_CLI_PutChar(char c, uint32_t color) {
	if(column >= 43) {
		LCD_CLI_NL();
		column = 0;
	}
	BSP_LCD_SetTextColor(color);
	BSP_LCD_DisplayChar(column*11, LINE(16), c);
	column++;
}

void LCD_CLI_Print(const char* c, uint32_t color) {
	for(uint16_t i = 0; i < strlen(c); i++) {
		switch(c[i]) {
		case '\n':
			LCD_CLI_NL();
			break;
		case '\r':
			break;
		case '\b':
			column--;
			if(column < 0) column = 0;
			LCD_CLI_PutChar(' ', LCD_COLOR_BLACK);
			column--;
			if(column < 0) column = 0;
			break;
		case '\t':
			LCD_CLI_PutChar(' ', color);
			LCD_CLI_PutChar(' ', color);
			LCD_CLI_PutChar(' ', color);
			LCD_CLI_PutChar(' ', color);
			break;
		default:
			LCD_CLI_PutChar(c[i], color);
		}
	}
}

void LL_FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex) {
  /* Register to memory mode with ARGB8888 as color Mode */
  hdma2d.Init.Mode         = DMA2D_R2M;
  hdma2d.Init.ColorMode    = DMA2D_ARGB8888;
  hdma2d.Init.OutputOffset = OffLine;
  hdma2d.Instance = DMA2D;

  HAL_DMA2D_Init(&hdma2d);
  HAL_DMA2D_ConfigLayer(&hdma2d, LayerIndex);
  HAL_DMA2D_Start(&hdma2d, ColorIndex, (uint32_t)pDst, xSize, ySize);
  HAL_DMA2D_PollForTransfer(&hdma2d, 10);
}
