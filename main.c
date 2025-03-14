/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @project        : The BARISTA Project
  ******************************************************************************

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "senseo.h"
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
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

enum st_t {
	heating,
	pumping,
	undef
	} state;

enum mode_t {
	power_off,
	idle,
	one_cup,
	two_cups,
	no_water,
	error,
	undefined
	} mode;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM5_Init(void);
/* USER CODE BEGIN PFP */
void Select_Watertemp(void);
void Select_Waterlevel(void);

uint16_t WaterTemp(void);
uint16_t WaterLevel_ok(void);
uint16_t OnOffKey_pressed(void);
uint16_t Cup1Key_pressed(void);
uint16_t Cup2Key_pressed(void);

void LED(uint16_t stat);
void Pump(uint16_t stat);
void Heater(uint16_t stat);

void ShowMode(mode_t md, int forced);
//void ShowState(st_t, int forced);
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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */

  char msg[200];
  unsigned int temp = 0;
  unsigned int tim = 0;
  unsigned int heat = 0;

  LED(OFF);
  Pump(OFF);
  Heater(OFF);

  mode = power_off;
  state = undef;

  sprintf(msg, "\n"); PRINT_TEXT;
  sprintf(msg, "Starting main loop\n"); PRINT_TEXT;

  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_Base_Start(&htim5);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
   {
	    /* USER CODE BEGIN 3 */
	    HAL_GPIO_TogglePin(TestSignal_GPIO_Port, TestSignal_Pin);

		switch (mode) {

		// ------------------------------------------------- Mode: Power Off
			case power_off: {
				ShowMode(mode, NOTFORCED);
				LED( OFF );
				Pump( OFF );
				Heater( OFF );
				if ( OnOffKey_pressed()) {
					mode = idle;
				} // eo if ( OnOffKey_pressed())
				break;
			} // case power_off:

		// ------------------------------------------------- Mode: Idle
			case idle: {
				ShowMode(mode, NOTFORCED);
				LED( ON );
				Pump( OFF );
				Heater( OFF );

				if ( OnOffKey_pressed() ) {
					mode = power_off;
				} // eo if ( OnOffKey_pressed() )

				if ( Cup1Key_pressed() ) {
					state = heating;
					mode  = one_cup;
				} // eo if ( Cup1Key_pressed() )

				if ( Cup2Key_pressed() ) {
					state = heating;
					mode  = two_cups;
				} // eo if ( Cup1Key_pressed() )

				break;
			} // eo case idle:

		// ------------------------------------------------- Mode: 1 Cup
			case one_cup: {
				ShowMode(mode, NOTFORCED);
				LED( SLOWBLINK );

				if ( !WaterLevel_ok() ) {
					mode = no_water;
					break;
				} // eo if ( !WaterLevel_ok()

				if ( OnOffKey_pressed() ) {
					mode = idle;
					break;
				} // eo if ( OnOffKey_pressed() )

				sprintf(msg, "OneCup | state: %i, Temp: %u, Time: %3i, Heater: %i\n", state, temp, tim, heat); PRINT_TEXT;

				switch (state) {
					case heating:  {
						temp = WaterTemp();

						if ( temp >= 0x8  ) {
							Heater(ON);
							heat = 1;
						}
						else {
							__HAL_TIM_SET_COUNTER(&htim5, 0);
							state = pumping;
						}

						break;
					} // eo case ST_HEATING:

					case pumping:  {
						Pump(ON);
						tim = __HAL_TIM_GET_COUNTER(&htim5) >> 12;
						temp = WaterTemp();

						if ( temp > 0x9  ) {
							Heater(ON);
							heat = 1;
						}
						if ( temp < 0x8  ) {
							Heater(OFF);
							heat = 0;
						}

						if ( tim > TIME_TO_FILL_ONE_CUP ) mode = idle;
						break;
					} //eo case ST_PUMPING:

					default:  {
						Pump( OFF );
						Heater( OFF );
						break;
					} // eo default:

				} // eo switch (state)
				break;
			} // eo case one_cup:

			case two_cups: {
				ShowMode(mode, NOTFORCED);
				LED( SLOWBLINK );

				if ( !WaterLevel_ok() ) {
					mode = no_water;
					break;
				} // eo if ( !WaterLevel_ok()

				if ( OnOffKey_pressed() ) {
					mode = idle;
					break;
				} // eo if ( OnOffKey_pressed() )

				sprintf(msg, "TwoCups | state: %i, Temp: %u, Time: %3i, Heater: %i\n", state, temp, tim, heat); PRINT_TEXT;

				switch (state) {
					case heating:  {
						temp = WaterTemp();

						if ( temp >= 0x8  ) {
							Heater(ON);
							heat = 1;
						}
						else {
							__HAL_TIM_SET_COUNTER(&htim5, 0);
							state = pumping;
						}

						break;
					} // eo case ST_HEATING:

					case pumping:  {
						Pump(ON);
						tim = __HAL_TIM_GET_COUNTER(&htim5) >> 12;
						temp = WaterTemp();

						if ( temp > 0x9  ) {
							Heater(ON);
							heat = 1;
						}
						if ( temp < 0x8  ) {
							Heater(OFF);
							heat = 0;
						}

						if ( tim > TIME_TO_FILL_TWO_CUPS ) mode = idle;
						break;
					} //eo case ST_PUMPING:

					default:  {
						Pump( OFF );
						Heater( OFF );
						break;
					} // eo default:

				} // eo switch (state)
				break;
			} // eo case TwoCups

			case no_water: {
				ShowMode(mode, NOTFORCED);
				Pump( OFF );
				Heater( OFF );
				LED(FASTBLINK);

				if ( OnOffKey_pressed() ) {
					mode = power_off;
				} // eo if ( OnOffKey_pressed() )

				if ( WaterLevel_ok() ) {
					mode = idle;
				} // eo if ( WaterLevel_ok()
				break;
			}

			case error: {
				ShowMode(mode, NOTFORCED);
				Pump( OFF );
				Heater( OFF );
				break;
			}

			default: {
				Pump( OFF );
				Heater( OFF );
				break;
			}
		} // eo switch

	/* USER CODE END 3 */

   } // eo while(1)
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	    HAL_GPIO_TogglePin(TestSignal_GPIO_Port, TestSignal_Pin);

		switch (mode) {

		// ------------------------------------------------- Mode: Power Off
			case power_off: {
				ShowMode(mode, NOTFORCED);
				LED( OFF );
				Pump( OFF );
				Heater( OFF );
				if ( OnOffKey_pressed()) {
					mode = idle;
				} // eo if ( OnOffKey_pressed())
				break;
			} // case power_off:

		// ------------------------------------------------- Mode: Idle
			case idle: {
				ShowMode(mode, NOTFORCED);
				LED( ON );
				Pump( OFF );
				Heater( OFF );

				if ( OnOffKey_pressed() ) {
					mode = power_off;
				} // eo if ( OnOffKey_pressed() )

				if ( Cup1Key_pressed() ) {
					state = heating;
					mode  = one_cup;
				} // eo if ( Cup1Key_pressed() )

				if ( Cup2Key_pressed() ) {
					state = heating;
					mode  = two_cups;
				} // eo if ( Cup1Key_pressed() )

				break;
			} // eo case idle:

		// ------------------------------------------------- Mode: 1 Cup
			case one_cup: {
				ShowMode(mode, NOTFORCED);
				LED( SLOWBLINK );

				if ( !WaterLevel_ok() ) {
					mode = no_water;
					break;
				} // eo if ( !WaterLevel_ok()

				if ( OnOffKey_pressed() ) {
					mode = idle;
					break;
				} // eo if ( OnOffKey_pressed() )

				sprintf(msg, "OneCup | state: %i, Temp: %u, Time: %3i, Heater: %i\n", state, temp, tim, heat); PRINT_TEXT;

				switch (state) {
					case heating:  {
						temp = WaterTemp();

						if ( temp >= 0x8  ) {
							Heater(ON);
							heat = 1;
						}
						else {
							__HAL_TIM_SET_COUNTER(&htim5, 0);
							state = pumping;
						}

						break;
					} // eo case ST_HEATING:

					case pumping:  {
						Pump(ON);
						tim = __HAL_TIM_GET_COUNTER(&htim5) >> 12;
						temp = WaterTemp();

						if ( temp > 0x9  ) {
							Heater(ON);
							heat = 1;
						}
						if ( temp < 0x8  ) {
							Heater(OFF);
							heat = 0;
						}

						if ( tim > TIME_TO_FILL_ONE_CUP ) mode = idle;
						break;
					} //eo case ST_PUMPING:

					default:  {
						Pump( OFF );
						Heater( OFF );
						break;
					} // eo default:

				} // eo switch (state)
				break;
			} // eo case one_cup:

			case two_cups: {
				ShowMode(mode, NOTFORCED);
				LED( SLOWBLINK );

				if ( !WaterLevel_ok() ) {
					mode = no_water;
					break;
				} // eo if ( !WaterLevel_ok()

				if ( OnOffKey_pressed() ) {
					mode = idle;
					break;
				} // eo if ( OnOffKey_pressed() )

				sprintf(msg, "TwoCups | state: %i, Temp: %u, Time: %3i, Heater: %i\n", state, temp, tim, heat); PRINT_TEXT;

				switch (state) {
					case heating:  {
						temp = WaterTemp();

						if ( temp >= 0x8  ) {
							Heater(ON);
							heat = 1;
						}
						else {
							__HAL_TIM_SET_COUNTER(&htim5, 0);
							state = pumping;
						}

						break;
					} // eo case ST_HEATING:

					case pumping:  {
						Pump(ON);
						tim = __HAL_TIM_GET_COUNTER(&htim5) >> 12;
						temp = WaterTemp();

						if ( temp > 0x9  ) {
							Heater(ON);
							heat = 1;
						}
						if ( temp < 0x8  ) {
							Heater(OFF);
							heat = 0;
						}

						if ( tim > TIME_TO_FILL_TWO_CUPS ) mode = idle;
						break;
					} //eo case ST_PUMPING:

					default:  {
						Pump( OFF );
						Heater( OFF );
						break;
					} // eo default:

				} // eo switch (state)
				break;
			} // eo case TwoCups

			case no_water: {
				ShowMode(mode, NOTFORCED);
				Pump( OFF );
				Heater( OFF );
				LED(FASTBLINK);

				if ( OnOffKey_pressed() ) {
					mode = power_off;
				} // eo if ( OnOffKey_pressed() )

				if ( WaterLevel_ok() ) {
					mode = idle;
				} // eo if ( WaterLevel_ok()
				break;
			}

			case error: {
				ShowMode(mode, NOTFORCED);
				Pump( OFF );
				Heater( OFF );
				break;
			}

			default: {
				Pump( OFF );
				Heater( OFF );
				break;
			}
		} // eo switch

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
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 21000-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 0xFFFF;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 21000-1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 0xFFFFFFFF;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, PumpControl_Pin|HeaterControl_Pin|LEDControl_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TestSignal_GPIO_Port, TestSignal_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Cup2Key_Pin Cup1Key_Pin OnOffKey_Pin */
  GPIO_InitStruct.Pin = Cup2Key_Pin|Cup1Key_Pin|OnOffKey_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PumpControl_Pin HeaterControl_Pin LEDControl_Pin */
  GPIO_InitStruct.Pin = PumpControl_Pin|HeaterControl_Pin|LEDControl_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : TestSignal_Pin */
  GPIO_InitStruct.Pin = TestSignal_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TestSignal_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//##############################################################################
