/*
 * image.c
 *
 *  Created on: 19 cze 2026
 *      Author: mikolaj
 */

#include "image.h"

#include <stdio.h>
#include "stm32746g_discovery_lcd.h"
#include "cli.h"
#include "fatfs.h"
#include "dma2d.h"

static uint8_t __attribute__((section(".ExternalRAMMemory"))) line[RK043FN48H_WIDTH * 4];
uint8_t __attribute__((section(".ExternalRAMMemory"))) bitmap[RK043FN48H_WIDTH * RK043FN48H_HEIGHT * 4 + 150];
extern uint32_t framebuffer[];

static char textbuff[256] = {0};

#define BMP_header 0x4D42
#define JPG_header 0xD8FF
#define PNG_header 0x8950

static int8_t LCD_Display_BMP(FIL* file);
static int8_t LCD_Display_JPG(FIL* file);
static int8_t LCD_Display_PNG(FIL* file);

int8_t LCD_Display_image(const char* filename) {
	FIL file;
	FRESULT fres;
	int8_t ret = 0;

	fres = f_open(&file, filename, FA_READ);
	if(fres != FR_OK) {
		sprintf(textbuff, "Error opening %s file\r\n", filename);
		CLI_Print(textbuff);
		ret = -1;
		goto error;
	}

	uint16_t header;
	unsigned int to_read = 2;
	fres = f_read(&file, (uint8_t*)&header, to_read, &to_read);
	if((fres != FR_OK) || (to_read != 2)) {
		sprintf(textbuff, "Error reading %s file\r\n", filename);
		CLI_Print(textbuff);
		ret = -1;
		goto error;
	}
	f_rewind(&file);
	switch(header) {
	case BMP_header:
		CLI_Print("BMP image\r\n");
		ret = LCD_Display_BMP(&file);
		break;
	case JPG_header:
		CLI_Print("JPG image\r\n");
		ret = LCD_Display_JPG(&file);
		break;
	case PNG_header:
		CLI_Print("PNG image\r\n");
		ret = LCD_Display_PNG(&file);
		break;
	default:
		CLI_Print("Unknown image type\r\n");
		ret = -1;
		break;
	}

error:
	f_close(&file);
	return ret;
}

static int8_t LCD_Display_BMP(FIL* file) {
	int8_t ret = 0;
	uint32_t height = 0, width = 0;
	FRESULT fres;

	unsigned int to_read = 30;
	fres = f_read(file, line, to_read, &to_read);
	if((fres != FR_OK) || (to_read != 30)) {
		CLI_Print("Error reading file\r\n");
		ret = -2; goto error;
	}
	width = line[18] + (line[19] << 8) + (line[20] << 16)  + (line[21] << 24);
	height = line[22] + (line[23] << 8) + (line[24] << 16)  + (line[25] << 24);

	if((width > RK043FN48H_WIDTH) || (height > RK043FN48H_HEIGHT)) {
		CLI_Print("Image too large\r\n");
		ret = -2; goto error;
	}

	f_lseek(file, 0);
	fres = f_read(file, bitmap, f_size(file), NULL);
	if(fres != FR_OK) {
		CLI_Print("Error reading file\r\n");
		ret = -2; goto error;
	}
	BSP_LCD_DrawBitmap(0, 0, bitmap);

error:
	return ret;
}

static int8_t LCD_Display_JPG(FIL* file) {
	int8_t ret = 0;
	CLI_Print("Not done yet\r\n");
	return ret;
}

static int8_t LCD_Display_PNG(FIL* file) {
	int8_t ret = 0;
	CLI_Print("Not done yet\r\n");
	return ret;
}
