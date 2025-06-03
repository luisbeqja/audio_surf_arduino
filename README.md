# Final project
## Audiosurf Arduino Game

## Overview
This project implements a variant of the classic Audiosurf game for Arduino with a multifunctional shield. 
Players control a spaceship to avoid blocks moving from right to left across a 4-digit 7-segment display.

## Game Description
- **Objective**: Navigate your spaceship to avoid incoming blocks and survive as long as possible
- **Controls**: Use buttons to move spaceship up/down on the leftmost display
- **Lives**: Start with 4 lives (represented by LEDs D1-D4)
- **Progression**: Game speed increases and more blocks spawn as you advance through levels
- **Scoring**: Points awarded for blocks dodged, with level-based multipliers

## Project Structure

```
audiosurf/
├── src/
│   └── main.c              # Main game implementation
├── platformio.ini          # Project configuration
└── README.md              # This documentation
```

### External Dependencies
The project uses the following libraries from the `../libraries/` directory:
- `led/` - LED control functions
- `display/` - 7-segment display management
- `button/` - Button input handling
- `potentiometer/` - Analog input reading
- `usart/` - Serial communication

## Code Organization

### Main Components

#### 1. Game State Management
```c
typedef struct {
    uint8_t level;
    uint8_t lives;
    uint8_t spaceship_position;  // 0-7 for 8-segment display positions
    uint16_t score;
    uint8_t game_running;
    uint32_t blocks_dodged;
} GameState;
```

#### 2. Dynamic Block System
```c
typedef struct Block {
    uint8_t position;  // 0-7 vertical position
    uint8_t column;    // 0-3 horizontal position (display column)
    struct Block* next;
} Block;
```

#### 3. Core Game Functions
- `initGame()` - Initialize game state and allocate memory
- `showTutorial()` - Display game instructions
- `selectLevel()` - Level selection with buttons and potentiometer
- `playGame()` - Main game loop
- `updateGame()` - Game logic updates
- `renderDisplay()` - Visual rendering
- `handleInput()` - Process button inputs
- `gameOver()` - End game and cleanup

## Technical Implementation

### ✅ Requirements Compliance

#### **Timer Usage**
- **Timer1**
- Generates interrupts for:
  - Display refresh (50ms intervals)
  - Game tick timing (level-dependent speed)
- Timer-based game speed progression

#### **Interrupt Implementation**
- **Timer Interrupt** (`TIMER1_COMPA_vect`): Game timing control
- **Pin Change Interrupt** (`PCINT1_vect`): Button press detection
- Non-blocking input handling during gameplay

#### **Dynamic Memory Allocation**
- **Game State**: `malloc(sizeof(GameState))` for main game data
- **Block System**: Dynamic allocation for each block using linked list
- **Memory Management**: Proper `free()` calls to prevent memory leaks
- **Error Handling**: Checks for allocation failures

### Game Flow

#### Phase 1: Game Initialization
1. **Tutorial Display**: Serial port instructions on game controls
2. **Level Selection**: 
   - Potentiometer adjustment (1-10)
   - Button controls (left/right to adjust, middle to confirm)
   - Display shows selected level
   - Random seed generation based on selection timing

#### Phase 2: Gameplay
1. **Spaceship Control**: 
   - Horizontal line on leftmost display represents spaceship
   - Button 1: Move up, Button 3: Move down
   - 8 possible vertical positions

2. **Block Movement**:
   - Blocks spawn on rightmost display
   - Move left one position per game tick
   - Spawn rate and frequency increase with level

3. **Collision Detection**:
   - Spaceship position compared with blocks at leftmost column
   - Collision triggers:
     - Life loss (LED turns off)
     - Spaceship flashing effect
     - Low-frequency beep sound

4. **Level Progression**:
   - Automatic advancement based on blocks dodged
   - Increased game speed and block spawn rate
   - Visual and audio feedback for level up

#### Phase 3: Game Over
1. **End Conditions**: All 4 lives lost
2. **Score Calculation**: `(blocks_dodged * 10) + (level² * 50)`
3. **Statistics Display**: Level reached, blocks dodged, final score
4. **Memory Cleanup**: Free all dynamically allocated memory

### Configuration Options

#### Timing Constants
```c
#define BASE_GAME_SPEED 2000     // Base speed in milliseconds
#define DISPLAY_REFRESH_RATE 50  // Display refresh every 50ms
#define BLOCK_SPAWN_PROBABILITY 30  // Base spawn probability percentage
```

#### Game Parameters
```c
#define MAX_LEVEL 10
#define MAX_LIVES 4
#define SPACESHIP_POSITION_COUNT 8
```

### Build Instructions
```bash
cd audiosurf
pio build
pio upload
```

### Serial Monitor
```bash
pio device monitor
```

## Game Controls

### Level Selection
- **Potentiometer**: Rotate to select level (1-10)
- **Button 1 (Left)**: Decrease level
- **Button 3 (Right)**: Increase level  
- **Button 2 (Middle)**: Confirm selection

### Gameplay
- **Button 1 (Left)**: Move spaceship up
- **Button 3 (Right)**: Move spaceship down
- **Button 2 (Middle)**: Not used during gameplay

### Visual Feedback
- **LEDs D1-D4**: Remaining lives
- **7-Segment Display**: Game field and score
- **Spaceship Flashing**: Collision indication

### Audio Feedback
- **High Beep**: Level progression
- **Low Beep**: Collision/life lost
- **Victory Tune**: Game over sequence


### Memory Management
- Proper allocation and deallocation of dynamic memory
- Error checking for malloc failures
- Cleanup on game over to prevent memory leaks

### Modular Design
- Clear separation of concerns
- Reusable functions for common operations
- Configurable parameters for easy tuning

### Error Handling
- Memory allocation failure detection
- Boundary checking for array access
- Safe pointer operations with null checks

## Future Enhancements

### Potential Improvements
1. **Music Integration**: Add background music playback (i somehow did it at the end)
2. **Power-ups**: Special blocks with beneficial effects
3. **High Score System**: Persistent score storage in EEPROM
4. **Multiple Spaceships**: Different spaceship types with unique abilities
5. **Network Play**: Multi-player capabilities via wireless modules

## Author
Luis Beqja 104A