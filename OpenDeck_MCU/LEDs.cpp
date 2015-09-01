/*

OpenDECK library v1.3
File: LEDs.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include <util/delay.h>
#include "LEDsettings.h"

//LED blink/constant state determination
#define LED_VELOCITY_C_OFF               0x00
#define LED_VELOCITY_B_OFF               0x3F

#ifdef BOARD_TANNIN
    //tannin 2 specific start-up routine
    void tannin2startup()   {

        uint16_t startUpLEDswitchTime = eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME) * 10;
        boardObject.setLEDTransitionSpeed(1);

        boardObject.setLEDstate(6, ledOn);
        boardObject.setLEDstate(30, ledOn);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(5, ledOn);
        boardObject.setLEDstate(29, ledOn);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(28, ledOn);
        boardObject.setLEDstate(4, ledOn);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(31, ledOn);
        boardObject.setLEDstate(7, ledOn);

        boardObject.newDelay(startUpLEDswitchTime);

        boardObject.setLEDstate(22, ledOn);
        boardObject.setLEDstate(14, ledOn);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(21, ledOn);
        boardObject.setLEDstate(13, ledOn);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(20, ledOn);
        boardObject.setLEDstate(12, ledOn);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(23, ledOn);
        boardObject.setLEDstate(15, ledOn);

        boardObject.newDelay(startUpLEDswitchTime);

        boardObject.setLEDstate(31, ledOff);
        boardObject.setLEDstate(7, ledOff);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(28, ledOff);
        boardObject.setLEDstate(4, ledOff);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(29, ledOff);
        boardObject.setLEDstate(5, ledOff);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(30, ledOff);
        boardObject.setLEDstate(6, ledOff);

        boardObject.newDelay(startUpLEDswitchTime);

        boardObject.setLEDstate(23, ledOff);
        boardObject.setLEDstate(15, ledOff);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(20, ledOff);
        boardObject.setLEDstate(12, ledOff);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(21, ledOff);
        boardObject.setLEDstate(13, ledOff);
        boardObject.newDelay(startUpLEDswitchTime);
        boardObject.setLEDstate(22, ledOff);
        boardObject.setLEDstate(14, ledOff);

        boardObject.newDelay(1000);

        boardObject.setLEDTransitionSpeed(eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_FADE_SPEED));
        boardObject.resetLEDtransitions();

    }
#endif


void OpenDeck::startUpRoutine() {

    //boardObject.setUpTimer();

    //turn off all LEDs before starting animation
    allLEDsOff();

    #ifdef BOARD_TANNIN
        tannin2startup();
    #else

    switch (eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_ROUTINE))  {

        case 1:
        openDeck.oneByOneLED(true, true, true);
        openDeck.oneByOneLED(false, false, true);
        openDeck.oneByOneLED(true, false, false);
        openDeck.oneByOneLED(false, true, true);
        openDeck.oneByOneLED(true, false, true);
        openDeck.oneByOneLED(false, false, false);
        break;

        case 2:
        openDeck.oneByOneLED(true, false, true);
        openDeck.oneByOneLED(false, false, false);
        break;

        case 3:
        openDeck.oneByOneLED(true, true, true);
        openDeck.oneByOneLED(false, true, true);
        break;

        case 4:
        openDeck.oneByOneLED(true, false, true);
        openDeck.oneByOneLED(true, false, false);
        break;

        case 5:
        openDeck.oneByOneLED(true, false, true);
        break;

        default:
        break;

    }
    #endif
    openDeck.allLEDsOff(); delay(1000); //boardObject.disconnectTimer();

}

void OpenDeck::oneByOneLED(bool ledDirection, bool singleLED, bool turnOn)  {

    /*

    Function accepts three boolean arguments.

    ledDirection:   true means that LEDs will go from left to right, false from right to left
    singleLED:      true means that only one LED will be active at the time, false means that LEDs
                    will turn on one by one until they're all lighted up

    turnOn:         true means that LEDs will be turned on, with all previous LED states being 0
                    false means that all LEDs are lighted up and they turn off one by one, depending
                    on second argument

    */

    uint16_t startUpLEDswitchTime = eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME) * 10;

    //while loop counter
    uint8_t passCounter = 0;

    //reset the timer on each function call
    uint32_t startUpTimer = 0;

    //index of LED to be processed next
    uint8_t ledNumber,
            _ledNumber[MAX_NUMBER_OF_LEDS];

    //get LED order for start-up routine
    for (int i=0; i<totalNumberOfLEDs; i++)    _ledNumber[i] = eeprom_read_byte((uint8_t*)EEPROM_LEDS_START_UP_NUMBER_START+i);

    //if second and third argument of function are set to false or
    //if second argument is set to false and all the LEDs are turned off
    //light up all LEDs
    if ((!singleLED && !turnOn) || (checkLEDsOff() && !turnOn)) allLEDsOn();

    if (turnOn) {

    //This part of code deals with situations when previous function call has been
    //left direction and current one is right and vice versa.

    //On first function call, let's assume the direction was left to right. That would mean
    //that LEDs had to be processed in this order:

    //LED 1
    //LED 2
    //LED 3
    //LED 4

    //Now, when function is finished, LEDs are not reset yet with allLEDsOff() function to keep
    //track of their previous states. Next function call is right to left. On first run with
    //right to left direction, the LED order would be standard LED 4 to LED 1, however, LED 4 has
    //been already turned on by first function call, so we check if its state is already set, and if
    //it is we increment or decrement ledNumber by one, depending on previous and current direction.
    //When function is called second time with direction different than previous one, the number of
    //times it needs to execute is reduced by one, therefore passCounter is incremented.

        //right-to-left direction
        if (!ledDirection)  {

            //if last LED is turned on
            if (boardObject.getLEDstate(_ledNumber[totalNumberOfLEDs-1]))  {

                //LED index is penultimate LED number
                ledNumber = _ledNumber[totalNumberOfLEDs-2];
                //increment counter since the loop has to run one cycle less
                passCounter++;

            }   else    ledNumber = _ledNumber[totalNumberOfLEDs-1]; //led index is last one if last one isn't already on

        }   else //left-to-right direction

                //if first LED is already on
                if (boardObject.getLEDstate(_ledNumber[0]))    {

                //led index is 1
                ledNumber = _ledNumber[1];
                //increment counter
                passCounter++;

                }   else    ledNumber = _ledNumber[0];

    }   else    {

                    //This is situation when all LEDs are turned on and we're turning them off one by one. Same
                    //logic applies in both cases (see above). In this case we're not checking for whether the LED
                    //is already turned on, but whether it's already turned off.

                    //right-to-left direction
                    if (!ledDirection)  {

                        if (!(boardObject.getLEDstate(_ledNumber[totalNumberOfLEDs-1])))   {

                            ledNumber = _ledNumber[totalNumberOfLEDs-2];
                            passCounter++;

                        }   else ledNumber = _ledNumber[totalNumberOfLEDs-1];

                    }   else

                            if (!(boardObject.getLEDstate(_ledNumber[0]))) {   //left-to-right direction

                                ledNumber = _ledNumber[1];
                                passCounter++;

                            }   else ledNumber = _ledNumber[0];

        }

    //on first function call, the while loop is called TOTAL_NUMBER_OF_LEDS+1 times
    //to get empty cycle after processing last LED
    while (passCounter < totalNumberOfLEDs+1)   {

            //only process LED after defined time
            if ((boardObject.newMillis() - startUpTimer) > startUpLEDswitchTime)  {

                if (passCounter < totalNumberOfLEDs)    {

                    //if we're turning LEDs on one by one, turn all the other LEDs off
                    if (singleLED && turnOn)            allLEDsOff();

                    //if we're turning LEDs off one by one, turn all the other LEDs on
                    else    if (!turnOn && singleLED)   allLEDsOn();

                    //set LED state depending on turnOn parameter
                    if (turnOn) boardObject.setLEDstate(ledNumber, ledOn);
                        else    boardObject.setLEDstate(ledNumber, ledOff);

                    //make sure out-of-bound index isn't requested from ledArray
                    if (passCounter < totalNumberOfLEDs-1)  {

                        //right-to-left direction
                        if (!ledDirection)  ledNumber = _ledNumber[totalNumberOfLEDs - 2 - passCounter];

                        //left-to-right direction
                        else    if (passCounter < totalNumberOfLEDs-1)  ledNumber = _ledNumber[passCounter+1];

                    }

                }

            //always increment pass counter
            passCounter++;

            //update timer
            startUpTimer = boardObject.newMillis();

        }

    }

}

