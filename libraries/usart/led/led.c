// to allow variables as parameter for the _delay-functions (must be placed before the include of delay.h):
#define __DELAY_BACKWARD_COMPATIBLE__ 
#include <util/delay.h>
#include <avr/io.h>

#define NUMBER_OF_LEDS 4 

//Ex1.11
void enableLed ( int lednumber ) //C has no classes; functions can be included directly in the .c file.
{
    if ( lednumber < 0 || lednumber > NUMBER_OF_LEDS-1 ) return;
    DDRB |= ( 1 << ( PB2 + lednumber ));    //Check the tutorial "Writing to a Pin". We know from the documentation on
                                            //the multifunctional shield that the LEDs start at PB2
}

void lightUpLed ( int lednumber )    //Note: enabled LEDs light up immediately ( 0 = on )
{
    if ( lednumber < 0 || lednumber > NUMBER_OF_LEDS-1 ) return;
    PORTB &= ~
    ( 1 << ( PB2 + lednumber ));  //Check the tutorial on "Bit Operations" to know what happens in this line.
}

void lightDownLed ( int lednumber )
{
    if ( lednumber < 0 || lednumber > 3 ) return;
    PORTB |= ( 1 << ( PB2 + lednumber ));  //Make sure you understand this line as well!
}


//EXTRA INFO/ NOTES:
//so, in this case, the leds are numbered from 0-3, meaning led0 is your first led on your arduino board and led3 is your last led
// the enableLed and lightDownLed functions are essentially the same, both will set the according bit to 1, it does this through an OR operation |=
// the lightUpLed does the same, but sets the target bit to 0 aka switching the led on. It does this through an AND operation on the 'NOT' version of the selected bit &= ~
// how specifying the port number works --> if you enbleLed(3), then you will get DDRB |= (1 << (PB2+3)) (we know that the leds start at PB2)
// 1 << (PB2 + 3) will give you 1 << 5, as PB5 has value five, as described in the avr/io.h file.
// so you will shift the value 1 left by 5 spaces and get 100000 or 0B00100000
// this DDRB value corresponds with PB5 or pin13 (aka the fourth/ last led light)

//EX 1.11:
//MULTIPLE LEDS
void enableMultipleLeds ( uint8_t leds )
{
    for (int i = 0; i < NUMBER_OF_LEDS; i++) { // we do this because we only have 4 leds
        if (leds & (1 << (PB2 + i))) { //aka if bit i in leds is "true" or 1
            // the AND operation for that bit will only be true/1 if that same bit is 1 in leds
            // Enable (set as output) pin PB2 + i
            DDRB |= (1 << (PB2 + i));
        }
    }                                               
}

void lightUpMultipleLeds( uint8_t leds) {
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        if (leds & (1 << (PB2 + i))) {
            PORTB &= ~(1 << (PB2 + i));
        }
    }
}

void lightDownMultipleLeds(uint8_t leds) {
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        if (leds & (1<< (PB2 + i))) {
            PORTB |= (1 << (PB2 + i));
        }
    }
}

//ALL LEDS
void enableAllLeds() {
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        DDRB |= (1 << (PB2 + i));
    }
}

void lightUpAllLeds() {
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        PORTB &= ~(1 << (PB2 + i));
    }
}

void lightDownAllLeds() {
    for (int i = 0; i < NUMBER_OF_LEDS; i++) {
        PORTB |= (1 << (PB2 + i));
    }
}

//toggling means switch it off if it is on and switch it on if it is off --> this is done via XOR operation!
void lightToggleOneLed(int lednumber) {
    if (lednumber < 0 || lednumber > NUMBER_OF_LEDS-1) return;
    PORTB ^= (1 << (PB2 + lednumber));
}

//Ex1.12
//before using this method, don't forget to call enableLed to set the DDRB
void dimLed (int lednumber, int percentage, int duration) {
    int cycleTime = 10; // total time of one PWM cycle is 10ms apparently
    int onTime = cycleTime * percentage / 100; //because led is only on a certain percentage of the time
    int offTime = cycleTime - onTime; //the other percentage of time led is off
    
    int nrOfCycles = duration/cycleTime;

    for (int i = 0; i <= nrOfCycles; i++) {
        lightUpLed(lednumber); //the light switches on for a duration of onTime
        _delay_ms(onTime);

        lightDownLed(lednumber); //the light is off for a duration of offTime
        _delay_ms(offTime);
  }
}

void fadeInLed(int lednumber, int duration) {
    int steps = 50; // i picked 10 first but the fading didn't look that smooth so i played around with this value
    int stepDuration = duration / steps;

    for (int i = 0; i <= steps; i++) {
        int percentage = (i*100) / steps;
        dimLed(lednumber, percentage, stepDuration);
    }
}

void fadeOutLed(int lednumber, int duration) {
    int steps = 50;
    int stepDuration = duration /steps;
    for (int i = steps; i >= 0; i--) {
        int percentage = (i*100) / steps;
        dimLed(lednumber, percentage, stepDuration);
    }
}

//Ex1.14
// Turn exercise 2 into a function, where the parameters are the LED number and the number of flashes. (add the function to your library)
void flashLed(int lednumber, int amountOfTimes) {
    enableLed(lednumber);
  
    for (int i = 0; i < amountOfTimes; i++) {
      lightUpLed(lednumber);
      _delay_ms(500);
      lightDownLed(lednumber);
      _delay_ms(500);
    }
}

void flashLedIndefinitely(int lednumber) {
    enableLed(lednumber);

    while (1) {
        lightUpLed(lednumber);
        _delay_ms(500);
        lightDownLed(lednumber);
        _delay_ms(500);
    }
}

//ex2_7_3
// Check if a specific LED is currently on (lit)
// Returns 1 if LED is on (PORTB bit is 0), 0 if LED is off (PORTB bit is 1)
int isLightOn(int lednumber) {
    if (lednumber < 0 || lednumber > NUMBER_OF_LEDS-1) return 0;
    
    // LED is on when the corresponding PORTB bit is 0 (inverted logic)
    // So we check if the bit is NOT set
    return !(PORTB & (1 << (PB2 + lednumber)));
}
