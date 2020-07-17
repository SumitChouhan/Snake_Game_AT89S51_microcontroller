
; File:       ConstAndDataDefs.inc
; Created by: Sumit Chouhan 

$TT ('ConstAndDataDefs') 

; This file lists the system wide constants, redefined SFRs and global variables used in the app.



;  PROGRAM CONSTANTS

; UNIVERSAL DEFINITIONS

TRUE                                   EQU 0FFH ; Mainly used as a fn. return val. to indicate the success
                                                ; of an action
FALSE                                  EQU 000H ; Mainly used as a fn. return val. to indicate the failure
                                                ; of an action

; CONST.S USED IN SYS. POWER MANAGEMENT

; When the system recovers from a warm reset, the RAM contents remain intact. If a
; warm reset occurs in power down mode, then the program execution is expected
; to resume from the point where the system had entered powerdown. Before entering
; power down, a particular bit pattern is written to the RAM. After a warm reset
; the RAM is scanned to identify this bit pattern. If the system succeeds to identify
; the pattern, the program execution will resume from the point where it stopped earlier.
; If the system fails, that means the warm reset was user initiated. So, the program starts
; executing from the beginning. Four bit patterns are used in this app.

RAM_PATTERN_1                          EQU 10100101B ; Written to R0 before power down.
RAM_PATTERN_2                          EQU 11000011B ; Written to R1 before power down.
RAM_PATTERN_3                          EQU 00100100B ; Written to R2 before power down.
RAM_PATTERN_4                          EQU 01100110B ; Written to R3 before power down.

MAX_SYS_IDLE_TIME_COUNT                EQU 0FFH ; In 100ms. The max. system idle time.
POF_BIT_MASK                           EQU 00010000B ; Mask of POF bit in PCON SFR
PD_BIT_MASK                            EQU 00000010B ; Mask of PD bit in PCON SFR
EXT_INT_0_PIN                          EQU P3.2 ; Mapping of the external interrupt 0 pin
EXT_INT_1_PIN                          EQU P3.3 ; Mapping of the external interrupt 1 pin




; CONSTANTS IN DISP MUXING/DISP BUFFER

DISP_BUF_UND_FLW_ADD                   EQU 01FH ; RAM addr. just before the first byte in disp. buffer.
                                                ; Used in exit condition check in loops 
DISP_BUF_START_ADDR                    EQU 020H ; Starting address of 16byte disp. buff.
DISP_BUF_END_ADDR                      EQU 02FH ; Ending address of 16byte disp. buff.
DISP_BUF_OVR_FLW_ADDR                  EQU 030H ; RAM addr. just after the last byte in disp. buffer.
                                                ; Used in exit condition check in loops

DISP_COL_DATA_HOLD_COUNT               EQU 004H ; in interrupt time period; init. val. a down counter
                                                ; that flags a disp col. switch operation
DISP_MUX_TIMER_TMOD_VAL                EQU 00000010B ; Mode 2 selected; Timer 0 operates as an 8 bit auto. reload timer
DISP_MUX_TIMER_RST_VAL                 EQU 000H ; Init. val. of Timer Reg.s
DISP_COL_SEL_SHAD_REG_LOW_INIT_VAL     EQU 00000001B ; Init val. of low order byte of 16 bit disp. col. shadow. reg.  
DISP_COL_SEL_SHAD_REG_HIGH_INIT_VAL    EQU 00000000B ; Init val. of high order byte of 16 bit disp. col. shadow. reg.
BLANK_DISP_COL_DATA_REG                EQU 0FFH ; When this val. is flushed into the disp. row. feed. reg. (port)
                                                ; they are inactivated. Since the PORT uses active LOW logic 
BLANK_DISP_BUF                         EQU 000H ; This val. is flushed into the any of the disp. buff. reg.s, if
                                                ; the corresponding col. in the disp. is to be blanked; It may
                                                ; be noted that disp. BUFFER reg.s use active HIGH logic