void OpenDeck::allLEDsOn()  {

    //turn on all LEDs
    for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)    boardObject.setLEDstate(i, ledOn);

}

void OpenDeck::allLEDsOff() {

    //turn off all LEDs
    for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)    boardObject.setLEDstate(i, ledOff);

}

bool OpenDeck::checkLEDsOn()    {

    //return true if all LEDs are on
    for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)    if (boardObject.getLEDstate(i))   return false;
    return true;

}

bool OpenDeck::checkLEDsOff()   {

    //return true if all LEDs are off
    for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)    if (!boardObject.getLEDstate(i))   return false;
    return true;

}

void OpenDeck::setLEDState()    {

    bool currentLEDstate;

    //if blinkMode is 1, the LED is blinking
    uint8_t blinkMode = 0;

        if ((receivedVelocity == LED_VELOCITY_C_OFF) || (receivedVelocity == LED_VELOCITY_B_OFF))
            currentLEDstate = false;

        else if (

        ((receivedVelocity > LED_VELOCITY_C_OFF) && (receivedVelocity < LED_VELOCITY_B_OFF)) ||
        ((receivedVelocity > LED_VELOCITY_B_OFF) && (receivedVelocity < 128))

        )    currentLEDstate = true;

        else return;

        if ((receivedVelocity >= LED_VELOCITY_B_OFF) && (receivedVelocity < 128))
            blinkMode = 1;

    handleLED(currentLEDstate, blinkMode, getLEDid());

}

