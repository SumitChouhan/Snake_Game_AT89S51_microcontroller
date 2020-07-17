
; File:       SystemFunctions.inc
; Created by: Sumit Chouhan

$TT ('SystemFunctions')

; This file defines hardware periphral interaction functions and
; system delay functions

; The functions in this file will be stored in page one of ROM








; Function:  _InitKeyPad
; Arguments: None
; Returns:   None

; Description:

; This function initialises the keypad port and global variables associated
; with key processing

_InitKeyPad:
                                ORL KEYPAD_ACCESS_PORT, #ALL_KEY_PINS_HIGH
                                MOV PREV_KEY_STATES, #ALL_KEY_PINS_HIGH
                                MOV CURR_KEY_STATES, #ALL_KEY_PINS_HIGH
                                RET









; Function:  ADetectedDepressedStateOfKeyMaskInA
; Arguments: A
; Returns:   A

; Description:

; Returns TRUE in A, if the key spec. by it's mask in A
; is depressed.

ADetectedDepressedStateOfKeyMaskInA:
                                ; Save the current context on the stack
                                PUSH REG_0
                                PUSH PSW

                                MOV R0, A
                                MOV A, CURR_KEY_STATES
                                CPL A
                                ANL A, R0

                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_0
                                RET











; Function:  ADetectedDepressingEdgeOfKeyMaskInA
; Arguments: A
; Returns:   A

; Description:

; In contrast to the prev. fn., this routine detects transition
; of the logic at the pin from 1(key released) to 0(key depressed).
; ie it detects the falling edge of the signal at the pin.
; If the previous state of the port pin corresponding to the key is
; high and it's current state is low, the function returns true in
; Accl.

ADetectedDepressingEdgeOfKeyMaskInA:
                                ; Save the current context on the stack
                                PUSH REG_0
                                PUSH REG_1
                                PUSH PSW

                                MOV R1, A
                                MOV A, PREV_KEY_STATES
                                ANL A, R1
                                MOV R0, A
                                MOV A, CURR_KEY_STATES
                                ANL A, R1
                                XRL A, R1
                                ANL A, R0

                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_1
                                POP REG_0
                                RET







; Function:  _WaitForAnyKeyPress
; Arguments: None
; Returns:   None

; Description:

; This function waits for a user event (a key press)
; NOTE: This function utilises the system power down feature.
;       If the max. allowed system idle period expires while
;       no user event is registered, a power down will be initiated 

_WaitForAnyKeyPress:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH PSW
                                PUSH CURR_KEY_STATES
                                PUSH PREV_KEY_STATES

                                MOV CURR_KEY_STATES, #ALL_KEYS_PRESSED
                                ACALL _ResetSystemIdleTime
Loop_WaitForAnyKeyPress:
                                MOV A, #DELAY_BETWEEN_KEY_SCANS
                                ACALL _DelayABy100ms
                                MOV PREV_KEY_STATES, CURR_KEY_STATES
                                MOV CURR_KEY_STATES, KEYPAD_ACCESS_PORT
                                MOV A, #ANY_KEY_MASK
                                ACALL ADetectedDepressingEdgeOfKeyMaskInA
                                JNZ ExitLoop_WaitForAnyKeyPress
                                ACALL _StepSystemIdleTimeAndPowerDownOnExpiry
                                SJMP Loop_WaitForAnyKeyPress
ExitLoop_WaitForAnyKeyPress:
                                ; Restore the previous context from the stack                                
                                POP PREV_KEY_STATES
                                POP CURR_KEY_STATES
                                POP PSW
                                POP ACCL
                                RET







; Function:  _WaitForAllActionKeyRelease
; Arguments: None
; Returns:   None

; Description:

; This function polls until user releases all game action keys

_WaitForAllActionKeyRelease:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH PSW
                                PUSH CURR_KEY_STATES
Loop_WaitForAllActionKeyRelease:
                                MOV CURR_KEY_STATES, KEYPAD_ACCESS_PORT
                                MOV A, #ANY_GAME_ACTION_KEY_MASK
                                ACALL ADetectedDepressedStateOfKeyMaskInA
                                JNZ Loop_WaitForAllActionKeyRelease

                                ; Restore the previous context from the stack
                                POP CURR_KEY_STATES
                                POP PSW
                                POP ACCL
                                RET









; Function:  _TurnOnDisplay
; Arguments: None
; Returns:   None

_TurnOnDisplay:
                                CLR DISP_CTRL_PIN
                                RET









; Function:  _TurnOffDisplay
; Arguments: None
; Returns:   None

_TurnOffDisplay:
                                SETB DISP_CTRL_PIN
                                RET







; Function:  _TurnOnBuzzer
; Arguments: None
; Returns:   None

_TurnOnBuzzer:
                                CLR BUZZ_CTRL_PIN
                                RET










; Function:  _TurnOffBuzzer
; Arguments: None
; Returns:   None

