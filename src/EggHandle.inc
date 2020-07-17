
; File:       EggHandle.inc
; Created by: Sumit Chouhan

$TT ('EggHandle')

; This file defines that functions that manage eggs!!! Eggs are generated after
; a time EGG_DELAY_SCALE_FACTOR * SNAKE_DELAY. Generation involves selecting a
; random coordinate on the screen not occupied by the snake. The egg is then
; highlighted at this point. We don't use a pseudo random sequence generator,
; instead we set Timer 1 as a free runncing 16bit timer. The value of lower byte
; is sampled and truncated to 7bits to get the egg location in DXY format.
; Now the egg is said to be active. The egg will remain active for another
; EGG_DELAY_SCALE_FACTOR * SNAKE_DELAY period. If the snake snatches the egg
; within this period, then GAME_SCORE is incremented, egg state is set to inactive
; and the process starts all over again. Since the snake is advanced after every
; SNAKE_DELAY period, the snake will have to snatch the egg in utmost
; EGG_DELAY_SCALE_FACTOR steps 


; NOTE: Functions in this file operate at the top most level and they do no care to
; back up the registers they modify!!!
;
; R5 - is used as a local variable at C51Main level as an Egg State Toggle down counter
; R6 - is used as a local variable at C51Main level as a buzzer turn off delay down counter

; This function initialises Timer 1 as a free running 16bit timer. When a random
; location for the egg is needed, the lower byte of the timer is sampled and
; processed as a DXY data.

; The functions in this file will be stored in page two of ROM












; Function:  _InitRandEggLocationTimer
; Arguments: None
; Returns:   None

; Description:

; This function initialises Timer 1 as a free running 8 bit timer. The value of the
; time register is sampled and the value is used to obatain location for the egg
; on the screen.

_InitRandEggLocationTimer:
                                ORL TMOD, #RAND_EGG_LOC_GEN_TIMER_TMOD_VAL ; Select Timer1 mode
                                                                           ; Assignment is not used since we don't want to ruin settings
                                                                           ; of Timer 0 assosiated with the same register
                                MOV RAND_EGG_LOC_GEN_TIMER_REG_HIGH, #RAND_EGG_LOC_GEN_TIMER_REG_RST_VAL ; Reset the higher byte of the timer
                                MOV RAND_EGG_LOC_GEN_TIMER_REG_LOW, #RAND_EGG_LOC_GEN_TIMER_REG_RST_VAL ; Reset the lower byte of the timer
                                RET







; Function:  _ResetEggState
; Arguments: None
; Returns:   None

; Description:

; This function is usually called before a game start. It sets
; the state of the egg to inactive, resets the egg delay and
; also resets the buzzer delay counter.
;   We don't implement a solution to the case when this function is called
; when an egg is already displayed on the screen.
; That is we don't clear the EGG_LOCATION pixel on the screen.

_ResetEggState:
                                MOV EGG_ACTIVITY_STATE, #FALSE ; Set egg state to inactive
                                ACALL _ResetEggDelay ; Reset the egg delay counter
                                MOV R6, #000H ; Reset the buzzer delay counter
                                RET











; Function:  _StartRandEggLocationTimer
; Arguments: None
; Returns:   None

; Description:

; Starts Timer 1. Called during a Game Start or Game Resume after
; returning from Game Pause/Delay Settings Modes. We don't attempt to reset
; timer reg.s here. They will get incremented from their existing contents!!! 

_StartRandEggLocationTimer:
                                SETB TR1 ; Start Timer 1
                                RET













; Function:  _StopRandEggLocationTimer
; Arguments: None
; Returns:   None

; Description:

; Stops Timer 1. Called during a Game Pause, before entering Delay Settings
; or at the end of the Game
 
_StopRandEggLocationTimer:
                                CLR TR1 ; Stop Timer 1
                                RET












; Function:  _ResetEggDelay
; Arguments: None
; Returns:   None

; Description:

; Resets the egg state change counter - R5
; R5 is loaded with a val. that is EGG_DELAY_SCALE_FACTOR times SNAKE_DELAY

