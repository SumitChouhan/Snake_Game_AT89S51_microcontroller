
; File:       GameBackEnd.inc
; Created by: Sumit Chouhan

$TT ('GameBackEnd')

; The definitions of this file constitute the heart of the program. The functions in
; this file take care of advancing the snake, changing it's direction and detecting
; egg snatch and collision.
 
; NOTE: Functions in this file operate at the top most level and they do no care to
; back up the registers they modify!!!

; R4 - is a local var. at C51Main level and is used as a snake advancement delay down counter

; The functions in this file will be stored in page two of ROM









; Function:  AAdvanceSnakeOnDelayMatch
; Arguments: None
; Returns:   A

; Description:

; The heart of the game...
; This function advances the snake when the snake delay counter underflows.
; This function computes new coordinates of head and tail, performs egg snatch
; operation and detects collision. This function is called periodically in game mode.
; This function returns TRUE in accumulator if it DOES NOT detect collision so that
; the game can continue. Otherwise it returns FALSE in A.
;
; NOTE: The function only manipulates the pixels at the head and tail of snake.
;       It does not alter the state of any other pixels.

AAdvanceSnakeOnDelayMatch: 
                                DEC R4 ; Decr. snake delay down counter
                                MOV A, R4 ; Copy the result to the Accumulator
                                JZ ResetSnakeDelayDownCounter ; If the result is zero, delay match has
                                                              ; occured and thus advance the snake
                                                              ; NOTE: We don't use DJNZ here because, the
                                                              ;       target of the jump (end of function)
                                                              ;       is out of relative jump inst.'s range
                                AJMP FlagNoCollisionAndReturn ; If not, jump to the end of the function
ResetSnakeDelayDownCounter:
                                MOV R4, SNAKE_DELAY ; Reset the snake delay down counter
                                                    ; Must preceed the all the statements that follow
                                                    ; since they sometimes read and modify this value!!!
CheckForAnyActionKeyEvents:
                                MOV A, NEXT_DIR_OF_ADVANCE
                                JZ ObtainNewCoordinatesOfTailAndHead ; Skip key processing block if no key
                                                                     ; press is detetcted
                                ACALL A_D_PerformKeyActionAndNotifyWhetherEnqueueRequired
                                JZ EndOfActionKeyEventProcessing ; Enqueue not required ie
                                                                 ; user key press has not invoked a change in
                                                                 ; the current snake's direction
                                ACALL A_R1_TestIfQueueOverflowsAndGetNextFreeAddrInQueue
                                JNZ EndOfActionKeyEventProcessing ; Queue is full, so ignore key press
                                MOV R0, HEAD_OF_SNAKE_PTR
                                MOV A, @R0
                                ANL A, #COORDINATE_MASK
                                ACALL _EnqueueAAtR1 ; Enqueue the new coordinates of head
                                ORL A, D
                                MOV @R0, A 
                                MOV CURR_DIR_OF_ADVANCE, NEXT_DIR_OF_ADVANCE ; Update the direction of advance
EndOfActionKeyEventProcessing:
                                MOV NEXT_DIR_OF_ADVANCE, #SNAKE_DIR_IMPLICIT
ObtainNewCoordinatesOfTailAndHead:
                                ACALL D_X_Y_GetNewCoordinatesOfTail ; Always the coordinates of tail
                                                                    ; are processed before the coordinates of head.
                                LCALL DXY_Unparse_D_X_Y
                                MOV R2, DXY
                                PUSH Y ; Save X on the stack
                                PUSH X ; Save Y on the stack
                                ACALL X_Y_GetNewCoordinatesOfHead
                                LCALL DXY_Unparse_D_X_Y
                                MOV R3, DXY

                                POP X0    ; X0, Y0 will now contain new coordinates of tail
                                POP Y0
                                ; R2 now contains the new coordinates of tail in DXY format
                                MOV X1, X ; X1, Y1 will now contain new coordinates of head
                                MOV Y1, Y
                                ; R3 now contains the new coordinates of head in DXY format