/* helpful function ----------------------------------------------------------*/
void ShowMode(mode_t md, int forced) {
	static mode_t current_mode = undefined;
	char t[30];

	if ( (md != current_mode) || forced ) {
		switch (md) {
			case power_off: {	sprintf( t, "\n**** Mode: power_off\n"); 	break;}
			case idle: 		{	sprintf( t, "\n**** Mode: idle\n"); 		break;}
			case one_cup: 	{	sprintf( t, "\n**** Mode: one_cup\n"); 		break;}
			case two_cups: 	{	sprintf( t, "\n**** Mode: two_cups\n"); 	break;}
			case no_water: 	{	sprintf( t, "\n**** Mode: no_water\n"); 	break;}
			case error: 	{	sprintf( t, "\n**** Mode: error\n"); 		break;}
			case undefined:
			default:		{	sprintf( t, "\n**** Mode: undefined\n"); 	break;}
		} // eo switch (md)
		HAL_UART_Transmit( &huart2, (uint8_t*)t,strlen(t),HAL_MAX_DELAY);
	} // eo if ( (md != current_mode) || forced )

	if (!forced ) current_mode = md;
	return;
} // eo function


void Select_Watertemp(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();
}


void Select_Waterlevel(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();
}


uint16_t WaterTemp(void) {
	int i;
	uint16_t dat;

	Select_Watertemp();
	dat = 0 ;
	for (i=0; i<10; i++) {
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
		dat = dat + (HAL_ADC_GetValue(&hadc1) >> 8); 	//lowest 8 bit ignored
	}
	return dat / 10;
}