_TurnOffBuzzer:
                                SETB BUZZ_CTRL_PIN
                                RET













; Gives 10.126ms delay!!! @ 27MHz F_XTAL =>  T = (4 / 9) us (period of one instruction)
; This functions uses Timer 0 for it's delay calculation. It clears the TIMER_OVERFLOW_FLAG_REG
; and waits until the flag is set by the Timer 0 overflow ISR. It may be noted that from the starting
; of the app., Timer 0 always runs and generates an interrupt periodically.
; 
; Overhead delay:          2T
; Aux. instruction delay:  8T + 1T + 10T
; Inner loop delay:       (1T + 2T + 306T (Cycles By Interrupt Timer) + 2T) * _10_MS_NUM_INNER_DELAY_LOOPS
; Outer loop delay:       (1T + Inner loop delay +  2T) * _10_MS_NUM_OUTER_DELAY_LOOPS
; Tot delay = Overhead delay + Aux. inst delay +  Outer loop delay
;
;        where T = (4 / 9) us (period of one instruction)

_Delay10ms:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH REG_0
                                PUSH REG_1
                                PUSH PSW

                                MOV R1, #_10_MS_NUM_OUTER_DELAY_LOOPS
OuterLoop_Delay10ms:
                                MOV R0, #_10_MS_NUM_INNER_DELAY_LOOPS
InnerLoop_Delay10ms:
                                MOV A, #TRUE
                                MOV TIMER_OVERFLOW_FLAG_REG, #FALSE
                                CJNE A, TIMER_OVERFLOW_FLAG_REG, $  ; Wait until the flag is set by Timer 0 ISR
                                DJNZ R0, InnerLoop_Delay10ms
                                DJNZ R1, OuterLoop_Delay10ms

                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_1
                                POP REG_0
                                POP ACCL
                                RET











; Gives 100.01ms delay!!! @ 27MHz F_XTAL
;
; NOTE: We have decided to implement this function separately and
; not as a dependent on _Delay10ms in order to minimise stack reqierments
; when this function is in turn nested by other delay routines
;
;; This functions uses Timer 0 for it's delay calculation. It clears the TIMER_OVERFLOW_FLAG_REG
; and waits until the flag is set by the Timer 0 overflow ISR. It may be noted that from the starting
; of the app., Timer 0 always runs and generates an interrupt periodically.
;
; Overhead delay:           2T
; Aux. instruction delay:   8T + 1T + 10T
; Inner loop delay:        (1T + 2T + 310T (Cycles By Interrupt Timer) + 2T) * _100_MS_NUM_INNER_DELAY_LOOPS
; Outer loop delay:        (1T + Inner loop delay +  2T) * _100_MS_NUM_OUTER_DELAY_LOOPS
; Tot delay = Overhead delay + Aux. inst delay +  Outer loop delay
;
;        where T = (4 / 9) us (period of one instruction)

_Delay100ms:                    
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH REG_0
                                PUSH REG_1
                                PUSH PSW

                                MOV R1, #_100_MS_NUM_OUTER_DELAY_LOOPS
OuterLoop_Delay100ms:
                                MOV R0, #_100_MS_NUM_INNER_DELAY_LOOPS
InnerLoop_Delay100ms:
                                MOV A, #TRUE
                                MOV TIMER_OVERFLOW_FLAG_REG, #FALSE
                                CJNE A, TIMER_OVERFLOW_FLAG_REG, $  ; Wait until the flag is set by Timer 0 ISR
                                DJNZ R0, InnerLoop_Delay100ms
                                DJNZ R1, OuterLoop_Delay100ms

                                POP PSW
                                POP REG_1
                                POP REG_0
                                POP ACCL
                                RET










; Will introduce a total delay of ~ A * 10ms + (2 * A + 14) / 2.25 us
; Total Stack Requirements: 10BYTES!!!

_DelayABy10ms:                 
                                ; Save the current context on the stack
                                PUSH REG_0
                                PUSH PSW

                                JZ EndOfABy10msDelay ; To get around the special case when A = 0
                                MOV R0, A
Loop_DelayABy10ms:
                                LCALL _Delay10ms
                                DJNZ R0, Loop_DelayABy10ms
EndOfABy10msDelay:
                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_0
                                RET








; Will introduce a total delay of ~ A * 100ms + (2 * A + 14) / 2.25 us
; Total Stack Requirements: 10BYTES!!!

_DelayABy100ms:                 
                                ; Save the current context on the stack
                                PUSH REG_0
                                PUSH PSW

                                JZ EndOfABy100msDelay ; To get around the special case when A = 0
                                MOV R0, A
Loop_DelayABy100ms:
                                LCALL _Delay100ms
                                DJNZ R0, Loop_DelayABy100ms
EndOfABy100msDelay:
                                ; Restore the previous context from the stack
                                POP PSW
                                POP REG_0
                                RET