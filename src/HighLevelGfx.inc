
; File:       HighLevelGfx.inc
; Created by: Sumit Chouhan

$TT ('HighLevelGfx')

; This file defines functions that alter that state of multiple pixels on
; the screen. They use complex mathematical computations for their execution.

; The functions in this file will be stored in page one of ROM







; Function:  _ClearScreen
; Arguments: None
; Returns:   None

; Description:

; This function clears the display. If dumps zeros to the display buffer.

_ClearScreen:
                                ; Save the current context on the stack
                                PUSH REG_0
                                PUSH PSW

                                MOV R0, #DISP_BUF_START_ADDR
Loop_ClearScreen:
                                MOV @R0, #BLANK_DISP_BUF
                                INC R0
                                CJNE R0, #DISP_BUF_OVR_FLW_ADDR, Loop_ClearScreen

                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_0
                                RET











; Function:  _DrawLine
; Arguments: D, X0, Y0, X, Y
; Returns:   None

; Description:

; Draws a line from (X0, Y0) to (X1, Y1) in the direction specified
; by D:
;    D = 0, => +ve X/Y direction
;    D = 1, => -ve X/Y direction
;
; By "drawing the line", I mean settings the pixels along the line.
;
; While drawing the line, if the function encounters any edge of the display,
; it circles around to the opposite edge and resumes drawing the line until it
; encounters the terminal coordinate of the line!
; 
; NOTE: The function can only draw lines || to X or Y axes.
; So always, X0 == X1 or Y0 == Y1, must be satisfied.

_DrawLine:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH PSW
                                PUSH X
                                PUSH Y
                                PUSH X0
                                PUSH Y0

                                MOV A, Y0
                                CJNE A, Y1, LineParallelToYAxis ; If Y is not a constant, then by the condition
                                                                ; that defines reliable operation of this
                                                                ; function X must be a constant. So, jump to
                                                                ; the block that draws a line parallel to Y axis
LineParallelToXAxis:
                                MOV A, D
                                JNZ AlongNegativeX ; Since D = 1, draw the line in -ve X direction
AlongPositiveX:
                                MOV Y, Y0 ; Y is a constant
PositiveXLoopBegin:
                                MOV X, X0
                                ACALL _SetPixel_X_Y
TestForPositiveXLoopBreak:
                                MOV A, X0
                                CJNE A, X1, IncrPositiveXLoopVariable
                                SJMP EndOfDrawLine
IncrPositiveXLoopVariable:
                                INC X0
                                ANL X0, #CIRCLE_BOUND_X_MASK ; In case X overflows (ie X > 15), X will be set to 0
                                SJMP PositiveXLoopBegin
AlongNegativeX:
                                MOV Y, Y0 ; Y is a constant
NegativeXLoopBegin:
                                MOV X, X0
                                ACALL _SetPixel_X_Y
TestForNegativeXLoopBreak:
                                MOV A, X0
                                CJNE A, X1, DecrNegativeXLoopVariable
                                SJMP EndOfDrawLine
DecrNegativeXLoopVariable:
                                DEC X0
                                ANL X0, #CIRCLE_BOUND_X_MASK ; In case X underflows (ie X < 0), X will be set to 15
                                SJMP NegativeXLoopBegin
LineParallelToYAxis:
                                MOV A, D
                                JNZ AlongNegativeY ; Since D = 1, draw the line in -ve Y direction
AlongPositiveY:
                                MOV X, X0 ; X is a constant
PositiveYLoopBegin:
                                MOV Y, Y0
                                ACALL _SetPixel_X_Y
TestForPositiveYLoopBreak:
                                MOV A, Y0
                                CJNE A, Y1, IncrPositiveYLoopVariable
                                SJMP EndOfDrawLine
IncrPositiveYLoopVariable:
                                INC Y0
                                ANL Y0, #CIRCLE_BOUND_Y_MASK ; In case Y overflows (ie Y > 7), Y will be set to 0
                                SJMP PositiveYLoopBegin
AlongNegativeY:
                                MOV X, X0 ; X is a constant
NegativeYLoopBegin:
                                MOV Y, Y0
                                ACALL _SetPixel_X_Y
TestForNegativeYLoopBreak:
                                MOV A, Y0
                                CJNE A, Y1, DecrNegativeYLoopVariable
                                SJMP EndOfDrawLine
DecrNegativeYLoopVariable:
                                DEC Y0
                                ANL Y0, #CIRCLE_BOUND_Y_MASK ; In case Y underflows (ie Y < 0), Y will be set to 7
                                SJMP NegativeYLoopBegin
EndOfDrawLine:
                                ; Restore the previous context from the stack
                                POP Y0
                                POP X0
                                POP Y
                                POP X
                                POP PSW
                                POP ACCL
                                RET








; Function:  _PrintDigitAAtB
; Arguments: A, B
; Returns:   None

; Description:

; This function prints the digit specified in A at the display column
; specified in B (the col. is specified by it's number). Every digit is
; five columns wide. The pattern of the pixels in the column containing
; a digit are stored in ROM. So, five bytes are necessary to store each
; digit. The digits are stored in a continuous fashion starting from zero to nine.
; A white space pixel pattern is saved at the tenth digit position. The pixel data
; corresponding to a number can be read by specifying it's relative address.

_PrintDigitAAtB:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH B
                                PUSH REG_0
                                PUSH REG_1
                                PUSH REG_2
                                PUSH PSW

                                MOV R0, A
                                MOV A, #DISP_BUF_START_ADDR
                                ADD A, B
                                MOV R1, A
                                MOV A, R0
                                MOV B, #WIDTH_OF_DIGIT
                                MUL AB
                                MOV R0, A
                                MOV R2, #WIDTH_OF_DIGIT
Loop_PrintDigitAAtB:
                                MOV A, R0
                                ACALL AGetDigitDataAtA
                                MOV @R1, A
                                INC R0
                                INC R1
                                DJNZ R2, Loop_PrintDigitAAtB

                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_2
                                POP REG_1
                                POP REG_0
                                POP B
                                POP ACCL
                                RET










; Function:  AGetDigitDataAtA
; Arguments: A
; Returns:   A

; Description:

; This function implements a look up table. When a digit is to be displayed on
; the screen, the pattern of the display pixels in the columns containing
; the digit is written in ROM, which follows this function definition.
; By passing the relative address of a pattern in A, this function will
; return the corresponding pattern in A

AGetDigitDataAtA:
                                INC A
                                MOVC A, @A + PC
                                RET
DB                              07CH ; 0
DB                              0A2H
DB                              092H
DB                              08AH
DB                              07CH
DB                              022H ; 1
DB                              042H
DB                              0FEH
DB                              002H
DB                              002H 
DB                              046H ; 2
DB                              08AH
DB                              092H
DB                              0A2H 
DB                              042H
DB                              054H ; 3
DB                              092H
DB                              092H 
DB                              092H
DB                              06CH 
DB                              018H ; 4
DB                              028H
DB                              048H
DB                              09EH
DB                              008H 
DB                              0E2H ; 5
DB                              0A2H
DB                              0A2H
DB                              0A2H
DB                              09CH 
DB                              07CH ; 6
DB                              092H
DB                              092H
DB                              092H
DB                              04CH 
DB                              082H ; 7
DB                              08CH
DB                              088H
DB                              098H
DB                              0E0H 
DB                              06CH ; 8
DB                              092H
DB                              092H
DB                              092H
DB                              06CH 
DB                              064H ; 9
DB                              092H
DB                              092H
DB                              092H
DB                              07CH
DB                              000H  ; White Space
DB                              000H
DB                              000H
DB                              000H
DB                              000H