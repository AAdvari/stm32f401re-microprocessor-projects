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
#include <stdbool.h> 
// GPIOB -> For interrupt lines
#define B1 (0) 
#define B2 (1) 
#define B3 (2) 
#define MASK(x)   (1UL << (x))


#define D7 GPIO_PIN_8
#define D7_GPIO GPIOA

#define D6 GPIO_PIN_10
#define D6_GPIO GPIOB

#define D5 GPIO_PIN_4
#define D5_GPIO GPIOB

#define D4 GPIO_PIN_5
#define D4_GPIO GPIOB

#define EN GPIO_PIN_7
#define EN_GPIO GPIOC

#define RS GPIO_PIN_9
#define RS_GPIO GPIOA

void delayMs(int n);
void LCD_nibble_write(char data, unsigned char control);
void LCD_command(unsigned char command);
void LCD_data(char data);
void LCD_init(void);

volatile uint16_t msec = 0; 
volatile uint16_t sec = 0;
volatile uint16_t min = 0; 

// Timer States 
enum State { STOPPED, COUNTING, STARTUP, OFF }; 
// Initial State 
enum State systemState = STARTUP; 

// Make Delay
void delay(uint16_t ms);
//Printing String or char
void printInLine(char* string);
void print(char ch);
// turn on lcd and set proper mod 
void init_lcd();
// clear second line 
void clear_line_2();
// Prints Digits of a signed number
void printDigit(int N);
//Committing commands or chars
void commitCommand(unsigned char command);
void commitChar(char data);
// Converting intToChar and CharToInt 
int32_t charToInt(char ch);
char intToChar(int32_t digit);
// print current time on lcd : 
void printTime(uint16_t min, uint16_t sec, uint16_t msec);
void clear_lcd();
void clear_line_1();
// Checks long-push(B3)
bool long_push();
bool B3_clicked();









/* USER CODE END Includes */






/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	// On System Startup we shouldnt accept any commands
	if ( systemState == STARTUP )
		return;
	
	// Count if the system is not off 
	if(GPIO_Pin == GPIO_PIN_0 ){
		//if( systemState != OFF ) 
			systemState = COUNTING; 
	}
	// Stop Timer 
	if(GPIO_Pin == GPIO_PIN_1 ){
		if(systemState == OFF)
			return; 
		systemState = STOPPED; 
	}
	// LongPush -> Turn off the system, else -> Reset Timer !
	if(GPIO_Pin == GPIO_PIN_2 ){

			HAL_NVIC_SetPendingIRQ(EXTI2_IRQn);
			
			min = sec = msec = 0; 	
			if ( long_push() ) {
				systemState = OFF;
				clear_line_2();
				while(B3_clicked()){}
			}
			else {
				if(systemState == OFF) 
					return; 
				systemState = STOPPED;
				printTime(0, 0, 0);
			}
			while( B3_clicked() ); 
			HAL_NVIC_ClearPendingIRQ(EXTI2_IRQn);
	}
}

// TIM2 -> For counting MiliSeconds, TIM3 -> For counting halfSeconds (in order to print and clear 'TURN OFF' )
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if (htim->Instance == TIM2){
		// Counter Timer :
		if( systemState == COUNTING ){
				msec+=8; 
			if(msec == 1000){
				sec++; 
				msec = 0;
			}
			if(sec == 60){
				min++; 
				sec = 0;
			}
			printTime( min, sec, msec);
		}
	}
	// Timer for OFF state : 
	else if (htim->Instance == TIM3){
		static bool turnOffPrinted = false; 
		if(systemState == OFF ){
			if(turnOffPrinted){
				clear_line_2();
			}
			else {
				printInLine("TURN OFF");
			}
			turnOffPrinted = !turnOffPrinted;
		}
	}		
}

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

// Make Delay
void delay(uint16_t ms);
//Printing String or char
void printInLine(char* string);
void print(char ch);
// turn on lcd and set proper mod 
void init_lcd();
// clear second line 
void clear_line_2();
// Prints Digits of a signed number
void printDigit(int N);
//Committing commands or chars
void commitCommand(unsigned char command);
void commitChar(char data);
// Converting intToChar and CharToInt 
int32_t charToInt(char ch);
char intToChar(int32_t digit);
// print current time on lcd : 
void printTime(uint16_t min, uint16_t sec, uint16_t msec);
void clear_lcd();
void clear_line_1();
// Checks long-push(B3)
bool long_push();
bool B3_clicked();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void LCD_init(void)
{
    delayMs(20); /* LCD controller reset sequence */
    LCD_nibble_write(0x30, 0);
    delayMs(5);
    LCD_nibble_write(0x30, 0);
    delayMs(1);
    LCD_nibble_write(0x30, 0);
    delayMs(1);

    LCD_nibble_write(0x20, 0); /* use 4-bit data mode */
    delayMs(1);
    LCD_command(0x28); /* set 4-bit data, 2-line, 5x7 font */
    LCD_command(0x06); /* move cursor right */
    LCD_command(0x01); /* clear screen, move cursor to home */
    LCD_command(0x0F); /* turn on display, cursor blinking */
}