_ResetEggDelay:
                                MOV A, SNAKE_DELAY ; Load the multiplicand
                                MOV B, #EGG_DELAY_SCALE_FACTOR ; Load the multiplier
                                MUL AB ; Perform the multiplication
                                MOV R5, A ; Init. R5 with the lower 8 bits, which is only signigficant
                                          ; All efforts have been putin to ensure that the result
                                          ; always fits in 8bit range even event when SNAKE_DELAY is max.
                                RET













; Function:  _ChangeEggStateOnDelayMatch
; Arguments: None
; Returns:   None

; Description:

; This function is called during every game time step. It decrements the counter
; and toggles the activity state of the egg whenever the counter becomes zero
; NOTE: The counter R5 has to be properly init. before the first call of the
; function ie the first call after a game start

_ChaneEggStateOnDelayMatch:
                                DJNZ R5, Return_ChaneEggStateOnDelayMatch ; First decr. the counter and return
                                                                          ; if not zero
                                MOV A, EGG_ACTIVITY_STATE ; Else, determine the crrent egg state
                                JZ SetEggStateToActive ; If inactive, jump to the block that sets egg state to
                                                       ; active
SetEggStateToInactive: ; Block that sets egg state to inactive
                                MOV EGG_ACTIVITY_STATE, #FALSE ; Set the egg state to inactive
                                MOV DXY, EGG_LOCATION ; Move the location of the egg to DXY
                                LCALL D_X_Y_Parse_DXY ; Extract D, X and Y from DXY
                                LCALL _ClearPixel_X_Y ; Cleat the pixel (X, Y)
                                SJMP ResetEggDelayAndReturn ; Jump to the block that resets egg delay counter 
SetEggStateToActive: ; Block that sets egg state to active
                                MOV EGG_ACTIVITY_STATE, #TRUE ; Set the egg state to active
                                ACALL A_DXY_GetNewEggLocation ; Get a random egg coordinate
                                JNZ UpdateNewEggLocation ; If the generation of coordinate succeedes, then
                                                         ; jump to block that processes the coordinate
                                MOV EGG_ACTIVITY_STATE, #FALSE  ; Else, set activity state to inactive,
                                SJMP ResetEggDelayAndReturn ; Jump to the block that resets egg delay counter
UpdateNewEggLocation: ; Block that processes new egg location
                                MOV EGG_LOCATION, DXY ; Save the egg location to EGG_LOCATION global variable
                                ACALL _PaintEggIfActive ; Highlight the egg on the screen
                                                        ; Though this function has the overhead of
                                                        ; checking whether the egg state is active (since the
                                                        ; egg state is already active) we bear the overhead
                                                        ; to avoid another function definition
ResetEggDelayAndReturn:
                                ACALL _ResetEggDelay ; Finally reset the egg delay after toggling the egg state
Return_ChaneEggStateOnDelayMatch:
                                RET












; Function:  A_DXY_GetNewEggLocation
; Arguments: None
; Returns:   A, DXY

; Description:

; This function generates a random location for the egg in DXY format
; It returns the result in DXY global variable. It also returns
; TRUE in the Accumulator on success. We will discuss shortly when
; the function fails after we discuss the algorithm.
; First the lower order byte of Timer 1 is sampled. We consider
; the contents as a DXY data. Next we check wheteher the cooresponding
; location is free on the screen ie is not occupied by the snake. If it
; is not free, then we traverse to the next free location. We move along
; the positive X direction. When we reach X_MAX, we circle around to X_MIN
; and step to the next upper row (incr. Y). We repeat this process until
; we locate an unhighlighted pixel or until we reach the starting co-ordinate.
; Now, if we finally come back to the coordinate where we started, that means
; no pixel is free on the screen ie the whole screen s covered by the snake!!!
; which we know is impossible. Any way, as programmers, we handle this hypothetical
; situation by returning a failure flag (FALSE) in the accumulator.

A_DXY_GetNewEggLocation:
                                MOV DXY, RAND_EGG_LOC_GEN_TIMER_REG_LOW ; Sample lower byte of Timer 1
                                LCALL D_X_Y_Parse_DXY ; Extract D, X and Y assuming the sample to be DXY data
                                MOV X0, X ; Consider X as the starting absissa of our loop (in case we need to loop)
                                MOV Y0, Y ; Consider X as the starting oordinate of our loop (in case we need to loop)