DISP_PORT_OFF                          EQU 0FFH ; When this val. is flushed into the disp. col. feed. reg.s (ports)
                                                ; they are inactivated. Since the PORTS use active LOW logic




; STACK ADDRESSES
                                   
STACK_UND_FLOW_ADD                     EQU 04FH ; RAM addr. just before the first byte in disp. buffer.
                                                ; Not used anywhere in the app. 
STACK_START_ADDR                       EQU 050H ; Starting address of the stack
STACK_END_ADDR                         EQU 07FH ; Ending address of the stack
STACK_OVR_FLW_ADD                      EQU 080H ; RAM addr. just after the last byte in disp. buffer.
                                                ; Not used anywhere in the app.  



; VARIOUS STATES OF THE KEYPAD ACCESS PORT

ALL_KEY_PINS_HIGH                      EQU 00111111B ; This val. indicates the val. of KEY_PAD_ACCESS port
                                                     ; when no keys are pressed. NOTE: The MSB and it's previous
                                                     ; bit don't correspond to any key and it's val. is
                                                     ; immaterial. The implementation must ensure that the
                                                     ; val. of these bits don't alter the result of the
                                                     ; operation
ALL_KEYS_PRESSED                       EQU 00000000B ; This val. indicates the val. of KEY_PAD_ACCESS port
                                                     ; when all keys (Game Action + Delay Settings + Start/Pause Keys)
                                                     ; are pressed. NOTE: The MSB and it's previous
                                                     ; bit don't correspond to any key and it's val. is
                                                     ; immaterial. The implementation must ensure that the
                                                     ; val. of these bits don't alter the result of the
                                                     ; operation




; KEYPAD - PIN MAPPING
                                   
KEYPAD_ACCESS_PORT                     EQU P3 ; Port that samples the key states

; The port pins that are mapped to various app. keys are defined below
                                   
START_KEY                              EQU P3.2 ; Used to start the game from stand by mode
PAUSE_KEY                              EQU P3.2 ; Used to pause a game/ exit pause mode
DEL_SET_KEY                            EQU P3.3 ; Used to switch to delay settings mode while in stand by,
                                                ; pause or during the game; Also used to exit delay settings mode
RIGHT_KEY                              EQU P3.0 ; Used to move the snake right
DECR_DEL_KEY                           EQU P3.0 ; Used to decr. snake delay (incr. snake speed) in del. settings mode
NO_SOFT_KEY                            EQU P3.0 ; Used to quit the game when in pause mode
UP_KEY                                 EQU P3.1  ; Used to move the snake up
LEFT_KEY                               EQU P3.4 ; Used to move the snake left
INCR_DEL_KEY                           EQU P3.4 ; Used to incr. snake delay (decr. snake speed) in del. settings mode
YES_SOFT_KEY                           EQU P3.2 ; Used to start the game when in stand by or cont. the game when in pause mode
DOWN_KEY                               EQU P3.5 ; Used to move the snake down




; MASKS TO EXTRACT VARIOUS KEYS

START_KEY_MASK                         EQU 00000100B
PAUSE_KEY_MASK                         EQU 00000100B
DEL_SET_KEY_MASK                       EQU 00001000B
RIGHT_KEY_MASK                         EQU 00000001B
DECR_DEL_KEY_MASK                      EQU 00000001B
NO_SOFT_KEY_MASK                       EQU 00000001B
UP_KEY_MASK                            EQU 00000010B
LEFT_KEY_MASK                          EQU 00010000B
INCR_DEL_KEY_MASK                      EQU 00010000B
YES_SOFT_KEY_MASK                      EQU 00010000B
DOWN_KEY_MASK                          EQU 00100000B
ANY_GAME_ACTION_KEY_MASK               EQU 00111100B ; Mask to test whether any game action key is pressed
ANY_KEY_MASK                           EQU 00111111B ; Mask to test whether any key is pressed




