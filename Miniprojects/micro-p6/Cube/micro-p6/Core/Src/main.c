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
#include <time.h> 
#include <stdlib.h>

// GPIOA 
#define RS (8)
#define RW (9)
#define E  (10)
#define A (11) 
#define B (12)
#define C (13)
#define D (14)
#define RESET_IC (15)

// GPIOB
#define LED_GREEN_PWM (0)
#define LED_RED_PWM (1)
#define C1 (2)
#define C2 (3) 
#define C3 (4) 
#define SA (7)
#define SB (8)
#define SC (9)
#define SD (10)
#define SE (12)
#define SF (13)
#define SG (14)

//GPIOC 
#define BLUE (1)
#define GREEN (2) 
#define YELLOW (3)
#define RED (4) 
#define RESET (5)
#define LED_BLUE (6)
#define LED_GREEN (7)
#define LED_YELLOW (8) 
#define LED_RED (9) 
#define GREEN_PWM_ENABLE (10)
#define RED_PWM_ENABLE (11)
#define CONTINUE (12)
#define MASK(x)   (1UL << (x))


const uint16_t allowedMisses = 3; 
const uint16_t gameTime = 60; 

volatile uint16_t currentRow = 0;
volatile char inputID[8]; 
volatile uint16_t inputIDIndex = 0;
volatile uint16_t misses = 0; 
char showingName[30]; 
enum State { STARTUP, SHOW_NAME, RUNNING, RESULT };
volatile enum State systemState = STARTUP; 
volatile uint16_t remainingTime = gameTime;

enum ResultType { WIN, LOOSE, UNKNOWN};
volatile enum ResultType resultType = UNKNOWN; 

// For IC 
 uint32_t count = 0; 
 bool errorPassed = false;
 uint32_t lastCaptured = 0; 



