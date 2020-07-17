;
;  C51 Snakes ~~~~~~~~~
;
;  Created by Sumit chouhan and vivek shah
;  Project Started:   8/07/11, 
;  Project Completed: 15/07/11, 

; File:       C51Snakes.asm
; Created by: Sumit chouhan and vivek shah

$TITLE ('C51Snakes')

; This file defines the Main Block and the entry point of the app.

; The app. uses polling technique to sequentially montior the state of the
; keys and then bring about the appropriate response. Since there are no blocking
; modules and each module will complete executuion within a deterministic time
; period, polling tech. proves more efficient over others in implementing this logic!!!





ORG 0000H ; System reset sets PC to this addr.
                                SJMP ResetRecoveryISR ; After a sys. reset, pgm. exe. begins at this inst.
ORG 0003H
                                SJMP ExternalInterrupt0ISR
ORG 000BH ; Sets PC to this addr. when a Timer 0 overflow signal is raised
                                AJMP DisplayMuxISR ; Jump to the corresponding ISR's block
ORG 0013H
                                SJMP ExternalInterrupt1ISR
ORG 0020H ; All the supporting librarires are place in page one of ROM.
          ; The game library (EggHandle.inc, GameBackEnd.inc and GameFrontEnd.inc) and
          ; C51Snakes.asm will be moved to page two of ROM!!!




$INCLUDE (ConstAndDataDefs.inc) ; Program Conts.s, redefinition of SFRs and Variable defs.
$INCLUDE (SystemPower.inc)      ; Functions that handle system power down & program recovery after a system reset 
$INCLUDE (DisplayMuxing.inc)    ; Function and ISR for Disp. Muxing.
$INCLUDE (SystemFunctions.inc)  ; Fns. to ctrl. disp., buzzer, scan keypad and delay the execution
$INCLUDE (LowLevelGfx.inc)      ; Fuctions that give pixel level access to the disp.
$INCLUDE (HighLevelGfx.inc)     ; Function to clear screen, print text and draw line
$INCLUDE (WelcomeGfx.inc)       ; Functions to display an animation when the console is switched on
$INCLUDE (DelaySettings.inc)    ; Functions to adjust the speed (prop. delay) of the snake
$INCLUDE (StandByState.inc)     ; Fuctions that are called when the app. enters stand by
$INCLUDE (PauseState.inc)       ; Functions that are called when the game is paused

ORG 0801H ; The core game library functions; Beginning of page two of ROM 

$INCLUDE (EggHandle.inc)        ; Functions to handle generation and management of eggs for the snake
$INCLUDE (GameBackEnd.inc)      ; Functions that not directly called from the Main block
                                ; The core game logic is handled by these functions
$INCLUDE (GameFrontEnd.inc)     ; Functions that are called from Main block to notify various states of the game






; Beginning of the Main Block

C51Main:
GameStandBy:
                                LCALL _StandBy ; Enter stand by
GameInit:
                                ACALL _NotifyGameBegin ; Initiate the operations before game begins
                                LCALL _WaitForAllActionKeyRelease
                                MOV PREV_KEY_STATES, #ALL_KEYS_PRESSED ; Force the user to release all keys
                                                                       ; to start processing them in the polling loop
                                LCALL _ResetSystemIdleTime
GameRun:
                                MOV CURR_KEY_STATES, KEYPAD_ACCESS_PORT ; Sample the state of all keys
TestForAnyKeyPress:
                                MOV A, #ANY_KEY_MASK
                                LCALL ADetectedDepressedStateOfKeyMaskInA
                                JNZ BeginKeyScan ; If a key press is detected, start scanning the keys sequentially
                                LCALL AStepSystemIdleTimeAndNotifyOnExpiry ; If not, advance that system idle time
                                JZ EndOfKeyScan         ; If system idle period expires,
                                ACALL _NotifyGameFreeze ; then freeze the game and
                                LCALL _PowerDownSystem  ; initiate system power down

                                ACALL _NotifyGameRestore ; Recover game from power down state 
                                LCALL _WaitForAnyKeyPress ; Wait for a user event
                                MOV CURR_KEY_STATES, #ALL_KEYS_PRESSED
                                SJMP EndOfKeyScan
BeginKeyScan:
                                LCALL _ResetSystemIdleTime
ScanGamePauseKey:
                                MOV A, #PAUSE_KEY_MASK
                                LCALL ADetectedDepressingEdgeOfKeyMaskInA ; Check whether the pause key is hit
                                JZ ScanDelaySettingsKey ; If not, check the state of Delay Settings key
                                ACALL _NotifyGameFreeze ; If yes, initiate a game freeze
                                LCALL AGamePause ; Enter paused state
                                JZ GameEnd; If the user wishes to quit, end the game and
                                          ; display the game score
                                ACALL _NotifyGameRestore ; Else, initiate game resume
                                LCALL _WaitForAnyKeyPress ; After restoring the display to the state when the user
                                                          ; hit the pause key, wait for a user event
                                MOV CURR_KEY_STATES, #ALL_KEYS_PRESSED 
                                SJMP EndOfKeyScan ; Perform no more key processing in this iteration 
ScanDelaySettingsKey:
                                MOV A, #DEL_SET_KEY_MASK
                                LCALL ADetectedDepressingEdgeOfKeyMaskInA ; Check whether the pause key is hit
                                JZ ScanGameActionKeys ; If not, check the state of Game Action Keys
                                ACALL _NotifyGameFreeze ; If yes, initiate a game freeze
                                LCALL _DelaySettings ; Enter the mode that helps the user to adjust the snake's speed
                                ACALL _NotifyGameRestore ; Restore the game once the user has switched back to game mode
                                LCALL _WaitForAnyKeyPress ; After restoring the display to the state when the user
                                                          ; hit the pause key, wait for a user event
                                MOV CURR_KEY_STATES, #ALL_KEYS_PRESSED
                                SJMP EndOfKeyScan ; Perform no more key processing in this iteration

ScanGameActionKeys:
                                ACALL _ScanGameActionKeys
EndOfKeyScan:
                                MOV PREV_KEY_STATES, CURR_KEY_STATES ; Init. PREV_KEY_STATES before next round of processing

                                MOV A, #GAME_TIME_STEP_PERIOD
                                LCALL _DelayABy100ms
                                ACALL ANotifyGameTimeStep ; Initiate a change of game state
                                JZ GameOver ; Collision detected; So, game over!
                                SJMP GameRun ; Loop around to Game Block
GameOver:
                                ACALL _EndOfGameGfx ; Alert user about end of the game
GameEnd:
                                ACALL _NotifyGameEnd
                                ACALL _DisplayGameScore
                                SJMP C51Main ; Loop around to Main Block





END ; End of app.