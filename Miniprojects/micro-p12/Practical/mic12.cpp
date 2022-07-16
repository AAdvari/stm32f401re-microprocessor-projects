// micro-p12-console.cpp : main project file.

#include "stdafx.h"
#include <conio.h>


// util functions for printing vectors 
void print_vector(float *ptr, int size);
void print_vectors(float *zi1, float *zi2, float *zi3);

// Calculates y2-a, y2-b and 1/2(y4+(b-a)y2+ab) and stores them in zi1, zi2 and zo1 
void f1(float a, float b, float *zi1, float *zi2, float *zo1);

// Calculates 1/2(zi1*zi2) and stores the result in zo2 (using assembly)
void div_asm(float *zi1, float *zi2, float *zo2);

// Copies the second array to first one :
void cp_fp_arrays(float *arr1, float *arr2, int size);

int main()
{

	float a, b;
	// Get Inputs 
	cprintf("Enter floating point numbers a, b\n");
	cscanf("%f %f", &a, &b);
	cprintf("%f %f\n", a, b);

	// Define Vectors 
	float zi1[50];
	float zi2[50];
	float zo1[50];
	float zo2[50];
	

	f1(a, b, zi1, zi2, zo1);
	cprintf("\nC program to calculate y2-a, y2-b and 1/2(y4+(b-a)y2+ab) \n\n");
	print_vectors(zi1, zi2, zo1);

	cprintf("\nassembly program to calculate 1/2(y4+(b-a)y2+ab) using y2-a, y2-b which has been calculated by c program: \n\n");
	div_asm(zi1, zi2, zo2);
	print_vectors(zi1, zi2, zo2);

	cprintf("\n\n");
	getch(); 
	getch(); 
    return 0;
}

void cp_fp_arrays(float *arr1, float *arr2, int size){
	for( int i = 0; i < size; i++){
		arr1[i] = arr2[i];
	}
}

void div_asm(float *zi1, float *zi2, float *zo2){
	__declspec(align(16)) float zi1_aligned[50];
	__declspec(align(16)) float zi2_aligned[50];
	__declspec(align(16)) float zo2_aligned[52];
	__declspec(align(16)) float ones[4] = { 1.0, 1.0, 1.0, 1.0};
	__declspec(align(16)) float twos[4] = { 2.0, 2.0, 2.0, 2.0};

	cp_fp_arrays(zi1_aligned, zi1, 50);
	cp_fp_arrays(zi2_aligned, zi2, 50);

	_asm {

	mov ecx, 0
ITERATE: 
	
	movaps xmm0, oword ptr zi1_aligned[ecx] 
	movaps xmm1, oword ptr zi2_aligned[ecx]
	movaps xmm2, oword ptr ones
	movaps xmm3, oword ptr twos

	mulps xmm0, xmm3 ; xmm0*2
	mulps xmm0, xmm1 ; xmm0*xmm1
	divps xmm2, xmm0 ; 1/xmm0 

	;now xmm2 holds 1/(2*zi1*zi2)

	movaps oword ptr zo2_aligned[ecx], xmm2

	add ecx, 16 
	cmp ecx, 208

	JE DONE 
	JMP short ITERATE
DONE:

	}
	cp_fp_arrays(zo2, zo2_aligned, 50); 
}
void f1(float a, float b, float *zi1, float *zi2, float *zo1){

	// fill y's 
	float y[50]; 
	float step = 0.0; 
	for(int i = 0; i < 50; i++){
		y[i] = step; 
		step += 0.2; 
	}

	float y2;
	for(int i = 0; i < 50; i++){
		y2 = (y[i]*y[i]);
		zi1[i] = y2 - a;
		zi2[i] = y2 + b; 
		zo1[i] = 0.5 * (1/( zi1[i]*zi2[i]));
	}
}
void print_vector(float *ptr, int size){
	for(int i = 0; i < size; i++){
		cprintf("%f ", ptr[i]);
	}
}
void print_vectors(float *zi1, float *zi2, float *zi3){
	cprintf("y2-a:\n");
	print_vector(zi1, 50);
	cprintf("\n\ny2-b:\n");
	print_vector(zi2, 50);
	cprintf("\n\n1/2(y4-y2(b-a)+ab):\n");
	print_vector(zi3, 50);
	cprintf("\n\n");
}
