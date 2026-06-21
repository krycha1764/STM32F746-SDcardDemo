/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "dma2d.h"
#include "fatfs.h"
#include "i2c.h"
#include "sdmmc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fatfs_platform.h"
#include "stm32746g_discovery_lcd.h"
#include "cli.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
FATFS fs;
FRESULT fr;
size_t textlen = 0;
char text[512] = {0};
char version[128] = "0.1.1";

static void App_CmdVersion(int argc, char **argv);
static void App_CmdReset(int argc, char **argv);
static void App_CmdSdPWD(int argc, char **argv);
static void App_CmdSdStatus(int argc, char **argv);
static void App_CmdSdLS(int argc, char **argv);
static void App_CmdSdCD(int argc, char **argv);
static void App_CmdSdTUCH(int argc, char **argv);
static void App_CmdSdMKDIR(int argc, char **argv);
static void App_CmdSdRM(int argc, char **argv);
static void App_CmdSdCP(int argc, char **argv);
static void App_CmdSdMV(int argc, char **argv);
static void App_CmdSdCAT(int argc, char **argv);
static void App_CmdSdHEAD(int argc, char **argv);
static void App_CmdSdTAIL(int argc, char **argv);
static void App_CmdSdDU(int argc, char **argv);
static void App_CmdSdOPEN_GRAPHIC(int argc, char **argv);

static const CLI_Command app_commands[] = {
  {"version", App_CmdVersion, "Show firmware version"},
  {"reset", App_CmdReset, "Perform MCU reset"},
  {"pwd", App_CmdSdPWD, "Print the current directory patch"},
  {"sd", App_CmdSdStatus, "sd status - show SD card info"},
  {"ls", App_CmdSdLS, "ls [path] - list files in directory"},
  {"cd", App_CmdSdCD, "cd [path] - change directory to"},
  {"tuch", App_CmdSdTUCH, "tuch <text> - make new file with text"},
  {"mkdir", App_CmdSdMKDIR, "mkdir [name] - create a directory"},
  {"rm", App_CmdSdRM, "rm [file] - remove file"},
  {"cp", App_CmdSdCP, "cp [path] [path] - copy file to"},
  {"mv", App_CmdSdMV, "mv [path] [path] - move file to"},
  {"cat", App_CmdSdCAT, "cat [path] - concatenate files and print on the standard output"},
  {"head", App_CmdSdHEAD, "head [path] - output the first part of files"},
  {"tail", App_CmdSdTAIL, "tail [path] - output the last part of files"},
  {"du", App_CmdSdDU, "du [path] - estimate file space usage"},
  {"open_file", App_CmdSdOPEN_GRAPHIC, "nie wiem, Mikołaj to dla Ciebie funkcja do grafiki"},
};
// Polecenie CLI: sd status
static void App_CmdSdStatus(int argc, char **argv)
{
    if (argc < 2 || strcmp(argv[1], "status") != 0) {
    CLI_Print("Usage: sd status\r\n");
    return;
  }

  CLI_Print("Work in progress...\r\n");

}

static void App_CmdVersion(int argc, char **argv){
	textlen = sprintf(text, "software version: \r\n %s \r\n", version);
	CLI_Print(text);
}

static void App_CmdReset(int argc, char **argv){
	NVIC_SystemReset();
}
static void App_CmdSdPWD(int argc, char **argv){
	f_getcwd(text, 512);
	CLI_Print(text);
}
static void App_CmdSdLS(int argc, char **argv){
	static FILINFO fno;
	DIR dir;
	FRESULT res;

	if ((argc < 2) || (strcmp(argv[1],".")==0)){
		if(f_opendir(&dir, ".")){
			CLI_Print("Open Error");
			return;
		}
	}else{
		if(!(f_opendir(&dir, argv[1])== FR_OK)){
			textlen = sprintf(text, "ls: this directory not exist: %s\r\n", argv[1]);
			CLI_Print(text);
		}
	}

	for (;;) {
	        res = f_readdir(&dir, &fno);
	        if (res != FR_OK || fno.fname[0] == 0) break;


	        if (fno.fattrib & AM_DIR) {

	            textlen=sprintf(text,"  [DIR]  %s\r\n", fno.fname);
	            CLI_Print(text);
	        } else {

	            textlen=sprintf(text,"  %9lu b  %s\r\n", fno.fsize, fno.fname);
	            CLI_Print(text);
	        }
	    }

	    f_closedir(&dir);
}