TestForEggSnatch:               ; An eggsnatch occurs if an egg is active and
                                ; the new coordinates of head coincides with the coordinates of egg
                                MOV A, EGG_ACTIVITY_STATE
                                JZ TestIfPresentCoordinatesOfTailAndNewCoordinatesOfHeadCoincide
                                MOV A, R3
                                ANL A, #COORDINATE_MASK
                                CJNE A, EGG_LOCATION, TestIfPresentCoordinatesOfTailAndNewCoordinatesOfHeadCoincide
                                ACALL _EggSnatched
                                SJMP UpdateNewCoordinatesOfHead
TestIfPresentCoordinatesOfTailAndNewCoordinatesOfHeadCoincide: ; This condition usually occurs when the
                                                               ; length of the snakem equals any of the screen
                                                               ; dimensions and the snake is aligned parallel
                                                               ; to the dimension. So we don't the snake to collide
                                                               ; with it's own tail pixel
                                MOV R0, TAIL_OF_SNAKE_PTR
                                MOV DXY, @R0
                                LCALL D_X_Y_Parse_DXY
                                MOV A, X
                                CJNE A, X1, TestForCollision
                                MOV A, Y
                                CJNE A, Y1, TestForCollision
                                SJMP UpdateNewCoordinatesOfTailAndHead ; If above case is found to be true,
                                                                       ; we skip the collision test as we know
                                                                       ; that the test will be positive
TestForCollision:
                                MOV X, X1
                                MOV Y, Y1
                                LCALL CGetPixel_X_Y
                                JNC UpdateNewCoordinatesOfTailAndHead
                                MOV A, #FALSE ; Collision detected; So, return immediately
                                RET
UpdateNewCoordinatesOfTailAndHead: ; Mirror the new head and tail pixels on the screen
UpdateNewCoordinatesOfTail:
                                MOV R0, TAIL_OF_SNAKE_PTR
                                MOV DXY, @R0
                                LCALL D_X_Y_Parse_DXY
                                LCALL _ClearPixel_X_Y

                                ACALL R1_GetAddressOfLastButOneElementInQueue
                                MOV DXY, @R1
                                LCALL D_X_Y_Parse_DXY
                                MOV A, X                                     ; Finally, if the tail merges with
                                CJNE A, X0, UpdateNewCoordinatesOfTailFromR2 ; the last but one segment of snake,
                                MOV A, Y                                     ; perform dequeue
                                CJNE A, Y0, UpdateNewCoordinatesOfTailFromR2
                                ACALL _DequeueToR1
                                SJMP UpdateNewCoordinatesOfHead
UpdateNewCoordinatesOfTailFromR2: ; The tail segment has not merged with the last but one segment, so simply
                                  ; update the new coordinates of tail in the queue
                                MOV DXY, R2
                                MOV @R0, DXY 
UpdateNewCoordinatesOfHead:
                                MOV X, X1
                                MOV Y, Y1
                                LCALL _SetPixel_X_Y

                                MOV DXY, R3
                                ANL DXY, #COORDINATE_MASK
                                MOV R0, HEAD_OF_SNAKE_PTR
                                MOV @R0, DXY                          
FlagNoCollisionAndReturn:
                                MOV A, #TRUE
                                RET











; Function:  _PaintSnake
; Arguments: None
; Returns:   None

; Description:

; This function paints the entire snake and also the egg (if it is active)
; This function is mainly called when the game mode is resumed
; after a mode switch. Since the collision detection block uses the display
; buffer itself, this function takes care of restoring the status of the
; display buffer as it were before a context switch.

_PaintSnake:
                                MOV R0, TAIL_OF_SNAKE_PTR ; Init. the first loop variable to the address
                                                          ; of the tail of snake
Loop_PaintSnake:
                                MOV A, R0 ; Copy the first loop variable 
                                MOV R1, A ; to the second loop variable
                                ; Next we increment the second loop variable
                                CJNE R1, #QUEUE_END_ADDR, IncrR1_PaintSnake ; If the address is the last in the 
                                MOV R1, #QUEUE_START_ADDR ; RAM space reserved for the queue, set the var. to
                                                          ; the starting address of RAM space reserved for queue
                                                          ; Note that we use a circular queue, else, simply
                                                          ; jump to the block that incr. the variable 
                                SJMP ParseTerminalPoints_PaintSnake ; Jump to the block that parses the terminal
