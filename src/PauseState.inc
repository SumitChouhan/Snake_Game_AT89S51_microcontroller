
; File:       PauseState.inc
; Created by: Sumit Chouhan

$TT ('PauseState')

; Defines the functions that implement the pause mode. The user can enter the pause
; mode from the game mode. The user can wish to continue or quit the game from this mode.
; The user can also enter delay settings mode from this mode. This mode implements
; the automatic power down feature.

; The functions in this file will be stored in page one of ROM









; Function:  _PrepareForGamePause
; Arguments: None
; Returns:   None

; Description:

; This function performs the initialisations in stand by mode

_PrepareForGamePause:
                                MOV PREV_KEY_STATES, #ALL_KEYS_PRESSED

                                ACALL _ClearScreen
                                ACALL _PaintGamePauseBackground
                                RET








; Function:  _GamePause
; Arguments: None
; Returns:   A

; Description:
; The entry point to the stand by mode.
; This function returns TRUE in Accumulator if the user wishes to continue the game.
; A false value means the user wishes to quit the game.

AGamePause:
                                ; Save the current context on the stack
                                PUSH REG_0 ; R0 is used as a local variable by most of the functions
                                PUSH REG_1 ; R1 is used as a local variable by most of the functions
                                PUSH PREV_KEY_STATES
                                PUSH CURR_KEY_STATES
                                PUSH PSW

                                ACALL _PrepareForGamePause
                                ACALL _ResetSystemIdleTime
Loop_GamePause:
                                MOV CURR_KEY_STATES, KEYPAD_ACCESS_PORT
TestForAnyKeyPress_GamePause:
                                MOV A, #ANY_KEY_MASK
                                ACALL ADetectedDepressedStateOfKeyMaskInA
                                JNZ BeginKeyScan_GamePause
                                ACALL _StepSystemIdleTimeAndPowerDownOnExpiry ; Enter power down automatically if
                                                                              ; the idle time period expires
                                SJMP EndOfKeyScan_GamePause
BeginKeyScan_GamePause:
                                ACALL _ResetSystemIdleTime
ScanPauseKey_GamePause:
                                MOV A, #PAUSE_KEY_MASK 
                                ACALL ADetectedDepressingEdgeOfKeyMaskInA
                                JZ ScanDelaySettingsKey_GamePause

                                ACALL _ClearScreen

                                POP PSW
                                POP CURR_KEY_STATES
                                POP PREV_KEY_STATES
                                POP REG_1
                                POP REG_0
                                MOV A, #TRUE
                                RET
ScanDelaySettingsKey_GamePause:   ; The user can switch to the delay settings mode from this mode
                                MOV A, #DEL_SET_KEY_MASK
                                ACALL ADetectedDepressingEdgeOfKeyMaskInA
                                JZ ScanNoSoftKey_GamePause
                                ACALL _DelaySettings
                                ACALL _PrepareForGamePause
                                SJMP EndOfKeyScan_GamePause
ScanNoSoftKey_GamePause:
                                MOV A, #NO_SOFT_KEY_MASK
                                ACALL ADetectedDepressingEdgeOfKeyMaskInA
                                JZ ScanYesSoftKey_GamePause
                                
                                ACALL _ClearScreen

                                ; Restore the previous context from the stack
                                POP PSW
                                POP CURR_KEY_STATES
                                POP PREV_KEY_STATES
                                POP REG_1
                                POP REG_0
                                MOV A, #FALSE
                                RET
ScanYesSoftKey_GamePause:
                                MOV A, #YES_SOFT_KEY_MASK
                                ACALL ADetectedDepressingEdgeOfKeyMaskInA
                                JZ EndOfKeyScan_GamePause

                                ACALL _ClearScreen

                                POP PSW
                                POP CURR_KEY_STATES
                                POP PREV_KEY_STATES
                                POP REG_1
                                POP REG_0
                                MOV A, #TRUE
                                RET
                   
EndOfKeyScan_GamePause:
                                MOV PREV_KEY_STATES, CURR_KEY_STATES

                                MOV A, #DELAY_BETWEEN_KEY_SCANS
                                ACALL _DelayABy100ms
                                SJMP Loop_GamePause











; Function:  _PaintGamePauseBackground
; Arguments: None
; Returns:   None

; Description:

; This function paints the Pause mode screen background

_PaintGamePauseBackground:
                                MOV R0, #X_MIN
                                MOV R1, #DISP_BUF_START_ADDR
Loop_PaintGamePauseBackground:
                                MOV A, R0
                                ACALL ALoadGamePauseBackgroundDataAtA
                                MOV @R1, A
                                INC R0
                                INC R1
                                CJNE R1, #DISP_BUF_OVR_FLW_ADDR, Loop_PaintGamePauseBackground
                                RET












; Function:  ALoadGamePauseBackgroundDataAtA
; Arguments: A
; Returns:   A

; Description:

; This function reads a bye from a look uptable in ROM. It requires
; the relative index of the byte to be passed in A and it returns
; the corresponding byte in A

ALoadGamePauseBackgroundDataAtA:
                                INC A
                                MOVC A, @A + PC
                                RET
DB                              002H
DB                              0F9H
DB                              0A2H
DB                              0E4H
DB                              000H
DB                              0F8H
DB                              008H
DB                              008H
DB                              000H
DB                              078H
DB                              0A0H
DB                              078H
DB                              000H
DB                              0C5H
DB                              03AH
DB                              0C5H
