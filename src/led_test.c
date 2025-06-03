#include <avr/io.h>
#include <util/delay.h>
#include "led.h"

/*
 * LED Test File for Arduino Multifunctional Shield
 * 
 * This test file demonstrates all LED functions available in the led library.
 * The shield has 4 LEDs connected to pins PB2-PB5 (Arduino pins 10-13).
 * 
 * LED numbering: 0, 1, 2, 3 (from left to right on the shield)
 * 
 * To use this test file:
 * 1. Update platformio.ini to build this file instead of main.c
 * 2. Upload to your Arduino board
 * 3. Watch the LED patterns and behaviors
 */

void test_individual_leds() {
    // Test each LED individually
    for (int led = 0; led < 4; led++) {
        enableLed(led);
        
        // Flash each LED 3 times
        for (int flash = 0; flash < 3; flash++) {
            lightUpLed(led);
            _delay_ms(300);
            lightDownLed(led);
            _delay_ms(300);
        }
        _delay_ms(500); // Pause between LEDs
    }
}

void test_multiple_leds() {
    // Test multiple LEDs at once using bit patterns
    uint8_t patterns[] = {
        0b00000100,  // LED 0 (PB2)
        0b00001000,  // LED 1 (PB3)
        0b00001100,  // LED 0 + 1
        0b00010000,  // LED 2 (PB4)
        0b00011100,  // LED 0 + 1 + 2
        0b00100000,  // LED 3 (PB5)
        0b00111100   // All LEDs (0, 1, 2, 3)
    };
    
    int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    
    for (int i = 0; i < num_patterns; i++) {
        enableMultipleLeds(patterns[i]);
        lightUpMultipleLeds(patterns[i]);
        _delay_ms(800);
        lightDownMultipleLeds(patterns[i]);
        _delay_ms(400);
    }
}

void test_all_leds() {
    // Test all LEDs together
    enableAllLeds();
    
    // Flash all LEDs 5 times
    for (int i = 0; i < 5; i++) {
        lightUpAllLeds();
        _delay_ms(400);
        lightDownAllLeds();
        _delay_ms(400);
    }
}

void test_led_toggle() {
    // Test toggle functionality
    enableAllLeds();
    lightDownAllLeds(); // Start with all LEDs off
    
    // Toggle each LED in sequence
    for (int cycle = 0; cycle < 3; cycle++) {
        for (int led = 0; led < 4; led++) {
            lightToggleOneLed(led);
            _delay_ms(500);
        }
    }
    
    // Turn all LEDs off
    lightDownAllLeds();
}

void test_led_dimming() {
    // Test dimming functionality
    for (int led = 0; led < 4; led++) {
        enableLed(led);
        
        // Test different brightness levels
        int brightness_levels[] = {10, 25, 50, 75, 100};
        int num_levels = sizeof(brightness_levels) / sizeof(brightness_levels[0]);
        
        for (int i = 0; i < num_levels; i++) {
            dimLed(led, brightness_levels[i], 1000); // 1 second at each brightness
        }
        
        lightDownLed(led);
        _delay_ms(500);
    }
}

void test_led_fading() {
    // Test fade in and fade out
    for (int led = 0; led < 4; led++) {
        enableLed(led);
        
        // Fade in
        fadeInLed(led, 2000);  // 2 seconds fade in
        _delay_ms(500);
        
        // Fade out
        fadeOutLed(led, 2000); // 2 seconds fade out
        _delay_ms(500);
    }
}

void test_led_flashing() {
    // Test the flash function
    for (int led = 0; led < 4; led++) {
        flashLed(led, 4); // Flash each LED 4 times
        _delay_ms(500);
    }
}

void test_running_lights() {
    // Create a running light effect
    enableAllLeds();
    lightDownAllLeds();
    
    for (int cycle = 0; cycle < 5; cycle++) {
        // Forward direction
        for (int led = 0; led < 4; led++) {
            lightUpLed(led);
            _delay_ms(200);
            lightDownLed(led);
        }
        
        // Backward direction
        for (int led = 3; led >= 0; led--) {
            lightUpLed(led);
            _delay_ms(200);
            lightDownLed(led);
        }
    }
}

void test_binary_counter() {
    // Display binary counting pattern
    enableAllLeds();
    
    for (int count = 0; count < 16; count++) {
        // Turn off all LEDs first
        lightDownAllLeds();
        
        // Light up LEDs based on binary representation
        for (int bit = 0; bit < 4; bit++) {
            if (count & (1 << bit)) {
                lightUpLed(bit);
            }
        }
        
        _delay_ms(800);
    }
    
    lightDownAllLeds();
}

void test_led_status_check() {
    // Test the isLightOn function
    enableAllLeds();
    lightDownAllLeds();
    
    // Turn on LED 1 and check status
    lightUpLed(1);
    _delay_ms(1000);
    
    // Check if LED 1 is on (this would require USART for output in a real test)
    // For now, we'll just demonstrate the function exists
    int led1_status = isLightOn(1);
    
    // Flash LED 0 if LED 1 is on, flash LED 3 if LED 1 is off
    if (led1_status) {
        flashLed(0, 3);
    } else {
        flashLed(3, 3);
    }
    
    lightDownAllLeds();
}

int main() {
    // Initialize - ensure all LEDs start in a known state
    enableAllLeds();
    lightDownAllLeds();
    _delay_ms(1000);
    
    // Welcome sequence - flash all LEDs 3 times
    for (int i = 0; i < 3; i++) {
        lightUpAllLeds();
        _delay_ms(200);
        lightDownAllLeds();
        _delay_ms(200);
    }
    
    _delay_ms(2000); // 2 second pause before starting tests
    
    while (1) {
        // Run all LED tests in sequence
        
        // Test 1: Individual LED control
        test_individual_leds();
        _delay_ms(1000);
        
        // Test 2: Multiple LED patterns
        test_multiple_leds();
        _delay_ms(1000);
        
        // Test 3: All LEDs together
        test_all_leds();
        _delay_ms(1000);
        
        // Test 4: Toggle functionality
        test_led_toggle();
        _delay_ms(1000);
        
        // Test 5: Dimming (PWM simulation)
        test_led_dimming();
        _delay_ms(1000);
        
        // Test 6: Fading effects
        test_led_fading();
        _delay_ms(1000);
        
        // Test 7: Flash function
        test_led_flashing();
        _delay_ms(1000);
        
        // Test 8: Running lights effect
        test_running_lights();
        _delay_ms(1000);
        
        // Test 9: Binary counter display
        test_binary_counter();
        _delay_ms(1000);
        
        // Test 10: LED status checking
        test_led_status_check();
        _delay_ms(1000);
        
        // Long pause before repeating the entire test sequence
        _delay_ms(5000);
    }
    
    return 0;
} 