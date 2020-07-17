
; File:       SystemPower.inc
; Created by: Sumit Chouhan

$TT ('SystemPower')

; This functions in this file navigates the program flow after a system reset.
; Also functions that initiate a system power down are also defined here.

; The functions in this file will be stored in page one of ROM











; ISR: ResetRecoveryISR

; Description:

;The entry point of the application. At first, the type of reset (warm/cold) is
; determined. On cold reset, the program executes from the beginning. On warm reset,
; the RAM is scanned for a particular bit pattern. This bit pattern will be written to
; the RAM just before a power down. If the bit pattern could not be identified, the program
; execution starts from the beginning as before. However, if the bit pattern could be identified,
; the execution resumes from the point where the system entered power down mode.
  
ResetRecoveryISR:
                                MOV A, PCON              ; Scan PCON for a set state of
                                ANL A, #POF_BIT_MASK     ; POF flag
                                JNZ RecoverFromColdReset ; If POF is set, jump to cold reset recovery block.

                                ; Scan the RAM for the bit pattern
                                CJNE R0, #RAM_PATTERN_1, RecoverFromWarmReset ; On failure,
                                CJNE R1, #RAM_PATTERN_2, RecoverFromWarmReset ; the case is as good as
                                CJNE R2, #RAM_PATTERN_3, RecoverFromWarmReset ; cold reset. So, start execution
                                CJNE R3, #RAM_PATTERN_4, RecoverFromWarmReset ; from the beginning.

                                AJMP RecoverFromAutoPowerDown ; On successful verfication of bit pattern,
                                                              ; jump to the point where the system entered
                                                              ; power down mode.
RecoverFromColdReset:
RecoverFromWarmReset:
                                MOV A, #POF_BIT_MASK ; Clear the power of flag in PCON
                                CPL A                ; This is required to detect a warm reset, in case
                                ANL A, PCON          ; it occurs.
                                MOV PCON, A
                                MOV SP, #STACK_UND_FLOW_ADD ; Init. stack pointer
                                ACALL _InitDisplayMuxResources
                                ACALL _InitKeyPad
                                LCALL _InitGameResources
                                ACALL _TurnOffDisplay
                                ACALL _TurnOffBuzzer
                                SETB EA ; Enable global interrupts
                                ACALL _StartDisplayMuxing ; Start the timer that raises disp. mux. interrupt
                                ACALL _WelcomeGraphics ; Play welcome anim...
                                ACALL _TurnOnDisplay
                                LJMP C51Main ; Jump to Main Block














; ISR: ExternalInterrupt0ISR

; Description:

; This interrupt is enabled only when the system enteres a power down
; mode and is disabled as soon as the system recovers from power down
; state.

ExternalInterrupt0ISR:
                                JNB EXT_INT_0_PIN, $ ; Poll until the user releases the ext. interrupt key,
                                                     ; since the interrupt is level triggered
                                CLR EX0 
                                RETI










; ISR: ExternalInterrupt1ISR

; Description:

; This interrupt is enabled only when the system enteres a power down
; mode and is disabled as soon as the system recovers from power down
; state.

ExternalInterrupt1ISR:
                                JNB EXT_INT_1_PIN, $ ; Poll until the user releases the ext. interrupt key,
                                                     ; since the interrupt is level triggered
                                CLR EX1
                                RETI











; Function:  _ResetSystemIdleTime
; Arguments: None
; Returns:   None

; Description:
 
; This function resets the system idle time counter. Any code block that utilises
; system power down feature is expected to call this function whenever a user
; event (key press) occurs.

_ResetSystemIdleTime:
                                MOV SYS_IDLE_TIME_COUNT, #MAX_SYS_IDLE_TIME_COUNT
                                RET










; Function:  AStepSystemIdleTimeAndNotifyOnExpiry
; Arguments: None
; Returns:   A

; Description:

; This function decrements the system idle time counter and returns true in Accl.
; when the count becomes zero.

AStepSystemIdleTimeAndNotifyOnExpiry:
                                DJNZ SYS_IDLE_TIME_COUNT, ReturnFalse_AStepSystemIdleTimeAndNotifyOnExpiry

                                MOV A, #TRUE
                                RET
ReturnFalse_AStepSystemIdleTimeAndNotifyOnExpiry:
                                MOV A, #FALSE
                                RET











; Function:  _StepSystemIdleTimeAndPowerDownOnExpiry
; Arguments: None
; Returns:   None

; Description:

; This function decrements the system idle time as previous function does. However, when
; the count drops to zero, it directly initiates a power down without notifying the caller. 

_StepSystemIdleTimeAndPowerDownOnExpiry:
                                DJNZ SYS_IDLE_TIME_COUNT, Return_StepSystemIdleTimeAndPowerDownOnExpiry

                                ACALL _PowerDownSystem
Return_StepSystemIdleTimeAndPowerDownOnExpiry:
                                RET












; Function:  _PowerDownSystem
; Arguments: None
; Returns:   None

; Description:

; This function initiates a system power down operation.

_PowerDownSystem:
                                ACALL _TurnOffDisplay
                                ACALL _StopDisplayMuxing

                                ; Save the SFRs and registers that are uses in the stack.
                                PUSH ACCL
                                PUSH B
                                PUSH REG_0
                                PUSH REG_1
                                PUSH REG_2
                                PUSH REG_3
                                PUSH REG_4
                                PUSH PSW
                                PUSH TCON
                                PUSH TMOD
                                PUSH TL0
                                PUSH TH0
                                PUSH TL1
                                PUSH TH1
                                PUSH IE
                                PUSH IP

                                ; Disable all port pins (set them to logic one)
                                MOV DISP_COLUMN_SELECT_REG_LOW, #DISP_PORT_OFF
                                MOV DISP_COLUMN_SELECT_REG_HIGH, #DISP_PORT_OFF
                                MOV DISP_COLUMN_DATA_REG, #DISP_PORT_OFF
                                ORL KEYPAD_ACCESS_PORT, #ALL_KEY_PINS_HIGH

                                ; Write the pattern that distinguishes a warm reset from
                                ; power down and a user initiated warm reset to the RAM
                                MOV R0, #RAM_PATTERN_1
                                MOV R1, #RAM_PATTERN_2
                                MOV R2, #RAM_PATTERN_3
                                MOV R3, #RAM_PATTERN_4

                                MOV R4, SP ; Back up SP; Since SP is also an SFR & SFRs are
                                           ; usually reset after a warm system reset.

                                ; Enable external interrupts (level triggered mode is enabled)
                                SETB EX0
                                SETB EX1
                                
                                ORL PCON, #PD_BIT_MASK ; Enter power down

; *************************************************** ;
;
; The starting point of the application after a warm reset from power down state

RecoverFromAutoPowerDown:
                                ; Clear external interrupts
                                CLR EX1
                                CLR EX0

                                ; Restore SP
                                MOV SP, R4

                                ; Restore SFRs and registeres used by this function
                                POP IP
                                POP IE
                                POP TH1
                                POP TL1
                                POP TH0
                                POP TL0
                                POP TMOD
                                POP TCON
                                POP PSW
                                POP REG_4
                                POP REG_3
                                POP REG_2
                                POP REG_1
                                POP REG_0
                                POP B
                                POP ACCL

                                ACALL _StartDisplayMuxing
                                ACALL _TurnOnDisplay
                                RET