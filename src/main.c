#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../libraries/usart/usart.h"
#include "../libraries/usart/led/led.h"
#include "../libraries/display/display.h"
#include "../libraries/button/button.h"
#include "../libraries/potentiometer/potentiometer.h"

// Game configuration
#define MAX_LEVEL 10
#define INITIAL_LEVEL 1
#define MAX_LIVES 4
#define DISPLAY_WIDTH 4
#define SPACESHIP_POSITION_COUNT 8
#define BLOCK_SPAWN_PROBABILITY 30  // Percentage chance per level

// Button definitions (based on the button library using PC1, PC2, PC3)
#define BUTTON_1 1  // Left button
#define BUTTON_2 2  // Middle button  
#define BUTTON_3 3  // Right button

// Display position definitions
#define DISPLAY_POS_1 0
#define DISPLAY_POS_2 1
#define DISPLAY_POS_3 2
#define DISPLAY_POS_4 3

// Timing constants (in timer ticks)
#define TIMER_PRESCALER 1024
#define TIMER_FREQUENCY (F_CPU / TIMER_PRESCALER)
#define BASE_GAME_SPEED 800  // Base speed in milliseconds (reduced from 2000 for faster movement)
#define DISPLAY_REFRESH_RATE 50  // Display refresh every 50ms
#define FLASH_DURATION 500  // Flash duration for collision

// Buzzer control macro - can be disabled for testing
#define BUZZER_PIN PD3
#define BUZZER_ENABLED 1

// Add frequency definitions
#define HIGH_TONE 880.00  // A5
#define LOW_TONE 523.250  // C5

// Game state structure
typedef struct {
    uint8_t level;
    uint8_t lives;
    uint8_t spaceship_position;  // 0-7 for 8-segment display positions
    uint16_t score;
    uint8_t game_running;
    unsigned long blocks_dodged;  // Changed to unsigned long to match printf format
} GameState;

// Block structure for dynamic memory allocation
typedef struct Block {
    uint8_t position;  // 0-7 vertical position
    uint8_t column;    // 0-3 horizontal position (display column)
    struct Block* next;
} Block;

// Global variables
static GameState* g_game_state = NULL;  // Pointer demonstration
static Block* g_block_list = NULL;      // Dynamic memory allocation
static volatile uint16_t g_timer_counter = 0;
static volatile uint8_t g_display_refresh_flag = 0;
static volatile uint8_t g_game_tick_flag = 0;
static volatile uint8_t g_button_pressed = 0;
static volatile uint8_t g_collision_flash = 0;
static uint8_t g_display_buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};  // Global display buffer for multiplexing
static volatile uint8_t g_current_column = 0;  // Current column being displayed

// Function prototypes
void initGame(void);
void initTimers(void);
void initInterrupts(void);
void initBuzzer(void);
void showTutorial(void);
void selectLevel(void);
void playGame(void);
void updateGame(void);
void renderDisplay(void);
void handleInput(void);
void spawnBlocks(void);
void moveBlocks(void);
void checkCollisions(void);
void addBlock(uint8_t position, uint8_t column);
void removeBlock(Block* block);
void clearAllBlocks(void);
void gameOver(void);
void playVictoryTune(void);
void updateGameStateByReference(GameState* state, uint8_t new_level);  // Pointer demonstration
uint16_t calculateScore(uint8_t level, unsigned long blocks_dodged);
void displayGameInfo(void);
void playBeep(void);
void playLowBeep(void);
void playTone(float frequency, uint32_t duration);