; SYSTEM CTRL. DEFS
 
DELAY_BETWEEN_KEY_SCANS                EQU 001H ; in 100ms; Various modes of the app. that scans the keypad must
                                                ; ensure this much delay between successive key scans
DISP_CTRL_PORT                         EQU P3 ; Display on/off is also mux. into the keypad access port                         
DISP_CTRL_PIN                          EQU P3.6 ; Pin of the port that can be used to access the display
                                                ; NOTE: ACTIVE LOW LOGIC IS USED
                                                ; 1 => OFF, 0 => ON
BUZZ_CTRL_PORT                         EQU P3 ; Buzzer on/off is also mux. into the keypad access port
BUZZ_CTRL_PIN                          EQU P3.7 ; Pin of the port that can be used to access the buzzer
                                                ; NOTE: ACTIVE LOW LOGIC IS USED
                                                ; 1 => OFF, 0 => ON

; The val.s def. below are based on F_XTAL = 27MHz, T_XTAL = 1/ 2.25 us and the no. of cycles. to exec. the
; inst. in the respective functions. Further these vals. are optimized to give delays as close as possible
; to the spec. val.s

_10_MS_NUM_INNER_DELAY_LOOPS           EQU 012H ; No: of inner loops to be executed by 10ms delay function
_10_MS_NUM_OUTER_DELAY_LOOPS           EQU 004H ; No: of outer loops to be executed by 10ms delay function
_100_MS_NUM_INNER_DELAY_LOOPS          EQU 0B2H ; No: of inner loops to be executed by 100ms delay function
_100_MS_NUM_OUTER_DELAY_LOOPS          EQU 004H ; No: of outer loops to be executed by 100ms delay function




; LOW LEVEL GFX. DEFS

WIDTH_OF_DIGIT                         EQU 005H ; Width of the digit printed by low level gfx. fns. (in disp. pixels)
WHITE_SPACE_CHAR                       EQU 00AH ; Index of the white space character. If this arg. is passed to
                                                ; the low level gfx. fn. that prints a digit, then a white
                                                ; space is printed on the screen




; GRAPHICS/SCREEN DEFS

POS_AXIS_DIR                           EQU 000H ; Indicates +ve axes direction; Used in draw line function(s)
NEG_AXIS_DIR                           EQU 080H ; Indicates -ve axes direction; Used in draw line function(s)
DXY_AXIS_DIR_MASK                      EQU 10000000B ; This mask is used to extract dir. attr. in DXY data
DXY_X_MASK                             EQU 01111000B ; This mask is used to extract X attr. in DXY data
                                                     ; NOTE: The val. extracted by this mask has to be
                                                     ;       right shifted 4 times to obtain the X in
                                                     ;       (X, Y) representation.
DXY_Y_MASK                             EQU 00000111B ; This mask is used to extract Y attr. in DXY data
COORDINATE_MASK                        EQU 01111111B ; This mask is used to extract the coordinate part in DXY data
X_MIN                                  EQU 000H ; Cooresponds to min. X val. in pixels on screen
X_MAX                                  EQU 00FH ; Cooresponds to max. X val. in pixels on screen
Y_MIN                                  EQU 000H ; Cooresponds to min. X val. in pixels on screen
Y_MAX                                  EQU 007H ; Cooresponds to max. X val. in pixels on screen
CIRCLE_BOUND_X_MASK                    EQU 00001111B ; The game uses a circulat coordinate system. That is when
                                                     ; X is incr. beyond X_MAX, it must circle around to X_MIN and
                                                     ; when decr. below X_MIN, it must circle around to X_MAX.
                                                     ; This can achieved by ANDing the result of incr./decr.
                                                     ; with this val.                                                                                         
