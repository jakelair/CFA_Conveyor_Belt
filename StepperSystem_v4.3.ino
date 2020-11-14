/*********************************************************************
 * Project          : Chick-fil-A Drive-thru Beverage Conveyor Belt
 * 
 * Program Name     : StepperSystem_v4.3
 * 
 * Author           : Jake Lair
 * 
 * Date created     : 11/14/2020
 * 
 * Purpose          : Software for operating the conveyor belt
 * 
 * Microcontroller  : Arduino Nano Every
 ********************************************************************/

// Include Files
/*
 * Nothing to Include
 */

// Pin Declaration
#define Stepper_En_NOT 10  // Stepper Motor Enable (Active Low)
#define Stepper_Pul     8   // Stepper Motor Driver Pulse
#define Stepper_Dir     6   // Stepper Motor Driver Direction
#define Sensor_Pin      4   // Photoelectric Sensor
#define EStop           2   // Latched Emergency Pushbutton

// Global Variables
const unsigned long Stepper_Speed = 200;
const unsigned long RampTime = 400;
unsigned long current_time;
unsigned long current_time_micro;

volatile unsigned long timestamp_milli;
volatile unsigned long timestamp_micro;
volatile bool EStop_State;          // 0 (EStop is unlatched) and 1 (EStop is latched)
volatile bool Sensor_State;         // 0 (No obstructions)    and 1 (Obstruction)
volatile bool Motor_Stopped = 1;    // 0 (Motor is moving)    and 1 (Motor is stopped)


//*****************************************//
// SETUP Function                          //
//*****************************************//
void setup() {
  // Setting pinMode for I/O signals
  pinMode(Sensor_Pin, INPUT);
  pinMode(EStop, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(Stepper_En_NOT, OUTPUT);
  pinMode(Stepper_Pul, OUTPUT);
  pinMode(Stepper_Dir, OUTPUT);

  // Verify correct code for end user by blinking LED_BUILTIN
  for (int i = 0; i < 1; i++) {
    for (int j = 0; j < 1; j++) {
      digitalWrite(LED_BUILTIN, HIGH);
          timestamp_milli = millis();
          while (millis() - current_time < 250);
      digitalWrite(LED_BUILTIN, LOW);
          timestamp_milli = millis();
          while (millis() - current_time < 250);
    }
    
    while (millis() - current_time < 1000);
  }


  // Setting Initial Output States and Routine States
  digitalWrite(Stepper_En_NOT, HIGH);
  digitalWrite(Stepper_Dir, LOW);  // High (CCW) and Low (CW)

  if (digitalRead(EStop)) {
    EStop_State = 1;
  }
  else {
    EStop_State = 0;
  }
  
  if (digitalRead(Sensor_Pin)) {
    Sensor_State = 1;
  }
  else {
    Sensor_State = 0;
  }

  // Setting Interrupt Service Routines (ISR) Functions
  attachInterrupt(digitalPinToInterrupt(EStop), EStop_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(Sensor_Pin), Sensor_ISR, CHANGE);

  // Start up the motor is both EStop and Sensor states are 0
  if (!(EStop_State || Sensor_State)) {
    Speed_Up();
  }
}


//*****************************************//
// LOOP Function                           //
//*****************************************//
void loop() {
  // Check if the system can move -> not in EStop or Sensor states
  if (!(EStop_State || Sensor_State)) {
    digitalWrite(Stepper_Pul, LOW);
    //delayMicroseconds(60);
        timestamp_micro = micros();
        while (micros() - timestamp_micro < (Stepper_Speed));
    digitalWrite(Stepper_Pul, HIGH);
        timestamp_micro = micros();
        while (micros() - timestamp_micro < (Stepper_Speed));
  }
  
  // Stopping the Motor and waiting until the EStop state is resolved
  else if (EStop_State) {
    if (!Motor_Stopped) {
      Speed_Down();
    }
        
    // Waiting until the EStop state is active
    while (digitalRead(EStop)) { }

    EStop_State = 0;

    // If the system is not in Sensor state and the motor is stopped
    //    Start the motor again
    if (Motor_Stopped && !Sensor_State) {
      Speed_Up();
    }
  }
  
  // Stopping the Motor and waiting until the Sensor state is resolved
  else if (Sensor_State) {
    if (!Motor_Stopped) {
      Speed_Down();
    }
    
    // Blinking Yellow LED only is the Pause State is active and EStop is not
    while (Sensor_State) {
      if (EStop_State) {
        break;
      }
    }

    // If the system is not in EStop state and the motor is stopped
    //    Start the motor again
    if (Motor_Stopped && !EStop_State) {
      Speed_Up();
    }
  }

  else {
    // Do Nothing
  }
}


//*****************************************//
// ISR Functions                           //
//*****************************************//
// Interrupt function called when EStop pin has a RISING edge
void EStop_ISR() {
  EStop_State = 1;
}

// Interrupt function called when Sensor pin changes states (HIGH to/from LOW)
void Sensor_ISR() {
  Sensor_State = !Sensor_State;
}


//*****************************************//
// Other Functions                         //
//*****************************************//
// Slowing speeding up the motor until set speed
void Speed_Up() {
  digitalWrite(Stepper_En_NOT, LOW);
  Motor_Stopped = 0;
  int i;
  for (i = Stepper_Speed + RampTime; i > Stepper_Speed; i--) {
    if (EStop_State || Sensor_State) {
      break;
    }
    digitalWrite(Stepper_Pul, LOW);
        timestamp_micro = micros();
        while (micros() - timestamp_micro < (i/2));
    digitalWrite(Stepper_Pul, HIGH);
        timestamp_micro = micros();
        while (micros() - timestamp_micro < i);
  }
}

// Slowing reducing the speed of the motor
void Speed_Down() {
  int i;
  for (i = Stepper_Speed; i < Stepper_Speed + RampTime; i++) {
    digitalWrite(Stepper_Pul, LOW);
        timestamp_micro = micros();
        while (micros() - timestamp_micro < (i/2));
    digitalWrite(Stepper_Pul, HIGH);
        timestamp_micro = micros();
        while (micros() - timestamp_micro < i);
  }
  digitalWrite(Stepper_En_NOT, HIGH);
  Motor_Stopped = 1;
}
