  AREA RESET, DATA, READONLY
  DCD          0    ;initial_sp
  DCD          START;reset_vector

  AREA    MyCode, CODE, READONLY  
	  
	  
START 

	;Initialize R0 by 0 (summation result will be stored in R1) 
	MOV R1, #0 
	
	;Rename R0 To COUNT and initialize by 8
COUNT RN R0 
	MOV COUNT, #8
	
	;Rename R2 To STEP and initialize by 1 
STEP RN R2 
	MOV STEP, #1
	
IN	
	;Loop Condition Checking:
	CMP STEP, COUNT 
	BGT OUT 
	
	;Loop Body: 
	ADD R1, R1, STEP  
	ADD STEP, STEP, #1
	
	;Branch to Condition Checking:
	B IN
OUT 


HERE
    B    HERE        ;stay here forever
	

  END