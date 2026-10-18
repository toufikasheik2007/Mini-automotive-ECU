// ============================================================================
// Project Title     : STM32 NUCLEO C031C6 - ECU SYSTEM
// GitHub Repository : https://github.com/toufikasheik2007/toufikasheik2007
// Author            : Toufika Sheik
// Wokwi project     : https://wokwi.com/projects/476752944692923393
// Description       : Engine Control Unit (ECU) simulation state machine 
//                     monitoring temperature, RPM, and battery level via ADC.
// ============================================================================

#include <Arduino.h>

// ==========================================
// STM32 NUCLEO C031C6 - ECU SYSTEM
// ==========================================

// Sensor pins
const int PIN_TEMP = PA0;
const int PIN_RPM  = PA1;
const int PIN_BATT = PA4;

// Ignition / fault button
const int PIN_BTN = D2;

// Cooling fan LED
const int PIN_FAN = D3;


// ==========================================
// ECU STATES
// ==========================================

enum ECUState {
  STATE_OFF,
  STATE_NORMAL,
  STATE_WARNING,
  STATE_FAULT
};

ECUState currentECUState = STATE_OFF;


// ==========================================
// VARIABLES
// ==========================================

bool lastButtonState = HIGH;

unsigned long lastTelemetry = 0;


// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_FAN, OUTPUT);

  Serial.println();
  Serial.println("========================================");
  Serial.println("       STM32 C031 ECU SYSTEM");
  Serial.println("========================================");
  Serial.println("ECU SYSTEM INITIALIZED");
  Serial.println("Turn the potentiometers to change values.");
  Serial.println("Press the red button for ignition.");
  Serial.println();
}


// ==========================================
// LOOP
// ==========================================

void loop() {

  // ----------------------------------------
  // READ ADC VALUES
  // ----------------------------------------

  int adcTemp = analogRead(PIN_TEMP);
  int adcRPM  = analogRead(PIN_RPM);
  int adcBatt = analogRead(PIN_BATT);


  // ----------------------------------------
  // CONVERT ADC TO ECU VALUES
  // ----------------------------------------

  // Temperature: 0 - 120 °C
  int temperature = map(adcTemp, 0, 4095, 0, 120);

  // RPM: 0 - 6000 RPM
  int rpm = map(adcRPM, 0, 4095, 0, 6000);

  // Battery: 0 - 15.0 V
  int battery10x = map(adcBatt, 0, 4095, 0, 150);


  // ----------------------------------------
  // READ IGNITION BUTTON
  // ----------------------------------------

  bool buttonState = digitalRead(PIN_BTN);

  bool buttonPressed =
    (lastButtonState == HIGH && buttonState == LOW);

  lastButtonState = buttonState;


  // ----------------------------------------
  // ECU STATE MACHINE
  // ----------------------------------------

  switch (currentECUState) {

    // ==============================
    // OFF
    // ==============================

    case STATE_OFF:

      analogWrite(PIN_FAN, 0);

      if (buttonPressed) {

        currentECUState = STATE_NORMAL;

        Serial.println("[ECU EVENT] Ignition turned ON.");
      }

      break;


    // ==============================
    // NORMAL
    // ==============================

    case STATE_NORMAL:

      // Fan approximately 20%
      analogWrite(PIN_FAN, 51);

      if (buttonPressed) {

        currentECUState = STATE_FAULT;

        Serial.println("[ECU CRITICAL] Manual Fault Override!");

      }

      else if (temperature > 100 || rpm > 5000) {

        currentECUState = STATE_FAULT;

        Serial.println("[ECU CRITICAL] Threshold Exceeded!");

      }

      else if (temperature > 80 || battery10x < 110) {

        currentECUState = STATE_WARNING;

        Serial.println("[ECU WARNING] Warning Level Detected!");
      }

      break;


    // ==============================
    // WARNING
    // ==============================

    case STATE_WARNING:

      // Maximum fan
      analogWrite(PIN_FAN, 255);

      if (buttonPressed ||
          temperature > 100 ||
          rpm > 5000) {

        currentECUState = STATE_FAULT;

        Serial.println("[ECU CRITICAL] FAULT State!");

      }

      else if (temperature <= 75 &&
               battery10x >= 115) {

        currentECUState = STATE_NORMAL;

        Serial.println("[ECU EVENT] Conditions Normal.");
      }

      break;


    // ==============================
    // FAULT
    // ==============================

    case STATE_FAULT:

      analogWrite(PIN_FAN, 0);

      if (buttonPressed) {

        currentECUState = STATE_OFF;

        Serial.println("[ECU EVENT] Fault Reset. System OFF.");
      }

      break;
  }


  // ========================================
  // SERIAL TELEMETRY
  // ========================================

  if (millis() - lastTelemetry >= 500) {

    lastTelemetry = millis();


    Serial.print("STATE: ");

    switch (currentECUState) {

      case STATE_OFF:
        Serial.print("OFF");
        break;

      case STATE_NORMAL:
        Serial.print("NORMAL");
        break;

      case STATE_WARNING:
        Serial.print("WARNING");
        break;

      case STATE_FAULT:
        Serial.print("FAULT");
        break;
    }


    Serial.print(" | Temp: ");
    Serial.print(temperature);
    Serial.print(" C");


    Serial.print(" | RPM: ");
    Serial.print(rpm);


    Serial.print(" | Batt: ");
    Serial.print(battery10x / 10);
    Serial.print(".");
    Serial.print(battery10x % 10);
    Serial.println(" V");
  }


  delay(10);
}