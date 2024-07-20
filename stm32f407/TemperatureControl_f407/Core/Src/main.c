/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "keyboard.h"
#include "dht11.h"
#include "ILI93xx.h"
#include <stdio.h> 
#include <string.h>
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
char buffer[100];
  
float get_max_temperature(){
	uint8_t key = get_each_key();
	float sum = 0.0;
	float point = 1.0;
    if(key == 251){
	    while(1){
			uint8_t pre_key = key;
			key = get_each_key();
			if(key == 249)break;
			if (key <= 9 && key != pre_key){
			    sum = ((sum*10.0)+key)/point;
			}
			sprintf(buffer, "maxTemperature=%.1f", sum);
			LCD_Clear(WHITE);
		    LCD_ShowString(30,40,310,24,24, "input max temp");
		    LCD_ShowString(5,120,310,24,24, buffer);
			if (key == 254 && key != pre_key) {point = 10.0;}
			HAL_Delay(50);
		}
	}
	if(sum)return sum;
	else return 255;
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int fputc(int ch,FILE *f)
{
    HAL_UART_Transmit(&huart1,(uint8_t *) &ch,1,HAL_MAX_DELAY);
    return ch;    
}
int fgetc(FILE *f)
{
    uint8_t ch;
    HAL_UART_Receive(&huart1,(uint8_t *)&ch,1,HAL_MAX_DELAY);
    return ch;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint16_t temperature;
  uint16_t humidity;
  float max_temperature = 99.9;
  uint8_t integer_part;
  uint8_t decimal_part;
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
  MX_FSMC_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  TFTLCD_Init();
  
  while(DHT11_Init())
  {
    printf("DHT11 Checked failed!!!\r\n");
    HAL_Delay(500);
  }
    printf("DHT11 Checked Sucess!!!\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	if(get_each_key() == 251)
	max_temperature = get_max_temperature();
	
	
	memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "maxTemperature=%.1f", max_temperature);
	LCD_Clear(WHITE);
	LCD_ShowString(5,40,310,24,24, buffer);
    printf("max temperature =%.1f \r\n",max_temperature);
	
	
	DHT11_Read_Data(&temperature,&humidity);
    printf("DHT11 Temperature = %d.%d degree\r\n",temperature>>8,temperature&0xff);
	integer_part = temperature >> 8;
	decimal_part = temperature & 0xff;
    float temp_float = integer_part + (decimal_part / 10.0);

    // 使用 sprintf 将浮点数格式化到字符串中
	memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Temperature=%.1f", temp_float);
	LCD_ShowString(30,120,210,24,24, buffer);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	if(temp_float >= max_temperature){
	  HAL_GPIO_WritePin(relay_out_GPIO_Port, relay_out_Pin, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
	}
	if(temp_float < max_temperature){
	  HAL_GPIO_WritePin(relay_out_GPIO_Port, relay_out_Pin, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
	}
	HAL_Delay(500);
		
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line numb
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