// Timer interrupt for game timing
ISR(TIMER1_COMPA_vect) {
    g_timer_counter++;
    
    // High-frequency display multiplexing (every ~2ms for smooth display)
    if (g_timer_counter % 2 == 0) {
        // Display current column
        writeRawToSegment(g_current_column, g_display_buffer[g_current_column]);
        
        // Move to next column
        g_current_column = (g_current_column + 1) % 4;
    }
    
    // Display refresh (every ~50ms) - now just updates the buffer content
    if (g_timer_counter % (TIMER_FREQUENCY * DISPLAY_REFRESH_RATE / 1000) == 0) {
        g_display_refresh_flag = 1;
    }
    
    // Game tick (speed depends on level)
    uint16_t game_speed = BASE_GAME_SPEED - (g_game_state->level * 60);  // Reduced from 150 for more gradual speed increase
    if (game_speed < 150) game_speed = 150;  // Reduced minimum speed from 300 to 150ms for faster gameplay
    
    if (g_timer_counter % (TIMER_FREQUENCY * game_speed / 1000) == 0) {
        g_game_tick_flag = 1;
    }
}

// Button interrupt handler
ISR(PCINT1_vect) {
    static uint8_t last_button_state = 0xFF;
    uint8_t current_state = PINC & 0x0F;  // Read PC0, PC1, PC2, PC3
    
    // Detect button press (falling edge)
    if ((last_button_state & ~current_state) != 0) {
        g_button_pressed = 1;
    }
    
    last_button_state = current_state;
}

int main(void) {
    // Initialize all systems
    initUSART();
    
    // Enable buttons (using the button library functions)
    enableButton(BUTTON_1);
    enableButton(BUTTON_2);
    enableButton(BUTTON_3);
    
    initADC();  // Initialize potentiometer ADC
    initDisplay();
    enableAllLeds();
    lightDownAllLeds();
    initBuzzer();
    initTimers();
    initInterrupts();
    
    printf("=== AUDIOSURF ARDUINO ===\n");
    printf("Welcome to Audiosurf!\n\n");
    
    // Main game loop
    while (1) {
        initGame();
        showTutorial();
        selectLevel();
        playGame();
        gameOver();
        
        printf("\nPress any button to play again...\n");
        while (!g_button_pressed) {
            _delay_ms(100);
        }
        g_button_pressed = 0;
        _delay_ms(500);  // Debounce
    }
    
    return 0;
}

void initGame(void) {
    // Dynamic memory allocation for game state
    if (g_game_state != NULL) {
        free(g_game_state);
    }

    // Dynamic memory allocation for game state
    // uses sizeof(GameState) to know how much memory bytes to allocate
    g_game_state = (GameState*)malloc(sizeof(GameState));
    
    if (g_game_state == NULL) {
        printf("Error: Could not allocate memory for game state!\n");
        while(1);  // Halt on memory allocation failure
    }
    
    // Initialize game state
    g_game_state->level = INITIAL_LEVEL;
    g_game_state->lives = MAX_LIVES;
    g_game_state->spaceship_position = 4;  // Middle position
    g_game_state->score = 0;
    g_game_state->game_running = 1;
    g_game_state->blocks_dodged = 0;
    
    // Clear any existing blocks
    clearAllBlocks();
    
    // Reset flags
    g_timer_counter = 0;
    g_display_refresh_flag = 0;
    g_game_tick_flag = 0;
    g_button_pressed = 0;
    g_collision_flash = 0;
    
    printf("Game initialized. Memory allocated for game state.\n");
}

void initTimers(void) {
    // Configure Timer1 for game timing (CTC mode)
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);  // CTC mode, prescaler 1024
    OCR1A = TIMER_FREQUENCY / 1000;  // 1ms intervals
    TIMSK1 |= (1 << OCIE1A);  // Enable compare match interrupt
}

void initInterrupts(void) {
    // Enable pin change interrupts for buttons
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT8) | (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11);  // PC0, PC1, PC2, PC3
    
    sei();  // Enable global interrupts
}

void initBuzzer(void) {
    #if BUZZER_ENABLED
    DDRD |= (1 << BUZZER_PIN);  // Set buzzer pin as output
    PORTD |= (1 << BUZZER_PIN);  // Initialize high (buzzer off)
    #endif
}

