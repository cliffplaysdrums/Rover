/* Sonar pins:
 *  pin 3: PWM input reading
 */

#include <Adafruit_MotorShield.h>
#include <Wire.h> 
#include "RoverConstants.h"

//prototypes
uint8_t distance();
uint8_t median(int, int, int);
void roverForward(uint8_t);
void roverBackward(uint8_t);
void roverLeft(uint8_t);
void roverRight(uint8_t);
void roverStop();
void guidanceSystem();
void safeRetreat();

const int sonarInPin = 3;
const int sonarTimeout = 100000; //timeout in microseconds if no pulse received
const unsigned long turnTime = 4000;
const unsigned long turnTime_short = 2000;

boolean turnRight;


Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *rightMotor = AFMS.getMotor(1);
Adafruit_DCMotor *leftMotor = AFMS.getMotor(2);

void setup() {
  Serial.begin(9600);

  pinMode(sonarInPin, INPUT);
  turnRight = true;

  AFMS.begin();

  Serial.println("Setup complete.");
}

void loop() {
  uint8_t d = distance();
  Serial.println(d);

  if (d >= FARTHEST) {
    roverForward(LUDACRIS);
  } else if (d >= FARTHER) {
    roverForward(FASTEST);
  } else if (d >= FAR) {
    roverForward(FAST);
  } else if (d >= CLOSE) {
    roverForward(NORMAL);
  } else if (d >= DANGER) {
    roverStop();
    guidanceSystem();
  }

} //end loop()

uint8_t distance() {
  uint8_t d1, d2, d3;
  d1 = pulseIn(sonarInPin, HIGH) / 147;
  d2 = pulseIn(sonarInPin, HIGH) / 147;
  d3 = pulseIn(sonarInPin, HIGH) / 147;

  return median(d1, d2, d3);
} //end distance()

uint8_t median(int a, int b, int c) {
  if (a == b || a == c) return a;
  if (b == c) return b;

  uint8_t max, min;
  if (a > b) {
    max = a;
    min = b;
  } else {
    max = b;
    min = a;
  }

  if (c > max) {
    return max;
  } else if (c < min) {
    return min;
  } else {
    return c;
  }
} //end median(int, int, int)

void roverForward(uint8_t motorSpeed) {
  rightMotor->setSpeed(motorSpeed);
  leftMotor->setSpeed(motorSpeed);
  rightMotor->run(FORWARD);
  leftMotor->run(FORWARD);
}

void roverBackward(uint8_t motorSpeed) {
  rightMotor->setSpeed(motorSpeed);
  leftMotor->setSpeed(motorSpeed);
  rightMotor->run(BACKWARD);
  leftMotor->run(BACKWARD);
}

void roverLeft(uint8_t motorSpeed) {
  rightMotor->setSpeed(motorSpeed);
  leftMotor->setSpeed(motorSpeed);
  rightMotor->run(FORWARD);
  leftMotor->run(BACKWARD);
}

void roverRight(uint8_t motorSpeed) {
  rightMotor->setSpeed(motorSpeed);
  leftMotor->setSpeed(motorSpeed);
  rightMotor->run(BACKWARD);
  leftMotor->run(FORWARD);
}

void roverStop() {
  rightMotor->run(RELEASE);
  leftMotor->run(RELEASE);
}

void guidanceSystem() {
  uint8_t d = distance();
  unsigned long startTime = millis();

  if (turnRight) {
    while (d <= (CLOSE + BUFFER)) {
      d = distance();
      if (d <= DANGER) {
        safeRetreat();
        startTime = millis();
      }
      if ((millis() - startTime) > turnTime) {
        turnRight = false;
        startTime = millis();
        while ((millis() - startTime) < turnTime_short) {
          roverLeft(SLOWEST);
        }
        break;
      }
      roverRight(CRAWL);
    }
  } else {
    while (d <= (CLOSE + BUFFER)) {
      d = distance();
      if (d <= DANGER) {
        safeRetreat();
        startTime = millis();
      }
      if ((millis() - startTime) > turnTime) {
        turnRight = true;
        startTime = millis();
        while ((millis() - startTime) < turnTime_short) {
          roverRight(SLOWEST);
        }
        break;
      }
      roverLeft(CRAWL);
    }
  }

} //end guidanceSystem()

/*
 * While the object is extremely close (DANGER), move backwards slowly.
 * After 3 seconds, test if the object is any farther away, and turn right
 *   if it isn't.
 */
void safeRetreat() {
  unsigned long startTime = millis();
  unsigned long currTime;
  uint8_t initialD = distance();
  while (distance() <= DANGER) {
    currTime = millis();
    if (((currTime - startTime) > 3000) && (distance() == initialD)) {
      while ((millis() - currTime) < 750) {
        if (turnRight) {
          roverRight(SLOWEST);
        } else {
          roverLeft(SLOWEST);
        }
      }
      safeRetreat();
    } else {
      roverBackward(CRAWL);
    }
  }
} //end safeRetreaat()