static void App_CmdSdCD(int argc, char **argv){
	if (argc < 2) {
	    CLI_Print("Usage: cd [directory]\r\n");
	    return;
	}
	FRESULT res;

	res = f_chdir(argv[1]);
	if (res != FR_OK) {
		textlen = sprintf(text, "cd: this directory not exist: %s\r\n", argv[1]);
		CLI_Print(text);
	}


}
static void App_CmdSdTUCH(int argc, char **argv){
	FIL file;
	if (argc < 2) {
	    CLI_Print("Usage: tuch file_name\r\n");
	    return;
	  }
	if(f_open(&file, argv[1], FA_WRITE | FA_CREATE_NEW)){
		CLI_Print("file make error");
		return;
	}
	if (argc >= 3) {
	        for (int i = 2; i < argc; i++) {
	            f_printf(&file, "%s", argv[i]);
	            if (i < argc - 1) {
	                f_printf(&file, " ");
	            }
	        }
	f_printf(&file, "\r\n");
	}
	f_close(&file);
}
static void App_CmdSdMKDIR(int argc, char **argv){
	if (argc < 2) {
		    CLI_Print("Usage: mkdir directory_name\r\n");
		    return;
	}
	switch (f_mkdir(argv[1])){
	case FR_EXIST:
		CLI_Print("dir exist\r\n");
		break;
	case FR_OK:
		break;
	default:
		CLI_Print("mkdir error\r\n");
	}

}
static void App_CmdSdRM(int argc, char **argv){
	if (argc < 2) {
			    CLI_Print("Usage: rm file_name\r\n");
			    return;
		}
	switch (f_unlink(argv[1])){
	case FR_NO_FILE:
		CLI_Print("File not exist\r\n");
		break;
	case FR_NO_PATH:
		CLI_Print("Directory not exist\r\n");
		break;
	case FR_DENIED:
		CLI_Print("Acces denied\r\n");
		break;
	case FR_OK:
		break;
	default:
		CLI_Print("rm: error\r\n");
	}
}
static void App_CmdSdCP(int argc, char **argv){
	if (argc < 3) {
		CLI_Print("Usage: rm source_file destyny_file\r\n");
		return;
	}
	FIL fsrc;
	FIL fdst;
	UINT br, bw;
	static BYTE buffer[512];

	if(f_open(&fsrc, argv[1], FA_READ)){
		CLI_Print("source file open failed\r\n");
		f_close(&fsrc);
		return;
	}

	if(f_open(&fdst, argv[2], FA_WRITE | FA_CREATE_ALWAYS)){
		CLI_Print("destiny file make failed\r\n");
		f_close(&fsrc);
		return;
	}

	while(1){
		if(f_read(&fsrc, buffer, sizeof(buffer), &br)){
			CLI_Print("saving data failed\r\n");
			break;
		}
		if (br == 0) break;
		if(f_write(&fdst, buffer, br, &bw)){
			CLI_Print("saving data failed\r\n");
			break;
		}
	}
	f_close(&fsrc);
	f_close(&fdst);

}
static void App_CmdSdMV(int argc, char **argv){
	if (argc < 3) {
		CLI_Print("Usage: mv source_file destyny_file\r\n");
		return;
	}
	switch (f_rename(argv[1], argv[2])){
	case FR_OK:
		break;
	case FR_NO_FILE:
		CLI_Print("source file not exsist\r\n");
		break;
	case FR_NO_PATH:
		CLI_Print("destiny path not exist\r\n");
		break;
	default:
		CLI_Print("mv: error\r\n");


	}

}
static void App_CmdSdCAT(int argc, char **argv){
	if (argc < 2) {
		CLI_Print("Usage: cat file \r\n");
		return;
	}
	FIL file;
	UINT br;
	if(f_open(&file, argv[1], FA_READ)){
		CLI_Print("file doesn't exist\r\n");
		return;
	}

	while(1){
		if(f_read(&file, text, sizeof(text) - 1, &br)){
			CLI_Print("file opening failed\r\n");
			return;
		}
		if (br == 0) break;
		text[br]='\0';
		CLI_Print(text);
	}
	f_close(&file);
	CLI_Print("\r\n");

}
static void App_CmdSdHEAD(int argc, char **argv){
	if (argc < 2) {
			CLI_Print("Usage: head file [line_number] \r\n");
			return;
		}
	int req_lines = (argc >= 3) ? atoi(argv[2]) : 10;
	FIL file;
	UINT br;
	int line_count = 0;
	if(f_open(&file, argv[1], FA_READ)){
			CLI_Print("file doesn't exist\r\n");
			return;
		}

		while(1){
			if(f_read(&file, text, sizeof(text) - 1, &br)){
				CLI_Print("file opening failed\r\n");
				return;

			}
			if(br == 0) break;
			for (UINT i = 0; i < br; i++) {
				if (text[i] == '\n') {
					line_count++;
					if (line_count >= req_lines) {
						text[i + 1] = '\0';
			            br = i + 1;
			            break;
			        }
				}
			}
			text[br] = '\0';
				CLI_Print(text);
			    if (line_count >= req_lines) break;
		}
	f_close(&file);
	CLI_Print("\r\n");

}
static void App_CmdSdTAIL(int argc, char **argv){
	if (argc < 2) {
				CLI_Print("Usage: tail file [line_number] \r\n");
				return;
			}
		int req_lines = (argc >= 3) ? atoi(argv[2]) : 10;
		FIL file;
		UINT br;
		FRESULT res;

		if(f_open(&file, argv[1], FA_READ)){
				CLI_Print("file doesn't exist\r\n");
				return;
			}
			DWORD file_size = f_size(&file);
		    if (file_size == 0) {
		        f_close(&file);
		        return;
		    }
			DWORD pos = file_size;
		    int lines_found = 0;
		    char c;

			while(pos>0){
				pos--;
				f_lseek(&file, pos);

				if(f_read(&file, &c, 1, &br)){
					CLI_Print("file opening failed\r\n");
					return;
				}
				if (c == '\n') {
					if (pos < file_size - 1) {
						lines_found++;
						if (lines_found > req_lines) {
							pos++;
							break;
						}
					}
				}
			}
			f_lseek(&file, pos);
			for (;;) {
				res = f_read(&file, text, sizeof(text) - 1, &br);
				if (res != FR_OK || br == 0) break;
				text[br] = '\0';
				CLI_Print(text);
			}
			f_close(&file);
			CLI_Print("\r\n");

}
static void App_CmdSdDU(int argc, char **argv){
	if (argc < 2) {
		CLI_Print("Usage: du file_name \r\n");
		return;
	}
	FILINFO fno;
	if(f_stat(argv[1], &fno)){
		CLI_Print("access dined\r\n");
		return;
	}
	if (fno.fattrib & AM_DIR){
		CLI_Print("i can't measured directories\r\n");
		return;
	}
	uint32_t file_bytes = fno.fsize;

	if (file_bytes < 1024) {
	        textlen = sprintf(text, "%lu B\t%s\r\n", (unsigned long)file_bytes, argv[1]);
	    } else if (file_bytes < 1024 * 1024) {
	        textlen = sprintf(text, "%lu KB\t%s\r\n", (unsigned long)(file_bytes / 1024), argv[1]);
	    } else {
	        unsigned long mb = (unsigned long)(file_bytes / (1024 * 1024));
	        unsigned long kb_remainder = (unsigned long)((file_bytes % (1024 * 1024)) / 102400);
	        textlen = sprintf(text, "%lu.%lu MB\t%s\r\n", mb, kb_remainder, argv[1]);
	    }

	CLI_Print(text);

}
static void App_CmdSdOPEN_GRAPHIC(int argc, char **argv){}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DMA2D_Init();
  MX_I2C3_Init();
  MX_SDMMC1_SD_Init();
  MX_USART1_UART_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */

  BSP_SDRAM_Init();
  BSP_LCD_Init();

  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_DisplayOn();
  BSP_LCD_SelectLayer(0);
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  BSP_LCD_SetFont(&Font16);
  BSP_LCD_Clear(LCD_COLOR_BLACK);

  CLI_Init(&huart1);
  CLI_RegisterCommands(app_commands, (uint16_t)(sizeof(app_commands) / sizeof(app_commands[0])));
  CLI_StartReception();

  while(BSP_PlatformIsDetected() != SD_PRESENT)
  {
	  textlen = sprintf(text, "Please insert SD Card...\r\n");
	  CLI_Print(text);
	  HAL_Delay(500);
  }
  HAL_SD_Init(&hsd1);
  //BSP_SD_Init();
  //FATFS_LinkDriver(&SD_Driver, SDPath);

  HAL_SD_CardInfoTypeDef sdinfo;
  HAL_SD_GetCardInfo(&hsd1, &sdinfo);
  uint64_t sdsize = (uint64_t)sdinfo.BlockNbr * (uint64_t)sdinfo.BlockSize;
  uint32_t sdsizeMB = sdsize/(1024*1024);

  textlen = sprintf(text, "SD Card info:\r\n\tType: %lu\r\n\tClass: %lu\r\n\tSize: %lu MiB\r\n"
		  , sdinfo.CardType, sdinfo.Class, sdsizeMB);
  CLI_Print(text);

  FIL file;
  fr = f_mount(&fs, SDPath, 0);
  fr = f_open(&file, "test.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
  textlen = sprintf(text, "Please insert SD Card...\r\n");
  fr = f_write(&file, text, textlen, NULL);
  fr = f_printf(&file, "timestamp: %lu\r\n", HAL_GetTick());
  fr = f_printf(&file, "Hello\r\n");
  fr = f_sync(&file);
  fr = f_close(&file);

  CLI_PrintPrompt();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  CLI_Process();
	  BSP_LCD_Clear(LCD_COLOR_RED);
	  HAL_Delay(1000);
	  BSP_LCD_Clear(LCD_COLOR_BLUE);
	  HAL_Delay(1000);


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_SDMMC1
                              |RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV8;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  CLI_RxCpltCallback(huart);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  CLI_TxCpltCallback(huart);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