void LCD_nibble_write(char data, unsigned char control)
{
    /* set R/S bit */

    if (control == 0)
        HAL_GPIO_WritePin(RS_GPIO, RS, GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(RS_GPIO, RS, GPIO_PIN_SET);

    HAL_GPIO_WritePin(D4_GPIO, D4, data % 2);
    data /= 2;
    HAL_GPIO_WritePin(D5_GPIO, D5, data % 2);
    data /= 2;
    HAL_GPIO_WritePin(D6_GPIO, D6, data % 2);
    data /= 2;
    HAL_GPIO_WritePin(D7_GPIO, D7, data % 2);

    /* pulse E */
    HAL_GPIO_WritePin(EN_GPIO, EN, GPIO_PIN_SET);
    delayMs(0);
    HAL_GPIO_WritePin(EN_GPIO, EN, GPIO_PIN_RESET);
}

void LCD_command(unsigned char command)
{
    LCD_nibble_write(((command & 0xF0) >> 4), 0); /* upper nibble first */
    LCD_nibble_write(command & 0x0F, 0);          /* then lower nibble */

    if (command < 4)
        delayMs(5); /* command 1 and 2 needs up to 1.64ms */
    else
        delayMs(2); /* all others 40 us */
}

void LCD_data(char data)
{
    LCD_nibble_write(((data & 0xF0) >> 4), 1); /* upper nibble first */
    LCD_nibble_write(data & 0x0F, 1);          /* then lower nibble */

    delayMs(2);
}

/* delay n milliseconds (16 MHz CPU clock) */
void delayMs(int n)
{
    int i;
    for (; n > 0; n--)
        for (i = 0; i < 3195; i++)
            __NOP();
}

// Check whether pushing was long. 
bool long_push(){
	  SysTick -> LOAD = 16000;
	SysTick -> VAL = 0;
	SysTick -> CTRL = 0x5;
	bool long_push = true; 
	
	for(int i = 0; i < 3000; i++){
		SysTick -> CTRL = 0x5;
		while((SysTick -> CTRL & 0x10000) == 0){
			if(B3_clicked() == 0){
				long_push = false; 
				break; 
			}
		}
		if(!long_push) 
			break; 
	}
	SysTick->CTRL = 0;
	return long_push; 

}
// Check for B3-Push
bool B3_clicked(){
	return HAL_GPIO_ReadPin(GPIOB, MASK(B3) ) == GPIO_PIN_SET; 
}
// Prints the given time ( in timer-format) on LCD
void printTime(uint16_t min, uint16_t sec, uint16_t msec){
	commitCommand(0xC0); 
	
	// Minutes:
	commitChar( intToChar(min/10) ); 
	commitChar( intToChar(min%10) ); 
	
	commitChar(':');
	
	// Seconds: 
	commitChar( intToChar(sec/10) ); 
	commitChar( intToChar(sec%10) ); 
	
	commitChar(':');
	
	// MiliSeconds:
	commitChar( intToChar(msec/100) ); 
	commitChar( intToChar((msec%100)/10 ) ); 
	commitChar( intToChar(msec%10) ); 
}


void commitCommand(unsigned char command){
	LCD_command(command);
	delayMs(20);
}
void commitChar(char data){
		LCD_data(data);
		delayMs(20);
}
void init_lcd(){

}
void printInLine(char* string){
	while(*string != '\0'){
		char ch = (char) (*string); 
		commitChar(ch); 
		string++;
	}
	commitCommand(0xC0);
}

void clear_line_2(){
	commitCommand(0xC0); 
	printInLine("                ");
}

void clear_line_1(){
	commitCommand(0x80); 
	printInLine("                ");
}
void print(char ch){
	commitChar(ch);
}


char intToChar(int32_t digit){
		switch(digit){
		case 0: return '0';
		case 1: return '1';
		case 2: return '2';
		case 3: return '3';
		case 4: return '4';
		case 5: return '5';
		case 6: return '6';
		case 7: return '7';
		case 8: return '8';
		case 9: return '9';
		default: return '0'; 
	}
	return '0'; 
}
int32_t charToInt(char ch){
	switch(ch){
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		default: return 0; 
	}
	return 0; 
}
void delay(uint16_t n){
    for(int i = 0; i < n; i++){
			for(int j = 0; j < 200 ;j++);
		}
}
void printDigit(int N){
		if(	N == 0 ){
			commitChar(intToChar(N));
			return; 
		}
		if( N < 0 ) {
			commitChar('-'); 
			N = -N; 
		}
		
    char arr[30];
    int i = 0;
    int j, r;
  
    while (N != 0) {
        r = N % 10;
        arr[i] = r;
        i++;
        N = N / 10;
    }
    for (j = i - 1; j > -1; j--) {
        commitChar(intToChar(arr[j]));
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
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	  LCD_init();
    LCD_command(0x01);
		
	printInLine("Welcome");
	delay(30000);
	clear_line_1();
	printTime(0, 0, 0);
	systemState = STOPPED;
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */
  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 15;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  /* USER CODE END TIM2_Init 2 */

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
  htim3.Init.Period = 124;
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
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 15999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA7
                           PA8 PA9 PA10 PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

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