IncrR1_PaintSnake:                                                  ; points of a segment of the snake
                                INC R1 ; Block that increments the second loop variable
ParseTerminalPoints_PaintSnake: ; Block that parses the terminal points of a segment of the snake
                                ; Now R0 will contain the starting point of the segment
                                ; and r1 will contain the terminal point of the segment
                                MOV DXY, @R1 ; Get the DXY data at R1
                                LCALL D_X_Y_Parse_DXY ; Extract D, X and Y
                                MOV X1, X ; Copy X to X1 and
                                MOV Y1, Y ; Copy Y to Y1
                                MOV DXY, @R0 ; Get the DXY data at R0
                                LCALL D_X_Y_Parse_DXY ; Extract D, X and Y
                                MOV X0, X ; Copy X to X0 and
                                MOV Y0, Y ; Copy Y to Y0
                                ; D will now contain the D data in R0
                                LCALL _DrawLine ; Draw line from (X0, Y0) to (X1, Y1) in the dir. D
                                MOV A, R1 ; Copy R1
                                MOV R0, A ; to R0
                                CJNE A, HEAD_OF_SNAKE_PTR, Loop_PaintSnake ; If R0, does not coincide with
                                                                           ; the address of the head of
                                                                           ; snake loop again, else break
                                ACALL _PaintEggIfActive ; Finally, paint egg if it was active and
                                RET ; Return











; Function:  _PrintGameScore
; Arguments: None
; Returns:   None

; Description:

; This function prints the game score on the screen
; This function can only print two decimal digits. So, the max. game
; score this function can print is 99. To ensure this, the game score
; is first moduloed with 100 before printing.
; However, practically, a game score of 100 => a snake of length 104
; pixels on 128 pixel screen is impossible unless the player
; is a superhuman!!!

_PrintGameScore:
                                MOV A, GAME_SCORE ; First, make sure that the game score is less than 100
                                MOV B, #GAME_SCORE_UPPER_BOUND ; by taking the modulo of the score with 100
                                DIV AB ; The remainder (score) is B
                                MOV A, B ; Move this val. to Accl.
                                MOV B, #DECIMAL_BASE ; To extract each digit, it is enough to divide
                                                     ; by 10. So, move divident 10 to B
                                DIV AB ; Perform the division; Tens digit will be in A and Units in B
                                MOV R0, B ; Move the unit's digit to R0
                                JNZ PrintTensDigit_PrintGameScore ; If the ten's digit is not zero, jump to
                                                                  ; the block that prints this digit
                                MOV A, #WHITE_SPACE_CHAR ; Else, replace zero by the index of White Space Char.
PrintTensDigit_PrintGameScore:
                                MOV B, #X_GAME_SCORE_TENS_DIGIT ; Move the screen loc. of ten's digit to B and 
                                LCALL _PrintDigitAAtB ; print the ten's digit
PrintUnitDigit_PrintGameScore:  ; Unit's digit is not replaced by a white space 
                                MOV A, R0 ; Move the unit's digit back to Accl.
                                MOV B, #X_GAME_SCORE_UNIT_DIGIT ; Move the screen loc. of unit's digit to B and 
                                LCALL _PrintDigitAAtB ; print the unit's digit
                                RET ; Then return
                                
 

 
                              
                              
                              
                              
                              
                                



; Function:  A_D_PerformKeyActionAndNotifyWhetherEnqueueRequired
; Arguments: None
; Returns:   A, D

; Description:

; This function processes the game action key press event. The user event is considered
; as a request to change the direction of the snake. The direction mapped to the key
; will be available in NEXT_DIR_OF_ADVANCE.
;
; This function returns TRUE in A if the user has requested to change the current axis of movement
; of the head segment to it's perpendicular segment. This means that an enqueue operation has
; to be performed. In this case, the new axis direction of advance is returned in D.
;
; On the other hand, if the user has pressed a key which implies movement in the current axis,
; the snake is accelerated/slowed down and FALSE id returned in A
;
; The pseudo code of this function is,
;
; switch (CURR_DIR_OF_ADVANCE)
;
; case RIGHT:
;               switch (NEXT_DIR_OF_ADVANCE)
;               
;               case RIGHT:     ACCELERATE SNAKE; return FALSE;
;               case UP:        D = POS_AXIS_DIR; return TRUE;
;               case LEFT:      SLOW DOWN SNAKE; return FALSE;
;               case DOWN:      D = NEG_AXIS_DIR; return TRUE;
; case UP:
;               switch (NEXT_DIR_OF_ADVANCE)
;
;               case RIGHT:     D = POS_AXIS_DIR; return TRUE;
;               case UP:        ACCELERATE SNAKE; return FALSE;
;               case LEFT:      D = NEG_AXIS_DIR; return TRUE; 
;               case DOWN:      SLOW DOWN SNAKE; return FALSE; 
; case LEFT:
;               switch (NEXT_DIR_OF_ADVANCE)
;
;               case RIGHT:     SLOW DOWN SNAKE; return FALSE;
;               case UP:        D = POS_AXIS_DIR; return TRUE;
;               case LEFT:      ACCELERATE SNAKE; return FALSE;
;               case DOWN:      D = NEG_AXIS_DIR; return TRUE; 
; case DOWN:
;               switch (NEXT_DIR_OF_ADVANCE)
;
;               case RIGHT:     D = POS_AXIS_DIR; return TRUE;
;               case UP:        SLOW DOWN SNAKE; return FALSE;
;               case LEFT:      D = NEG_AXIS_DIR; return TRUE; 
;               case DOWN:      ACCELERATE SNAKE; return FALSE;
                        
A_D_PerformKeyActionAndNotifyWhetherEnqueueRequired:
                                MOV A, CURR_DIR_OF_ADVANCE
KeyActionIfCurrDirIsRight:
                                CJNE A, #SNAKE_DIR_RIGHT, KeyActionIfCurrDirIsUp
                                MOV A, NEXT_DIR_OF_ADVANCE
KeyActionForRightKeyWhileAdvancingRight:
                                CJNE A, #SNAKE_DIR_RIGHT, KeyActionForUpKeyWhileAdvancingRight
                                ACALL _HalfSnakeDelayCount
                                MOV A, #FALSE
                                RET
KeyActionForUpKeyWhileAdvancingRight:
                                CJNE A, #SNAKE_DIR_UP, KeyActionForLeftKeyWhileAdvancingRight
                                MOV D, #POS_AXIS_DIR
                                MOV A, #TRUE
                                RET
KeyActionForLeftKeyWhileAdvancingRight:
                                CJNE A, #SNAKE_DIR_LEFT, KeyActionForDownKeyWhileAdvancingRight
                                ACALL _DoubleSnakeDelayCount
                                MOV A, #FALSE
                                RET
KeyActionForDownKeyWhileAdvancingRight:
                                MOV D, #NEG_AXIS_DIR
                                MOV A, #TRUE
                                RET
KeyActionIfCurrDirIsUp:
                                CJNE A, #SNAKE_DIR_UP, KeyActionIfCurrDirIsLeft
                                MOV A, NEXT_DIR_OF_ADVANCE
KeyActionForRightKeyWhileAdvancingUp:
                                CJNE A, #SNAKE_DIR_RIGHT, KeyActionForUpKeyWhileAdvancingUp
                                MOV D, #POS_AXIS_DIR
                                MOV A, #TRUE
                                RET
KeyActionForUpKeyWhileAdvancingUp:
                                CJNE A, #SNAKE_DIR_UP, KeyActionForLeftKeyWhileAdvancingUp
                                ACALL _HalfSnakeDelayCount
                                MOV A, #FALSE
                                RET
