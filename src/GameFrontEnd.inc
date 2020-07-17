
; File:       GameFrontEnd.inc
; Created by: Sumit Chouhan

$TT ('GameFrontEnd')

; Defines the functions that handle various states of the game. All the functions in
; file are directly called by the Main Block in response to various user events, game
; state changes, etc However, these functions rely on the back ends functions and
; functions in the egg handle file for their functioning.

; NOTE: Functions in this file operate at the top most level and they do no care to
; back up the registers they modify!!!

; The functions in this file will be stored in page two of ROM









; Function:  _InitGameResources
; Arguments: None
; Returns:   None

; Description:

; This function is called during the app. launch. These functions init.
; some SFRs and global variables that other modes/states of the app.
; depend on.

_InitGameResources:
                                MOV SNAKE_DELAY, #INIT_SNAKE_DELAY ; Init. snake delay; This var. is also
                                                                   ; used by Delay Settings Mode
                                ACALL _InitRandEggLocationTimer ; Init. the SFRs associated with the timer
                                RET













; Function:  _NotifyGameBegin
; Arguments: None
; Returns:   None

; Description:

; Called immediately before a game begin. Resets various variables used
; in the game and paints the init. status of the snake.

_NotifyGameBegin:
                                MOV TAIL_OF_SNAKE_PTR, #QUEUE_START_ADDR ; Init. ptr. to the tail of snake queue
                                MOV HEAD_OF_SNAKE_PTR, #QUEUE_START_ADDR ; Init. ptr. to the
                                INC HEAD_OF_SNAKE_PTR ;  head of snake queue
                                MOV GAME_SCORE, #INIT_GAME_SCORE ; Reset the game score
                                MOV CURR_DIR_OF_ADVANCE, #SNAKE_DIR_RIGHT ; Init. the starting dir. of advanc.
                                                                          ; of snake
                                MOV NEXT_DIR_OF_ADVANCE, #SNAKE_DIR_IMPLICIT ; This init. indicates that no
                                                                             ; key has been pressed by the user yet,
                                                                             ; and during the next advancement dir.
                                                                             ; is same as the current dir.
                                MOV D, #INIT_AXIS_DIR_OF_SNAKE ; Init. the axis dir. (-ve/+ve) of the snake
                                                               ; NOTE: The axis dir. is different from screen dir.
                                                               ; Allowed axis dir.s are: -ve and +ve
                                                               ; Allowed screen dir.s are: Right, Up, Left and Down
                                MOV Y, #INIT_Y_OF_SNAKE ; Init. ordinates of the head and tail (= since snake is init. horizontal)
                                MOV X, #INIT_TAIL_X_OF_SNAKE ; Absissa of the tail of snake...
                                LCALL DXY_Unparse_D_X_Y ; Conver to DXY format, the coordinates of the tail
                                MOV R0, TAIL_OF_SNAKE_PTR ; and move the contents to the queue loc. designated
                                MOV @R0, DXY ; by the tail ptr. using indirect adressing
                                MOV X, #INIT_HEAD_X_OF_SNAKE ; Absissa of the of snake...
                                LCALL DXY_Unparse_D_X_Y ; Conver to DXY format, the coordinates of the head,
                                ANL DXY, #COORDINATE_MASK ; retain only the coordinate data (_XY) and
                                MOV R0, HEAD_OF_SNAKE_PTR ; move the contents to the queue loc. designated
                                MOV @R0, DXY ; by the head ptr. using indirect adressing

                                MOV R4, SNAKE_DELAY ; Reset the snake delay local varibale
                                ACALL _ResetEggState ; Reset the egg activity state
                                ACALL _StartRandEggLocationTimer ; Start Timer 1 as a free running 16bit timer

                                LCALL _ClearScreen ; Empty the display buffer
                                ACALL _PaintSnake ; and Paint the snake on the screen (into the disp. buffer)
                                RET















; Function:  _NotifyGameFreeze
; Arguments: None
; Returns:   None

; Description:

; Initiates the sequence of events during a game freeze. Freeze implies temporary
; halting of the game when user initiates a switch to Paused State/Delay Settings Mode

_NotifyGameFreeze:
                                ACALL _StopRandEggLocationTimer ; Stop Timer 1
                                LCALL _TurnOffBuzzer ; Possibly the buzzer could have been turned on during
                                                     ; an egg snatch and the period of the down counter
                                                     ; associated with the buzzer turn off might not have expired
                                                     ; This causes the buzzer to cont. sounding; So, insighting
                                                     ; this situation, we turn off the buzzer prematurely.
                                RET