void showTutorial(void) {
    printf("\033[2J\033[H"); // Clear screen and move cursor to home
    printf("\n=== GAME TUTORIAL ===\n");
    printf("How to play Audiosurf:\n");
    printf("1. Use buttons to move your spaceship up/down\n");
    printf("   - Button 1 (left): Move up\n");
    printf("   - Button 3 (right): Move down\n");
    printf("   - Button 2 (middle): Confirm level selection\n");
    printf("2. Avoid the blocks coming from the right\n");
    printf("3. Your spaceship is shown on the leftmost display\n");
    printf("4. Blocks move from right to left each game tick\n");
    printf("5. You have 4 lives (shown by LEDs D1-D4)\n");
    printf("6. Game speeds up as you progress through levels\n");
    printf("7. Score is based on blocks dodged and level reached\n\n");
    
    printf("Press any button to continue...\n");
    
    // Welcome text animation on 8-segment display
    char* welcome_text = "LUIS"; // Static text to display
    writeString(welcome_text);
    
    while (!g_button_pressed) {
        _delay_ms(100);  // Small delay to prevent busy waiting
    }
    
    g_button_pressed = 0;
    _delay_ms(500);
}

void selectLevel(void) {
    printf("\033[2J\033[H"); // Clear screen and move cursor to home
    printf("\n=== LEVEL SELECTION ===\n");
    printf("Use pot/buttons: level (1-%d)\n", MAX_LEVEL);
    printf("Press middle button to confirm\n\n");
    
    uint8_t selected_level = INITIAL_LEVEL;
    unsigned long seed_counter = 0;
    uint16_t last_pot_value = readADC();  // Initialize with current potentiometer value
    uint8_t last_pot_level = (last_pot_value * MAX_LEVEL) / 1023 + 1;
    selected_level = last_pot_level;  // Start with potentiometer position
    printf("Level: %d\n", selected_level);
    
    while (1) {
        seed_counter++;  // For random seed generation
        
        // Read potentiometer for level selection - only update if significantly changed
        uint16_t pot_value = readADC();
        uint8_t pot_level = (pot_value * MAX_LEVEL) / 1023 + 1;  // Map 0-1023 to 1-MAX_LEVEL
        
        // Only update from potentiometer if there's a significant change (more than 50 ADC units)
        // This prevents noise from constantly changing the selection
        if (abs(pot_value - last_pot_value) > 50 && pot_level != selected_level) {
            selected_level = pot_level;
            last_pot_value = pot_value;
            last_pot_level = pot_level;
            printf("Level: %d (pot)\n", selected_level);
        }
        
        // Check button presses - these take priority over potentiometer
        if (g_button_pressed) {
            if (buttonPushed(BUTTON_1)) {  // Left button - decrease level
                if (selected_level > 1) {
                    selected_level--;
                    printf("Level: %d (btn)\n", selected_level);
                    // Update potentiometer tracking to prevent immediate override
                    last_pot_value = pot_value;
                    last_pot_level = selected_level;
                }
            } else if (buttonPushed(BUTTON_3)) {  // Right button - increase level
                if (selected_level < MAX_LEVEL) {
                    selected_level++;
                    printf("Level: %d (btn)\n", selected_level);
                    // Update potentiometer tracking to prevent immediate override
                    last_pot_value = pot_value;
                    last_pot_level = selected_level;
                }
            } else if (buttonPushed(BUTTON_2)) {  // Middle button - confirm
                break;
            }
            
            g_button_pressed = 0;
            _delay_ms(200);  // Debounce
        }
        
        // Update display buffer to show selected level (compatible with timer interrupt multiplexing)
        // Reset all display segments to off
        for (uint8_t i = 0; i < 4; i++) {
            g_display_buffer[i] = 0xFF;  // All segments off
        }
        
        // Convert selected_level to individual digits and display using SEGMENT_MAP
        extern const uint8_t SEGMENT_MAP[];  // Access the segment map from display library
        
        // Show level number starting from the leftmost position
        if (selected_level >= 10) {
            g_display_buffer[0] = SEGMENT_MAP[selected_level / 10];    // Tens digit
            g_display_buffer[1] = SEGMENT_MAP[selected_level % 10];    // Units digit
        } else {
            g_display_buffer[0] = SEGMENT_MAP[selected_level];         // Units digit only
        }
        
        _delay_ms(50);
    }
    
    // Use seed counter for random generation
    srand(seed_counter);
    
    // Update game state using pointer (demonstration of pass by reference)
    updateGameStateByReference(g_game_state, selected_level);
    
    printf("Starting level %d! (Seed: %lu)\n", selected_level, seed_counter);
    _delay_ms(1000);
}

