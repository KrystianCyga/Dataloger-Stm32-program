/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "hx711.h"
#include "ds18b20.h"
#include "i2c-lcd.h"
#include "fatfs_sd.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_LEN  1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint32_t lastCapture = 0;
volatile uint32_t capturedBuffer[2] = {0};
volatile uint8_t bufferIndex = 0;
volatile uint32_t compensation = 0;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart2;
uint8_t RX_BUFFER[BUFFER_LEN] = {0};
enum LCD_state {RPM_TEM, WGHT_RPM, TEM_WGHT};
enum LCD_state current_lcd_state = RPM_TEM;
//tests
FATFS fs;  // file system
FIL fil; // File
FILINFO fno;
FRESULT fresult;  // result
UINT br, bw;  // File read/write count

/**** capacity related *****/
FATFS *pfs;
DWORD fre_clust;
uint32_t total, free_space;

#define BUFFER_SIZE 128
char buffer[BUFFER_SIZE];  // to store strings..

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint32_t measure_RPM(); // if data from sensor is available return RPM value
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int bufsize (char *buf)
{
	int i=0;
	while (*buf++ != '\0') i++;
	return i;
}

void clear_buffer (void)
{
	for (int i=0; i<BUFFER_SIZE; i++) buffer[i] = '\0';
}

