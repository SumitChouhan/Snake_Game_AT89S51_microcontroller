
; File:       DelaySettings.inc
; Created by: Sumit Chouhan

$TT ('DelaySettings')

; This file defines the functions called when the program enters delay settings mode -
; which enables the user to adjust the crawl speed of the snake
;
; This mode can be entered by pressing the delay settings key during the game or when in
; stand by/paused states and can be exited by again pressing the delay settings key. The
; mode is also exited automatically if the user does not press any key in ten seconds.

; The functions in this file will be stored in page one of ROM








; Function:  _DelaySettings
; Arguments: None
; Returns:   None

; Description:

; The entery point to the delay settings mode.

_DelaySettings:
                                ; Save the current context on the stack
                                PUSH ACCL
                                PUSH REG_0 ; R0 is used by most of the functions as a local variable
                                PUSH REG_1 ; R1 is used by most of the functions as a local variable
                                PUSH REG_2 ; R2 is used to count down the time spent without any key press in this mode
                                PUSH REG_3 ; R3 is used as a down counter to decide when the snake is to be shifted 
                                PUSH PSW
                                PUSH D
                                PUSH X0
                                PUSH Y0
                                PUSH X1
                                PUSH Y1
                                PUSH PREV_KEY_STATES
                                PUSH CURR_KEY_STATES

                                MOV R2, #MAX_DEL_SET_IDLE_TIME
                                MOV R3, SNAKE_DELAY

                                ACALL _ClearScreen
                                ACALL _PaintDelaySettingsBackground
                                CJNE R3, #MAX_SNAKE_DELAY, PaintDelayBarGraph_DelaySettings
                                SJMP PaintSnake_DelaySettings
PaintDelayBarGraph_DelaySettings:
                                ACALL _PaintDelayBarGraph
PaintSnake_DelaySettings:
                                ACALL _PaintSnake_DelaySettings

                                ACALL _WaitForAllActionKeyRelease
                                MOV PREV_KEY_STATES, #ALL_KEYS_PRESSED
MainLoop_DelaySettings:
                                MOV CURR_KEY_STATES, KEYPAD_ACCESS_PORT
ScanDelaySettingsKey_DelaySettings:
                                MOV A, #DEL_SET_KEY_MASK
                                ACALL ADetectedDepressingEdgeOfKeyMaskInA
                                JZ ScanDecrDelayKey_DelaySettings
                                SJMP Exit_DelaySettings
ScanDecrDelayKey_DelaySettings: ; Press and hold action of this key is processed
                                MOV A, #DECR_DEL_KEY_MASK
                                ACALL ADetectedDepressedStateOfKeyMaskInA
                                JZ ScanIncrDelayKey_DelaySettings
                                ACALL _ProcessDecrDelayKeyPress
                                MOV R2, #MAX_DEL_SET_IDLE_TIME ; Reset the auto exit time down counter
                                SJMP EndOfKeyScan_DelaySettings
ScanIncrDelayKey_DelaySettings: ; Press and hold action of this key is processed
                                MOV A, #INCR_DEL_KEY_MASK
                                ACALL ADetectedDepressedStateOfKeyMaskInA
                                JZ ScanAnyKeyPress_DelaySettings
                                ACALL _ProcessIncrDelayKeyPress
                                MOV R2, #MAX_DEL_SET_IDLE_TIME ; Reset the auto exit time down counter
                                SJMP EndOfKeyScan_DelaySettings
ScanAnyKeyPress_DelaySettings:  ; In case any other key is presses, we reset the auto exit time down counter
                                MOV A, #ANY_KEY_MASK
                                ACALL ADetectedDepressingEdgeOfKeyMaskInA
                                JZ EndOfKeyScan_DelaySettings
                                MOV R2, #MAX_DEL_SET_IDLE_TIME ; Reset the auto exit time down counter
                                MOV CURR_KEY_STATES, #ALL_KEYS_PRESSED
EndOfKeyScan_DelaySettings:
                                MOV PREV_KEY_STATES, CURR_KEY_STATES
FirstDelayBlock_DelaySettings:  ; Since press and hold action of certain keys are allowed in this mode
                                ; we don't use the standard key scan delay. We process a pressed and held down
                                ; key only five times a second (the standard key scan delay will double the
                                ; processing rate). We also split the delay into two blocks because some program
                                ; features are dependent on the standard key processing delay period. 
                                MOV A, #HALF_THE_DEL_SET_TIME_STEP_PERIOD
                                ACALL _DelayABy100ms
                                ACALL _ShiftDelSetSnakeOnDelayMatch
                                DJNZ R2, SecondDelayBlock_DelaySettings
                                SJMP Exit_DelaySettings ; The auto exit time has expired. So this mode is exited.
