
; File:       LowLevelGfx.inc
; Created by: Sumit Chouhan

$TT ('LowLevelGfx')

; This file contains functions that give pixel level control over the display

; The functions in this file will be stored in page one of ROM







; Function:  DXY_Unparse_D_X_Y
; Arguments: D, X, Y
; Returns:   DXY

; Description:

; This function combines D, X and Y data into a single DXY byte.
; Takes input from D, X, Y global variables and writes output to
; DXY global variable
; Format of D:   0000000D
; Format of X:   0000XXXX
; Format of Y:   00000YYY
; Format of DXY: DXXXXYYY

DXY_Unparse_D_X_Y:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH PSW

                                MOV A, X
                                RL A
                                RL A
                                RL A
                                ORL A, D
                                ORL A, Y
                                MOV DXY, A

                                ; Restore the previous context from the stack
                                POP PSW
                                POP ACCL
                                RET









; Function: D_X_Y_Parse_DXY
; Arguments: DXY
; Returns:   D, X, Y

; Description:

; This function decomposes a DXY byte into D, X and Y data.
; Takes input from DXY global variable and writes the result
; into D, X, Y global variables
; Format of DXY: DXXXXYYY
; Format of D:   0000000D
; Format of X:   0000XXXX
; Format of Y:   00000YYY

D_X_Y_Parse_DXY:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH PSW

                                MOV D, DXY
                                ANL D, #DXY_AXIS_DIR_MASK

                                MOV A, DXY
                                ANL A, #DXY_X_MASK ; We AND through the ACCL since we don't want to alter DXY
                                RR A               ; Since X occupies B6 - B3
                                RR A
                                RR A
                                MOV X, A

                                MOV Y, DXY
                                ANL Y, #DXY_Y_MASK ; Since Y occupies B2 - B0 
                                               
                                ; Restore the previous context from the stack
                                POP PSW
                                POP ACCL
                                RET









; Function:  _SetPixel_X_Y
; Arguments: X, Y
; Returns:   None

; Description:

; Sets the pixel specified by (X, Y) on the screen. The function actually
; sets the corresponding bit in display buffer.
; (X, Y) is read from global variables X and Y.

_SetPixel_X_Y:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH REG_0
                                PUSH PSW

                                MOV A, #DISP_BUF_START_ADDR ; Copy the base address of display buffer to A
                                ADD A, X ; Translate to the column containing (X, Y)
                                MOV R0, A ; Back Up the address to R0
                                ACALL ADecode_Y ; Get the decoded value of Y in A
                                ORL A, @R0 ; Set the corresponding bit in the reg. and save result in A
                                MOV @R0, A ; Mirror the result into the register in the display buffer

                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_0
                                POP ACCL
                                RET










; Function:  _ClearPixel_X_Y
; Arguments: X, Y
; Returns:   None

; Description:

; Clears the pixel specified by (X, Y) on the screen. The function actually
; clears the corresponding bit in display buffer.
; (X, Y) is read from global variables X and Y.

_ClearPixel_X_Y:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH REG_0
                                PUSH PSW

                                MOV A, #DISP_BUF_START_ADDR  ; Copy the base address of display buffer to A
                                ADD A, X ; Translate to the column containing (X, Y)
                                MOV R0, A ; Back Up the address to R0
                                ACALL ADecode_Y ; Get the decoded value of Y in A
                                CPL A
                                ANL A, @R0 ; Force the corresponding bit in the reg to. 0 and save result in A

                                MOV @R0, A ; Mirror the result into the register in the display buffer

                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_0
                                POP ACCL
                                RET







; Function:  CGetPixel_X_Y
; Arguments: X, Y
; Returns:   C

; Description:

; This function returns that state of the pixel specified by (X, Y) in the
; carry flag.
; (X, Y) is read from global variables X and Y.
 
CGetPixel_X_Y:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH REG_0
                                PUSH PSW

                                MOV A, #DISP_BUF_START_ADDR  ; Copy the base address of display buffer to A
                                ADD A, X ; Translate to the column containing (X, Y)
                                MOV R0, A ; Back Up the address to R0
                                ACALL ADecode_Y ; Get the decoded value of Y in A
                                ANL A, @R0 ; Force all other bits to zero
                                JZ ClearCarryAndReturn_CGetPixel_X_Y ; If the specified bit is also zero,
                                                                     ; Jump to the block that clears
                                                                     ; carry flag

                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_0
                                POP ACCL
                                SETB C ; Return the result in carry
                                RET
ClearCarryAndReturn_CGetPixel_X_Y:
                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_0
                                POP ACCL
                                CLR C  ; Return the result in carry
                                RET








; Function:
; Arguments: Y
; Returns:   A

; Description:

; This function decodes the contents of global variable Y and returns the result
; in accumulator

ADecode_Y:
                                MOV A, Y ; NOTE: THE FUNC. FAILS IF Y > 7!!!
                                INC A
                                MOVC A, @A + PC
                                RET
DB                              00000001B
DB                              00000010B
DB                              00000100B
DB                              00001000B
DB                              00010000B
DB                              00100000B
DB                              01000000B
DB                              10000000B