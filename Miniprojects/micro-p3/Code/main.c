#include <stm32f4xx.h> 
#include <stdbool.h> 

// GPIOA -> For outputs : 
#define RS (8)
#define RW (9)
#define E  (10)
#define C1 (11)
#define C2 (12)
#define C3 (13)
#define C4 (14)

// GPIOB -> For inputs 
#define R1 (1)
#define R2 (2)
#define R3 (3)
#define R4 (4)
#define B1 (5) 
#define B2 (6) 

#define MASK(x)   (1UL << (x))

volatile int currentCol = 0; 
volatile bool firstCharPresent = false; 
volatile bool isSecondOperandPresent = false; 
volatile bool isFirstOperandPresent = false; 
volatile int32_t firstOperand = 0;
volatile int32_t secondOperand = 0;
volatile char op = ' '; 
volatile bool unaryPresent = false; 


// Checks which button has been clicked 
char clicked_button();

// Checks which unary operator's button has been clicked 
char unary_click();

// Check type of chars :
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

// Prints Digits of a signed number
void printDigit(int N);

// Print inc(x) or dec(x) 
void printUnaryOperator(int32_t operand, bool increment);

//Committing commands or chars
void commitCommand(char command);
void commitChar(char data);

// Interrupt Handler
void EXTI0_IRQHandler(void){
	EXTI->PR |= MASK(0);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	char clickedButton = clicked_button(); 
	
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

int main(void){
	
	
	//Configurations : 
	init_configs(); 
	init_lcd(); 
	printInLine("Advari-Heidari"); 
	printInLine("Welcome"); 
	delayms(5000);
	clear_line_2();
	
	// Main Loop of program: 
	char cols[] = { C1, C2, C3, C4}; 
	// in each iteration one columns's pin is set to 1; 
	while(1){
		
		GPIOA->ODR &= ~MASK(cols[currentCol]); 
		currentCol =(currentCol+1) % 4;
		GPIOA->ODR |= MASK(cols[currentCol]); 
		delayms(1);
	}
}

void commitCommand(char command){
	GPIOA->ODR &= 0x00;
	GPIOA->ODR |= MASK(E);	
	
	GPIOA->ODR &= ~MASK(RW); 
	GPIOA->ODR &= ~MASK(RS);
	GPIOA->ODR |= command; 
	GPIOA->ODR &= ~MASK(E);
	delayms(1);
}
void commitChar(char data){
	GPIOA->ODR &= 0x00;
	GPIOA->ODR |= MASK(E);	
	
	GPIOA->ODR &= ~MASK(RW); 
	GPIOA->ODR |= MASK(RS) | data;
	GPIOA->ODR &= ~MASK(E);
	delayms(1);
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

	// Set Moder For GPIOA (14 outputs)
	GPIOA->MODER = 0x55555555;
	
	// Set Moder For GPIOB 
	GPIOB->MODER &= 0xFFFFC000;
	
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

}
void printDigit(int N){
		if(	N == 0 ){
			printInLine2(intToChar(N));
			return; 
		}
		if( N < 0 ) {
			printInLine2('-'); 
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
        printInLine2(intToChar(arr[j]));
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