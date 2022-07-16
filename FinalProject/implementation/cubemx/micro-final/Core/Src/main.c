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

// coeffk = 2cos(2PI(freq/sampling_freq))
#define f_697Hz     1.7300996575860355
#define f_770Hz     1.6722788385415637
#define f_852Hz     1.6012981177149554
#define f_941Hz     1.5173419552846932
#define f_1209Hz    1.2249619542608279
#define f_1336Hz    1.0681880511122386
#define f_1477Hz    0.8827313049309178
#define f_1633Hz    0.6660201856861759
#define f_1394Hz    0.993244825179317
#define f_1540Hz    0.7965165138339212
#define f_1704Hz    0.5641556617974589
#define f_1882Hz    0.30232660926717536
#define f_2418Hz    -0.4994682106134935
#define f_2672Hz    -0.8589742874610377
#define f_2954Hz    -1.220785443294959
#define f_3266Hz    -1.5564171122585517

#define VREF 5

#define N 114 // changing N affects all CoeffK factors  
float samples[N]; 

#define A (0)      // output
#define B (1)      // output
#define C (2)      // output
#define D (3)      // output
#define E (4)      // output
#define F (5)      // output
#define G (6)     // output
#define MASK(x) (1UL << (x))


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
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t segment(char value) {
	switch(value){
		case '0':
			return MASK(A) | MASK(B) | MASK(C) | MASK(D) | MASK(E) | MASK(F);
		case '1':
			return MASK(B) | MASK(C);
		case '2':
			return MASK(A) | MASK(B) | MASK(D) | MASK(E) | MASK(G);
		case '3':
			return MASK(A) | MASK(B) | MASK(C) | MASK(D) | MASK(G);
		case '4':
			return MASK(B) | MASK(C) | MASK(F) | MASK(G);
		case '5':
			return MASK(A) | MASK(C) | MASK(D) | MASK(F) | MASK(G);
		case '6':
			return MASK(A) | MASK(C) | MASK(D) | MASK(E) | MASK(F) | MASK(G);
		case '8':
			return MASK(A) | MASK(B) | MASK(C) | MASK(D) | MASK(E) | MASK(F) | MASK(G);
		case '7':
			return MASK(A) | MASK(B) | MASK(C);
		case '9':
			return MASK(A) | MASK(B) | MASK(C) | MASK(D) | MASK(F) | MASK(G);
		case 'A':
			return MASK(A) | MASK(B) | MASK(C) | MASK(E) | MASK(F) | MASK(G);
		case 'b':
			return MASK(C) | MASK(D) | MASK(E) | MASK(F) | MASK(G);
		case 'c':
			return MASK(A) | MASK(D) | MASK(E) | MASK(F);
		case 'd':
			return MASK(B) | MASK(C) | MASK(D) | MASK(E) | MASK(G);
		case '*':
			return MASK(B) | MASK(C) | MASK(E) | MASK(F) | MASK(G);
		case '#':
			return MASK(C) | MASK(D) | MASK(E) | MASK(G);
		default:
			return MASK(A) | MASK(B) | MASK(C) | MASK(D) | MASK(E) | MASK(F);
	}
}

uint32_t Read_ADC_OSC(){
	uint32_t raw;
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	raw = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return raw;
}

void logStr(char* string){
	HAL_UART_Transmit(&huart2,(uint8_t*) string , strlen(string), HAL_MAX_DELAY);
}
void logNum(uint32_t input){
	char number[10]; 
	sprintf(number, "%u", input);
	HAL_UART_Transmit(&huart2,(uint8_t*) number , strlen(number), HAL_MAX_DELAY);
	endLine();
}
void logFloat(float input){
	char number[10]; 
	sprintf(number, "%f", input);
	HAL_UART_Transmit(&huart2,(uint8_t*) number , strlen(number), HAL_MAX_DELAY);
	endLine();
}
void endLine(){
	logStr("\n\r");
}