CIRCLE_BOUND_Y_MASK                    EQU 00000111B ; The game uses a circulat coordinate system. That is when
                                                     ; Y is incr. beyond Y_MAX, it must circle around to Y_MIN and
                                                     ; when decr. below Y_MIN, it must circle around to Y_MAX.
                                                     ; This can achieved by ANDing the result of incr./decr.
                                                     ; with this val.




; CONSTANTS IN WELCOME GRAPHICS

WEL_GFX_PROC_INFO_DISP_DEL             EQU 014H ; in 100ms; Time period for which the processor name is
                                                ; displayed on the screen
WEL_GFX_MAIN_LOOP_INDEX_OVF_COUNT      EQU 033H ; This variable indicates the loop break condition that
                                                ; occurs when the loop variable that points to a segment
                                                ; in the look up table that holds the anim. data
                                                ; over bounds
WEL_GFX_TEXT_PERSIST_DELAY             EQU 008H ; in 10ms; The state of the screen during the second phase
                                                ; of the welcome gfx. anim. persists for so much period.

; DELAY DEFS

MAX_SNAKE_DELAY                        EQU 00CH ; in 100ms; Max. allowed delay b/w two successive snake advancements
                                                ; Decelaration can get around this limit!!!
MIN_SNAKE_DELAY                        EQU 002H ; in 100ms; Min. allowed delay b/w two successive snake advancements
                                                ; Decelaration can get around this limit!!!
INIT_SNAKE_DELAY                       EQU 007H ; in 100ms; Init. delay b/w two successive snake advancements (when
                                                ; the app. starts)

; DELAY SET. CONSTS.

MAX_DEL_SET_IDLE_TIME                  EQU 064H ; in 100ms; If no key is pressed within this period, the app.
                                                ; auto. switches to the previous state
HALF_THE_DEL_SET_TIME_STEP_PERIOD      EQU 001H ; in 100ms; Delay settings uses custom pause period b/w key
                                                ; scans and not the standard global time used in other
                                                ; contexts. Moreover, it may be noted thatn the actual
                                                ; pause time is twice this value!!!
DELAY_BAR_GRAPH_X_MIN                  EQU 002H ; Absicca of the delay bar graph when the user has opted for zero speed
                                                ; Corresponds to MIN_DELAY and no pixel in the bar graph is highlighted
                                                ; in this setting
DELAY_BAR_GRAPH_Y                      EQU 000H ; Ordinate of the bar graph in delay settings mode that indicates
                                                ; the speed of snake to the user
TAIL_INIT_X_OF_SNAKE_DEL_SET           EQU 000H ; Initial abisissa of tail of snake coordinate in del. settings mode
HEAD_INIT_X_OF_SNAKE_DEL_SET           EQU 003H ; Initial abisissa of head of snake coordinate in del. settings mode
Y_OF_SNAKE_SET_MODE                    EQU 002H ; Ordinate of haed and tail of snake coordinates in del. settings mode
                                                ; Since Y = const., snake is init. parallel to X_AXIS
                                                ; NOTE: Since the library function draw line is not used anywhere
                                                ; in settings mode and also since the snake direction is consistent,
                                                ; no init. D param. is specified in this mode. 
ECCLIPSED_PIXEL_X_DEL_SET              EQU 00EH ; Absicca of the pixel in the del. settings. background that is
                                                ; ecclipsed by the aux. snake as it moves on the screen
ECCLIPSED_PIXEL_Y_DEL_SET              EQU 002H ; Ordinate of the pixel in the del. settings. background that is
                                                ; ecclipsed by the aux. snake as it moves on the screen

; GAME CONSTANTS

; INIT. VALUES

INIT_GAME_SCORE                        EQU 000H ; Initial score is of course zero

; These val.s rep. the initial loc. of the snake when the Game Begins