uint16_t WaterLevel_ok(void) {
	int i;
	uint16_t dat;

	Select_Waterlevel();
	dat = 0 ;
	for (i=0; i<10; i++) {
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
		dat = dat + HAL_ADC_GetValue(&hadc1);
	} // eo for
	if ( dat > 1000 ) return 1; else return 0;
} // eo of function


uint16_t OnOffKey_pressed(void) {
	static int lastkeystate = OFF;				//is static to save state between calls

	int keystate = !HAL_GPIO_ReadPin(OnOffKey_GPIO_Port, OnOffKey_Pin);	// get key state from input pin

	if ( keystate != lastkeystate ){			// change since last call?
		lastkeystate = keystate;				// save the keystate after change
		if ( keystate ) return PRESSED;			// key changed to 'pressed'
	} // eo if ( keystate != lastkeystate )

	return NOCHANGE;
}


uint16_t Cup1Key_pressed(void) {
	static int lastkeystate = OFF;				//is static to save state between calls

	int keystate = !HAL_GPIO_ReadPin(Cup1Key_GPIO_Port, Cup1Key_Pin);	// get key state from input pin

	if ( keystate != lastkeystate ){			// change since last call?
		lastkeystate = keystate;				// save the keystate after change
		if ( keystate ) return PRESSED;			// key changed to 'pressed'
	} // eo if ( keystate != lastkeystate )

	return NOCHANGE;
}