; Function:  _NotifyGameRestore
; Arguments: None
; Returns:   None

; Description:

; Initiates the sequence of events during a game restoration from Paused State/Delay Settings Mode.
; The display buffer would have been corrupted by the mode switch operation. So, this function
; restores the display to the state it was before the mode switch. This includes repainting
; the entire snake and the egg

_NotifyGameRestore:
                                MOV R4, SNAKE_DELAY ; The prev. mode might have been a paused state or the
                                                    ; the user might have switched to Delay Settings Mode but
                                                    ; might have left the SNAKE_DELAY unchanged. However, we simply
                                                    ; assume that user has altered the SNAKE_DELAY and so we
                                                    ; reload it. We ignore the existing contents of snake delay
                                                    ; down counter. We don't attempt to modify the down counter
                                                    ; in propotion to the ratio it bore to the old snake delay
                                                    ; This is because we will have to reserve another RAM
                                                    ; byte to store the old snake delay and do something
                                                    ; like R4 <- new snake delay * (R4 / ols dnake delay)
                                                    ; The user will practically notice no change since
                                                    ; the snake would simply be stationary after a game restore
                                                    ; until the user presses any key
                                ACALL _ResetEggDelay ; On similar grounds as explained above, we reset the egg
                                                     ; delay state toggle down counter also. But we don't
                                                     ; reset the egg state!!!
                                ACALL _StartRandEggLocationTimer ; Resume Timer 1
                                LCALL _ClearScreen ; Clear the disp. buffer,
                                ACALL _PaintSnake ; Repaint the snake
                                ACALL _PaintEggIfActive ; Paint the egg if the egg was active.
                                ; However, we don't attempt to resume the buzzer if it had been on during game
                                ; freeze. So, if the buzzer had been previously on, the buzzer turn off time
                                ; down counter will not be reset immediately but only after a few
                                ; game time steps!!!           
                                RET












; Function:  ANotifyGameTimeStep
; Arguments: None
; Returns:   None

; Description:

; This function notifies back end functions about a game time step so that
; they can change the state of the game (snake, egg, etc). This function
; returns TRUE in the Accumulator if the game may continue
; (ie the back end fn.s detected no collision) and FALSE otherwise.
; This function is called every time a fixed time period elapses. The time
; interval b/w two successive calls may vary depending on the time of
; execution of other tasks in between. However, this period is very large
; compared to other tasks that, we can safely argue that this function
; is called periodically.

ANotifyGameTimeStep:
                                ACALL _TurnOffBuzzerOnDelayMatch ; This function services a buzzer turn off time
                                                                 ; down counter and turns off the buzzer when the
                                                                 ; count becomes zero.
                                ACALL _ChaneEggStateOnDelayMatch ; This is another function that services the
                                                                 ; egg activity state toggle down counter
                                                                 ; and toggles the activity state of the egg when
                                                                 ; the count becomes zero.
                                ACALL AAdvanceSnakeOnDelayMatch ; This function is the icon of the suite. The entire
                                                                ; game is controlled by the logic embedded in this
                                                                ; function. This function is only informed last
                                                                ; about the game time step event.
                                ; NOTE: If the order of calling of the last two functions are interchanged, then
                                ;       during an exceptional situation when the last count of the down counters
                                ;       serviced by both these functions are in coherence, the snake will still
                                ;       snatch the egg if the egg is in snake's path.
                                ; This will be liberal game design policy and we don't do it!!!
                                RET











; Function:  _NotifyGameEnd
; Arguments: None
; Returns:   None

; Description:

; Initiates the sequence of events at the game end

_NotifyGameEnd:
                                ACALL _NotifyGameFreeze ; Since the seq. of operations to be performed are 
                                                        ; similat to that of a game freeze event,
                                                        ; simply he game freeze function is called
                                RET













; Function:  _ScanGameActionKeys
; Arguments: None
; Returns:   None

; Description:

; This function scans the game action keys (Right, Up, Left and Down) and registers the
; press state of these in a global variable NEXT_DIR_OF_ADVANCE. This function is called
; before the game time step function.

