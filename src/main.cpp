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


// ---------- Event handlers and callbacks ----------


// ---------- Interrupt Service Routines (ISRs) ----------


void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Lab 10: Event-Driven Programming");

  // Inputs and outputs

  // Interrupts

  // Callbacks
}

void loop() {
  // Button A (polling)

  // Button B (interrupt)

  // Touch sensor (interrupt)

  // Timer
}
