/* Initial Gun Test

The circuit:
Pusher switch on pin 4
Acceleration motor relay input on pin 7
Firing motor relay input on pin 8
Manual input on pin 12

*/

#include <Servo.h>

#define REV_UP_TIME      300
#define MAX_FIRE_TIME   1000
#define MAX_TILT_UP     2000
#define MAX_TILT_DOWN   1300
#define MAX_PAN_LEFT     800
#define MAX_PAN_RIGHT   2200
#define TILT_MIDPOINT   1500
#define PAN_MIDPOINT    1455

#define ACCELERATION       5
#define UPDATE_INTERVAL 2000

const int pusherSwitchPin      =  4;
const int accelerationMotorPin =  7;
const int pusherMotorPin       =  8;
const int tiltServoPin         = 10;
const int panServoPin          = 11;
const int buttonPin            = 12;
const int ledPin               = 13;

const char waitCode      = 'w';
const char errorCode     = 'x';
const char invalidCode   = 'i';

const char startCode     = 's';
const char endCode       = 'e';
const char revCode       = 'r';
const char fireCode      = 'f';
const char tiltCode      = 't';
const char panCode       = 'p';
const char velocityCode  = 'v';
const char heartbeatCode = 'h';
const char killCode      = 'k';

const int commandInputCount = 7;
char validInputs[commandInputCount] = {
    revCode, fireCode, tiltCode, panCode, velocityCode, heartbeatCode, killCode
};

const int startAutoFireCode = 65535;
const int stopAutoFireCode = 0;

// state variables
float panState       = PAN_MIDPOINT;
int panGoal          = (int) panState;
int panHalfway       = (int) panState;
float panVelocity    = 0.0;
int tiltGoal         = TILT_MIDPOINT;
int tiltState        = TILT_MIDPOINT;
int tiltVelocity     = 0;
int tiltAcceleration = 0;

bool velocityMode = false;

long lastUpdateTime = micros();

long timeElapsedSinceUpdate;
int serialAction;
unsigned int serialParameter;
String s;
int i;
// PWM range 556-2420 => 1488
Servo panServo;
Servo tiltServo;

bool withinRange(int var, int low, int high) {
    return low <= var && var <= high;
}

void revUp() {
    digitalWrite(accelerationMotorPin, LOW);
}

void revDown() {
    digitalWrite(accelerationMotorPin, HIGH);
}

void pusherOn() {
    digitalWrite(pusherMotorPin, LOW);
}

void pusherOff() {
    digitalWrite(pusherMotorPin, HIGH);
}

void pulsePusher() {
    pusherOn();
    delay(15);
    pusherOff();
    delay(80);
}

void resetPusher() {
    pusherOff();
    while (digitalRead(pusherSwitchPin) == HIGH) pulsePusher();
}

/*
---------------------------------------------------------
                        Movement
*/

void setPanLocation(int argument) {
    // argument: [-700, 700]
    panHalfway = (panGoal + (PAN_MIDPOINT + argument)) / 2;
    panGoal = PAN_MIDPOINT + argument;
}

void setTiltLocation(int argument) {
    // argument: [-300, 500]
    tiltGoal = TILT_MIDPOINT + argument;
}

void updateState() {

}

void pan(int location) {
    if (withinRange(location, MAX_PAN_LEFT, MAX_PAN_RIGHT)) {
        panServo.writeMicroseconds(location);
    }// else {
    //     reportError("Pan location out of range!");
    // }
}

void tilt(int location) {
    if (withinRange(location, MAX_TILT_DOWN, MAX_TILT_UP)) {
        tiltServo.writeMicroseconds(location);
    }// else {
    //     reportError("Tilt location out of range!");
    // }
}

/*
---------------------------------------------------------
                         Firing
*/

//void rev(int argument) {
//    if (argument == 0) {
//        revDown();
//    } else if (argument == 65535) {
//        revUp();
//    } else {
//        reportError("Bad rev argument!");
//    }
//}
//
//void fireShots(int numShots) {
//    startTime = millis();
//    revUp();
//    while (millis() - startTime < REV_UP_TIME);
//    for ( ; numShots > 0; numShots--) {
//        startTime = millis();
//        while (digitalRead(pusherSwitchPin) == LOW && millis() - startTime < MAX_FIRE_TIME) pulsePusher();
//        while (digitalRead(pusherSwitchPin) == HIGH && millis() - startTime < MAX_FIRE_TIME) pulsePusher();
//        if (millis() - startTime > MAX_FIRE_TIME) {
//            revDown();
//            reportError("Firing sensor not responding!");
//        }
//    }
//    revDown();
//}
//
//void fire(int argument) {
//    if (withinRange(argument, 1, 37)) {
//        fireShots(argument);
//    } else if (argument == 0) {
//        resetPusher();
//    } else if (argument == 65535) {
//        pusherOn();
//    } else {
//        reportError("Inproper number of shots received!");
//    }
//}

