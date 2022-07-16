#include <stm32f4xx.h> 
#include <stdbool.h> 

// GPIOA -> For outputs : ( 0 to 7 for DataBus ) 
#define RS (8)
#define RW (9)
#define E  (10)
// GPIOB -> For interrupt lines
#define B1 (0) 
#define B2 (1) 
#define B3 (2) 
#define MASK(x)   (1UL << (x))

volatile uint16_t msec = 0; 
volatile uint16_t sec = 0;
volatile uint16_t min = 0; 
volatile uint32_t sysTimerTick = 0;
enum State { STOPPED, COUNTING, STARTUP, OFF }; 
enum State systemState = STARTUP; 

// Make Delay
void delay(uint16_t ms);
//Printing String or char
void printInLine(char* string);
void print(char ch);
// Initialize Configuations: set and activate interrupts, inputs, outputs
void init_configs(); 
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

// Config GPIOS
void init_gpios();

// Config Timers And Interrupt Lines
void init_interrupts_timers();

// Button 1
void EXTI0_IRQHandler(void){
	EXTI->PR |= MASK(0);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	systemState = COUNTING; 
}

// Button 2
void EXTI1_IRQHandler(void) {
	EXTI->PR |= MASK(1);
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	if(systemState == OFF)
		return; 
	
	systemState = STOPPED; 
}

uint32_t B3_clicked(){
	return GPIOB->IDR & MASK(B3);
}

// Checks long-push(B3)
bool long_push(){
  SysTick -> LOAD = 16000;
	SysTick -> VAL = 0;
	SysTick -> CTRL = 0x5;
	bool long_push = true; 
	
	for(int i = 0; i < 400; i++){
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

// Button 3
void EXTI2_IRQHandler(void) {
	EXTI->PR |= MASK(2);
	NVIC_ClearPendingIRQ(EXTI2_IRQn);
	
	EXTI-> IMR &= ~MASK(2); 
	
	min = sec = msec = 0; 	
	if ( long_push() ) {
		clear_line_2();
		systemState = OFF; 
		while(B3_clicked());
	}
	else {
		printTime(0, 0, 0);
		systemState = STOPPED;
	}
	EXTI-> IMR |= MASK(2); 
}

// Timer2 
void TIM2_IRQHandler(void){ 
	TIM2-> SR = 0; 
	// Counter Timer :
	if( systemState == COUNTING ){
			msec++; 
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

// Timer3 
void TIM3_IRQHandler(void){
	TIM3-> SR = 0; 
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

int main(void){
	//Configurations : 
	
	init_gpios(); 
	init_lcd(); 
	
	printInLine("Welcome");
	
	delay(50000);
	
	clear_line_1();
	printTime(0, 0, 0);
	init_interrupts_timers();
	systemState = STOPPED;
	
	while(1){
	}
}

void init_gpios(){
	// Enable GPIOA And GPIOB:
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOAEN; 
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN; 

	// Set Moder For GPIOA (11 outputs)
	GPIOA->MODER = 0x55555555;
	
	// Set Moder For GPIOB 
	GPIOB->MODER &= 0xFFFFFFA0;
	
	// Pull-down for inputs:
	GPIOB->PUPDR = 0x0000002A;
}

void init_interrupts_timers(){
	
		//Activate Timers 
	RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;	
	RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN; 
	
	// Config TIM2
	TIM2 -> PSC = 5 - 1; 
	TIM2 -> ARR = 8  - 1; 
	TIM2 -> CR1 = 1; 
	TIM2 -> DIER |= 1; 

	// Config TIM3
	TIM3 -> PSC = 16000 - 1; 
	TIM3 -> ARR = 125  - 1; 
	TIM3 -> CR1 = 1; 
	TIM3 -> DIER |= 1; 
	
	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	// Config Interrupt line
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB; 
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PB;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PB; 
	EXTI->IMR |= MASK(0) | MASK(1) | MASK(2); 

	EXTI->FTSR = EXTI->RTSR = 0x0; 
	EXTI->FTSR |= MASK(0) | MASK(1) | MASK(2); 
	EXTI->RTSR |= MASK(2); 
	
	
	// Enable Interrupts:
	__enable_irq(); 
	
	NVIC_ClearPendingIRQ(TIM2_IRQn);
	NVIC_ClearPendingIRQ(TIM3_IRQn);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	NVIC_ClearPendingIRQ(EXTI2_IRQn);
	
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
	
	
	
}
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
	GPIOA->ODR &= 0x00;
	GPIOA->ODR |= MASK(E);	
	
	GPIOA->ODR &= ~MASK(RW); 
	GPIOA->ODR &= ~MASK(RS);
	GPIOA->ODR |= command; 
	GPIOA->ODR &= ~MASK(E);
	
	if(command < 4)
		delay(2); 
	else 
		delay(1);
}
void commitChar(char data){
	GPIOA->ODR &= 0x00;
	GPIOA->ODR |= MASK(E);	
	
	GPIOA->ODR &= ~MASK(RW); 
	GPIOA->ODR |= MASK(RS) | data;
	GPIOA->ODR &= ~MASK(E);
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

void  clear_line_1(){
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
    int i;
    for (; n > 0; n--)
        for (i = 0; i < 200; i++) ;
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