volatile bool stopped = false; 

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
void clear_line_1();
uint16_t segment(uint16_t value);
void loose();
void win();
void showMisses();
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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t segment(uint16_t value) {
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

void commitCommand(unsigned char command){
	
	HAL_GPIO_WritePin(GPIOA, 0xFF, GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GPIOA, MASK(RS) | MASK(RW), GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, MASK(E) | command, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, MASK(E), GPIO_PIN_RESET);

	if(command < 4)
		delay(2); 
	else 
		delay(1);
}
void commitChar(char data){
	HAL_GPIO_WritePin(GPIOA, 0xFF | MASK(RW), GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, MASK(E) | MASK(RS) | data , GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, MASK(E) , GPIO_PIN_RESET);
	delay(1);
}
void init_lcd(){
	// Turn On : 
	commitCommand(0x0C); 
	
	// 8-bit 2 line mode :
	commitCommand(0x38);
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
// SET name to proper variable
void setName(char* name){
	  uint16_t index = 0;
		while(*name != '\0'){
		showingName[index++] = (char) (*name); 
		name++;
	}
}
// check clicked button
char clickedButton(){
		if( HAL_GPIO_ReadPin( GPIOB, MASK(C1)) == GPIO_PIN_SET) {
			while(( HAL_GPIO_ReadPin( GPIOB, MASK(C1)) == GPIO_PIN_SET));
			switch(currentRow) {
				case 0:
					return '1';
				case 1:
					return '4';
				case 2:
					return '7';
				case 3:
					return '*';
				default:
					return '!';
			}
		}
		else if(HAL_GPIO_ReadPin( GPIOB, MASK(C2)) == GPIO_PIN_SET){
			while(( HAL_GPIO_ReadPin( GPIOB, MASK(C2)) == GPIO_PIN_SET));
			switch(currentRow) {
				case 0:
					return '2';
				case 1:
					return '5';
				case 2:
					return '8';
				case 3:
					return '0';
				default:
					return '!';
			}
		}
		else if(HAL_GPIO_ReadPin( GPIOB, MASK(C3)) == GPIO_PIN_SET){
			while(( HAL_GPIO_ReadPin( GPIOB, MASK(C3)) == GPIO_PIN_SET));
			switch(currentRow) {
				case 0:
					return '3';
				case 1:
					return '6';
				case 2:
					return '9';
				case 3:
					return '#';
				default:
					return '!';
			}
		}
		else { // RESET has been clicked
			if (HAL_GPIO_ReadPin( GPIOC, MASK(RESET)) == GPIO_PIN_SET){
					uint32_t start = HAL_GetTick(); 
					uint32_t delay = 300; 
					bool long_push = true; 
					while( (HAL_GetTick() - start) < delay){
							if(HAL_GPIO_ReadPin( GPIOC, MASK(RESET)) == GPIO_PIN_RESET){
								long_push = false; 
								break; 
							}
						
						if(!long_push) 
							break; 
					}
					
					if(long_push)
						return 'l';
					else 
						return 'e'; 
					
			}

				
		}
}
void clearLEDs(){
	HAL_GPIO_WritePin(GPIOC, MASK(LED_GREEN) | MASK(LED_RED) |MASK(LED_YELLOW) | MASK(LED_BLUE) , GPIO_PIN_RESET);
}
// reset all program states & vars. 
void resetAll(){
	resultType = UNKNOWN;
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
	HAL_GPIO_WritePin(GPIOC, MASK(RED_PWM_ENABLE), GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, MASK(GREEN_PWM_ENABLE), GPIO_PIN_RESET);
	systemState = STARTUP;
	clear_line_2();
	clearLEDs(); 
	remainingTime = gameTime;
	inputIDIndex = 0;
	count = 0; 
	misses = 0; 
	showMisses();
	commitCommand(0x80); // Move Pointer to firstLine
	
	MX_TIM3_Init();
	HAL_TIM_Base_Start_IT(&htim3);
}
// show given color on leds. 
void showColor(char color){
	switch(color){
		case 'b':
			HAL_GPIO_WritePin(GPIOC, MASK(LED_GREEN) | MASK(LED_RED) |MASK(LED_YELLOW) , GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, MASK(LED_BLUE), GPIO_PIN_SET);
			return;
		case 'y':
			HAL_GPIO_WritePin(GPIOC, MASK(LED_BLUE) | MASK(LED_RED) |MASK(LED_GREEN) , GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, MASK(LED_YELLOW), GPIO_PIN_SET);
			return;
		case 'r':
			HAL_GPIO_WritePin(GPIOC, MASK(LED_BLUE) | MASK(LED_GREEN) |MASK(LED_YELLOW) , GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, MASK(LED_RED), GPIO_PIN_SET);
			return;
		case 'g':
			HAL_GPIO_WritePin(GPIOC, MASK(LED_BLUE) | MASK(LED_RED) |MASK(LED_YELLOW) , GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, MASK(LED_GREEN), GPIO_PIN_SET);
			return;
	}
}
// show misses on 7segment
void showMisses(){
	HAL_GPIO_WritePin( GPIOB, 0xFF << 7, GPIO_PIN_RESET); 
	HAL_GPIO_WritePin( GPIOB, segment(misses), GPIO_PIN_SET);
}
char clickedLED(){
	if( HAL_GPIO_ReadPin( GPIOC, MASK(BLUE)) == GPIO_PIN_SET) 
		return 'b';
	else if (HAL_GPIO_ReadPin( GPIOC, MASK(GREEN)) == GPIO_PIN_SET)
		return 'r';
	else if (HAL_GPIO_ReadPin( GPIOC, MASK(YELLOW)) == GPIO_PIN_SET)
		return 'y';
	else if (HAL_GPIO_ReadPin( GPIOC, MASK(RED)) == GPIO_PIN_SET)
		return 'g';
	else 
		return 'n';
}
void delayms(uint16_t ms){
	uint32_t start = HAL_GetTick(); 
	while( (HAL_GetTick() - start) < ms);
}
char randomColor(){
	int val = (int) rand() % 4; 
	switch(val){
		case 0: 
			return 'b';
		case 1:
			return 'y';
		case 2:
			return 'g'; 
		case 3:
			return 'r';

	}
}
// turnOn random led and check user input !
void showAndCheck(uint16_t delayms){
	if( systemState != RUNNING) 
		return;
	bool passed = false;
	char rand = randomColor();
	showColor(rand);
	uint32_t start = HAL_GetTick(); 
	while( (HAL_GetTick() - start) < delayms){
		if(systemState != RUNNING) 
			return; 
		char clicked = clickedLED();
		if( clicked == rand)
			passed = true;
		else if( clicked != 'n' && !passed){
				break;
		}
	}
	while( (HAL_GetTick() - start) < delayms && systemState == RUNNING); 
	if(systemState != RUNNING) 
			return; 
	if(!passed){
		misses++; 
		showMisses();
		if(misses >= allowedMisses){
			loose();
		}
	}
}
// run game with given scenario and timings .
void runGame(){
		uint16_t timeScaler = 8;
		for(int i = 0; i<3; i++){
			if(systemState != RUNNING) 
				return; 
			showAndCheck(3000/timeScaler);
		}
		for(int i = 0; i<3; i++){
			if(systemState != RUNNING) 
				return; 
			showAndCheck(2000/timeScaler);
		}
		uint16_t delay = 1000; 
		while(time>0 && systemState == RUNNING) {
			for(int i=0; i < 10; i++)
				showAndCheck(delay/timeScaler);
			delay -= 100;
		}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_PIN_0) {
			if( HAL_GPIO_ReadPin(GPIOC, MASK(CONTINUE)) == GPIO_PIN_SET) {
				stopped = false; 
				return; 
			}
			
			char clickedBtn = clickedButton();

			if( systemState == STARTUP ){
				if( clickedBtn != 'e') 
					inputID[inputIDIndex++] = clickedBtn;
				if(inputIDIndex == 8){
					clear_line_1();
					commitCommand(0xC0);
					if( inputID[7] == '4')
						setName("Advari");
				  else 
						setName("Heidari");
					systemState = SHOW_NAME; 
				}
				else if( clickedBtn != 'e') 
					print(clickedBtn);
			}
			else if (systemState == RUNNING || systemState == RESULT || systemState == SHOW_NAME){
				if(clickedBtn == '*' || clickedBtn == 'l')
					resetAll();
			}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
 
	
	if( !errorPassed){
		errorPassed = true; 
		return; 
	}

	if( systemState == SHOW_NAME) {
		count++;
		// first and third edges (rising)
		if(count == 1 || count == 3){
			lastCaptured = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		}
		// second edge  (falling)
		else if(count == 2){
			uint32_t currentCaptured = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			if( currentCaptured - lastCaptured < 2000/16) 
				count = lastCaptured = 0; 
		}
		// check wether appropriate pulse has been s een or not. 
		else if(count == 4) {
			uint32_t currentCaptured = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			if( currentCaptured - lastCaptured >= 2000/16) 
			 {
				count = lastCaptured = 0;
				systemState = RUNNING; 
				remainingTime = gameTime; 
			}
			else 
				count = lastCaptured = 0;
		}
		
	}
	else if(systemState == RUNNING) {
		static bool risingEdge = true; 
		static uint32_t lastCap = 0; 
		
		if(risingEdge){
			lastCap = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			risingEdge = false;
		}
		else { 
			if( HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1) - lastCap > 2000/16 ) {
				stopped = true; 
				SysTick->CTRL = 0;
			}
			else {
				risingEdge = true; 
				lastCap = 0; 
			}
			
		}
	}
	
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	// for toggling name 
	if( htim->Instance == TIM4) {
		if( systemState == SHOW_NAME){
			static bool printed = false; 
			if(printed)
				clear_line_2();
			else 
				printInLine(showingName);
			printed = !printed;
		}
		else if( systemState== RESULT && (resultType == WIN || resultType== LOOSE) ){
			static bool tprinted = false; 
			if(tprinted)
				clear_line_2();
			else 
				printInLine(resultType == WIN ? "winner" : "looser" );
			tprinted = !tprinted;
		}
	}
	// decrementing time and checking win-conditions
	if( htim->Instance == TIM3){
		if( systemState == RUNNING && !stopped) {
			clear_line_2();
			remainingTime--;
			if(remainingTime == 0){
				if(misses < allowedMisses ) 
					win();
				else 
					loose();
			}
			else 
				printDigit(remainingTime);
		}
	}
}
// bring system to loose state ( red pwm ) 
void loose(){
	if(resultType == WIN)
		return;
	resultType = LOOSE;
	clearLEDs();
	systemState = RESULT; 
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
	HAL_GPIO_WritePin(GPIOC, MASK(RED_PWM_ENABLE), GPIO_PIN_SET);
	clear_line_2();
	printInLine("Looser");
}
// bring system to win state ( green pwm ) 
void win(){
	if(resultType == LOOSE)
		return;
	resultType = WIN; 
	clearLEDs();
	systemState = RESULT; 
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
	HAL_GPIO_WritePin(GPIOC, MASK(GREEN_PWM_ENABLE), GPIO_PIN_SET);
	clear_line_2();
	printInLine("Winner"); 
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
		init_lcd();
		HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
		HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
		HAL_TIM_Base_Start_IT(&htim2);
		HAL_TIM_Base_Start_IT(&htim3);
		HAL_TIM_Base_Start_IT(&htim4);



		HAL_GPIO_WritePin( GPIOB, segment(misses), GPIO_PIN_SET);
		uint16_t rows[] = { A, B, C, D};
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if( systemState != RUNNING){
		HAL_GPIO_WritePin(GPIOA, MASK(rows[currentRow]), GPIO_PIN_RESET);
		currentRow = (currentRow+1)% 4;
		HAL_GPIO_WritePin(GPIOA, MASK(rows[currentRow]), GPIO_PIN_SET);
		delay(180);
		}
		else {
			HAL_GPIO_WritePin(GPIOA, MASK(D), GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, MASK(A)|MASK(B)|MASK(C) , GPIO_PIN_RESET);
			currentRow = 3; 
			runGame();
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
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 15999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999999;
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
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 2;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
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
  TIM_OC_InitTypeDef sConfigOC = {0};

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
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 60;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

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
  htim4.Init.Period = 124;
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC1 PC2 PC3
                           PC4 PC5 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA7
                           PA8 PA9 PA10 PA11
                           PA12 PA13 PA14 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 PB3 PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB12 PB13 PB14
                           PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC6 PC7 PC8 PC9
                           PC10 PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
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

