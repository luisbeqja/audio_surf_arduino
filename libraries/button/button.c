#include <avr/io.h>
#include <avr/interrupt.h> //so you can do the whole interrupts/ISR() stuff

// For buttons, you want the pin to receive input, so you do DDRC &= ~
// Unlike with LEDs, we want to set the corresponding bit to 0, making that pin ready for INPUT (not output, like with the LEDs) 
void enableButton( int button ) {
    if (button < 1 || button > 4) return;     // 1 is our leftmost button -- and we know that corresponds with PC1
    int pin = PC1 + (button - 1);
    DDRC &= ~(1 << pin);     // write 0 to DDRC pin to set as input
    PORTC |= (1 << (PC1 + (button -1)));      // write 1 to pin to enable internal pull-up resistor
}
/* when button is not pressed, voltage reads as high due to internal pull-up resistor, when button is pressed it's read as low*/

int buttonPushed( int button ) {
    if (button < 1 || button > 4) return; 
    int pin = PC1 + (button - 1);
    if (( PINC & ( 1 << pin )) == 0 ) {
        return 1;
    }
    return 0;
    // or more easy: return !(PINC & (1 << pin)); 
    // PINC & (1 << pin) will return 0 if pressed -- so we do a 'not' (!) so we receive 1 if true and 0 if false
}

//needed for buttonRelease is smt to store the previous state in (make it static): 
uint8_t static previousState[3] = {1, 1, 1}; //pull-up: when button is unpressed, it returns 1

int buttonReleased( int button ) {
    if (button < 1 || button > 4) return; 

    int pin = PC1 + (button - 1);
    uint8_t currentState = PINC & (1 << pin);

    uint8_t wasPressed = !previousState[button - 1] && currentState; 
    //if previously pressed, then previousState would be 0, not-version is 1
    //and if released, then PINC aka currentState would return 1 --> so if both are true, you return 1
    
    //lastly we remember the state we are now in
    if (currentState) {
        previousState[button - 1] = 1; 
    }
    else {
        previousState[button - 1] = 0;
    }

    return wasPressed;
}

//Ex 2.7.4
void enableButtonInterrupt(int button) {
    if (button < 1 || button > 4) return; 
    PCICR |= (1 << PCIE1);               // Enable Pin Change Interrupts for Port C
    PCMSK1 |= (1 << button);             // Enable interrupt for specific pin (PC0..PC2)
}

void enableAllButtonInterrupts() {
    PCICR |= (1 << PCIE1);               // Enable Pin Change Interrupts for Port C
    PCMSK1 |= _BV(PC1);                //Enble intterupt for all three buttons
    PCMSK1 |= _BV(PC2);
    PCMSK1 |= _BV(PC3);
}