uint8_t OpenDeck::getLEDid()   {

    //match LED activation note with its index
    for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)
        if (ledActNote[i] == receivedNote) return i;

    //since 128 is impossible note, return it in case
    //that received note doesn't match any LED
    return 128;

}

uint8_t OpenDeck::getLEDActivationNote(uint8_t ledNumber)   {

    return ledActNote[ledNumber];

}

bool OpenDeck::checkSameLEDvalue(uint8_t type, uint8_t number)  {

    //do not allow same activation or start-up number for multiple LEDs

    switch(type)    {

        case SYS_EX_MST_LEDS_ACT_NOTE:
        //led activation note
        for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)
            if (ledActNote[i] == number)    return false;
        break;

        case SYS_EX_MST_LEDS_START_UP_NUMBER:
        //LED start-up number
        for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)
            if (eeprom_read_byte((uint8_t*)EEPROM_LEDS_START_UP_NUMBER_START+i) == number)    return false;
        break;

    }   return true;

}

void OpenDeck::handleLED(bool currentLEDstate, bool blinkMode, uint8_t ledNumber) {

    /*

    LED state is stored into one byte (ledState). The bits have following meaning (7 being the MSB bit):

    7: x
    6: x
    5: x
    4: Blink bit (timer changes this bit)
    3: "Remember" bit, used to restore previous LED state
    2: LED is active (either it blinks or it's constantly on), this bit is OR function between bit 0 and 1
    1: LED blinks
    0: LED is constantly turned on

    */

    //if blink note is received, and blinking is disabled, exit the function
    //if (blinkMode && (!(bitRead(ledFeatures, SYS_EX_FEATURES_LEDS_BLINK))))
        //return;

    uint8_t state = boardObject.getLEDstate(ledNumber);

    switch (currentLEDstate) {

        case false:
        //note off event

        //if remember bit is set
        if (bitRead(state, LED_REMEMBER_BIT))   {

            //if note off for blink state is received
            //clear remember bit and blink bits
            //set constant state bit
            if (blinkMode) state = ledOn;
            //else clear constant state bit and remember bit
            //set blink bits
            else           state = ledBlink;

        }   else state = ledOff;

        break;

        case true:
        //note on event

        if ((!blinkMode) && bitRead(state, LED_BLINK_ON_BIT))   state = ledOnRemember;
        else if ((blinkMode) && bitRead(state, LED_ON_BIT))     state = ledBlinkRemember;

        else    {

            bitWrite(state, LED_ACTIVE_BIT, 1);
            if (blinkMode)  {

                bitWrite(state, LED_BLINK_ON_BIT, 1);
                bitWrite(state, LED_BLINK_STATE_BIT, 1);

            }   else bitWrite(state, LED_ON_BIT, 1);

        }

    }

    boardObject.setLEDstate(ledNumber, state);
    if (blinkMode && currentLEDstate)   boardObject.ledBlinkingStart();
    else    checkBlinkLEDs();

}

void OpenDeck::checkBlinkLEDs() {

    //this function will disable blinking
    //if none of the LEDs is in blinking state

    //else it will enable it

    bool _blinkEnabled = false;
    uint8_t ledState;

    //if any LED is blinking, set timerState to true and exit the loop
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    {

        ledState = boardObject.getLEDstate(i);

        if (bitRead(ledState, LED_BLINK_ON_BIT)) {

            _blinkEnabled = true;
            break;

        }

    }

    if (_blinkEnabled)  boardObject.ledBlinkingStart();

    //don't bother reseting variables if blinking is already disabled
    else    if (!_blinkEnabled && boardObject.ledBlinkingActive()) {

        //reset blinkState to default value
        boardObject.ledBlinkingStop();

    }

}