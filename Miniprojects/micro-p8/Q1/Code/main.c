#include <stm32f4xx.h> 
#include <stdbool.h> 

#define MASK(x)   (1UL << (x))

// sw delay 
void delay(uint32_t ms);


// Configure usart6 with given baudrate 
void usart2_init(uint32_t baudrate);


// send data using usart6 
void usart2_send(uint16_t data);

// Initialize adc
void init_adc(); 


// read adc pin 
uint32_t read_adc();

// conv digit to char 
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
// send a char via uart 
void usart2_send_char(uint8_t ch){
	USART2->DR = ch; 
	while(!READ_BIT(USART2->SR, USART_SR_TC)) {} 
}

// send digits of an integer: 
void printDigit(int N){
		if(	N == 0 ){
			usart2_send_char(intToChar(N));
			return; 
		}
		if( N < 0 ) {
			usart2_send_char('-'); 
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
        usart2_send_char(intToChar(arr[j]));
    }
		usart2_send_char('\n\r');
}


int main(void){
	
	//Configurations : 
	init_adc(); 
	usart2_init(115200);
	
	// Main Loop of program: 
	delay(2000);
	while(1){
		delay(2000);
		printDigit( (uint16_t) read_adc() );
	}
}



void delay(uint32_t ms){
	uint32_t counter = 0;
	while(ms > 0) {
		counter = 0;
		while( counter < 1000){
			counter++; 
		}
		ms--; 
	}
}


uint32_t read_adc(){
	while(1){
		ADC1->CR2 |= 0x40000000;  // Start 
		while( !(ADC1->SR & 2)){} // Wait for conv to finish 
		float val =  (ADC1->DR/4095.0)*5*100; 
		return (uint16_t) val; 
	}
}
void usart2_init(uint32_t baudrate){
		
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
		GPIOA->MODER |= GPIO_MODER_MODER2_1;//Pin2 mode AF
		GPIOA->OSPEEDR|= GPIO_OSPEEDER_OSPEEDR2_1;
		GPIOA->AFR[0]|= 0x00000700;//Set the AF to AF7(USART1~3);
		
		USART2->CR1 |= USART_CR1_UE;
		USART2->BRR = SystemCoreClock/baudrate;
		USART2->CR1 |= USART_CR1_TE;
}


void init_adc(){
	
		/* set up pin PA1 for analog input */
		RCC->AHB1ENR |= 1; /* enable GPIOA clock */
		GPIOA->MODER |= 0xC; /* PA1 analog */
		/* setup ADC1 */
		RCC->APB2ENR |= 0x00000100; /* enable ADC1 clock */
		ADC1->CR2 = 0; /* SW trigger */
		ADC1->SQR3 = 1; /* conversion sequence starts at ch 1 */
		ADC1->SQR1 = 0; /* conversion sequence length 1 */
		ADC1->CR2 |= 1; /* enable ADC1 */
		// read_adc() to read value 
	
}
