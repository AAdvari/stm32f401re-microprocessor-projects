.MODEL SMALL 
.STACK 64 

.DATA 

    STRING_START  DB 'tryyourbesttsebruoyyrt$' 
    STRING_END DB 0  ;Points to the end of above string. 
        
    YES DB 'YES', 13, 10, '$'
    NO DB 'NO', 13, 10,   '$'        
    
.CODE 
MAIN PROC FAR 
    ;Init Data Segment : 
    MOV AX, @DATA 
    MOV DS, AX 
    

    MOV DI, OFFSET STRING_START
    MOV SI, OFFSET STRING_END-2
    MOV AX, 0 
    MOV BX, 0

START_PALINDROME: 

    CMP DI, SI 
    JA PRINT_YES
    
    MOV AL, [SI]
    MOV BL, [DI]
    CMP AL, BL 
    JNE PRINT_NO
    INC DI
    DEC SI 
    JMP short START_PALINDROME

PRINT_YES:
    ; Load address of string : 
    LEA DX, YES

    ; Print String loaded in DX:
    MOV AH, 09H
    INT 21H 
    JMP short END_PALINDROME

PRINT_NO: 
    ; Load address of string : 
    LEA DX, NO

    ; Print String loaded in DX:
    MOV AH, 09H
    INT 21H 

END_PALINDROME: 

    MOV BX, OFFSET STRING_START
    MOV AX, '$' 
    MOV CL, 'y' 
    MOV DL, 0 

START_COUNT: 
    CMP [BX], AX
    JE  END_COUNT
    
    CMP CL, [BX]
    JE INCREMENT
    INC BX 
    JMP short START_COUNT
INCREMENT: 
    INC DL
    INC BX 
    JMP short START_COUNT

END_COUNT:
    
    MOV AH, 2
    INT 21H 
    JMP short FINISH 
   
FINISH: 
    ;get back to DOS
    MOV AH, 4CH 
    INT 21H 

MAIN ENDP
END MAIN 



