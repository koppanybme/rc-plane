/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "nrf24.h"
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
static uint8_t rxSerialized[32];
static uint32_t time;
static volatile uint32_t packets_received;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint16_t ComputePulseWidth(uint16_t adcVal, uint16_t min, uint16_t max);
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
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  TIM3->CCR1 = 1500;
  TIM3->CCR2 = 1500;
  TIM3->CCR3 = 1500;
  TIM3->CCR4 = 1500;
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  NRF24_Init(1);
  uint8_t config = NRF24_ReadReg(NRF24_REG_CONFIG);
  uint8_t status = NRF24_ReadReg(NRF24_REG_STATUS);
  uint8_t ch = NRF24_ReadReg(NRF24_REG_RF_CH);
  uint8_t pw = NRF24_ReadReg(NRF24_REG_RX_PW_P0);
  uint8_t addr[5];
  for(int i = 0; i<5; i++) addr[i] = NRF24_ReadReg(NRF24_REG_RX_ADDR_P0);
  for(int i = 0; i<32; i++) rxSerialized[i] = 0xFF;
  NRF24_WriteReg(NRF24_REG_STATUS, NRF24_RX_DR | NRF24_TX_DS | NRF24_MAX_RT);
  NRF24_Receive_IT(rxSerialized);
  //HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  volatile uint32_t packets = packets_received;
		//Control values deserialized MSB first
		volatile uint16_t aileron_t = ((uint16_t)rxSerialized[4] << 8) + rxSerialized[5];
		volatile uint16_t elevator_t = ((uint16_t)rxSerialized[6] << 8) + rxSerialized[7];
		volatile uint16_t throttle_t = ((uint16_t)rxSerialized[8] << 8) + rxSerialized[9];
		volatile uint16_t rudder_t = ((uint16_t)rxSerialized[10] << 8) + rxSerialized[11];
		volatile uint32_t ccr1 = ComputePulseWidth(aileron_t, 500, 2500);
		volatile uint32_t ccr2 = ComputePulseWidth(elevator_t, 500, 2500);
		volatile uint32_t ccr3 = ComputePulseWidth(throttle_t, 500, 2500);
		volatile uint32_t ccr4 = ComputePulseWidth(rudder_t, 500, 2500);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        // This runs at 250 Hz
    	if(hspi2.State == HAL_SPI_STATE_READY){
    		NRF24_Receive_IT(rxSerialized);
    		//Time deserialized MSB first
    		time =
				(rxSerialized[0] << 24) +
				(rxSerialized[1] << 16) +
				(rxSerialized[2] << 8)	+
				(rxSerialized[3] << 0);
    		//Control values deserialized MSB first
    		uint16_t aileron = ((uint16_t)rxSerialized[4] >> 8) + rxSerialized[5];
    		uint16_t elevator = ((uint16_t)rxSerialized[6] >> 8) + rxSerialized[7];
    		uint16_t throttle = ((uint16_t)rxSerialized[8] >> 8) + rxSerialized[9];
    		uint16_t rudder = ((uint16_t)rxSerialized[10] >> 8) + rxSerialized[11];
    		TIM3->CCR1 = ComputePulseWidth(aileron, 500, 2500);
    		TIM3->CCR2 = ComputePulseWidth(elevator, 500, 2500);
    		TIM3->CCR3 = ComputePulseWidth(throttle, 500, 2500);
    		TIM3->CCR4 = ComputePulseWidth(rudder, 500, 2500);
    	}
    }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == NRF24_IRQ_Pin) {
        // NRF24 IRQ triggered (falling edge)
    	NRF24_Receive_IT(rxSerialized);
    	packets_received++;
		//Time deserialized MSB first
		time =
			(rxSerialized[0] << 24) +
			(rxSerialized[1] << 16) +
			(rxSerialized[2] << 8)	+
			(rxSerialized[3] << 0);
		//Control values deserialized MSB first
		uint16_t aileron = ((uint16_t)rxSerialized[4] << 8) + rxSerialized[5];
		uint16_t elevator = ((uint16_t)rxSerialized[6] << 8) + rxSerialized[7];
		uint16_t throttle = ((uint16_t)rxSerialized[8] << 8) + rxSerialized[9];
		uint16_t rudder = ((uint16_t)rxSerialized[10] << 8) + rxSerialized[11];
		TIM3->CCR1 = ComputePulseWidth(aileron, 500, 2500);
		TIM3->CCR2 = ComputePulseWidth(elevator, 500, 2500);
		TIM3->CCR3 = ComputePulseWidth(throttle, 500, 2500);
		TIM3->CCR4 = ComputePulseWidth(rudder, 500, 2500);
    }
}
uint16_t ComputePulseWidth(uint16_t adcVal, uint16_t min, uint16_t max){
	if(min > max || adcVal > 4095) return 1500;
	uint16_t pulse = (((uint32_t)adcVal * (max-min)) >> 12) + min;
	return pulse;
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