uint16_t Cup2Key_pressed(void) {
	static int lastkeystate = OFF;				//is static to save state between calls

	int keystate = !HAL_GPIO_ReadPin(Cup2Key_GPIO_Port, Cup2Key_Pin);	// get key state from input pin

	if ( keystate != lastkeystate ){			// change since last call?
		lastkeystate = keystate;				// save the keystate after change
		if ( keystate ) return PRESSED;			// key changed to 'pressed'
	} // eo if ( keystate != lastkeystate )

	return NOCHANGE;
}


void LED(uint16_t stat) {
	static uint32_t lapcounter = 0;
	uint32_t timestamp;

	switch ( stat ) {
		case OFF: {
			HAL_GPIO_WritePin(LEDControl_GPIO_Port, LEDControl_Pin, RESET);
			break;
		} // eo case OFF:

		case ON: {
			HAL_GPIO_WritePin(LEDControl_GPIO_Port, LEDControl_Pin, SET);
			break;
		} // eo case ON:

		case SLOWBLINK: {
			timestamp = __HAL_TIM_GET_COUNTER(&htim3);
			if ( (timestamp - lapcounter) >= 4096 ) {
				HAL_GPIO_TogglePin(LEDControl_GPIO_Port, LEDControl_Pin);
				lapcounter = timestamp;
			}
			break;
		} // eo case SLOWBLINK:

		case FASTBLINK: {
			timestamp = __HAL_TIM_GET_COUNTER(&htim3);
			if ( (timestamp - lapcounter) >= 350 ) {
				HAL_GPIO_TogglePin(LEDControl_GPIO_Port, LEDControl_Pin);
				lapcounter = timestamp;
			}
			break;
		} // eo case FASTBLINK:
	} // eo switch

} // eo function

void Pump(uint16_t stat) {
	if ( stat == ON ) {
		HAL_GPIO_WritePin(PumpControl_GPIO_Port, PumpControl_Pin, SET);
	}
	else {
		HAL_GPIO_WritePin(PumpControl_GPIO_Port, PumpControl_Pin, RESET);
	}
}


void Heater(uint16_t stat) {
	if ( stat == ON ) {
		HAL_GPIO_WritePin(HeaterControl_GPIO_Port, HeaterControl_Pin, SET);
	}
	else {
		HAL_GPIO_WritePin(HeaterControl_GPIO_Port, HeaterControl_Pin, RESET);
	}
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
  char msg[30];
  sprintf(msg, "\nCrashed!\nReset Board\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