SecondDelayBlock_DelaySettings:
                                MOV A, #HALF_THE_DEL_SET_TIME_STEP_PERIOD
                                ACALL _DelayABy100ms
                                ACALL _TurnOffBuzzer ; In case the buzzer was left on by incr/decr delay functions
                                ACALL _ShiftDelSetSnakeOnDelayMatch
                                DJNZ R2, MainLoop_DelaySettings ; This mode is exited automatically when R2 becomes
Exit_DelaySettings:                                             ; zero
                                ACALL _TurnOffBuzzer
                                ACALL _ClearScreen

                                ; Restore the previous context from the stack
                                POP CURR_KEY_STATES
                                POP PREV_KEY_STATES
                                POP Y1
                                POP X1
                                POP Y0
                                POP X0
                                POP D
                                POP PSW
                                POP REG_3
                                POP REG_2
                                POP REG_1
                                POP REG_0
                                POP ACCL
                                RET








; Function:  _PaintDelaySettingsBackground
; Arguments: None
; Returns:   None

; Description:

; This function paints the delay settings mode background.
; The function calls it's associate ALoadDelSetBackgroundColumnDataInA to
; obtain the pixel pattern at each column. The pattern is than written to
; the byte in the display buffer corresponding to the column.

_PaintDelaySettingsBackground:
                                MOV R0, #X_MIN
                                MOV R1, #DISP_BUF_START_ADDR
Loop_PaintDelaySettingsBackground:
                                MOV A, R0
                                ACALL ALoadDelSetBackgroundColumnDataInA
                                MOV @R1, A
                                INC R0
                                INC R1
                                CJNE R1, #DISP_BUF_OVR_FLW_ADDR, Loop_PaintDelaySettingsBackground
                                RET










; Function:  ALoadDelSetBackgroundColumnDataInA
; Arguments: A
; Returns:   A

; Description:

; This function reads the pixel pattern at a column on the display from
; the ROM. If the relative column address is passed to the function in A,
; it returns the corresponding pixel data as a byte in A.

ALoadDelSetBackgroundColumnDataInA:
                                INC A
                                MOVC A, @A + PC
                                RET
DB                              0EAH
DB                              0AAH
DB                              0BAH
DB                              000H
DB                              0F8H
DB                              0E0H
DB                              000H
DB                              0F8H
DB                              0A8H
DB                              000H
DB                              0F8H
DB                              0A8H
DB                              000H
DB                              0FAH
DB                              08FH
DB                              072H








; Function:  _PaintDelayBarGraph
; Arguments: None
; Returns:   None

; Description:

; The speed of the snake is indicated by a delay bargraph. This function paints the
; initial state of the delay bar graph based on the current delay settings.
;
; NOTE: * Thought it appers to the user that the speed of the snake is being changed,
;         actually, it is the delay of the snake b/w two consequent advancements that
;         is being adjusted. So, if speed is incemented, internally delay is decremented and
;         vice versa. The bar graph indicates the speed of the snake and not it's delay.
;
;       * This function is called only once when the delay settings mode is entered. If
;         the current delay setting is maximun delay (ie snake speed minimum), this
;         function won't be called at all.

_PaintDelayBarGraph:
                                MOV X, #DELAY_BAR_GRAPH_X_MIN
                                INC X ; Since it is guaranteed that the function is called with delay non-maximum
                                MOV Y, #DELAY_BAR_GRAPH_Y
                                MOV A, #MAX_SNAKE_DELAY
                                CLR C
                                SUBB A, SNAKE_DELAY
                                MOV R0, A
Loop_PaintDelayBarGraph:
                                ACALL _SetPixel_X_Y
                                INC X
                                DJNZ R0, Loop_PaintDelayBarGraph
                                ; We chose not to call the library function draw line because there
                                ; ia a possibility of stack overflow!!!
                                RET











; Function:  _PaintSnake_DelaySettings
; Arguments: None
; Returns:   None

; Description:

; In delay settings mode, the current speed of the snake can be determined from the
; bar graph. However, to have a better feel, we animate a snake on the delay settings
; screen. This function paints the initial position of the snake.

_PaintSnake_DelaySettings:
                                MOV Y, #Y_OF_SNAKE_SET_MODE
                                MOV R0, #TAIL_INIT_X_OF_SNAKE_DEL_SET
