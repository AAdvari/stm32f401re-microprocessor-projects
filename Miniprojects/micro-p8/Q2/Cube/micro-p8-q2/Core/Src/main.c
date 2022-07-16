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
#include <stm32f4xx.h> 
#include <stdbool.h> 
#include <string.h>

// GPIOA -> For outputs : 
#define RS (8)
#define RW (9)
#define E  (10)
#define C1 (9)
#define C2 (10)
#define C3 (11)
#define C4 (12)

// GPIOB -> For inputs 
#define R1 (1)
#define R2 (2)
#define R3 (3)
#define R4 (4)
#define B1 (5) 
#define B2 (6) 

#define MASK(x)   (1UL << (x))


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
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


volatile int currentCol = 0; 
volatile bool firstCharPresent = false; 
volatile bool isSecondOperandPresent = false; 
volatile bool isFirstOperandPresent = false; 
volatile int32_t firstOperand = 0;
volatile int32_t secondOperand = 0;
volatile char op = ' '; 
volatile bool unaryPresent = false; 

volatile uint8_t ubuff[1]; 
// Will be filled after USART interrupt ( after a character is received ) 
volatile uint8_t charFromUSART = ' ';

// send a char via uart 
void usart2_send_char(uint8_t ch);



// init USART2 
void usart2_init(uint32_t baudrate);

// Checks which button has been clicked 
char clicked_button();

// Checks which unary operator's button has been clicked 
char unary_click();

// Check type of chars 
bool is_binary_operator(char ch);
bool is_unary_operator(char ch);

void delayms(uint16_t ms);

//Printing String or char
void printInLine(char* string);
void printInLine2(char ch);

// Initialize Configuations: set and activate interrupts, inputs, outputs
void init_configs(); 

// turn on lcd and set proper mod 
void init_lcd();

// clear second line 
void clear_line_2();

// calculate the result of current operation based of provided data
void calculate();

// Converting intToChar and CharToInt 
int32_t charToInt(char ch);
char intToChar(int32_t digit);

// Prints Digits of a signed number using uart
void printDigit(int N);

// Print inc(x) or dec(x) 
void printUnaryOperator(int32_t operand, bool increment);

//Committing commands or chars
void commitCommand(char command);
void commitChar(char data);
void logStr(char* string, uint16_t size){
		HAL_UART_Transmit(&huart2,(uint8_t*) string , size, HAL_MAX_DELAY);
}
void logNum(uint32_t input){
	char number[10]; 
	sprintf(number, "%u", input);
	HAL_UART_Transmit(&huart2,(uint8_t*) number , 10, HAL_MAX_DELAY);
}
void usart2_send_char(uint8_t ch){
	USART2->DR = ch; 
	while(!READ_BIT(USART2->SR, USART_SR_TC)) {} 
}

