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

const int TOUCH_THRESHOLD = 465;

// A Callback is a variable that holds a function (no inputs, returns nothing).
typedef void (*Callback)();
Callback buttonACallback;   // what button A does
Callback buttonBCallback;   // what button B does
Callback touchCallback;     // what a touch does

// Changed inside touch_ISR(), so they are volatile.
volatile bool touchFlag = false;        // raised by the ISR on a touch
volatile unsigned long lastTouch = 0;   // time of the last accepted touch

// The state of the alarm (only used outside the ISRs).
bool armed = false;                     // true = system armed (green LED on)
bool alarmOn = false;                   // true = red LED and buzzer on
unsigned long alarmStart = 0;           // the time the alarm started
const unsigned long ALARM_TIME = 3000;  // how long the alarm lasts: 3000 ms


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

// Used when the alarm times out, when button B silences it, and when the system is disarmed.
void alarmOff() {
  alarmOn = false;
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);
}

// Turns the alarm ON and starts the 3-second timer.
// It is called by onTouchDetected() when the wire is touched while armed.
void startAlarm() {
  alarmOn = true;

  // The // Timer code in loop() compares millis() with alarmStart
  // and turns the alarm off after ALARM_TIME (3000 ms).
  alarmStart = millis();

  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
}

// toggleArmed() – the callback for button A.
// Each press switches the system between ARMED and DISARMED.
void toggleArmed() {
  // 1. Flip the variable armed: true becomes false, false becomes true.
  armed = !armed;

  if (armed == true) {
    digitalWrite(GREEN_LED, HIGH);
    Serial.println("System ARMED");
  }
  else {
    digitalWrite(GREEN_LED, LOW);
    alarmOff();
    Serial.println("System DISARMED");
  }
}

// silenceAlarm() – the callback for button B.
// It stops the alarm but keeps the system armed.
void silenceAlarm() {
  alarmOff();
  Serial.println("Alarm silenced");
}

// onTouchDetected() – the callback for the touch sensor.
// loop() calls it every time touch_ISR() raises touchFlag.
void onTouchDetected() {
  //    The alarm must only start when the system is armed.
  if (armed == true) {
    Serial.println("TOUCH! Alarm On");
    startAlarm();
  }
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

// While a finger stays on the wire this ISR keeps running, so only one
// touch per second is accepted.
void IRAM_ATTR touch_ISR() {
  if (millis() - lastTouch > 1000) {
    lastTouch = millis();   // remember when this touch happened
    touchFlag = true;       // raise the flag for loop()
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

  // Attach touch_ISR() to the touch pin. It runs when the
  // touch value drops below TOUCH_THRESHOLD (Common Commands: "Touch Interrupt").
  touchAttachInterrupt(TOUCH_PIN, touch_ISR, TOUCH_THRESHOLD);

  // Callbacks
  // No brackets here – you store the function, you do not run it.
  buttonACallback = toggleArmed;     // button A arms and disarms
  buttonBCallback = silenceAlarm;    // button B must silence the alarm
  touchCallback = onTouchDetected;   // a touch starts the alarm (if armed)

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
      buttonACallback();   // call the callback
    }
  }

  // Button B (interrupt)
  // Flag pattern: did the ISR raise the flag?
  if (buttonBFlag == true) {
    buttonBFlag = false;   // lower the flag first, so the press is handled once
    buttonBCallback();    // then call the callback
  }

  // Touch sensor (interrupt)
  if (touchFlag == true) {
    touchFlag = false;     // lower the flag
    touchCallback();      // call the touch callback ("Call a Callback")
  }

  // Timer – turn the alarm off when ALARM_TIME has passed since alarmStart
  if (alarmOn == true && millis() - alarmStart >= ALARM_TIME) {
    alarmOff();
    Serial.println("Alarm timed out");
  }
}