
; File:       StandByState.inc
; Created by: Sumit Chouhan

$TT ('StandByState')

; Defines the functions that are called when the application enters stand by state.
; This is the first mode that is entered when the application starts (soon after
; the welcome graphics is displayed) and is also reentered everytime when the game
; ends. This mode implements the auto. power down feature.

; The functions in this file will be stored in page one of ROM










; Function:  _PrepareForStandBy
; Arguments: None
; Returns:   None

; Description:

; This function performs the initialisations in stand by mode

_PrepareForStandBy:
                                MOV PREV_KEY_STATES, #ALL_KEYS_PRESSED

                                ACALL _ClearScreen
                                ACALL _PaintStandByBackground
                                RET








; Function:  _StandBy
; Arguments: None
; Returns:   None

; Description:

; The entry point to the stand by mode.

_StandBy:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH REG_0 ; R0 is used as a local variable by most of the functions
                                PUSH REG_1 ; R1 is used as a local variable by most of the functions
                                PUSH PREV_KEY_STATES
                                PUSH CURR_KEY_STATES
                                PUSH PSW

                                ACALL _PrepareForStandBy
                                ACALL _ResetSystemIdleTime
Loop_StandBy:
                                MOV CURR_KEY_STATES, KEYPAD_ACCESS_PORT
TestForAnyKeyPress_StandBy:
                                MOV A, #ANY_KEY_MASK
                                ACALL ADetectedDepressedStateOfKeyMaskInA
                                JNZ BeginKeyScan_StandBy
                                ACALL _StepSystemIdleTimeAndPowerDownOnExpiry ; Enter power down automatically if
                                                                              ; the idle time period expires
                                SJMP EndOfKeyScan_StandBy
BeginKeyScan_StandBy:
                                ACALL _ResetSystemIdleTime
ScanStartKey_StandBy:
                                MOV A, #START_KEY_MASK 
                                ACALL ADetectedDepressingEdgeOfKeyMaskInA
                                JZ ScanDelaySettingsKey_StandBy
                                SJMP Exit_StandBy
ScanDelaySettingsKey_StandBy:   ; The user can switch to the delay settings mode from this mode
                                MOV A, #DEL_SET_KEY_MASK
                                ACALL ADetectedDepressingEdgeOfKeyMaskInA
                                JZ ScanYesSoftKey_StandBy
                                ACALL _DelaySettings
                                ACALL _PrepareForStandBy
                                SJMP EndOfKeyScan_StandBy
ScanYesSoftKey_StandBy:
                                MOV A, #YES_SOFT_KEY_MASK
                                ACALL ADetectedDepressingEdgeOfKeyMaskInA
                                JZ EndOfKeyScan_StandBy
                                SJMP Exit_StandBy                   
EndOfKeyScan_StandBy:
                                MOV PREV_KEY_STATES, CURR_KEY_STATES

                                MOV A, #DELAY_BETWEEN_KEY_SCANS
                                ACALL _DelayABy100ms
                                SJMP Loop_StandBy
Exit_StandBy:
                                ACALL _ClearScreen
                                
                                ; Restore the previous context from the stack
                                POP PSW
                                POP CURR_KEY_STATES
                                POP PREV_KEY_STATES
                                POP REG_1
                                POP REG_0
                                POP ACCL
                                RET







; Function:  _PaintStandByBackground
; Arguments: None
; Returns:   None

; Description:

; This function paints the Stand By mode screen background

_PaintStandByBackground:
                                MOV R0, #X_MIN
                                MOV R1, #DISP_BUF_START_ADDR
Loop_PaintStandByBackground:
                                MOV A, R0
                                ACALL ALoadStandByBackgroundDataAtA ; Associate function that actually reads the data from ROM
                                MOV @R1, A
                                INC R0
                                INC R1
                                CJNE R1, #DISP_BUF_OVR_FLW_ADDR, Loop_PaintStandByBackground
                                RET









; Function:  ALoadStandByBackgroundDataAtA
; Arguments: A
; Returns:   A

; Description:

; This function reads a bye from a look uptable in ROM. It requires
; the relative index of the byte to be passed in A and it returns
; the corresponding byte in A

ALoadStandByBackgroundDataAtA:
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
DB                              0C0H
DB                              038H
DB                              0C0H