void playGame(void) {
    printf("\n=== GAME START ===\n");
    printf("Avoid the blocks! Good luck!\n\n");
    
    // Show initial lives
    for (uint8_t i = 0; i < g_game_state->lives; i++) {
        lightUpLed(i);
    }
    
    while (g_game_state->game_running && g_game_state->lives > 0) {
        // Handle display refresh
        if (g_display_refresh_flag) {
            renderDisplay();
            g_display_refresh_flag = 0;
        }
        
        // Handle game tick
        if (g_game_tick_flag) {
            updateGame();
            g_game_tick_flag = 0;
        }
        
        // Handle input
        if (g_button_pressed) {
            handleInput();
            g_button_pressed = 0;
        }
        
        // Handle collision flash
        if (g_collision_flash > 0) {
            g_collision_flash--;
            if (g_collision_flash == 0) {
                // Stop flashing
            }
        }
        
        _delay_ms(1);  // Small delay to prevent busy waiting
    }
}

void updateGame(void) {
    moveBlocks();
    spawnBlocks();
    checkCollisions();
    
    // Level progression based on blocks dodged
    uint8_t new_level = (g_game_state->blocks_dodged / 10) + g_game_state->level;
    if (new_level > g_game_state->level && new_level <= MAX_LEVEL) {
        g_game_state->level = new_level;
        printf("Level up! Now at level %d\n", g_game_state->level);
        playBeep();
    }
    
    displayGameInfo();
}

void renderDisplay(void) {
    // Reset display buffer for each column
    for (uint8_t i = 0; i < 4; i++) {
        g_display_buffer[i] = 0xFF;  // All segments off initially
    }
    
    // Render spaceship on leftmost display (position 0) with flicker effect
    uint8_t show_spaceship = 0;
    
    if (g_collision_flash > 0) {
        // During collision, flash rapidly
        show_spaceship = (g_collision_flash % 10 < 5);
    } else {
        // Normal flicker - spaceship blinks every ~150ms for visibility
        show_spaceship = ((g_timer_counter / 25) % 2) == 0; 
    }
    
    if (show_spaceship) {
        // Show spaceship
        // Map spaceship position (0-7) to a simple pattern
        uint8_t spaceship_pattern = 0x01 << g_game_state->spaceship_position;
        g_display_buffer[DISPLAY_POS_1] &= ~spaceship_pattern;  // Combine with existing pattern
    }
    
    // Render blocks - combine all blocks for each column
    Block* current = g_block_list;
    while (current != NULL) {
        if (current->column < DISPLAY_WIDTH) {
            // Combine block pattern with existing pattern for this column
            uint8_t block_pattern = 0x01 << current->position;
            g_display_buffer[current->column] &= ~block_pattern;  // Combine patterns (AND operation for common cathode)
        }
        current = current->next;
    }
    
    // No need to write to display here - the timer interrupt handles multiplexing automatically
}

