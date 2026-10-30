// Lab 10 – Event-Driven Programming with Push Buttons and a Touch Sensor

#include <Arduino.h>

// ---------- Pin numbers ----------
#define BUTTON_A   32   // polled button
#define BUTTON_B   33   // interrupt button
#define TOUCH_PIN  27   // touch sensor wire
#define GREEN_LED  25
#define RED_LED    26
#define BUZZER     13

// ---------- Global variables ----------
int lastStateA = HIGH;
int pressCountA = 0;                          // How many presses have been counted.
unsigned long lastChangeA = 0; // The time (in ms) of the last change that was accepted.

// Changes closer together than 50 ms are bounce, so they are ignored.
const unsigned long DEBOUNCE_TIME = 50;

// buttonBFlag: raised (true) by the ISR when button B is pressed.
volatile bool buttonBFlag = false;
// lastInterruptB: the time of the last accepted press (used to debounce).
volatile unsigned long lastInterruptB = 0;
// pressCountB is only used in loop(), so it needs no keyword.
int pressCountB = 0;


// ---------- Event handlers and callbacks ----------
void onButtonAPressed() {
  pressCountA++;                                // add 1 to the count
  Serial.print("Button A pressed. Count = ");   // print the text...
  Serial.println(pressCountA);                  // ...then the number and a new line
}

// Called from loop(), never from the ISR, so Serial.print() is allowed here.
void onButtonBPressed() {
  pressCountB++;                                            // add 1 to the count
  Serial.print("Button B pressed (interrupt). Count = ");   // print the text...
  Serial.println(pressCountB);                              // ...then the number
}

// ---------- Interrupt Service Routines (ISRs) ----------
// ISR for button B – runs the instant the pin falls, even if loop() is busy.
void IRAM_ATTR buttonB_ISR() {
  // Ignore presses less than 200 ms apart (debounce inside the ISR).
  if (millis() - lastInterruptB > 200) {
    lastInterruptB = millis();   // remember when this press happened
    buttonBFlag = true;        // raise the flag for loop(): true or false?
  }
}


void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Lab 10: Event-Driven Programming");

  // Inputs and outputs
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Interrupts
  // Run buttonB_ISR() when the pin of button B changes 
  attachInterrupt(digitalPinToInterrupt(BUTTON_B), buttonB_ISR, FALLING);

  // Callbacks
}

void loop() {
  // Button A (polling)
  // 1. Read button A now 
  int stateA = digitalRead(BUTTON_A);
  // 2. Did the state change, AND has more than DEBOUNCE_TIME passed since the last change?
  if (stateA != lastStateA && millis() - lastChangeA > DEBOUNCE_TIME) {
    // 3. Accept the change: save the time now, and remember the new state.
    lastChangeA = millis();
    lastStateA = stateA;
    // 4. Only a change to "pressed" is a press event.
    if (stateA == LOW) {
      onButtonAPressed();   // call the event handler
    }
  }

  // Button B (interrupt)
  // Flag pattern: did the ISR raise the flag?
  if (buttonBFlag == true) {
    buttonBFlag = false;   // lower the flag first, so the press is handled once
    onButtonBPressed();    // then call the handler
  }


  // Touch sensor (interrupt)

  // Timer

}