void commitCommand(char command){
	GPIOC->ODR &= 0x00;
	GPIOC->ODR |= MASK(E);	
	
	GPIOC->ODR &= ~MASK(RW); 
	GPIOC->ODR &= ~MASK(RS);
	GPIOC->ODR |= command; 
	GPIOC->ODR &= ~MASK(E);
	HAL_Delay(2);
}
void commitChar(char data){
	GPIOC->ODR &= 0x00;
	GPIOC->ODR |= MASK(E);	
	
	GPIOC->ODR &= ~MASK(RW); 
	GPIOC->ODR |= MASK(RS) | data;
	GPIOC->ODR &= ~MASK(E);
	HAL_Delay(2);
}
void init_lcd(){
	// Turn On : 
	commitCommand(0x0E); 
	
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
char unary_click(){
	if( GPIOB->IDR & MASK(B1)){
		while(GPIOB->IDR & MASK(B1));
		return 'i';
	}
	else if(GPIOB->IDR & MASK(B2)){
		while(GPIOB->IDR & MASK(B2));
		return 'd';
	}
}
void delayms(uint16_t ms){
	uint32_t counter = 0;
	while(ms > 0) {
		counter = 0;
		while( counter < 1000){
			counter++; 
		}
		ms--; 
	}
}
void printInLine2(char ch){
	commitChar(ch);
}
bool is_binary_operator(char ch){
	return ch == '+' || ch == '-' || ch == '*' || ch =='/'; 
}
bool is_unary_operator(char ch){
	return ch == 'i' || ch == 'd'; 
}
char clicked_button(){
	
	if( GPIOB->IDR & MASK(R1)) {
		while( GPIOB->IDR & MASK(R1));
		switch(currentCol){
			case 0: return '7'; 
			case 1: return '8'; 
			case 2: return '9';
			case 3: return '/'; 
		}
	}
	if( GPIOB->IDR & MASK(R2)) {
		while(GPIOB->IDR & MASK(R2));
		switch(currentCol){
			case 0: return '4'; 
			case 1: return '5'; 
			case 2: return '6';
			case 3: return '*'; 
		}
	}
	if( GPIOB->IDR & MASK(R3)) {
		while(GPIOB->IDR & MASK(R3));
		switch(currentCol){
			case 0: return '1'; 
			case 1: return '2'; 
			case 2: return '3';
			case 3: return '-'; 
		}
	}
	if( GPIOB->IDR & MASK(R4)) {
		while(GPIOB->IDR & MASK(R4));
		switch(currentCol){
			case 0: return 'C'; 
			case 1: return '0'; 
			case 2: return '=';
			case 3: return '+'; 
		}
	}
	return unary_click();
}
void init_configs(){
	// Enable GPIOA And GPIOB:
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOAEN; 
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN; 
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOCEN; 

	
	// Set Moder For GPIOA
	GPIOA->MODER |= 0x55555555;

	// Set Moder For GPIOC (LCD Outputs) 
	GPIOC->MODER = 0x55555555;

	
	// Set Moder For GPIOB 
	GPIOB->MODER &= 0xFFFFF000;
	
	// Pull-down for inputs:
	GPIOB->PUPDR = 0x00002AAA;
	
	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	// Config Interrupt line
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB; 

	
	EXTI->IMR |= MASK(0);
	EXTI->RTSR |= MASK(0);
	__enable_irq(); 
	
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI0_IRQn);
	
	NVIC_ClearPendingIRQ(USART2_IRQn);
	NVIC_EnableIRQ(USART2_IRQn);

	NVIC_SetPriority(EXTI0_IRQn, 1);
	NVIC_SetPriority(USART2_IRQn, 0);
	
}
void printDigit(int N){
	// first we send the char via uart tx, then we receive it with rx
	// at last we print it on lcd 
		if(	N == 0 ){
			char btn[1] = { intToChar(N) };
			logStr(btn, 1); 
			printInLine2(charFromUSART);
			return; 
		}
		if( N < 0 ) {
			char btn[1] = { '-' };
			logStr(btn, 1);
			printInLine2(charFromUSART);
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
			char btn[1] = { intToChar(arr[j]) };
			logStr(btn, 1);
			printInLine2(charFromUSART);
    }
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
void calculate(){
	switch(op){
		case '+':
			firstOperand += secondOperand;
			break;
		case '-':
			firstOperand -= secondOperand;
			break;
		case '/':
			firstOperand /= secondOperand; 
			break;
		case '*':
			firstOperand *= secondOperand; 
			break; 
	}
	isFirstOperandPresent = true;
	isSecondOperandPresent = false; 
	secondOperand = 0;
	firstCharPresent = true; 
	op = ' '; 
	printDigit(firstOperand);
}
void printUnaryOperator(int32_t operand, bool increment){ 
	int32_t operandCopy = operand;
	
	if(unaryPresent && increment){
			operand--;  
	}
	if(unaryPresent && !increment) {
		operand++; 
		commitCommand(0x10);
		commitChar(' ');
	}
	if(operand == 0) 
		commitCommand(0x10);

	while(operand != 0){
		operand /= 10; 
		// move cursor left 
		commitCommand(0x10);
	}
	if(unaryPresent){
		commitCommand(0x10);
		commitCommand(0x10);
		commitCommand(0x10);
		commitCommand(0x10);
		commitCommand(0x10);
	}
	if( operandCopy < 0) 
		commitCommand(0x10);
	
	if(increment){
		printInLine2('i');
		printInLine2('n');
		printInLine2('c');
	}
	else {
		printInLine2('d');
		printInLine2('e');
		printInLine2('c');
	}
	printInLine2('(');
	printDigit(operandCopy);
	printInLine2(')');
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if( huart->Instance == USART2 ){
		charFromUSART = ubuff[0]; // Read the received character 
		HAL_UART_Receive_IT(&huart2, (uint8_t*) ubuff, 1);
	}
}
 //Interrupt Handler
void EXTI0_IRQHandler(void){
	EXTI->PR |= MASK(0);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	char clickedButton = clicked_button(); 
	
	char btn[1] = { clickedButton };
	logStr(btn, 1); 
	
	// pick the filled var after USART ISR completion 
	clickedButton = charFromUSART;
	
	// Uncheck unaryFlag when numbers are clicked 
	if (!is_unary_operator(clickedButton) && !is_binary_operator(clickedButton) ){
		unaryPresent = false; 
	}
	
	// Reset All Control VARS 
	if( clickedButton == 'C') {
		firstCharPresent = false; 
		isSecondOperandPresent = false;
		isFirstOperandPresent = false;
		firstOperand = 0;
		secondOperand = 0;
		unaryPresent = false; 
		op = ' ';
		clear_line_2(); 
	}
	
	// Calculate result
	else if( clickedButton == '='){
		clear_line_2(); 
		calculate();
	}
	
	// Checking Binary Operators : 
	else if( is_binary_operator(clickedButton) ){	
		// Unary Minus Checking : 
		if( !firstCharPresent ){
			if(clickedButton == '-'){
				firstOperand = 0; 
				isFirstOperandPresent = true; 
				firstCharPresent = true; 
				op = '-'; 
				printInLine2('0');
				printInLine2('-'); 
			}
		}
		else {
			// Check wether operator has came after a incompleted operation 
			if(isSecondOperandPresent){
				clear_line_2();
				calculate();
			}
			else{
				isFirstOperandPresent = true; 
				// overwrite last operator 
				if( op != ' ')
					commitCommand(0x10);
			}
			op = clickedButton;
			printInLine2(op);
		}
		
	}
	
	// unary operatos : (Increment or decrement proper operand and print )  
	else if (is_unary_operator(clickedButton)){
		int32_t operand;
		bool increment;
		if(isSecondOperandPresent){
			if(clickedButton == 'i'){ // i = increment _ d = decrement 
				operand = secondOperand++;
				increment = true;
			}
			else{  
				operand = secondOperand--;
				increment = false;
			}
		}
		else if (op != ' ')
			return;
		else if (!firstCharPresent) 
			return; 
		else{
			if(clickedButton == 'i'){
				operand = firstOperand++;
				increment = true;
			}				
			else{
				operand = firstOperand--;	
				increment = false; 
			}
		}
		printUnaryOperator(operand, increment);
		unaryPresent = true; 
	}
	// Read first operand:
	else if( !isFirstOperandPresent ){ 
		firstOperand *= 10; 
		firstOperand += charToInt(clickedButton);
		if( ! ( clickedButton == '0' && firstOperand == 0 && firstCharPresent ) ) 
			printInLine2(clickedButton);
		firstCharPresent = true; 

	}
	// Read Second Operand:
	else{
		isSecondOperandPresent = true; 
		secondOperand *= 10; 
		secondOperand += charToInt(clickedButton);
		printInLine2(clickedButton); 
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
	init_configs(); 
	init_lcd();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	
	//Configurations 
	HAL_UART_Receive_IT(&huart2, (uint8_t*) ubuff, 1);
	
	printInLine("Advari-Heidari"); 
	printInLine("Welcome"); 
	HAL_Delay(1000);
	clear_line_2();
	
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 2);
	HAL_NVIC_SetPriority(USART2_IRQn, 0,0);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
			GPIOA->ODR &= ~MASK(currentCol+9); 
			currentCol =(currentCol+1) % 4;
			GPIOA->ODR |= MASK(currentCol+9); 
			delayms(1);
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