INIT_AXIS_DIR_OF_SNAKE                 EQU POS_AXIS_DIR ; Init axis. dir. of propagation of snake
INIT_TAIL_X_OF_SNAKE                   EQU 000H ; Initial abisissa of tail of snake coordinate when the Game begins
INIT_HEAD_X_OF_SNAKE                   EQU 003H ; Initial abisissa of head of snake coordinate when the Game begins
INIT_Y_OF_SNAKE                        EQU 003H ; Initial ordinate of head and tail of snake coordinates
                                                ; when the Game begins
                                                ; Since Y = const., snake is init. parallel to X_AXIS

; QUEUE ADDRESSES 

; The snake is represented in the form a circular queue. The queue can be used to traverse through all
; coordinates of the snake on the screen. Each element in the queue contains the coordinates of
; the terminal point of a segment of the snake in DXY format. The part of the snake b/w two bends
; is a segment. Addresses of the head and tail of queue are held in two ponters. The size of the queue
; is 32 bytes; To traverse the snake, begin from the tail; The tail coordinates is the starting
; point and the last but one element in the queue (next byte in RAM) is the ending point of the tail.
; Then last but one and the last but elements are respectively the starting and terminal points of the
; last but one segment of the snake, assuming the snake is traversed from the tail; Thus first element
; of the queue (the head) is the terminal point of the last segment of the snake. A simple algorithm to print
; the snake on the screen from the circular queue is given below:-
;
; P0 = TAIL OF SNAKE
; LOOP:
;     P1 = P0 - 1th ELEMENT OF QUEUE (FIRST ELEMENT IS THE HEAD OF QUEUE)
;      D = DIR. ATTRIBUTE OF DATA AT P0
;     X0 = X OF DATA AT P0
;     Y0 = Y OF DATA AT P0
;     X1 = X OF DATA AT P1
;     Y1 = Y OF DATA AT P1
;     DRAW LINE IN AXID DIR. (+VE IF D = 0, -VE OTHERWISE) FROM (X0, Y0) TO (X1, Y1)
;     IF P1 = HEAD OF SNAKE, EXIT, ELSE
;     PO = P1
;     GOTO LOOP 

QUEUE_UND_FLW_ADD                      EQU 02FH ; RAM addr. just before the first byte of the circular queue that
                                                ; rep. the snake; May be used to circle around to end of the queue
                                                ;  in loops or during queue traversal
QUEUE_START_ADDR                       EQU 030H ; RAM addr. of the first byte of the circlular queue that reps.
                                                ; the snake
QUEUE_END_ADDR                         EQU 04FH ; RAM addr. of the last byte of the circlular queue that reps.
                                                ; the snake
QUEUE_OVR_FLW_ADDR                     EQU 050H ; RAM addr. just after the last byte of the circular queue that
                                                ; rep. the snake; May be used to circle around to the beginning of
                                                ; the queue in loops or during queue traversal

; GAME ACTION DIRECTIONS

SNAKE_DIR_IMPLICIT                     EQU 000H ; This const. indicates that the user has made no dir. change
                                                ; req. so far and defaults to current dir. of movement of
                                                ; the snake
SNAKE_DIR_RIGHT                        EQU 001H ; Indicates right direction of movement of snake
SNAKE_DIR_UP                           EQU 002H ; Indicates upward direction of movement of snake
SNAKE_DIR_LEFT                         EQU 003H ; Indicates left direction of movement of snake
SNAKE_DIR_DOWN                         EQU 004H ; Indicates downward direction of movement of snake

; CONSTS. IN EGG MANAGEMENT

RAND_EGG_LOC_GEN_TIMER_TMOD_VAL        EQU 00100000B ; Mode 1 selected; Timer 1 operates as a 16 bit timer in this mode
RAND_EGG_LOC_GEN_TIMER_REG_RST_VAL     EQU 000H ; Timer regs. that ganerate a rand. loc. for the egg coordinates
                                                ; are init. to this val.
