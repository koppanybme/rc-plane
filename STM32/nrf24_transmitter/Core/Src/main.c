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
#include "adc.h"
#include "dma.h"
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
#define JOYSTICK_DEADZONE 25
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t txSerialized[32];
volatile uint32_t adc_results[4];

//Hardcoded values for now
static uint16_t aileron_raw_min = 692;
static uint16_t aileron_raw_middle = 2158;
static uint16_t aileron_raw_max = 3562;
static uint16_t elevator_raw_min = 276;
static uint16_t elevator_raw_middle = 2047;
static uint16_t elevator_raw_max = 3562;
static uint16_t throttle_raw_min = 306;
static uint16_t throttle_raw_max = 3368;
static uint16_t rudder_raw_min = 581;
static uint16_t rudder_raw_middle = 2045;
static uint16_t rudder_raw_max = 3549;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint16_t mapChannel(uint16_t raw, uint16_t min, uint16_t middle, uint16_t max, uint8_t is_throttle, uint8_t is_reversed);
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
  for(int i = 0; i<32; i++) txSerialized[i] = i;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_results, 4);
  NRF24_Init(0);
  uint8_t status = NRF24_ReadReg(NRF24_REG_STATUS);
  uint8_t ch = NRF24_ReadReg(NRF24_REG_RF_CH);
  HAL_TIM_Base_Start_IT(&htim2);
  TIM2->CCR2 = 3900;	//Every ADC conversion starts 100us before transmitting
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  volatile uint32_t ch0 = adc_results[0];
	  volatile uint32_t ch1 = adc_results[1];
	  volatile uint32_t ch2 = adc_results[2];
	  volatile uint32_t ch3 = adc_results[3];
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        // This runs at 250 Hz
        uint32_t time = HAL_GetTick();
        //Serialize time MSB first
        txSerialized[0] = (time & 0xff000000) >> 24;
        txSerialized[1] = (time & 0x00ff0000) >> 16;
        txSerialized[2] = (time & 0x0000ff00) >> 8;
        txSerialized[3] = (time & 0x000000ff) >> 0;
        NRF24_Transmit_IT(txSerialized, 32);
    }
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	uint16_t aileron = mapChannel(adc_results[0], aileron_raw_min, aileron_raw_middle, aileron_raw_max, 0, 0);
	uint16_t elevator = mapChannel(adc_results[1], elevator_raw_min, elevator_raw_middle, elevator_raw_max, 0, 0);
	uint16_t throttle = mapChannel(adc_results[2], throttle_raw_min, throttle_raw_min, throttle_raw_max, 1, 0);
	uint16_t rudder = mapChannel(adc_results[3], rudder_raw_min, rudder_raw_middle, rudder_raw_max, 0, 0);
	//Serialize adc results MSB first
	txSerialized[4] = aileron >> 8;
	txSerialized[5] = aileron & 0xff;
	txSerialized[6] = elevator >> 8;
	txSerialized[7] = elevator & 0xff;
	txSerialized[8] = throttle >> 8;
	txSerialized[9] = throttle & 0xff;
	txSerialized[10] = rudder >> 8;
	txSerialized[11] = rudder & 0xff;
}
uint16_t mapChannel(uint16_t raw, uint16_t min, uint16_t middle, uint16_t max, uint8_t is_throttle, uint8_t is_reversed){
	if(min >= max || middle >= max) return 2048;
	if (raw <= min) raw = min;
	if (raw >= max) raw = max;
	uint16_t normalized = 0;
	if(is_throttle){
		uint16_t range = max-min;
		normalized = (uint32_t)(raw-min)*4095/range;
	} else {
		if(raw > middle-JOYSTICK_DEADZONE && raw <= middle+JOYSTICK_DEADZONE){
			//If inside deadzone
			return 2048;
		} else if(raw <= middle-JOYSTICK_DEADZONE){
			//Map linearly to lower half
			uint16_t range = middle-min-JOYSTICK_DEADZONE;
			normalized = (uint32_t)(raw-min)*2048/range;
		} else{
			//Map linearly to upper half
			uint16_t range = max-middle-JOYSTICK_DEADZONE;
			normalized = 2047+(uint32_t)(raw-middle-JOYSTICK_DEADZONE)*2048/range;
		}
	}
	if(is_reversed){
		normalized = 4095 - normalized;
	}
	return (uint16_t)normalized;
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
