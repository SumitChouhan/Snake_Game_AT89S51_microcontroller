
; File:       WelcomeGfx.inc
; Created by: Sumit Chouhan

$TT ('WelcomeGfx')

; This file defines that functions that plays an animation on the screen, during the
; first launch of the program.
;
; The functions in this file will be stored in page one of ROM








; Function:  _WelcomeGraphics
; Arguments: None
; Returns:   None

; Description:

; This function is the entry point to the starting animation. It displays the name of the
; processor (8051) for about 2secs and then scrolls the name of the game on the screen once.

_WelcomeGraphics:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH REG_0 ; R0 is used as a local variable by most of the functions
                                PUSH REG_1 ; R1 is used as a local variable by most of the functions
                                PUSH PSW

                                ACALL _ClearScreen
                                ACALL _PrintProcessorInfo ; Display the name of the processor on the screen
                                ACALL _TurnOnDisplay
                                MOV A, #WEL_GFX_PROC_INFO_DISP_DEL
                                ACALL _DelayABy100ms
                                ACALL _TurnOffDisplay
                                ACALL _DelayABy100ms   ; No clear sceen is required here because the first frame
                                ACALL _TurnOnDisplay   ; of the next anim. is a blank screen! 

                                MOV R0, #X_MIN
MainLoop_WelcomeGraphics:       ; Scroll the name of the game on the screen, once
                                ACALL _PrintPartOfGameInfoFromIndexR0
                                MOV A, #WEL_GFX_TEXT_PERSIST_DELAY
                                ACALL _DelayABy10ms
                                INC R0
                                CJNE R0, #WEL_GFX_MAIN_LOOP_INDEX_OVF_COUNT, MainLoop_WelcomeGraphics

                                ACALL _ClearScreen
                                ACALL _TurnOffDisplay

                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_1
                                POP REG_0
                                POP ACCL
                                RET











; Function:  _PrintProcessorInfo
; Arguments: None
; Returns:   None

; Description:

; This function prints the name of the processor on the screen. It calls
; it's associate AGetProcessorInfoDataAtA to obtain the pixel pattern at
; a column on the display

_PrintProcessorInfo:
                                MOV R0, #X_MIN
                                MOV R1, #DISP_BUF_START_ADDR
Loop_PrintProcessorInfo:
                                MOV A, R0
                                ACALL AGetProcessorInfoDataAtA
                                MOV @R1, A
                                INC R0
                                INC R1
                                CJNE R1, #DISP_BUF_OVR_FLW_ADDR, Loop_PrintProcessorInfo
                                RET













; Function:  AGetProcessorInfoDataAtA
; Arguments: A
; Returns:   A

; Description:

; This function reads a bye from a look uptable in ROM. It requires
; the relative index of the byte to be passed in A and it returns
; the corresponding byte in A

AGetProcessorInfoDataAtA:
                                INC A
                                MOVC A, @A + PC
                                RET
DB                              06CH ; 8
DB                              092H
DB                              092H
DB                              06CH
DB                              07CH ; 0
DB                              082H
DB                              082H
DB                              07CH
DB                              0E2H ; 5
DB                              0A2H
DB                              0A2H
DB                              09CH 
DB                              022H ; 1
DB                              042H
DB                              0FEH
DB                              002H












; Function:  _PrintPartOfGameInfoFromIndexR0
; Arguments: R0
; Returns:   None

; Description:

; This function prints a part of the name of the game on the screen. It requires the
; relative starting address (index) of the name as an argument in R0

_PrintPartOfGameInfoFromIndexR0:
                                PUSH REG_0

                                MOV R1, #DISP_BUF_START_ADDR
Loop_PrintPartOfGameInfoFromIndexR0:
                                MOV A, R0
                                ACALL AGetGameInfoDataAtA ; Associate function that actually reads the data from ROM
                                MOV @R1, A
                                INC R0
                                INC R1
                                CJNE R1, #DISP_BUF_OVR_FLW_ADDR, Loop_PrintPartOfGameInfoFromIndexR0
                                
                                POP REG_0
                                RET                                












; Function:  AGetGameInfoDataAtA
; Arguments: A
; Returns:   A

; Description:

; This function reads a bye from a look uptable in ROM. It requires
; the relative index of the byte to be passed in A and it returns
; the corresponding byte in A

AGetGameInfoDataAtA:
                                INC A
                                MOVC A, @A + PC
                                RET
DB                              000H ; 16 White Spaces
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H

DB                              060H ; S
DB                              092H
DB                              092H
DB                              092H
DB                              00CH

DB                              000H ; White Space

DB                              0FEH ; N
DB                              040H
DB                              038H
DB                              004H
DB                              0FEH

DB                              000H ; White Space

DB                              07EH ; A
DB                              090H
DB                              090H
DB                              090H
DB                              07EH

DB                              000H ; White Space

DB                              0FEH ; K
DB                              010H
DB                              028H
DB                              044H
DB                              082H

DB                              000H ; White Space

DB                              07CH
DB                              092H
DB                              092H
DB                              092H
DB                              092H

DB                              000H ; White Space

DB                              060H ; S
DB                              092H
DB                              092H
DB                              092H
DB                              00CH

DB                              000H ; 16 White Spaces
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H
DB                              000H