_ScanGameActionKeys:           
                                MOV A, NEXT_DIR_OF_ADVANCE ; Test whether any game action key had been already registered.
                                JZ ScanRightActionKey_ScanGameActionKeys ; If not, then jump to the block that scans the
                                                                         ; Right action key.
                                              ; If yes, then it means a previous key action was left
                                              ; unprocessed. The key processing block is expected to clear
                                              ;  this variable ie set it's val. to the const. SNAKE_DIR_IMPLICIT
                                              ; after processing the key.
                                RET
ScanRightActionKey_ScanGameActionKeys: ; Block that scans the Right Action Key.
                                MOV A, #RIGHT_KEY_MASK ; Get the state of
                                LCALL ADetectedDepressedStateOfKeyMaskInA ; Right Action Key
                                JZ ScanUpActionKey_ScanGameActionKeys ; If not pressed, check the state of Up Action Key.
                                MOV NEXT_DIR_OF_ADVANCE, #SNAKE_DIR_RIGHT ; If pressed, update NEXT_DIR_OF_ADVANCE and
                                RET
ScanUpActionKey_ScanGameActionKeys: ; Block that scans the Up Action Key.
                                MOV A, #UP_KEY_MASK ; Get the state of
                                LCALL ADetectedDepressedStateOfKeyMaskInA ; Up Action Key
                                JZ ScanLeftActionKey_ScanGameActionKeys ; If not pressed, check the state of Up Action Key.
                                MOV NEXT_DIR_OF_ADVANCE, #SNAKE_DIR_UP ; If pressed, update NEXT_DIR_OF_ADVANCE and
                                RET
ScanLeftActionKey_ScanGameActionKeys: ; Block that scans the Left Action Key.
                                MOV A, #LEFT_KEY_MASK ; Get the state of
                                LCALL ADetectedDepressedStateOfKeyMaskInA ; Left Action Key
                                JZ ScanDownActionKey_ScanGameActionKeys ; If not pressed, check the state of Up Action Key.
                                MOV NEXT_DIR_OF_ADVANCE, #SNAKE_DIR_LEFT ; If pressed, update NEXT_DIR_OF_ADVANCE and
                                RET
ScanDownActionKey_ScanGameActionKeys: ; Block that scans the Down Action Key.
                                MOV A, #DOWN_KEY_MASK ; Get the state of
                                LCALL ADetectedDepressedStateOfKeyMaskInA ; Down Action Key
                                JZ Return_ScanGameActionKeys ; If not pressed, check the state of Up Action Key.
                                MOV NEXT_DIR_OF_ADVANCE, #SNAKE_DIR_DOWN ; If pressed, update NEXT_DIR_OF_ADVANCE and
Return_ScanGameActionKeys:
                                RET











; Function:  _EndOfGameGfx
; Arguments: None
; Returns:   None

; Description:

; This function alerts the user about the end of the game by flashing the screen
; and intermittently sounding the buzzer twice.

_EndOfGameGfx:
                                MOV A, #GAME_STATE_FLASH_PERIOD ; The total period of a flash is
                                                                ; twice this value (in 100ms).
                                LCALL _TurnOffDisplay ; First, turn off the display 
                                LCALL _TurnOnBuzzer ; and then turn on the buzzer.
                                LCALL _DelayABy100ms ; Wait for half the flash period.
                                LCALL _TurnOnDisplay ; Then turn on the display
                                LCALL _TurnOffBuzzer ; and turn off the buzzer.
                                LCALL _DelayABy100ms ; Again wait for half the flash period.
                                ; We once again repaet the above sequence...
                                LCALL _TurnOffDisplay ; First, turn off the display 
                                LCALL _TurnOnBuzzer ; and then turn on the buzzer.
                                LCALL _DelayABy100ms ; Wait for half the flash period.
                                LCALL _TurnOnDisplay ; Then turn on the display
                                LCALL _TurnOffBuzzer ; and turn off the buzzer.
                                LCALL _DelayABy100ms ; Again wait for half the flash period.

                                LCALL _ClearScreen ; Finally, clear the display buffer
                                RET











; Function:  _DisplayGameScore
; Arguments: None
; Returns:   None

; Description:

; This function displays the game score and is called at the end of the game.

_DisplayGameScore:
                                LCALL _ClearScreen ; Clear the display buffer
                                ACALL _PrintGameScore ; Call the back end function that writes the display
                                                      ; score to the display buffer
                                MOV A, #GAME_SCORE_DISP_TIME ; Display the score
                                LCALL _DelayABy100ms ; for some time
                                LCALL _ClearScreen ; Clear the screen
                                RET