KeyActionForLeftKeyWhileAdvancingUp:
                                CJNE A, #SNAKE_DIR_LEFT, KeyActionForDownKeyWhileAdvancingUp
                                MOV D, #NEG_AXIS_DIR
                                MOV A, #TRUE
                                RET
KeyActionForDownKeyWhileAdvancingUp:
                                ACALL _DoubleSnakeDelayCount
                                MOV A, #FALSE
                                RET
KeyActionIfCurrDirIsLeft:
                                CJNE A, #SNAKE_DIR_LEFT, KeyActionIfCurrDirIsDown
                                MOV A, NEXT_DIR_OF_ADVANCE
KeyActionForRightKeyWhileAdvancingLeft:
                                CJNE A, #SNAKE_DIR_RIGHT, KeyActionForUpKeyWhileAdvancingLeft
                                ACALL _DoubleSnakeDelayCount
                                MOV A, #FALSE
                                RET
KeyActionForUpKeyWhileAdvancingLeft:
                                CJNE A, #SNAKE_DIR_UP, KeyActionForLeftKeyWhileAdvancingLeft
                                MOV D, #POS_AXIS_DIR
                                MOV A, #TRUE
                                RET
KeyActionForLeftKeyWhileAdvancingLeft:
                                CJNE A, #SNAKE_DIR_LEFT, KeyActionForDownKeyWhileAdvancingLeft
                                ACALL _HalfSnakeDelayCount
                                MOV A, #FALSE
                                RET
KeyActionForDownKeyWhileAdvancingLeft:
                                MOV D, #NEG_AXIS_DIR
                                MOV A, #TRUE
                                RET
KeyActionIfCurrDirIsDown:
                                MOV A, NEXT_DIR_OF_ADVANCE
KeyActionForRightKeyWhileAdvancingDown:
                                CJNE A, #SNAKE_DIR_RIGHT, KeyActionForUpKeyWhileAdvancingDown
                                MOV D, #POS_AXIS_DIR
                                MOV A, #TRUE
                                RET
KeyActionForUpKeyWhileAdvancingDown:
                                CJNE A, #SNAKE_DIR_UP, KeyActionForLeftKeyWhileAdvancingDown
                                ACALL _DoubleSnakeDelayCount
                                MOV A, #FALSE
                                RET
KeyActionForLeftKeyWhileAdvancingDown:
                                CJNE A, #SNAKE_DIR_LEFT, KeyActionForDownKeyWhileAdvancingDown
                                MOV D, #NEG_AXIS_DIR
                                MOV A, #TRUE
                                RET
KeyActionForDownKeyWhileAdvancingDown:
                                ACALL _HalfSnakeDelayCount
                                MOV A, #FALSE
                                RET










; Function:  A_R1_TestIfQueueOverflowsAndGetNextFreeAddrInQueue
; Arguments: None
; Returns:   A, R1

; Description:

; This function checks whether the queue is full. If so, it returns TRUE in Accumulator.
; If not so, it returns FALSE in Accl. and the next free location in the circular queue
; is returned in R1

A_R1_TestIfQueueOverflowsAndGetNextFreeAddrInQueue:
                                MOV R1, HEAD_OF_SNAKE_PTR
                                INC R1 ; Obtain the next free location in queue
                                CJNE R1, #QUEUE_OVR_FLW_ADDR, TestQueueOverFlowCondition ; If R1 overflows the
                                MOV R1, #QUEUE_START_ADDR ; queue's address space, R1 is reset to the first address
                                                          ; in the space
TestQueueOverFlowCondition:
                                MOV A, R1
                                CJNE A, TAIL_OF_SNAKE_PTR, FlagNoOverflowAndReturn
FlagOverflowAndReturn:
                                MOV A, #TRUE
                                RET
FlagNoOverflowAndReturn:
                                MOV A, #FALSE
                                RET











; Function:  _EnqueueAAtR1
; Arguments: R1
; Returns:   None

; Description:

; This function updates the HEAD_OF_SNAKE_PTR to the address contained in R1
_EnqueueAAtR1:
                                MOV @R1, A
                                MOV HEAD_OF_SNAKE_PTR, R1
                                RET












