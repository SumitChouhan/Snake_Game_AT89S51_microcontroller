
; File:       DisplayMuxing.inc
; Created by: Sumit Chouhan

$TT ('DisplayMuxing')

; The functions in this file handle display multiplexing. Display multiplexing
; is the only task that runs in the background.

; The functions in this file will be stored in page one of ROM










; ISR: DisplayMuxISR

; Description:

; This is the only interrupt that occurs during normal program execution.
; This interrupt occurs periodically and multiplexes the next display column.
; This ISR always completes execution in a deterministic time. At the end of
; its execution, it set a flag which is utilised by system delay functions.
; Due to it's deterministic nature, internal system timing can be done
; in a predictive manner.
; CPU Cycles required: 55
; Time interval b/w two successive interrupts: ~137us

DisplayMuxISR:                  
                                CLR TR0 ; Stop timer 0
                                ; Save the current context in the stack
                                PUSH ACCL
                                PUSH REG_0
                                PUSH PSW

                                DJNZ DISP_COL_DATA_HOLD_COUNT_REG, BalanceISRPeriod ; Decrement a software prescaler
                                                                                    ; and jump to the block that
                                                                                    ; balances the ISR period if
                                                                                    ; the prescaler count is not zero

                                MOV DISP_COL_DATA_HOLD_COUNT_REG, #DISP_COL_DATA_HOLD_COUNT ; Reset the prescaler
                                MOV DISP_COLUMN_DATA_REG, #DISP_PORT_OFF ; Initially, blank display
                                                                         ; to prevent a ghost
SelectNextDispColumn:
                                ; Then select the next column to be illuminated. A 16bit shadow register is
                                ; used for the two display column driver ports. This 16bit register is
                                ; rotated left to select the next display column!
                                MOV A, DISP_COL_SEL_SHADOW_REG_LOW
                                CLR C
                                RLC A
                                MOV DISP_COL_SEL_SHADOW_REG_LOW, A

                                MOV A, DISP_COL_SEL_SHADOW_REG_HIGH
                                RLC A
                                MOV DISP_COL_SEL_SHADOW_REG_HIGH, A
                                CPL A
                                MOV DISP_COLUMN_SELECT_REG_HIGH, A ; Since the physical registers(ports) work on
                                                                   ; active low logic!

                                MOV A, DISP_COL_SEL_SHADOW_REG_LOW
                                ADDC A, #000H
                                MOV DISP_COL_SEL_SHADOW_REG_LOW, A
                                CPL A
                                MOV DISP_COLUMN_SELECT_REG_LOW, A ; Since the physical registers(ports) work on
                                                                  ; active low logic!
                                ; Here we have assumed that DISP_COL_SEL_SHADOW_REG_LOW was initialized to 001H somewhere
                                ; prior to the first call of this ISR
IncrDispDataPtr:
                                INC DISP_COLUMN_DATA_PTR ; Incr display column data pointer
                                MOV A, DISP_COLUMN_DATA_PTR
                                CJNE A, #DISP_BUF_OVR_FLW_ADDR, WriteNewDataToDispColumn 
                                MOV DISP_COLUMN_DATA_PTR, #DISP_BUF_START_ADDR ; Reset the pointer if it's address
                                                                               ; contents overflows
                                ; Here we have assumed that NEXT_DISP_COLUMN_TO_MUX_ADDR was initialized somewhere else
                                ; prior to the first call of this ISR
WriteNewDataToDispColumn:
                                MOV R0, DISP_COLUMN_DATA_PTR
                                MOV A, @R0
                                CPL A
                                MOV DISP_COLUMN_DATA_REG, A ; Since the physical registers work on active low logic!

                                SJMP FlagTimerOverFlow
