#include <avr/io.h>
#include <avr/interrupt.h> //so you can do the whole ISR() stuff
#include <stdint.h>
#include "timer.h"

void initTimer0()
{
    // STEP 1: Use NORMAL mode (no waveform generation, just counting)
    TCCR0A = 0; // Normal mode = WGM01 = 0, WGM00 = 0
    TCCR0B = 0;

    // STEP 2: Set PRESCALER (CPU clock / 64 = ~250kHz, overflow every 1ms approx)
    TCCR0B |= (1 << CS01) | (1 << CS00);  // Prescaler = 64

    // STEP 3: Enable Overflow Interrupt
    TIMSK0 |= (1 << TOIE0);  // Enable Timer0 overflow interrupt

    sei();  // Enable global interrupts
}

void initTimer1() {
    cli(); // Disable interrupts during setup

    TCCR1A = 0;                // Normal operation
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12);    // CTC mode
    OCR1A = 249;               // 1 ms at 16 MHz / 64
    TIMSK1 |= (1 << OCIE1A);   // Enable Timer1 compare A interrupt
    TCCR1B |= (1 << CS01) | (1 << CS00); // Start timer with prescaler 64

    sei(); // Re-enable global interrupts
}