void sample(){
	for(short i = 0; i < N; i++){
		samples[i] = (((float) Read_ADC_OSC()) - 1401.0)* VREF/317.0; 
	}
}


uint32_t goertzel(float factor)  {
  float  v0, v1, v2; 
  float v12, v22, v1v2;
	
	// base values for recursive equation
  v1  = 0; 
  v2  = 0;
	
	// calculate all Vk values recursively 
  for (int i = 0; i < N; i++) {
    v0 = (factor*v1)- v2 + samples[i];
    v2 = v1;
    v1 = v0;
  }
  v12  = v1*v1;
  v22  = v2*v2;
  v1v2 = v1*v2;
  return  (uint32_t)(v12 + v22 - (factor*v1v2)) ;          
}



uint32_t error(){
	logStr("Not a Valid Tone!");
	endLine();
  return 100;
}

uint32_t dtmf_detect()  {
  uint32_t row_max, col_max;
  uint32_t row=0, col=0, row2=0 , col2=0;
  uint32_t pw[4], pw2[4];
	short i = 0;
	
	/* Check Row Freqs :  */ 
  pw[0] = goertzel(f_697Hz);
  pw[1] = goertzel(f_770Hz);
  pw[2] = goertzel(f_852Hz);
  pw[3] = goertzel(f_941Hz);

  row = 0; row_max = 0x40;  // threshold 
	for(i = 0; i < 4; i++){
		if(pw[i] > row_max){
			row = i+1;
			row_max = pw[i];
		}
	}
	if( row == 0 ) return error();
	
	/* Check Col Freqs: */ 
  pw[0] = goertzel(f_1209Hz);
  pw[1] = goertzel(f_1336Hz);
  pw[2] = goertzel(f_1477Hz);
  pw[3] = goertzel(f_1633Hz);


  col = 0; col_max = 0x50;  // threshold
	for(i = 0; i < 4; i++){
		if(pw[i] > col_max){
			col = i+1;
			col_max = pw[i];
		}
	}
	if( col == 0 ) return error();

	// Second Harmonic: 
 	/* Check Row Freqs :  */ 
  pw2[0] = goertzel(f_1394Hz);
  pw2[1] = goertzel(f_1540Hz);
  pw2[2] = goertzel(f_1704Hz);
  pw2[3] = goertzel(f_1882Hz);
	row2 = 0; row_max = 0x50;  // threshold 
	for(int i = 0; i < 4; i++){
		if(pw2[i] > row_max){
			row2 = i+1;
			row_max = pw2[i];
		}
	}
	/* Check Col Freqs: */ 
  pw2[0] = goertzel(f_2418Hz);
  pw2[1] = goertzel(f_2672Hz);
  pw2[2] = goertzel(f_2954Hz);
  pw2[3] = goertzel(f_3266Hz);
  col2 = 0; col_max = 0x60;  // threshold
	for(int i = 0; i < 4; i++){
		if(pw2[i] > col_max){
			col2 = i+1;
			col_max = pw2[i];
		}
	}
	
	if( col == col2 && row == row2)
		return error();
	
	
	logStr("Tone Found :  \n");
	endLine();
	logStr("row: ");logNum(row);
	logStr("column: "); logNum(col);
  return ((row-1) * 4) + (col-1);
}

char digits[16] = {
  '1', '2', '3', 'A', 
  '4', '5', '6', 'b', 
  '7', '8', '9', 'c', 
  '*', '0', '#', 'd', 
};


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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	// Enable GPIOB:
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN; 
	// Set Input & Outputs as decared at the beginning  (00 for input and 10 for output) 
	GPIOB->MODER = 0xFFD55555;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		sample();
		uint32_t digit = dtmf_detect();
		if( digit != 100){
			logStr("7segment updated"); endLine();
			GPIOB->ODR = 0;
			GPIOB->ODR = segment(digits[digit]);
		}
		HAL_Delay(290);
		
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

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
  hadc1.Init.NbrOfConversion = 1;
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
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

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