EGG_DELAY_SCALE_FACTOR                 EQU 014H ; in mul. of snake delay!!!
                                                ; This value must fit in a byte taking into
                                                ; account the max. snake delay!!!
                                                ; The activity state of the egg is toggled when this period expires
EGG_SNATCH_BUZZER_SOUND_DELAY          EQU 002H ; in 100ms; When the snake snatches an egg, the buzzer
                                                ; is sounded for this period

; GAME EXIT DEFS.

GAME_STATE_FLASH_PERIOD                EQU 008H ; in 100ms; The last state of the snake when a collision occurs
                                                ; there by ending the game is flashed twice on the screen
                                                ; The display flash period is twice this val. and of 50% duty
                                                    

; GAME SCORE CONSTS.

GAME_SCORE_UPPER_BOUND                 EQU 064H ; Since the implementation has allotted only two digits
                                                ; on the screen to display the game score, the actual score is
                                                ; first muduloed with this value before processing. This sets the
                                                ; max. possible game score to 99!!! However, the constraints
                                                ; imposed by the gaming console may not permit the score to approach
                                                ; even somewhere close to this val.!!!
DECIMAL_BASE                           EQU 00AH ; Base of the decimal system (= 10); Used to extract Tens and Unit
                                                ; digit of the Game Score
X_GAME_SCORE_TENS_DIGIT                EQU 002H ; Absicca of the Tens Digit of the Game Score on the screen
X_GAME_SCORE_UNIT_DIGIT                EQU 009H ; Absicca of the Unit Digit of the Game Score on the screen
GAME_SCORE_DISP_TIME                   EQU 020H ; in 100ms; Time period to disp. to Game Score on the screen

; GAME TIMING

GAME_TIME_STEP_PERIOD                  EQU 001H ; in 100ms; The state of the game is changed
                                                ; every time this period is lapsed





;  SFRS

;  GENERAL PURPOSE SFRS

ACCL                                   EQU 0E0H ; Addr. of accululator; Used in stack operation to back up Accl.

; THE FOLLOWING DEFS. ARE VALID ONLY IF REG. BANK 1 IS USED ie RS0 = 0; RS1 = 0
; These def.s help in saving the reg.s to the stack durin a task s/w, fn. call

REG_0                                  EQU 000H ; Addr. of R0
REG_1                                  EQU 001H ; Addr. of R1
REG_2                                  EQU 002H ; Addr. of R2
REG_3                                  EQU 003H ; Addr. of R3
REG_4                                  EQU 004H ; Addr. of R4
REG_5                                  EQU 005H ; Addr. of R5
REG_6                                  EQU 006H ; Addr. of R6
REG_7                                  EQU 007H ; Addr. of R7




; SFRS FOR DISP MUXING

DISP_MUX_TIMER_HIGH_REG                EQU TH0 ; Redef. to Timer 0 high reg.
DISP_MUX_TIMER_LOW_REG                 EQU TL0 ; Redef. to Timer 0 low reg.
DISP_COLUMN_DATA_REG                   EQU P0 ; Disp. row feed port
DISP_COLUMN_SELECT_REG_LOW             EQU P1 ; Lower order byte(port) of 16 bit. disp. col. feed reg
DISP_COLUMN_SELECT_REG_HIGH            EQU P2 ; Higher order byte(port) of 16 bit. disp. col. feed reg

; SFRS IN EGG MANAGEMENT

RAND_EGG_LOC_GEN_TIMER_REG_HIGH        EQU TH1 ; Redef. to Timer 1 high reg.
RAND_EGG_LOC_GEN_TIMER_REG_LOW         EQU TL1 ; Redef. to Timer 1 low reg.
                                               ; Timer 1 is used as a free running 16 bit timer, the
                                               ; low order byte of which is sampled and used to
                                               ; specify the random co-ordinates of the egg
                                               ; A question may arise: Why not use the Timer 0,
                                               ; which is already and always running anyway?
                                               ; Ans:
                                               ; My game, my rules!!!





