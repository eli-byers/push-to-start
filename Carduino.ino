

/*
PROGRAMMING
https://www.allaboutcircuits.com/projects/read-and-write-on-nfc-tags-with-an-arduino/

A Transitional State runs once and immediately
moves on to a Static State.

State 0: All low, waiting to start
State 1: Turn on Acc and Start
State 2: Waiting for car to start
State 3: Acc only
State 4: Waiting for shutdown
State 5: Shutdown

*/

#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>   // The following files are included in the libraries Installed
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);  // Indicates the Shield you are using

// States: use names so it's easier to
// understand what the code is doing
const int S_AUTHENTICATE = 0;
const int S_CAROFF = 1;
const int S_STARTCAR = 2;
const int S_WAITING2START = 3;
const int S_ACCONLY = 4;
const int S_WAITING4SHUTDOWN = 5;
const int S_SHUTDOWN = 6;

// Pin numbers
const int P_BUTTON = 2;
const int P_BUTTONLED = 3;
const int P_DASHLED = 4;
const int P_ACC = 5;
const int P_RUN = 6;
const int P_START = 7;

//Timers
const unsigned long T_DEAUTH = 7500000;
const unsigned long T_OFFPRESS = 200000;
const unsigned long T_SHUTDOWN = 3000;
const unsigned long T_START = 100;
const long T_LEDFLASH = 100;           // LEDTEST interval at which to blink (milliseconds)

// Variables will change
int ledState = LOW;             // LEDTEST ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // LEDTEST will store last time LED was updated


void setup()
{
  pinMode(P_BUTTON, INPUT);
  pinMode(P_ACC, OUTPUT);
  digitalWrite(P_ACC, HIGH);
  pinMode(P_START, OUTPUT);
  digitalWrite(P_START, HIGH);
  pinMode(P_RUN, OUTPUT);
  digitalWrite(P_RUN, HIGH);
  pinMode(P_BUTTONLED, OUTPUT);
  pinMode(P_DASHLED, OUTPUT);
  Serial.begin(9600);
  // Serial.println("NFC TAG READER"); // Header used when using the serial monitor
  nfc.begin();
}

void loop()
{

  // start off with the car is off and waiting for the NFC tag to start
  // The keyword "static" makes sure the variable
  // isn't destroyed after each loop
  static int state = S_AUTHENTICATE;
  // static bool wasRunning = false;
  static unsigned long timePressed = 0;
  static unsigned long offTimer = 0;
  static unsigned long startDelay = 0;
  unsigned long currentMillis = millis();

  switch (state)
  {

    case S_AUTHENTICATE:
      // Serial.println("\nWaiting for Authenticaion Scan your NFC\n");  // Command so that you an others will know what to do
      if (nfc.tagPresent()) { //to do, add authentication
        digitalWrite(P_DASHLED, LOW);
        state = S_CAROFF;
      }
      if (currentMillis - previousMillis >= 50) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;
        // if the LED is off turn it on and vice-versa:
        if (ledState == LOW) {
          ledState = HIGH;
        } else {
          ledState = LOW;
        }
        // set the LED with the ledState of the variable:
        digitalWrite(P_DASHLED, ledState);
      }
      // Serial.println(offTimer);
      break;

    case S_CAROFF:
      if (offTimer == 0){
        digitalWrite( P_ACC, LOW);
        digitalWrite( P_BUTTONLED, HIGH);
      }
      // Serial.println("\nCaroff");
      if (digitalRead(P_BUTTON) == HIGH) { //to do, add authentication
        // Serial.println("\nCaroff");
        if (startDelay > T_START){
          startDelay = 0;
          offTimer = 0;
          state = S_STARTCAR;
          break;
        }
        startDelay += 1;
        // Serial.println(startDelay);
      }
      if (offTimer > T_DEAUTH){
        offTimer = 0;
        digitalWrite( P_BUTTONLED, LOW);
        digitalWrite( P_ACC, HIGH);
        state = S_AUTHENTICATE;
        break;
      }
      offTimer +=1;

      //Serial.println(offTimer);
      break;

    case S_STARTCAR:
      // Serial.println("\nstart car\n");
      digitalWrite( P_RUN, LOW);
      digitalWrite( P_START, LOW);
      digitalWrite( P_BUTTONLED, HIGH);
      state = S_WAITING2START;
      break;


    case S_WAITING2START:
      // Serial.println("\nCar is waiting to start\n");
      if (digitalRead(P_BUTTON) == LOW) {
        state = S_ACCONLY;
      }
      if (currentMillis - previousMillis >= T_LEDFLASH) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;
        // if the LED is off turn it on and vice-versa:
        if (ledState == LOW) {
          ledState = HIGH;
        } else {
          ledState = LOW;
        }
        // set the LED with the ledState of the variable:
        digitalWrite(P_BUTTONLED, ledState);
      }

      break;

    case S_ACCONLY:
      // Serial.println("\nACC only\n");
      digitalWrite( P_BUTTONLED, LOW);  //Stop flashing
      digitalWrite( P_START, HIGH);
      state = S_WAITING4SHUTDOWN;

      break;

    case S_WAITING4SHUTDOWN:
      // Serial.println("\nWaiting 4 shutdown\n");
      if (digitalRead(P_BUTTON) == HIGH) { //to do, check button status
        if (timePressed == 0){
          digitalWrite(P_BUTTONLED, HIGH);
        }
        timePressed += 1;
        // Serial.println(timePressed);
        // Serial.println("\n");
        if (timePressed > T_OFFPRESS){
          timePressed = 0;
          // wasRunning = true;
          state = S_SHUTDOWN;
        }
      } else {
        timePressed = 0;
        digitalWrite(P_BUTTONLED, LOW);
      }
      if (currentMillis - previousMillis >= T_LEDFLASH) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;
        // if the LED is off turn it on and vice-versa:
        if (ledState == LOW) {
          ledState = HIGH;
        } else {
          ledState = LOW;
        }
        // set the LED with the ledState of the variable:
        digitalWrite(P_BUTTONLED, ledState);
      }
      break;

    case S_SHUTDOWN:
      // Serial.println("\nCar is shutting down\n");  // Command so that you an others will know what to do
      digitalWrite( P_RUN, HIGH);
      digitalWrite(P_BUTTONLED, LOW);
      // if (wasRunning = true){
        delay(T_SHUTDOWN);
      // }
      // wasRunning = false;
      state = S_CAROFF;
      break;

  } // end of switch

  // other things could go on here, and they would not affect the timing
  // of the traffic light

} // end of loop