void handleInput(void) {
    if (buttonPushed(BUTTON_1)) {  // Left button - move up
        if (g_game_state->spaceship_position > 0) {
            g_game_state->spaceship_position--;
        }
    } else if (buttonPushed(BUTTON_3)) {  // Right button - move down
        if (g_game_state->spaceship_position < SPACESHIP_POSITION_COUNT - 1) {
            g_game_state->spaceship_position++;
        }
    }
    
    _delay_ms(100);  // Debounce
}

void spawnBlocks(void) {
    // Spawn probability increases with level
    uint8_t spawn_chance = BLOCK_SPAWN_PROBABILITY + (g_game_state->level * 5);
    if (spawn_chance > 80) spawn_chance = 80;  // Cap at 80%
    
    // Potentially spawn multiple blocks
    uint8_t max_spawns = (g_game_state->level / 3) + 1;
    
    for (uint8_t i = 0; i < max_spawns; i++) {
        if ((rand() % 100) < spawn_chance) {
            uint8_t position = rand() % SPACESHIP_POSITION_COUNT;
            addBlock(position, DISPLAY_WIDTH - 1);  // Spawn at rightmost column
        }
    }
}

void moveBlocks(void) {
    Block* current = g_block_list;
    Block* prev = NULL;
    
    while (current != NULL) {
        current->column--;
        
        // Remove blocks that have moved off screen
        if (current->column == 255) {  // Underflow indicates off-screen
            g_game_state->blocks_dodged++;
            g_game_state->score += 10 * g_game_state->level;
            
            if (prev == NULL) {
                g_block_list = current->next;
            } else {
                prev->next = current->next;
            }
            
            Block* to_remove = current;
            current = current->next;
            free(to_remove);  // Dynamic memory deallocation
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void checkCollisions(void) {
    Block* current = g_block_list;
    
    while (current != NULL) {
        // Check collision with spaceship (column 0)
        if (current->column == 0 && current->position == g_game_state->spaceship_position) {
            // Collision detected!
            g_game_state->lives--;
            g_collision_flash = 50;  // Flash for 50 display refreshes
            
            // Turn off one LED
            lightDownLed(g_game_state->lives);
            
            // Play buzzer sound when losing a life
            playLowBeep();
            
            printf("Collision! Lives remaining: %d\n", g_game_state->lives);
            
            // Remove the collided block
            if (current == g_block_list) {
                g_block_list = current->next;
            } else {
                Block* prev = g_block_list;
                while (prev->next != current) {
                    prev = prev->next;
                }
                prev->next = current->next;
            }
            
            free(current);
            break;  // Only handle one collision per tick
        }
        current = current->next;
    }
    
    // Check game over condition
    if (g_game_state->lives == 0) {
        g_game_state->game_running = 0;
    }
}

void addBlock(uint8_t position, uint8_t column) {
    // Dynamic memory allocation for new block
    Block* new_block = (Block*)malloc(sizeof(Block));
    if (new_block == NULL) {
        printf("Warning: Could not allocate memory for new block\n");
        return;
    }
    
    new_block->position = position;
    new_block->column = column;
    new_block->next = g_block_list;
    g_block_list = new_block;
}

void clearAllBlocks(void) {
    while (g_block_list != NULL) {
        Block* to_remove = g_block_list;
        g_block_list = g_block_list->next;
        free(to_remove);  // Dynamic memory deallocation
    }
}

void gameOver(void) {
    printf("\n=== GAME OVER ===\n");
    
    if (g_game_state->lives == 0) {
        printf("All spaceships destroyed!\n");
        
        printf("Press any button to continue...\n");
        
        // Clear any pending button press
        g_button_pressed = 0;
        
        // Continuously toggle all segments on and off until button pressed
        uint8_t blink_state = 0;  // 0 = all on, 1 = all off
        while (!g_button_pressed) {
            // Set all displays based on blink state
            for (uint8_t j = 0; j < 4; j++) {
                g_display_buffer[j] = (blink_state == 0) ? 0x00 : 0xFF;  // All segments ON then OFF (common cathode)
            }
            
            // Toggle blink state
            blink_state = 1 - blink_state;
            
            // Wait for half a second
            _delay_ms(500);
        }
        
        g_button_pressed = 0;  // Clear the button press flag
        _delay_ms(500);  // Debounce delay
        
        playVictoryTune();  // Actually a defeat tune
    } else {
        printf("Game ended.\n");
    }
    
    // Calculate final score
    g_game_state->score = calculateScore(g_game_state->level, g_game_state->blocks_dodged);
    
    printf("Final Statistics:\n");
    printf("- Level reached: %d\n", g_game_state->level);
    printf("- Blocks dodged: %lu\n", g_game_state->blocks_dodged);
    printf("- Final score: %d\n", g_game_state->score);
    
    // Display score on 7-segment display
    writeNumber(g_game_state->score);
    
    // Turn off all LEDs
    lightDownAllLeds();
    
    // Clean up dynamic memory
    clearAllBlocks();
    if (g_game_state != NULL) {
        free(g_game_state);
        g_game_state = NULL;
    }
}

void playVictoryTune(void) {
    #if BUZZER_ENABLED
    // Play a simple sequence
    for (int i = 0; i < 3; i++) {
        PORTD &= ~(1 << BUZZER_PIN);
        _delay_ms(2);
        PORTD |= (1 << BUZZER_PIN);
        _delay_ms(1);
    }
    _delay_ms(100);
    for (int i = 0; i < 3; i++) {
        PORTD &= ~(1 << BUZZER_PIN);
        _delay_ms(1);
        PORTD |= (1 << BUZZER_PIN);
        _delay_ms(1);
    }
    #endif
}

void playBeep(void) {
    #if BUZZER_ENABLED
    // Play a short beep
    for (int i = 0; i < 5; i++) {
        PORTD &= ~(1 << BUZZER_PIN);
        _delay_ms(1);
        PORTD |= (1 << BUZZER_PIN);
        _delay_ms(1);
    }
    #endif
}

void playLowBeep(void) {
    #if BUZZER_ENABLED
    // Play a longer, lower beep
    for (int i = 0; i < 10; i++) {
        PORTD &= ~(1 << BUZZER_PIN);
        _delay_ms(2);
        PORTD |= (1 << BUZZER_PIN);
        _delay_ms(2);
    }
    #endif
}

// Demonstration of pass by reference using pointers
void updateGameStateByReference(GameState* state, uint8_t new_level) {
    if (state != NULL) {
        state->level = new_level;
        printf("Game state updated by reference. New level: %d\n", state->level);
    }
}

uint16_t calculateScore(uint8_t level, unsigned long blocks_dodged) {
    // Score calculation: base points for blocks dodged, bonus for level
    return (blocks_dodged * 10) + (level * level * 50);
}

void displayGameInfo(void) {
    static uint32_t last_info_time = 0;
    
    // Display info every 5 seconds
    if (g_timer_counter - last_info_time > 5000) {
        printf("Level: %d, Lives: %d, Score: %d, Blocks dodged: %lu\n", 
               g_game_state->level, g_game_state->lives, 
               g_game_state->score, g_game_state->blocks_dodged);
        last_info_time = g_timer_counter;
    }
}

// Add tone generation function
void playTone(float frequency, uint32_t duration) {
    // Calculate number of cycles needed for the duration
    uint32_t cycles = (uint32_t)((float)duration * frequency / 1000.0);
    
    // Use fixed delays that are known at compile time
    for (uint32_t i = 0; i < cycles; i++) {
        PORTD &= ~(1 << BUZZER_PIN);  // turn the buzzer on
        _delay_ms(1);  // Fixed 1ms delay
        PORTD |= (1 << BUZZER_PIN);   // turn the buzzer off
        _delay_ms(1);  // Fixed 1ms delay
    }
} 