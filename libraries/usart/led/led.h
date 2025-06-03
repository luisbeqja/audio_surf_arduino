/*
Basic functions to control the leds of your multifunctional shield
*/

//ex1.11
void enableLed ( int lednumber );
void lightUpLed ( int lednumber );
void lightDownLed ( int lednumber );

void enableMultipleLeds(uint8_t leds);
void lightUpMultipleLeds(uint8_t leds);
void lightDownMultipleLeds(uint8_t leds);

void enableAllLeds();
void lightUpAllLeds();
void lightDownAllLeds();

void lightToggleOneLed(int lednumber);

//ex1.12
void dimLed (int lednumber, int percentage, int duration);
void fadeInLed (int led, int duration);
void fadeOutLed (int led, int duration);

//ex1.14
void flashLed(int lednumber, int amountOfTimes);

//ex2_7_3
int isLightOn(int lednumber);
//ex2_7_4
void flashLedIndefinitely(int lednumber);
