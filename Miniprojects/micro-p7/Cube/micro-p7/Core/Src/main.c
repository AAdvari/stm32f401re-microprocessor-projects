/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h> 
#include <stdbool.h>
#include <string.h> 
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define GREEN (4)
#define ORANGE (5)
#define RED (6) 
#define B1 (7)
#define B2 (8) 
#define B3 (9) 
#define PWM_ORANGE (13)
#define PWM_RED (14) 
#define SA (1)
#define SB (2)
#define SC (3)
#define SD (4)
#define SE (5)
#define SF (6)
#define SG (7)
#define SAA (8)
#define SBB (9)
#define SCC (10)
#define SDD (12)
#define SEE (13)
#define SFF (14)
#define SGG (15)
#define MASK(x)   (1UL << (x))
#define VREF (5)

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
void logStr(char* string);
void logNum(uint32_t input);
void endLine();
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
enum SystemState { STARTUP, OK, CHECKING, WARNING, COOLING, DANGER, OFF };
volatile enum SystemState systemState = STARTUP; 
volatile float avg = 0.0; 
volatile float lastAvg = 0.0; 
volatile uint8_t count = 0; 


void logState(enum SystemState state){
	switch(state) {
		case OK:
			logStr("State -> OK");
			break;
		case CHECKING:
			logStr("State -> CHECKING");
			break;
		case WARNING:
			logStr("State -> WARNING");
			break;
		case COOLING:
			logStr("State -> COOLING");
			break;
		case DANGER:
			logStr("State -> DANGER");
			break;
		case OFF:
			logStr("State -> OFF");
			break;
		case STARTUP:
			logStr("State -> STARTUP");
			break;
			default:
			logStr("Unexpected Behaviour");
	}
	endLine();
}
uint16_t segment1(uint16_t value) {
	switch(value){
		case 0:
			return MASK(SA) | MASK(SB) | MASK(SC) | MASK(SD) | MASK(SE) | MASK(SF);
		case 1:
			return MASK(SB) | MASK(SC);
		case 2:
			return MASK(SA) | MASK(SB) | MASK(SD) | MASK(SE) | MASK(SG);
		case 3:
			return MASK(SA) | MASK(SB) | MASK(SC) | MASK(SD) | MASK(SG);
		case 4:
			return MASK(SB) | MASK(SC) | MASK(SF) | MASK(SG);
		case 5:
			return MASK(SA) | MASK(SC) | MASK(SD) | MASK(SF) | MASK(SG);
		case 6:
			return MASK(SA) | MASK(SC) | MASK(SD) | MASK(SE) | MASK(SF) | MASK(SG);
		case 8:
			return MASK(SA) | MASK(SB) | MASK(SC) | MASK(SD) | MASK(SE) | MASK(SF) | MASK(SG);
		case 7:
			return MASK(SA) | MASK(SB) | MASK(SC);
		case 9:
			return MASK(SA) | MASK(SB) | MASK(SC) | MASK(SD) | MASK(SF) | MASK(SG);
		default:
			return MASK(SA) | MASK(SB) | MASK(SC) | MASK(SD) | MASK(SE) | MASK(SF);
	}
}

uint16_t segment2(uint16_t value) {
	switch(value){
		case 0:
			return MASK(SAA) | MASK(SBB) | MASK(SCC) | MASK(SDD) | MASK(SEE) | MASK(SFF);
		case 1:
			return MASK(SBB) | MASK(SCC);
		case 2:
			return MASK(SAA) | MASK(SBB) | MASK(SDD) | MASK(SEE) | MASK(SGG);
		case 3:
			return MASK(SAA) | MASK(SBB) | MASK(SCC) | MASK(SDD) | MASK(SGG);
		case 4:
			return MASK(SBB) | MASK(SCC) | MASK(SFF) | MASK(SGG);
		case 5:
			return MASK(SAA) | MASK(SCC) | MASK(SDD) | MASK(SFF) | MASK(SGG);
		case 6:
			return MASK(SAA) | MASK(SCC) | MASK(SDD) | MASK(SEE) | MASK(SFF) | MASK(SGG);
		case 8:
			return MASK(SAA) | MASK(SBB) | MASK(SCC) | MASK(SDD) | MASK(SEE) | MASK(SFF) | MASK(SGG);
		case 7:
			return MASK(SAA) | MASK(SBB) | MASK(SCC);
		case 9:
			return MASK(SAA) | MASK(SBB) | MASK(SCC) | MASK(SDD) | MASK(SFF) | MASK(SGG);
		default:
			return MASK(SAA) | MASK(SBB) | MASK(SCC) | MASK(SDD) | MASK(SEE) | MASK(SFF);
	}
}


// Transform System  to given State 
void ChangeState(enum SystemState state){
	HAL_GPIO_WritePin( GPIOA, MASK(ORANGE) | MASK(GREEN) | MASK(RED) | MASK(PWM_RED) | MASK(PWM_ORANGE), GPIO_PIN_RESET); 
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
	systemState = state;
	switch( state ){
		case OK: 
			HAL_GPIO_WritePin( GPIOA, MASK(GREEN), GPIO_PIN_SET); 
			break; 
		case WARNING: 
			HAL_GPIO_WritePin( GPIOA, MASK(ORANGE), GPIO_PIN_SET); 
			break;
		case CHECKING:
				HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
				HAL_GPIO_WritePin( GPIOA, MASK(PWM_ORANGE), GPIO_PIN_SET);
			break;
		case DANGER: 
				HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
				HAL_GPIO_WritePin( GPIOA, MASK(PWM_RED), GPIO_PIN_SET);
			break;
		default:
			break;
	}
	logState(state);
}
// Show given Number on segments 
void showOnSegments(uint16_t value){
		HAL_GPIO_WritePin(GPIOB, 0xFFFF , GPIO_PIN_RESET);
		uint16_t seg1 = segment1(value/10); 
		uint16_t seg2 = segment2(value%10); 
		HAL_GPIO_WritePin(GPIOB, (seg1) | (seg2) , GPIO_PIN_SET);
}