Loop_PaintSnake_DelaySettings:
                                MOV X, R0
                                ACALL _SetPixel_X_Y
                                INC R0
                                CJNE R0, #HEAD_INIT_X_OF_SNAKE_DEL_SET, Loop_PaintSnake_DelaySettings
                                INC X
                                ACALL _SetPixel_X_Y ; Highlight the head pixel of snake
                                ; We chose not to call the library function draw line because there
                                ; ia a possibility of stack overflow!!!

                                MOV X0, #TAIL_INIT_X_OF_SNAKE_DEL_SET
                                MOV Y0, #Y_OF_SNAKE_SET_MODE
                                MOV X1, #HEAD_INIT_X_OF_SNAKE_DEL_SET
                                MOV Y1, #Y_OF_SNAKE_SET_MODE

                                MOV X, #ECCLIPSED_PIXEL_X_DEL_SET
                                MOV Y, #ECCLIPSED_PIXEL_Y_DEL_SET
                                ACALL _SetPixel_X_Y
                                RET









; Function:  _ProcessIncrDelayKeyPress
; Arguments: None
; Returns:   None

; Description:

; This function will process the increment snake delay key press event.
; It increments the snake delay if the existing delay is not max. and then
; accordingly modifies the delay bar grpah.

_ProcessIncrDelayKeyPress:
                                MOV R0, SNAKE_DELAY
                                CJNE R0, #MAX_SNAKE_DELAY, KeyAction_ProcessIncrDelayKeyPress
                                ACALL _TurnOnBuzzer
                                RET
KeyAction_ProcessIncrDelayKeyPress:
                                MOV A, #MAX_SNAKE_DELAY
                                CLR C
                                SUBB A, SNAKE_DELAY
                                ADD A, #DELAY_BAR_GRAPH_X_MIN
                                MOV X, A
                                MOV Y, #DELAY_BAR_GRAPH_Y
                                ACALL _ClearPixel_X_Y
                                INC SNAKE_DELAY
                                INC R3
                                ACALL _TurnOnBuzzer
                                RET








; Function:  _ProcessDecrDelayKeyPress
; Arguments: None
; Returns:   None

; Description:

; This function will process the decrement snake delay key press event.
; It decrements the snake delay if the existing delay is not min. and then
; accordingly modifies the delay bar grpah.

_ProcessDecrDelayKeyPress:
                                MOV R0, SNAKE_DELAY
                                CJNE R0, #MIN_SNAKE_DELAY, KeyAction_ProcessDecrDelayKeyPress
                                ACALL _TurnOnBuzzer
                                RET
KeyAction_ProcessDecrDelayKeyPress:
                                MOV A, #MAX_SNAKE_DELAY
                                CLR C
                                SUBB A, SNAKE_DELAY
                                ADD A, #DELAY_BAR_GRAPH_X_MIN
                                INC A
                                MOV X, A
                                MOV Y, #DELAY_BAR_GRAPH_Y
                                ACALL _SetPixel_X_Y
                                DEC SNAKE_DELAY
                                ACALL _TurnOnBuzzer
                                RET








; This function will decrement the snake delay counter local variable and
; advances the snake if the counter becomes zero.

_ShiftDelSetSnakeOnDelayMatch:
                                DJNZ R3, Return_ShiftDelSetSnakeOnDelayMatch
                                MOV R3, SNAKE_DELAY
ShiftingHead_ShiftDelSetSnakeOnDelayMatch:
                                INC X1
                                ANL X1, #CIRCLE_BOUND_X_MASK ; In case X overflows (ie X > 15), X will be set to 0
HighlightHead_ShiftDelSetSnakeOnDelayMatch:
                                MOV X, X1
                                MOV Y, Y1
                                ACALL _SetPixel_X_Y
DehighlightTail_ShiftDelSetSnakeOnDelayMatch:
                                MOV X, X0
                                MOV Y, Y0
                                ACALL _ClearPixel_X_Y
ShiftingTail_ShiftDelSetSnakeOnDelayMatch:
                                INC X0
                                ANL X0, #CIRCLE_BOUND_X_MASK ; In case X overflows (ie X > 15), X will be set to 0
HighlightMaskedPixel_ShiftDelSetSnakeOnDelayMatch:
                                MOV X, #ECCLIPSED_PIXEL_X_DEL_SET ; The snake will run over a delay settings
                                MOV Y, #ECCLIPSED_PIXEL_Y_DEL_SET ; background pixel. As snake passes by, this pixel
                                ACALL _SetPixel_X_Y               ; will be cleared. So, we paint this pixel everytime.
Return_ShiftDelSetSnakeOnDelayMatch:                              ; We don't care when the snake runs over it.
                                RET