int _write(int file, char *ptr, int len)
{
	for(int i = 0; i < len; i++)
	{
		ITM_SendChar(*ptr++);
	}
	return len;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == RPM_SENSOR_Pin)
  {
	  uint32_t now = TIM16->CNT; // Get current timer count + compensation
	  uint32_t tmp_compensation = compensation;
	  compensation = 0; // zero compensation value
	  uint32_t delta = now + tmp_compensation - lastCapture; // Calculate time difference
	  lastCapture = now;
	  capturedBuffer[bufferIndex] = delta; // Store time difference in buffer
	  bufferIndex = (bufferIndex + 1) % 2; // Wrap buffer index
  }
  if(GPIO_Pin == B1_Pin){
	  current_lcd_state++;
	  if(current_lcd_state > TEM_WGHT)
		  current_lcd_state = RPM_TEM;
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim16)
	{
		compensation += 0xFFFF; // compensate for timer overflow
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == huart1.Instance)
    {
    HAL_UART_Receive_IT(&huart1, RX_BUFFER, BUFFER_LEN);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_FATFS_Init();
  MX_RTC_Init();
  MX_TIM6_Init();
  MX_TIM16_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */

  //TEST STARTs
  	HAL_Delay (500);
	fresult = f_mount(&fs, "/", 1);
	if (fresult != FR_OK){
		printf("ERROR!!! in mounting SD CARD...\n\n");
		printf("%d\n", fresult);
	}
	else printf("SD CARD mounted successfully...\n\n");


	/*************** Card capacity details ********************/

	/* Check free space */
	f_getfree("", &fre_clust, &pfs);

	total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
	sprintf (buffer, "SD CARD Total Size: \t%lu\n",total);
	printf(buffer);
	clear_buffer();
	free_space = (uint32_t)(fre_clust * pfs->csize * 0.5);
	sprintf (buffer, "SD CARD Free Space: \t%lu\n\n",free_space);
	printf(buffer);
	clear_buffer();


	/************* The following operation is using PUTS and GETS *********************/

	/* Open file to write/ create a file if it doesn't exist */
	fresult = f_open(&fil, "file1.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

	/* Writing text */
	f_puts("This data is from the FILE1.txt. And it was written using ...f_puts... ", &fil);

	/* Close file */
	fresult = f_close(&fil);

	if (fresult == FR_OK)printf ("File1.txt created and the data is written \n");

	/* Open file to read */
	fresult = f_open(&fil, "file1.txt", FA_READ);

	/* Read string from the file */
	f_gets(buffer, f_size(&fil), &fil);

	printf("File1.txt is opened and it contains the data as shown below\n");
	printf(buffer);
	printf("\n\n");

	/* Close file */
	f_close(&fil);

	clear_buffer();
  //TEST ENDS

  hx711_t loadcell = {0};
  hx711_init(&loadcell, HX711_CLK_GPIO_Port, HX711_CLK_Pin, HX711_DAT_GPIO_Port, HX711_DAT_Pin);
  //hx711_coef_set(&loadcell, 354.5); // read after calibration
  //hx711_tare(&loadcell, 10);
  loadcell.coef = 1;
  loadcell.offset = 0;
  HAL_TIM_Base_Start_IT(&htim16);

  lcd_init();

  //Temperatura
    float temperature;
    char uart_buf[32];
    DS18B20_Init(DS18B20_Resolution_12bits);
    DS18B20_ReadAll();
    DS18B20_StartAll();
    uint8_t ROM_tmp[8];
    uint8_t i;


  //Bluetooth
  HAL_UART_Receive_IT(&huart1, RX_BUFFER, BUFFER_LEN);
  char data[20]; // Array to store the formatted string
  uint8_t txData[20]; // Array to hold the bytes to be transmitted


  //RTC
  	  	char time[30];
    	int meslen=0;
    	RTC_TimeTypeDef RtcTime;
    	RTC_TimeTypeDef RtcDate;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  char rpm_msq[16], temp_msg[16], weight_msg[16];

	  int rpm = measure_RPM();
	  if(current_lcd_state == RPM_TEM || current_lcd_state == WGHT_RPM)
		  sprintf(rpm_msq, "RPM: %d", rpm);

	  float weight = hx711_weight(&loadcell, 1);
	  if(current_lcd_state == WGHT_RPM || current_lcd_state == TEM_WGHT)
		  sprintf(weight_msg, "Weight: %f", weight);

	  for(i = 0; i < DS18B20_Quantity(); i++)
	    	{
	    		if(DS18B20_GetTemperature(i, &temperature))
	    		{
	    			DS18B20_GetROM(i, ROM_tmp);

	    		}
	    	}
	  if(current_lcd_state == RPM_TEM || current_lcd_state == TEM_WGHT)
		  sprintf(temp_msg, "Temp: %f", temperature);

	  lcd_clear();
	  lcd_put_cur(0, 0);
	  switch(current_lcd_state){
	  case RPM_TEM:
		  lcd_send_string(rpm_msq);
		  lcd_put_cur(1, 0);
		  lcd_send_string(temp_msg);
		  break;
	  case WGHT_RPM:
		  lcd_send_string(weight_msg);
		  lcd_put_cur(1, 0);
		  lcd_send_string(rpm_msq);
		  break;
	  case TEM_WGHT:
		  lcd_send_string(temp_msg);
		  lcd_put_cur(1, 0);
		  lcd_send_string(weight_msg);
		  break;
	  }
	  HAL_Delay(500);

	  //pobieranie czasu
	  HAL_RTC_GetTime(&hrtc, &RtcTime, RTC_FORMAT_BIN);
	  HAL_RTC_GetDate(&hrtc, &RtcDate, RTC_FORMAT_BIN);
	  meslen=sprintf((char*)time, "Time: %02d:%02d:%02d\n\r", RtcTime.Hours, RtcTime.Minutes, RtcTime.Seconds);

	 // Wysyłanie do bluetooth
		sprintf(data, "%d,%d,%d,%d;", rpm_msq, temp_msg, weight_msg, 420);
		memcpy(txData, data, strlen(data));
		HAL_UART_Transmit(&huart1, txData, strlen(data), HAL_MAX_DELAY);


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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_USART3
                              |RCC_PERIPHCLK_I2C2;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
uint32_t measure_RPM(){
	uint32_t delta1 = capturedBuffer[0];
	uint32_t delta2 = capturedBuffer[1];
	if (delta1 > 0 && delta2 > 0) {
		uint32_t delta = delta1 + delta2;
		float frequency = 1.0f / (delta / 2.0f / 10000.0f ); // Calculate frequency in Hz
		uint32_t rpm = frequency * 60.0f; // Calculate RPM
		capturedBuffer[0] = 0;
		capturedBuffer[1] = 0;
		return rpm;
	}
	else return 0;
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

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