BalanceISRPeriod:               ; Display muxing is performed only if the prescaler rolls over. In all other
                                ; cases the following code block is executed that compensates for the time
                                ; gained by skipping the display muxing code block. This guarantees that
                                ; the ISR will always complete execution in a deterministic time. 
                                NOP
                                NOP ; MOV DISP_COL_DATA_HOLD_COUNT_REG, #DISP_COL_DATA_HOLD_COUNT
                                NOP
                                NOP ; MOV DISP_COLUMN_DATA_REG, #DISP_PORT_OFF
                                NOP ; MOV A, DISP_COL_SEL_SHADOW_REG_LOW
                                NOP ; CLR C
                                NOP ; RLC A
                                NOP ; MOV DISP_COL_SEL_SHADOW_REG_LOW, A
                                NOP ; MOV A, DISP_COL_SEL_SHADOW_REG_HIGH
                                NOP ; RLC A
                                NOP ; MOV DISP_COL_SEL_SHADOW_REG_HIGH, A
                                NOP ; CPL A
                                NOP ; MOV DISP_COLUMN_SELECT_REG_HIGH, A
                                NOP ; MOV A, DISP_COL_SEL_SHADOW_REG_LOW
                                NOP ; ADDC A, #000H
                                NOP ; MOV DISP_COL_SEL_SHADOW_REG_LOW, A
                                NOP ; CPL A
                                NOP ; MOV DISP_COLUMN_SELECT_REG_LOW, A
                                NOP ; DISP_COLUMN_DATA_PTR
                                NOP ; MOV A, DISP_COLUMN_DATA_PTR
                                NOP
                                NOP ; CJNE A, #DISP_BUF_OVR_FLW_ADDR, WriteNewDataToDispColumn 
                                NOP
                                NOP ; MOV DISP_COLUMN_DATA_PTR, #DISP_BUF_START_ADDR
                                NOP
                                NOP ; MOV R0, DISP_COLUMN_DATA_PTR
                                NOP
                                NOP ; MOV A, @R0
                                NOP ; CPL A
                                NOP ; MOV DISP_COLUMN_DATA_REG, A
                                NOP ; SJMP FlagTimerOverFlow
FlagTimerOverflow:      
                                MOV TIMER_OVERFLOW_FLAG_REG, #TRUE ; Timer overflow listener is expected to reset this flag
                                MOV DISP_MUX_TIMER_LOW_REG, #DISP_MUX_TIMER_RST_VAL  ; Reset Timer0 register 

                                ; Restore the previous context
                                POP PSW
                                POP REG_0
                                POP ACCL
                                SETB TR0
                                RETI









; Function:  _InitDisplayMuxResources
; Arguments: None
; Returns:   None

; Description:

; This function initialises the SFRs, varriables and ports associated with display muxing

_InitDisplayMuxResources:
                                ORL TMOD, #DISP_MUX_TIMER_TMOD_VAL ; Select Timer0 mode
                                MOV DISP_MUX_TIMER_HIGH_REG, #DISP_MUX_TIMER_RST_VAL
                                MOV DISP_MUX_TIMER_LOW_REG, #DISP_MUX_TIMER_RST_VAL  ; Reset Timer0 registers
                                MOV DISP_COL_DATA_HOLD_COUNT_REG, #DISP_COL_DATA_HOLD_COUNT
                                SETB ET0 ; Enable Timer0 interrupt 
                                SETB PT0 ; Give max. priority to Timer0

                                ; Initialise the shadow registers of the display col. driver ports.
                                MOV DISP_COL_SEL_SHADOW_REG_LOW, #DISP_COL_SEL_SHAD_REG_LOW_INIT_VAL
                                MOV DISP_COL_SEL_SHADOW_REG_HIGH, #DISP_COL_SEL_SHAD_REG_HIGH_INIT_VAL
                                MOV DISP_COLUMN_DATA_PTR, #DISP_BUF_START_ADDR ; Init. the display col. data pointer

                                MOV DISP_COLUMN_SELECT_REG_LOW, #DISP_PORT_OFF ; We choose NOT to mirror the shadow register
                                MOV DISP_COLUMN_SELECT_REG_HIGH, #DISP_PORT_OFF    ; initialization on the corresponding
                                MOV DISP_COLUMN_DATA_REG, #DISP_PORT_OFF      ; physical registers!!!
                                ; Since display muxing first incr. the column, then the column data pointer
                                ; and then writes the new data, muxing actually starts from column 2
                                LCALL _TurnOffDisplay
                                RET







; Function:  _StartDisplayMuxing
; Arguments: None
; Returns:   None

; Description:

; Start display muxing timer

_StartDisplayMuxing:
                                SETB TR0 ; Start Timer0
                                RET











; Function:  _StopDisplayMuxing
; Arguments: None
; Returns:   None

; Description:

; Stop display muxing timer

_StopDisplayMuxing:
                                CLR TR0 ; Stop Timer0
                                RET