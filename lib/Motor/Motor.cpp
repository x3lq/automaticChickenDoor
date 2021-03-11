#include "Motor.h"
#include <Arduino.h>

#define MOTOR_STEPS 64

#define       MOTOR_PHASE_A           13                                  // motor phase a pin
#define       MOTOR_PHASE_B           12                                  // motor phase b pin
#define       MOTOR_PHASE_C           14                                  // motor phase c pin
#define       MOTOR_PHASE_D           27                                  // motor phase d pin

#define       MILLISECONDS_STEP_HOLD  1                                 // time in milliseconds between motor steps


void Motor::motorOff() {
    digitalWrite(MOTOR_PHASE_A, LOW);
    digitalWrite(MOTOR_PHASE_B, LOW);
    digitalWrite(MOTOR_PHASE_C, LOW);
    digitalWrite(MOTOR_PHASE_D, LOW);
}

void  Motor::step(int nDirection)
{
  // Local variables.
  
  static  int nPhase = 0;

  // Update phase.
  
  nPhase = ((nDirection < 0) ? (nPhase - 1) : (nPhase + 1)) & 7;

  // Step this phase.
  
  switch(nPhase)
  {
    case 0:
    {
      digitalWrite(MOTOR_PHASE_D, HIGH);
      digitalWrite(MOTOR_PHASE_C, LOW);
      digitalWrite(MOTOR_PHASE_B, LOW);
      digitalWrite(MOTOR_PHASE_A, LOW);
    }
    break;

    case 1:
    {
      digitalWrite(MOTOR_PHASE_D, HIGH);
      digitalWrite(MOTOR_PHASE_C, HIGH);
      digitalWrite(MOTOR_PHASE_B, LOW);
      digitalWrite(MOTOR_PHASE_A, LOW);
    }
    break;

    case 2:
    {
      digitalWrite(MOTOR_PHASE_D, LOW);
      digitalWrite(MOTOR_PHASE_C, HIGH);
      digitalWrite(MOTOR_PHASE_B, LOW);
      digitalWrite(MOTOR_PHASE_A, LOW);
    }
    break;

    case 3:
    {
      digitalWrite(MOTOR_PHASE_D, LOW);
      digitalWrite(MOTOR_PHASE_C, HIGH);
      digitalWrite(MOTOR_PHASE_B, HIGH);
      digitalWrite(MOTOR_PHASE_A, LOW);
    }
    break;

    case 4:
    {
      digitalWrite(MOTOR_PHASE_D, LOW);
      digitalWrite(MOTOR_PHASE_C, LOW);
      digitalWrite(MOTOR_PHASE_B, HIGH);
      digitalWrite(MOTOR_PHASE_A, LOW);
    }
    break;

    case 5:
    {
      digitalWrite(MOTOR_PHASE_D, LOW);
      digitalWrite(MOTOR_PHASE_C, LOW);
      digitalWrite(MOTOR_PHASE_B, HIGH);
      digitalWrite(MOTOR_PHASE_A, HIGH);
    }
    break;

    case 6:
    {
      digitalWrite(MOTOR_PHASE_D, LOW);
      digitalWrite(MOTOR_PHASE_C, LOW);
      digitalWrite(MOTOR_PHASE_B, LOW);
      digitalWrite(MOTOR_PHASE_A, HIGH);
    }
    break;

    case 7:
    {
      digitalWrite(MOTOR_PHASE_D, HIGH);
      digitalWrite(MOTOR_PHASE_C, LOW);
      digitalWrite(MOTOR_PHASE_B, LOW);
      digitalWrite(MOTOR_PHASE_A, HIGH);
    }
    break;
  }

  // Hold this step for MILLISECONDS_STEP_HOLD milliseconds.
   
  delay(MILLISECONDS_STEP_HOLD);
}

void Motor::write(int a,int b,int c,int d){
    digitalWrite(MOTOR_PHASE_A,a);
    digitalWrite(MOTOR_PHASE_B,b);
    digitalWrite(MOTOR_PHASE_C,c);
    digitalWrite(MOTOR_PHASE_D,d);
}

void Motor::setup() {
    pinMode(MOTOR_PHASE_A,OUTPUT);
    pinMode(MOTOR_PHASE_B,OUTPUT);
    pinMode(MOTOR_PHASE_C,OUTPUT);
    pinMode(MOTOR_PHASE_D,OUTPUT);
}

void Motor::oneTurnCW() {
    for(int i=0; i<4000; i++) {
        step(i);
    }
}

void Motor::oneTurnCCW() {
    for(int i=0; i>-4000; i--) {
        step(i);
    }
}

void Motor::nTurnsCCW(float turns) {
    int realTurns = (int) (turns * 4000);
    int revolutions = (int) (realTurns / 4000);
    int steps = realTurns % 4000;

    for(int i=revolutions; i!=0; i--) {
        oneTurnCCW();
    }

    for(int i=steps; i!=0; i--) {
        step(-i);
    }
}

//1000 is 1/4 step
void Motor::nTurnsCW(float turns) {
    int realTurns = (int) (turns * 4000);
    int revolutions = (int) (realTurns / 4000);
    int steps = realTurns % 4000;

    for(int i=revolutions; i!=0; i--) {
        oneTurnCW();
    }

    for(int i=steps; i!=0; i--) {
        step(i);
    }
}