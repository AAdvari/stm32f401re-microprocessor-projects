.MODEL SMALL 
.386
.STACK 64 
.DATA 

	DATA1 DD 0EH	   ; Div/Mult first Operand
	DATA2 DD 07H       ; Div/Mult Second Operand 

	DATA3 DD 0,0       ; Multiplication res 
	DATA4 DD 0         ; Division res 
	
	OUT_NUMBER DD 10000000H   ; for testing print32
	
	
	OUT_STR DB 11 DUP ('$')   ; used to cast int32 to string 

	BUFF DB 8
		 DB ? 
		 DB 8 DUP(0)         ; read string from input 
		 
	NEW_LINE DB 13,10, '$' 
	
	IN_NUMBER DD 00000000H   ; store input number in binary format 
.CODE 

										   
; -----------------------------------------    
PRINT32 PROC FAR 
	PUSH BX
	ADD BX, 2 	
	CALL PRINT16
	POP BX
	CALL PRINT16
	
	LEA DX,OUT_STR
    MOV AH,9
    INT 21H 
	RET
PRINT32 ENDP 

PRINT16	PROC FAR 
	   MOV AX, [BX] 
	   MOV CX, 4
	   MOV BX, 16
LOOP1:
	   MOV DX,0
       DIV BX
	   
	   CMP DL, 9
	   JG CHAR
       ADD DL,30H
	   JMP short PUSH_STACK
CHAR:
	   ADD DL, 37H
PUSH_STACK:
   
       PUSH DX
       LOOP LOOP1 
       
	   MOV CX, 4
LOOP2: 
	   POP AX                         
       MOV [SI],AL
	   INC SI
       LOOP LOOP2	
	   RET 
PRINT16 ENDP 


; -----------------------------------------

GETNUM		PROC FAR 

	MOV AH, 0AH 
	MOV DX, OFFSET BUFF 
	INT 21H 
	
	MOV SI, OFFSET BUFF+1 
	MOV CL, [SI] 
	MOV CH, 0 
	INC CX 
	ADD SI, CX 
	MOV AL, '$' 
	MOV [SI], AL 
	
	MOV AH, 9 
    MOV DX, OFFSET NEW_LINE 
    int 21h
	
	mov ah, 9 
    mov dx, offset BUFF + 2 ;START OF READ VALUE 
    int 21h
	
	DEC CX ; now cx holds the length
	MOV SI, OFFSET BUFF+1 
	ADD SI, CX 
	
	MOV DI, OFFSET IN_NUMBER
CHAR_TO_INT:
	CMP CX, 0 
	JLE END_CONV
	MOV AL, [SI] 
	
	CMP AL, '9' 
	JG	CHAR
	SUB AL, 30H
	JMP short SET
CHAR:
	SUB AL, 37H
SET:

	DEC CX
	CMP CX, 0 
	JNE CONTINUE
	MOV [DI], AL 
	JMP short END_CONV
	
CONTINUE:
	DEC SI
	MOV BL, [SI]
	
	CMP BL, '9' 
	JG	CHAR1
	SUB BL, 30H
	JMP short SET1
CHAR1:
	SUB BL, 37H
SET1:
	SHL BL,1
	SHL BL,1
	SHL BL,1
	SHL BL,1
	
	OR  AL, BL 
	MOV [DI], AL 
	
	DEC CX
	DEC SI 
	INC DI 
	JMP short CHAR_TO_INT
END_CONV:

	LEA SI, OUT_STR
	MOV BX, OFFSET IN_NUMBER
	CALL PRINT32

RET 
GETNUM ENDP
; -----------------------------------------

DIVNUMS		PROC FAR 
	MOV EAX, DATA1 
	IDIV DATA2  
	
	MOV DATA4, EAX 
	
	LEA SI, OUT_STR
	LEA BX, DATA4
	CALL PRINT32	
	RET

DIVNUMS ENDP 
; -----------------------------------------
MULTNUMS	PROC FAR 
	MOV eax, DATA1
    IMUL DATA2 
	
	MOV DATA1, eax ; LSB
	MOV DATA2, edx ; MSB
	
	MOV SI, OFFSET DATA3+4
	MOV [SI], EAX 
	
	SUB SI,4 
	MOV [SI], EDX 
	
	
	LEA SI, OUT_STR
	LEA BX, DATA3
	CALL PRINT32
	
	LEA SI, OUT_STR
	LEA BX, DATA3+4
	CALL PRINT32
	
RET 
MULTNUMS ENDP 

; -----------------------------------------


; -----------------------------------------


; ----------------------------------------
MAIN 	PROC FAR 
	
	;Init Data Segment : 
    MOV AX, @DATA 
    MOV DS, AX 
	
	
	;;;;;;;;;;;;;;;;;; In order to test functions uncomment them : ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;Get Int 
	;CALL GETNUM
	
	
	; Multiply and show result :
	;CALL MULTNUMS

	;Divide and show result 
	;CALL DIVNUMS
	;;;;;;;;;;;;;;;;; Printint32 is used in DIVNUMS and MULTNUMS  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	

	; Finish Program :
	MOV AH,4CH
    INT 21H  
	
MAIN ENDP 


END MAIN 