; PROGRAM VARIABLES

; VARIABLES FOR SYS. POWER ADMIN.

SYS_IDLE_TIME_COUNT                    EQU 008H ; This variable is a counter that is incremented as
                                                ; the system idle time grows. The system is said to be
                                                ; idle if the user leaves the keys undisturbed. When this
                                                ; counter rolls over, the gaming console hibernates!

; VARIABLES FOR DISPLAY MUXING

DISP_COL_DATA_HOLD_COUNT_REG           EQU 009H ; This reg. is decr. each time a Timer0 interrupt occurs
                                                ; Disp. col. switch takes place when it becomes zero
TIMER_OVERFLOW_FLAG_REG                EQU 00AH ; This reg is set to true just before the Disp. Mux. ISR returns
DISP_COLUMN_DATA_PTR                   EQU 00BH ; This ptr. holds. the addr. of the byte in the disp. buff.
                                                ; that is to be mirrored into the dosp. row feed reg
                                                ; durin the next disp. col. s/w event
DISP_COL_SEL_SHADOW_REG_LOW            EQU 00CH ; Reg. that shadows the lower byte of the 16 bit disp. col. sel. reg. 
DISP_COL_SEL_SHADOW_REG_HIGH           EQU 00DH ; Reg. that shadows the higher byte of the 16 bit disp. col. sel. reg.




; VARIABLE TO HOLD THE PREV STATE OF KEYS

PREV_KEY_STATES                        EQU 00EH ; Usually set to CURR_KEY_STATES immediately after a
                                                ; key scan loop
CURR_KEY_STATES                        EQU 00FH ; Usually init. immediately before a key scan loop




; VARIABLES USED IN GRAPHICS (Heavily used as fn. args and ret. val.s in low and high level gfx. fn.s)

DXY                                    EQU 018H ; D: the MSB; 0 => +ve axes dir.; 1 => -ve axis dir.
                                                ; X: bits 6 to 3: rep. absissa val. (0 - 15) on disp.
                                                ; Y: bits 2 to 0: rep. ordinate val. (0 - 7) on disp.
D                                      EQU 019H ; Separate var. to hold D val. alone
X                                      EQU 01AH ; Separate var. to hold X val. alone
                                                ; When this var holds X, the data is right justified as usual
                                                ; in contrast to the X in DXY
Y                                      EQU 01BH ; Separate var. to hold Y val. alone
X0                                     EQU 01CH ; Usually sepc. the starting absicca of a line; right justified rep.
Y0                                     EQU 01DH ; Usually sepc. the starting ordinate of a line; right justified rep.
X1                                     EQU 01EH ; Usually sepc. the ending absicca of a line
Y1                                     EQU 01FH ; Usually sepc. the ending ordinate of a line




; GAME VARIABLES

SNAKE_DELAY                            EQU 010H ; This var. holds the period after which the snake is advanced 
TAIL_OF_SNAKE_PTR                      EQU 011H ; Ptr. to the coordinates of the tail of snake(in DXY format) in the queue
HEAD_OF_SNAKE_PTR                      EQU 012H ; Ptr. to the coordinates of the head of snake(in DXY format) in the queue
GAME_SCORE                             EQU 013H ; Holds the game score; incr. whenever the snake snatches an egg
EGG_ACTIVITY_STATE                     EQU 014H ; Indicates the state of the egg
                                                ; TRUE => egg is active ie currently disp. on screen
                                                ; FALSE => egg is inactive ie not displayed on the screen
EGG_LOCATION                           EQU 015H ; Coordinates of the egg currently in DXY format
CURR_DIR_OF_ADVANCE                    EQU 016H ; Holds the curr dir of the snake; Right, Up, Left or Down
NEXT_DIR_OF_ADVANCE                    EQU 017H ; Defaults to implicit dir.
                                                ; Set to one of the four snake dir. const.s when the correponding
                                                ; action key is hit        