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
#define BUFFER_LEN  1
#include "liquidcrystal_i2c.h"
#include "fatfs_sd.h"
#include "string.h"
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
volatile uint32_t lastCapture = 0;
volatile uint32_t capturedBuffer[2] = {0};
volatile uint8_t bufferIndex = 0;
volatile uint32_t compensation = 0;
UART_HandleTypeDef huart1;
uint8_t RX_BUFFER[BUFFER_LEN] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint32_t measure_RPM(); // if data from sensor is available return RPM value
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart2;
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
  FATFS fs;
  FIL fil;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_FATFS_Init();
  MX_RTC_Init();
  MX_TIM6_Init();
  MX_TIM16_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  hx711_t loadcell = {0};
  uint32_t rpm;
  float weight;
  hx711_init(&loadcell, HX711_CLK_GPIO_Port, HX711_CLK_Pin, HX711_DAT_GPIO_Port, HX711_DAT_Pin);
  //hx711_coef_set(&loadcell, 354.5); // read after calibration
  //hx711_tare(&loadcell, 10);
  loadcell.coef = 1;
  loadcell.offset = 0;
  HAL_TIM_Base_Start_IT(&htim16);
  HAL_GPIO_WritePin(RED_DIODE_GPIO_Port, RED_DIODE_Pin, GPIO_PIN_RESET);

  float temperature;
  DS18B20_Init(DS18B20_Resolution_12bits);
  char uart_buf[32];

  HAL_UART_Receive_IT(&huart1, RX_BUFFER, BUFFER_LEN);

    HD44780_Init(2);
    HD44780_Clear();
    HD44780_SetCursor(0,0);
    HD44780_PrintStr("HELLO");
    HD44780_SetCursor(10,1);
    HD44780_PrintStr("WORLD");
    HAL_Delay(2000);

      HAL_Delay(500);
      f_mount(&fs, "", 0);
      f_open(&fil, "write.txt", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
      f_lseek(&fil, fil.fptr);
      f_puts("Hello \n", &fil);
      f_close(&fil);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /*if(HAL_GPIO_ReadPin(RPM_SENSOR_GPIO_Port, RPM_SENSOR_Pin) == GPIO_PIN_RESET)
		  HAL_GPIO_WritePin(RED_DIODE_GPIO_Port, RED_DIODE_Pin, GPIO_PIN_SET);
	  else
		  HAL_GPIO_WritePin(RED_DIODE_GPIO_Port, RED_DIODE_Pin, GPIO_PIN_RESET);*/
	  rpm = measure_RPM();
	  if(rpm > 0) printf("RPM value: %lu\n", rpm);
	  weight = hx711_weight(&loadcell, 1);
	  printf("Weight value : %f\n", weight);

	  	  	DS18B20_ReadAll();
	    	DS18B20_StartAll();
	    	uint8_t ROM_tmp[8];
	    	uint8_t i;


	  for(i = 0; i < DS18B20_Quantity(); i++)
	    	{
	    		if(DS18B20_GetTemperature(i, &temperature))
	    		{
	    			DS18B20_GetROM(i, ROM_tmp);
	    					sprintf(uart_buf, "Temperature: %.2f\r\n", temperature);
	    			        HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, strlen(uart_buf), 100);
	    			        HAL_UART_Transmit(&huart1, (uint8_t*)uart_buf, strlen(uart_buf), 100);

	    		}
	    	}

	  if(RX_BUFFER[0] == '1')
	  	          {
		  	  	  HAL_GPIO_WritePin(test_GPIO_Port, test_Pin, 1);
	  	          }
	  	          else if(RX_BUFFER[0] == '0')
	  	          {

	  	        	HAL_GPIO_WritePin(test_GPIO_Port, test_Pin, 0);
	  	          }
	  //HAL_Delay(500);


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
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
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
