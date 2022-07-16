  AREA RESET, DATA, READONLY
  DCD          0    ;initial_sp
  DCD          START;reset_vector
	  
	  
;make area for stack and allocate memory : 
  AREA StackArea, DATA, READWRITE 
Stack_Mem SPACE 0x18 			 

  AREA    MyCode, CODE, READONLY  
  
	  
START 

;initialize SP as a descending stack pointer: 
	LDR R0, =Stack_Mem 
	ADD R0, R0, 0x18 
	MOV SP, R0

;Renaming registers for simplicity : 
INPUT RN R0	 
	MOV INPUT, #0xFFFFFFF0
	
ONES RN R1 
	MOV ONES, #0 
	
ZEROS RN R2
	MOV ZEROS, #0 
	
STEP RN R3 
	MOV STEP, #1 
	
RESULT RN R4 
	MOV RESULT, #0
	
	
;Counting count of ones and zeros: 	
IN_COUNTER

	;Loop Condition Checking:
	CMP STEP, #32                   ; Compare STEP and 32 
	BGT OUT_COUNTER                 ; if STEP > 32 then go to OUT_COUNTER
	
	
	ANDS RESULT, INPUT, #0x00000001 ; RESULT = INPUT & 1 
	
	ADDEQ ZEROS, ZEROS, #1          ; if result==0 then zeros++
	ADDNE ONES, ONES, #1            ; if result!=0 then ones++ 
	
	LSR INPUT, INPUT, #1            ; input = input >> 1 
	
	ADD STEP, STEP, #1              ; step++
	B IN_COUNTER                    ; go to IN_COUNTER 
	
OUT_COUNTER
	
	BL FUNC 

END_START 
	B END_START 


FUNC 
; ONES  : R1
; ZEROS : R2

	PUSH {ONES, ZEROS, LR} 
	
	MOV R3, #3 
	MUL R2, R1, R3
	
	MOV R3, #100
	SUB R3, R3, R2 
	
	POP {ONES, ZEROS, LR} 
	MOV PC, LR 
	

END