; Function:  D_X_Y_GetNewCoordinatesOfTail
; Arguments: None
; Returns:   D, X, Y

; Description:

; This function obtains the new coordinates of tail. It compares the present coordinates of tail with
; the coordinates at last but one queue location ie coordinates of the last but one bend in snake.
; If the X coordinates of tail and the bend are equal, Y coordinate of tail is incr/decr based on whether
; D of tail is 0/1.
; Similarly, if the Y coordinates of tail and the bend are equal, X coordinate of tail is incr/decr based
; on whether D of tail is 0/1.
;
; The new coordinates are written to global variables D, X and Y

D_X_Y_GetNewCoordinatesOfTail:
                                MOV R0, TAIL_OF_SNAKE_PTR ; Copy the addr. of the tail of snake (queue)
                                                          ; coordinates to R0
                                ACALL R1_GetAddressOfLastButOneElementInQueue 
ParseTerminalPoints_D_X_Y_GetNewCoordinatesOfTail: ; Block that extracts D, X, Y data from the elements
                                                             ; addressed by R0 and R1
                                MOV DXY, @R1 ; First DXY pointed by R1 is parsed (T)
                                LCALL D_X_Y_Parse_DXY ; Extract D, X and Y data
                                MOV X1, X ; Copy X to X1 and
                                MOV Y1, Y ; Y to Y1
                                MOV DXY, @R0 ; Next DXY pointed by R0 is parsed (S)
                                LCALL D_X_Y_Parse_DXY ; Extract D, X and Y data of the tail

                                ; Next, the axis along which the tail shifts has to be found
                                MOV A, Y ; So, compare Y of S, with Y of T
                                CJNE A, Y1, ShiftYOfTail_RetreatTail ; Ys are not equal; So, shift is along
                                                                     ; Y axis; So, jump to the block that
                                                                     ; shifts the tail of snake along Y axis
ShiftXOfTail_RetreatTail: ; Ys are equal!!!; So, shift is along X axis. Block that shifts the snake
                          ; along X axis (NOTE: Always, either Ys or Xs will be equal!!!)
                                MOV A, D ; Next, the dir. of shift along X axis is to be determined!
                                JNZ DecrX_RetreatTail ; If dir is -ve, jump to the block that shifts the tail
                                                      ; of snake along -ve X axis
IncrX_RetreatTail: ; Block that shifts the snake along +ve X axis             
                                INC X ; Shift X of S along +ve X axis
                                ANL X, #CIRCLE_BOUND_X_MASK ; If X happens to overflow the screen bounds, circle
                                                            ; around X to the start of the screen
                                RET
DecrX_RetreatTail: ; Block that shifts the snake along -ve X axis 
                                DEC X ; Shift X of S along -ve X axis
                                ANL X, #CIRCLE_BOUND_X_MASK ; If X happens to underflow the screen bounds, circle
                                                            ; around X to the end of the screen
                                RET 
ShiftYOfTail_RetreatTail: ; Block that shifts the snake along Y axis
                                MOV A, D ; Next, the dir. of shift along Y axis is to be determined!
                                JNZ DecrY_RetreatTail ; If dir is -ve, jump to the block that shifts the tail
                                                      ; of snake along -ve Y axis
IncrY_RetreatTail: ; Block that shifts the snake along +ve Y axis
                                INC Y ; Shift Y of S along +ve Y axis
                                ANL Y, #CIRCLE_BOUND_Y_MASK ; If Y happens to overflow the screen bounds, circle
                                                            ; around Y to the bottom of the screen
                                RET
DecrY_RetreatTail:
                                DEC Y ; Shift Y of S along -ve Y axis
                                ANL Y, #CIRCLE_BOUND_Y_MASK ; If Y happens to underflow the screen bounds, circle
                                                            ; around Y to the top of the screen
                                RET












; Function:  X_Y_GetNewCoordinatesOfHead
; Arguments: None
; Returns:   X, Y

; Description:

; This function obtains the new coordinates of head. Based on the CURR_DIR_OF_ADVANCE, it decides
; whether to incr/decr X/Y. The new coordinates are written to global variables X and Y

X_Y_GetNewCoordinatesOfHead:
                                MOV R0, HEAD_OF_SNAKE_PTR ; Get the coordinates of head in DXY format
                                MOV DXY, @R0 ; in the queue and referenced by head of snake ptr. 
                                LCALL D_X_Y_Parse_DXY ; Extract D, X and Y of head
                                MOV A, CURR_DIR_OF_ADVANCE ; Next we find out the current direction
                                                           ; of advance
ImplicitStepSnakeRight: ; Block that advances the snake right if the current direction is right
                                CJNE A, #SNAKE_DIR_RIGHT, ImplicitStepSnakeUp ; If current direction is not right
                                                                              ; jump to the block tests whether
                                                                              ; the dir. is up
                                INC X ; Step the snake right
                                ANL X, #CIRCLE_BOUND_X_MASK ; If X happens to overflow the screen bounds, circle
                                                            ; around X to the start of the screen
                                RET ; Return
ImplicitStepSnakeUp: ; Block that advances the snake up if the current direction is up
                                CJNE A, #SNAKE_DIR_UP, ImplicitStepSnakeLeft ; If current direction is not up
                                                                             ; jump to the block tests whether
                                                                             ; the dir. is left
                                INC Y; Step the snake up
                                ANL Y, #CIRCLE_BOUND_Y_MASK ; If Y happens to overflow the screen bounds, circle
                                                            ; around Y to the bottom of the screen
                                RET ; Return 
ImplicitStepSnakeLeft: ; Block that advances the snake left if the current direction is left
                                CJNE A, #SNAKE_DIR_LEFT, ImplicitStepSnakeDown ; If current direction is not left
                                                                               ; jump to the block that advances
                                                                               ; the head down
                                DEC X; Step the snake left
                                ANL X, #CIRCLE_BOUND_X_MASK ; If X happens to underflow the screen bounds, circle
                                                            ; around X to the end of the screen
                                RET ; Return
ImplicitStepSnakeDown: ; Block that advances the snake down; Here we don't perform any cofirmation
                       ; test as this is the only possibility remaining. So we take it for
                       ; granted that the current direction is down
                                DEC Y ; Step the snake down
                                ANL Y, #CIRCLE_BOUND_Y_MASK ; If Y happens to underflow the screen bounds, circle
                                                            ; around Y to the top of the screen
                                RET









; Function:  R1_GetAddressOfLastButOneElementInQueue
; Arguments: None
; Returns:   R1

; Description:

; This function returns the address of the last but one element in queue in R1

R1_GetAddressOfLastButOneElementInQueue:
                                MOV R1, TAIL_OF_SNAKE_PTR 
                                INC R1
                                CJNE R1, #QUEUE_OVR_FLW_ADDR, Return_R1_GetAddressOfLastButOneElementInQueue
                                MOV R1, #QUEUE_START_ADDR ; Since we have used a circular queue, we circle
                                                          ; around to the starting address of the queue in case
                                                          ; the contents of R1 overflows
Return_R1_GetAddressOfLastButOneElementInQueue:
                                RET









; Function:  _DequeueToR1
; Arguments: R1
; Returns:   None

; Description:

; This function updates the TAIL_OF_SNAKE_PTR to the address contained in R1

_DequeueToR1:
                                MOV TAIL_OF_SNAKE_PTR, R1
                                RET












; Function:  _DoubleSnakeDelayCount
; Arguments: None
; Returns:   None

; Description:

; This function doubles the snake advance time down counter contents

_DoubleSnakeDelayCount:
                                MOV A, R4
                                CLR C
                                RLC A
                                MOV R4, A
                                RET









; Function:  _DoubleSnakeDelayCount
; Arguments: None
; Returns:   None

; Description:

; This function reduces the snake advance time down counter contents to half

_HalfSnakeDelayCount:
                                MOV A, R4
                                CLR C
                                RRC A
                                MOV R4, A
                                RET