// Reverse Given Value 
uint16_t reverse(uint16_t input){
	return ((4095 - input)  << 4); 
}

/************************************************************************/
/* Configurations for selecting/reading adc channels */ 
void Select_ADC_OSC(){
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}
void Select_ADC_TEMP(){
	 ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}
uint32_t Read_ADC_OSC(){
	HAL_TIM_Base_Stop_IT(&htim3);
	uint32_t raw;
	Select_ADC_OSC();
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	raw = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	HAL_TIM_Base_Start_IT(&htim3);
	return raw;
}

uint32_t Read_ADC_TEMP(){
		Select_ADC_TEMP();
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	 	uint32_t raw = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
		return raw;
}
/********************************************************************************/

void delay(uint16_t n){
	for(int i = 0; i < n ; i++){
		uint16_t del = 300; 
		while( del > 0 ) 
			del--;
	}
}

// Debug Funcs:
void logStr(char* string){
	HAL_UART_Transmit(&huart2,(uint8_t*) string , strlen(string), HAL_MAX_DELAY);
}
void logNum(uint32_t input){
	char number[10]; 
	sprintf(number, "%u", input);
	HAL_UART_Transmit(&huart2,(uint8_t*) number , strlen(number), HAL_MAX_DELAY);
}
void endLine(){
	logStr("\n\r");
}

void EXTI0_IRQHandler(){
		if( HAL_GPIO_ReadPin(GPIOA, MASK(B1)) == GPIO_PIN_SET ){
			if( systemState == STARTUP){
				ChangeState(CHECKING);
			}
		}
		else if( HAL_GPIO_ReadPin(GPIOA, MASK(B2)) == GPIO_PIN_SET ){
			if( systemState == OFF) 
				ChangeState(CHECKING); 
		}
	
}
// Scale and return ADC via Read_ADC_TEMP() 
double readTemp(){
	double temp = Read_ADC_TEMP();
	temp = ((temp/4095.0)*VREF*100);
	return temp;
}
// This Function handles Danger State. (Cooling Check , Turn Off,  .... ) 
void handleDanger(){
	uint32_t start = HAL_GetTick(); 
	
	bool coolingPressed = false;
	while( HAL_GetTick() - start < 1000 ){
		
		if( HAL_GPIO_ReadPin(GPIOA, MASK(B3)) == GPIO_PIN_SET ){
			coolingPressed = true; 
			break; 
		}
	}
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
	HAL_GPIO_WritePin( GPIOA, MASK(PWM_RED) , GPIO_PIN_RESET); 
	
	if( !coolingPressed )
		ChangeState(OFF); 
	else {
			HAL_GPIO_WritePin( GPIOA, MASK(ORANGE) | MASK(GREEN) | MASK(RED) | MASK(PWM_RED) | MASK(PWM_ORANGE), GPIO_PIN_RESET); 
			HAL_GPIO_WritePin( GPIOA, MASK(RED) , GPIO_PIN_SET); 
			uint32_t miliSeconds = 0; 
			while( miliSeconds < 2000 ){
				if( lastAvg <= 35){
					ChangeState(OK); 
					return; 
				}
				HAL_Delay(1);
				miliSeconds++;
			}
			HAL_GPIO_WritePin( GPIOA, MASK(RED) , GPIO_PIN_RESET); 
			HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
			HAL_GPIO_WritePin( GPIOA, MASK(PWM_RED) , GPIO_PIN_SET); 
			HAL_Delay(500);
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
			ChangeState(OFF); 
	}
	
}

// Change AVG temprature periodically 
void TIM3_IRQHandler(void){
		if( systemState != STARTUP && systemState != OFF  ){
			if( count == 10){
				
				count = 0; 
				showOnSegments( (uint16_t) avg);
				if( avg <= 35){
					if(systemState != OK) 
						ChangeState(OK);
				}
				if( avg > 35 && avg < 46){
					if( systemState != WARNING) 
						ChangeState(WARNING);
				}
				else if( avg >= 46 ){
					if( systemState != DANGER)
						ChangeState(DANGER);
				}
				lastAvg = avg;
				avg = 0; 
			}
			else{
				avg += readTemp()/10; 
				count++;
			}
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
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	uint32_t in; 
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
	HAL_NVIC_SetPriority(TIM3_IRQn, 1, 1);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 1);

	HAL_TIM_Base_Start_IT(&htim3);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

  while (1)
  {
		// If systemState has been changed to DANGER (via TIMER) handle it.
		if(systemState == DANGER)
			handleDanger(); 
		// if systemState is OK, read ADC, reverse and put on PC Pin. 
		else if(systemState == OK ) {
//			in = Read_ADC_OSC(); 
//		GPIOC->ODR = reverse( in );
		}
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 15999;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 120;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 60;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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
  htim3.Init.Prescaler = 15999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 19;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0
                          |GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_13
                          |GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 PC15 PC0
                           PC1 PC2 PC3 PC4
                           PC5 PC6 PC7 PC8
                           PC9 PC10 PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0
                          |GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 PA13
                           PA14 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_13
                          |GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA7 PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB2 PB10 PB12
                           PB13 PB14 PB15 PB3
                           PB4 PB5 PB6 PB7
                           PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_12
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

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

