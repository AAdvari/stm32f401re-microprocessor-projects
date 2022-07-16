#include <stm32f4xx.h> 
#include <stdbool.h>
#define B1 (0)     // input
#define B2 (1)		 // input 
#define RED (2)    // output
#define GREEN (3)  // output
#define A (4)      // output
#define B (5)      // output
#define C (6)      // output
#define D (7)      // output
#define E (8)      // output
#define F (9)      // output
#define G (10)     // output
#define MASK(x) (1UL << (x))

volatile int8_t counter = 9; 
volatile bool is_red_active = 1; 
volatile uint8_t b1_clicks = 0;
volatile uint8_t b2_clicks = 0; 

void check_clicks(void){
		if(GPIOA->IDR & MASK(B1) ){
				b1_clicks++;
			
			//Stay here until the user releases the button:
				while(GPIOA->IDR & MASK(B1)){}
		}
		if(GPIOA->IDR & MASK(B2) ) {
				b2_clicks++;
			
				//Stay here until the user releases the button:
				while(GPIOA->IDR & MASK(B2)){}

		}
}
void delay(uint8_t halfsec){
	for(int i = 0; i < halfsec; i++){
		for(int j = 0; j < 400000; j++) {
			
			// check push-buttons :
			check_clicks();
		}
	}
}

// Convert integer to its correspondig 7-segment  format
uint16_t segment(uint16_t value) {
	switch(value){
		case 0:
			return MASK(A) | MASK(B) | MASK(C) | MASK(D) | MASK(E) | MASK(F);
		case 1:
			return MASK(B) | MASK(C);
		case 2:
			return MASK(A) | MASK(B) | MASK(D) | MASK(E) | MASK(G);
		case 3:
			return MASK(A) | MASK(B) | MASK(C) | MASK(D) | MASK(G);
		case 4:
			return MASK(B) | MASK(C) | MASK(F) | MASK(G);
		case 5:
			return MASK(A) | MASK(C) | MASK(D) | MASK(F) | MASK(G);
		case 6:
			return MASK(A) | MASK(C) | MASK(D) | MASK(E) | MASK(F) | MASK(G);
		case 8:
			return MASK(A) | MASK(B) | MASK(C) | MASK(D) | MASK(E) | MASK(F) | MASK(G);
		case 7:
			return MASK(A) | MASK(B) | MASK(C);
		case 9:
			return MASK(A) | MASK(B) | MASK(C) | MASK(D) | MASK(F) | MASK(G);
		default:
			return MASK(A) | MASK(B) | MASK(C) | MASK(D) | MASK(E) | MASK(F);
	}
}

int main(void){
	
	// Enable GPIOA:
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOAEN; 
	
	// Set Input & Outputs as decared at the beginning  (00 for input and 10 for output) 
	GPIOA->MODER = 0xFFD55550;

	// Pull-down for first and second input (B1 & B2) 
	GPIOA->PUPDR &= ~MASK(0);
	GPIOA->PUPDR |= MASK(1);
	GPIOA->PUPDR &= ~MASK(2);
	GPIOA->PUPDR |= MASK(3);
	
	// Main Loop of program: 
	while(1){
		
		// Check wether user has pushed buttons:
		check_clicks();

		// If b1 has been pushed two times, change the timer and led side. 
		if( b1_clicks == 2){
			b1_clicks = 0; 
			b2_clicks = 0;
			counter = 9; 
			is_red_active = !is_red_active;
		}
		
		// If b2 has been pushed for 4th time, count !
		if( b2_clicks == 4){
			b2_clicks = b1_clicks =  0;
		}
		
		//if b2 has been pushed 3 times, stop the timer else set proper outputs & wait 1s
		if( b2_clicks != 3){
			
			// if active LED is red, activate red and deactivate green and show counter on segments:
			if( is_red_active ) {
				GPIOA->ODR &= 0x0;
				GPIOA->ODR |= MASK(GREEN) | segment(counter); 
			}
			// if active LED is green, activate green and deactivate red and show counter on segments:
			else {
				GPIOA->ODR = 0x0;
				GPIOA->ODR |= MASK(RED) | segment(counter); 
			}
			//decrement counter :
			counter--; 
			// if counter reached -1, reset counter, change led and 7segment's side :
			if(counter == -1) {
				counter = 9; 
				is_red_active = !is_red_active;
			}
			delay(2); 
		}
	}
	return 0;
}