/*
---------------------------------------------------------
                Serial Communication
*/

void requestInput() {
    Serial.write(waitCode);
}

void reportInvalidInput(char message[]) {
    Serial.write(invalidCode);
    Serial.println(message);
}

bool isValidInput(char operationCode) {
    for (i = 0; i < commandInputCount; i++) {
        if (operationCode == validInputs[i]) return true;
    }
    return false;
}

byte getNextByte() {
    do {
        serialAction = Serial.read();
    } while (serialAction == -1);
    return byte(serialAction);
}

void processInput() {
    while (Serial.available() >= 5) {
        if (Serial.read() != startCode) {
            reportInvalidInput("Expected start byte");
            break;
        }
        char operationCode = Serial.read();
        if (!isValidInput(operationCode)) {
            reportInvalidInput("Invalid operation code");
            break;
        }

        byte byte0 = Serial.read();
        byte byte1 = Serial.read();

        if (Serial.read() != endCode) {
            reportInvalidInput("Expected end byte");
            break;
        }

        switch (operationCode) {
            case revCode:
                {
                    // [0, 255]
                    uint8_t speed = byte0;
                    uint8_t timeout = byte1;
                }
                break;
            case fireCode:
                {
                    // [0, 255]
                    uint8_t shotCount = byte0;
                    uint8_t shootingFrequency = byte1;
                }
                break;
            case tiltCode:
                lastUpdateTime = micros();
                setTiltLocation((int(byte0) << 8) + int(byte1));
                break;
            case panCode:
                lastUpdateTime = micros();
                setPanLocation((int(byte0) << 8) + int(byte1));
                break;
            case velocityCode:
                // [-128, 127]
//                int8_t panVelocity = byte0;
//                int8_t tiltVelocity = byte1;
                break;
            case heartbeatCode:
                break;
            case killCode:
                break;
        }

        requestInput();
    }
}

/*
---------------------------------------------------------
*/

void setup() {
    Serial.begin(9600);
    pinMode(pusherSwitchPin, INPUT_PULLUP);
    pinMode(accelerationMotorPin, OUTPUT);
    pinMode(pusherMotorPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    panServo.attach(panServoPin);
    tiltServo.attach(tiltServoPin);
    pan(PAN_MIDPOINT);
    tilt(TILT_MIDPOINT);
    revDown();
    resetPusher();
    //Serial.flush();
    requestInput();
}

void loop() {
    processInput();
    timeElapsedSinceUpdate = micros() - lastUpdateTime;
    if ((int) panState != panGoal && timeElapsedSinceUpdate > UPDATE_INTERVAL) {
        lastUpdateTime = micros();

        bool movingRight = panHalfway < panGoal;
        bool halfwayDone = (movingRight && panState >= (float) panHalfway) || (!movingRight && panState <= (float) panHalfway);
        float increment = ACCELERATION * timeElapsedSinceUpdate / 1000000.0;
        if (!halfwayDone) {
            panVelocity += increment;
        } else {
            panVelocity -= increment;
            if (panVelocity < 0.1) {
                panVelocity = 0.1;
            }
        }
        if (movingRight) {
            panState += panVelocity;
        } else {
            panState -= panVelocity;
        }
        if ((movingRight && (int) panState > panGoal) || (!movingRight && (int) panState < panGoal)) {
            panState = (float) panGoal;
            panVelocity = 0.0;
        }
        // if (movingRight) {
        //     bool halfwayDone = panState >= (float) panHalfway;
        //     if (!halfwayDone) {
        //         panVelocity += increment;
        //     } else {
        //         panVelocity -= increment;
        //         if (panVelocity < 0.1) {
        //             panVelocity = 0.1;
        //         }
        //     }
        //     panState += panVelocity;
        //     if ((int) panState > panGoal) {
        //         panState = (float) panGoal;
        //         panVelocity = 0.0;
        //     }
        // } else {
        //     bool halfwayDone = panState <= (float) panHalfway;
        //     if (!halfwayDone) {
        //         panVelocity += increment;
        //     } else {
        //         panVelocity -= increment;
        //         if (panVelocity < 0.1) {
        //             panVelocity = 0.1;
        //         }
        //     }
        //     panState -= panVelocity;
        //     if ((int) panState < panGoal) {
        //         panState = (float) panGoal;
        //         panVelocity = 0.0;
        //     }
        // }
        pan((int) panState);
    }
}