Loop_ADXYGetNewEggLocation:
                                LCALL CGetPixel_X_Y ; Get the state of the pixel at (X, Y)
                                JNC FlagSuccessAndReturn_ADXYGetNewEggLocation ; If clear, flag success and return
                                                                               ; the pixel location
                                INC X ; Else incr. X
                                ANL X, #CIRCLE_BOUND_X_MASK ; Bring X to display bounds, in case it overflows
                                MOV A, X ; Test whether X is zero
                                JNZ TestForStartingPointCoincidence_ADXYGetNewEggLocation ; If not, jump to the block
                                                                                          ; that tests for coincidence
                                                                                          ; with the starting coordinate
                                INC Y ; If X is zero, step to the upper row
                                ANL Y, #CIRCLE_BOUND_Y_MASK ; If there is no upper row, circle around to the bottom row
TestForStartingPointCoincidence_ADXYGetNewEggLocation: ; Now we check for the hypothetical loop break condition...
                                MOV A, X ; First test whether X and starting X are equal
                                CJNE A, X0, Loop_ADXYGetNewEggLocation ; If not, loop again...
                                MOV A, Y ; If yes, test whether Y and starting Y are equal
                                CJNE A, Y0, Loop_ADXYGetNewEggLocation ; If not, loop again...
                                MOV A, #FALSE ; If yes, the hypothetical situation has arisen; So, inform the caller
                                              ; about the failure
                                RET
FlagSuccessAndReturn_ADXYGetNewEggLocation:
                                MOV D, #POS_AXIS_DIR ; We set the dir. attribute to zero (though dir. of a single
                                                     ; pixel is immaterial
                                LCALL DXY_Unparse_D_X_Y ; Compose DXY from D, X and Y
                                MOV A, #TRUE ; Inform the caller about the success
                                RET




              






; Function:  _EggSnatched
; Arguments: None
; Returns:   None

; Description:

; Performs the sequence of events when the head of snake coincides with
; the location of the egg

_EggSnatched:
                                ACALL _TurnOnBuzzerAndResetBuzzerDelay ; The buzzer is sounded
                                INC GAME_SCORE ; Game score is incremented
                                MOV EGG_ACTIVITY_STATE, #FALSE ; Egg state is changed to inactive
                                ACALL _ResetEggDelay ; The egg delay is reset
                                RET













; Function:  _PaintEggIfActive
; Arguments: None
; Returns:   None

; Description:

; This function paints the egg on the screen if the egg state is active
; This function is uaually called when the Game resumes after a Pause/Dealy Settings 
; modes.

_PaintEggIfActive:
                                MOV A, EGG_ACTIVITY_STATE ; First the state of the egg is to be determined
                                JZ Return_PaintEggIfActive ; If inactive, no action is taken
                                MOV DXY, EGG_LOCATION ; Else load the loc. of the egg to DXY global variable
                                LCALL D_X_Y_Parse_DXY ; Extract D, X and Y
                                LCALL _SetPixel_X_Y ; Turn on the cooresponding pixel on the screen
Return_PaintEggIfActive:
                                RET













; Function:  _TurnOnBuzzerAndResetBuzzerDelay
; Arguments: None
; Returns:   None

; Description:

; This function turns on the buzzer and init.s a delay counter R6; This function is
; called when the snake snatches the egg

_TurnOnBuzzerAndResetBuzzerDelay:
                                LCALL _TurnOnBuzzer ; Turn on the buzzer
                                MOV R6, #EGG_SNATCH_BUZZER_SOUND_DELAY ; Reset the buzzer delay counter
                                RET













; Function:  _TurnOffBuzzerOnDelayMatch
; Arguments: None
; Returns:   None

; Description:

; This function first dectrements the val. of the delay counter and tests wheteher it is
; equal to zero. If so, it turns off the buzzer. This function is usually called
; during every game time step. So, it has to make sure that it takes no action if
; the delay counter (R6) is already zero

_TurnOffBuzzerOnDelayMatch:
                                MOV A, R6 ; Monitor the state of R6
                                JZ Return_TurnOffBuzzerOnDelayMatch ; If already zero, then take no action 
                                DJNZ R6, Return_TurnOffBuzzerOnDelayMatch ; Else, decr. R6 and return if not zero
                                LCALL _TurnOffBuzzer ; If zero, turn off the buzzer
Return_TurnOffBuzzerOnDelayMatch:                             
                                RET
