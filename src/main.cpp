#include <AccelStepper.h>
#include <SerialCommand.h>
#include "Arduino.h"
#include <EEPROM.h>
#include <IRremote.h>
#include "FocusHandController.h"

const int RECV_PIN = 8;
IRrecv irrecv(RECV_PIN);
decode_results results;


//-  - FFC23D
//- 51 - FFFFFFFF
//- 107 - FFFFFFFF
//- 108 - FFFFFFFF
//- 107 - FFFFFFFF
//- 107 - FFFFFFFF
//- 107 - FFFFFFFF
//- 107 - FFFFFFFF

const long BTN_RIGHT = 0xFFC23D;
const long BTN_LEFT = 0xFF22DD;
const long BTN_TOP = 0xFF629D;
const long BTN_BOTTOM = 0xFFA857;
const long BTN_OK = 0xFF02FD;
const long BTN_0 = 0xFF4AB5;
const long BTN_1 = 0xFF6897;
const long BTN_2 = 0xFF9867;
const long BTN_3 = 0xFFB04F;
const long BTN_4 = 0xFF30CF;
const long BTN_5 = 0xFF18E7;
const long BTN_6 = 0xFF7A85;
const long BTN_7 = 0xFF10EF;
const long BTN_8 = 0xFF38C7;
const long BTN_9 = 0xFF5AA5;
const long BTN_STAR = 0xFF5AA5;
const long BTN_SHARP = 0xFF52AD;

//AccelStepper motor(AccelStepper::HALF4WIRE, 10, 11, 13, 12);
//AccelStepper motor(AccelStepper::HALF4WIRE, pc3, pc1, pc2, pc0);
AccelStepper motor(AccelStepper::HALF4WIRE, 17, 15, 16, 14);
SerialCommand serialCommand;

#define DEVICE_RESPONSE "BAA.Focuser"

int tmp_position = 0;

bool is_mov	= false;
bool revers = false;

double temperature1 = 36.61;
double temperature2 = 36.62;
double temperature3 = 36.63;

const int DEFAULT_SPEED = 80;
const int DEFAULT_ACCELERATION = 20;

unsigned int speed = 80;
unsigned int acceleration = 20;

const int ADDR_SPEED = 0;
const int ADDR_ACCELERATION = ADDR_SPEED + sizeof(speed);
const int ADDR_POSITION = ADDR_ACCELERATION + sizeof(acceleration);

int safe_max_position = 4000;
int safe_min_position = 0;

unsigned long lastPositionSave = 0;
int lastPositionSaveValue = 0;

void savePosition(int val, bool force) {
    if (lastPositionSaveValue == val) {
        return;
    }
    if (lastPositionSaveValue != val && millis() - lastPositionSave > 60000) {
        EEPROM.put(ADDR_POSITION, val);
        lastPositionSaveValue = val;
        lastPositionSave = millis();
    }
    if (force) {
        EEPROM.put(ADDR_POSITION, val);
        lastPositionSaveValue = val;
        lastPositionSave = millis();
    }
}

void savePosition(int val) {
    return savePosition(val, val == 0);
}

void handshake()
{
    Serial.print("A:T:");
    Serial.print(F(DEVICE_RESPONSE));
    Serial.print("#\n");
}

void setRevers()
{
    char *arg = serialCommand.next();
    revers = atoi(arg);
    motor.setPinsInverted(revers);

    Serial.print("A:Z ");
    Serial.print(revers);
    Serial.print("#\n");
}

void moveTo()
{
    char *arg = serialCommand.next();
    tmp_position = atoi(arg);
    motor.moveTo(tmp_position);

    Serial.print("A:M ");
    Serial.print(tmp_position);
    Serial.print("#\n");
}

void getTemperature1()
{
    Serial.print("A:T1:");
    Serial.print(temperature1);
    Serial.print("#\n");
}

void getTemperature2()
{
    Serial.print("A:T2:");
    Serial.print(temperature2);
    Serial.print("#\n");
}

void getTemperature3()
{
    Serial.print("A:T3:");
    Serial.print(temperature3);
    Serial.print("#\n");
}

void halt()
{
    motor.stop();
    motor.runToPosition();

    Serial.print("A:H:");
    Serial.print(motor.currentPosition());
    Serial.print("#\n");
}

void isMoving()
{
    if (is_mov)
    {
        Serial.println("A:I:1#");
    }
    else
    {
        Serial.println("A:I:0#");
    }
}

void setPosition()
{
    char *arg = serialCommand.next();
    tmp_position = atoi(arg);
    motor.setCurrentPosition(tmp_position);

    Serial.print("A:SP ");
    Serial.print(tmp_position);
    Serial.print("#\n");
}

void getPosition()
{
    Serial.print("A:GP:");
    Serial.print(motor.currentPosition());
    Serial.print("#\n");
}

void setAcceleration()
{
    char *arg = serialCommand.next();
    acceleration = atoi (arg);
    motor.setAcceleration(acceleration * 10);

    EEPROM.put(ADDR_ACCELERATION, acceleration);

    Serial.print("A:SA ");
    Serial.print(acceleration);
    Serial.print("#\n");
}

void getAcceleration()
{
    Serial.print("A:GA:");
    Serial.print(acceleration);
    Serial.print("#\n");
}

void setSpeed()
{
    char *arg = serialCommand.next();
    speed = atoi (arg);
    motor.setMaxSpeed(speed * 10);

    EEPROM.put(ADDR_SPEED, speed);

    Serial.print("A:SS ");
    Serial.print(speed);
    Serial.print("#\n");
}

void getSpeed()
{
    Serial.print("A:GS:");
    Serial.print(speed);
    Serial.print("#\n");
}

void unrecog(const char *command)
{
    Serial.print("E:");
    Serial.print(command);
    Serial.print("#\n");
}

void setup()
{
    Serial.begin(9600);

    EEPROM.get(ADDR_SPEED, speed);
    if (speed <= 0 || speed > 100) {
        speed = DEFAULT_SPEED;
    }

    EEPROM.get(ADDR_ACCELERATION, acceleration);
    if (acceleration <= 0 || acceleration > 100) {
        acceleration = DEFAULT_ACCELERATION;
    }

    EEPROM.get(ADDR_POSITION, tmp_position);
    if (tmp_position < -10000 || tmp_position > 10000) {
        tmp_position = 0;
    }

    motor.setMaxSpeed(speed * 10);
    motor.setAcceleration(acceleration * 10);
    motor.setCurrentPosition(tmp_position);

    serialCommand.addCommand("T", handshake);

    serialCommand.addCommand("Z", setRevers);

    serialCommand.addCommand("M", moveTo);
    serialCommand.addCommand("T1", getTemperature1);
    serialCommand.addCommand("T2", getTemperature2);
    serialCommand.addCommand("T3", getTemperature3);
    serialCommand.addCommand("H", halt);
    serialCommand.addCommand("I", isMoving);

    serialCommand.addCommand("SA", setAcceleration);
    serialCommand.addCommand("GA", getAcceleration);

    serialCommand.addCommand("SS", setSpeed);
    serialCommand.addCommand("GS", getSpeed);

    serialCommand.addCommand("SP", setPosition);
    serialCommand.addCommand("GP", getPosition);

    serialCommand.setDefaultHandler(unrecog);

    irrecv.enableIRIn();
}

long activeButton = 0;
unsigned long activeButtonDownTime = 0;
unsigned long lastButtonRepeatTime = 0;
int btnDirection = 1;

void buttonUp(long btn, long downTime) {
    bool is_click = downTime < 700;

    if (btn == BTN_RIGHT) {
        btnDirection = 1;
        if (is_click) {
            if (motor.currentPosition() + 1 <= safe_max_position) {
                motor.move(1);
            }
        }
        else {
            motor.stop();
            motor.runToPosition();
        }
    }
    if (btn == BTN_LEFT) {
        btnDirection = -1;
        if (is_click) {
            if (motor.currentPosition() - 1 >= safe_min_position) {
                motor.move(-1);
            }
        }
        else {
            motor.stop();
            motor.runToPosition();
        }
    }

    if (btn == BTN_TOP) {
        btnDirection = 1;
        if (is_click) {
            motor.move(1);
        }
        else {
            motor.stop();
            motor.runToPosition();
        }
        if (motor.currentPosition() > safe_max_position) {
            motor.setCurrentPosition(safe_max_position);
        }
    }
    if (btn == BTN_BOTTOM) {
        btnDirection = -1;
        if (is_click) {
            motor.move(-1);
        }
        else {
            motor.stop();
            motor.runToPosition();
        }
        if (motor.currentPosition() < safe_min_position) {
            motor.setCurrentPosition(safe_min_position);
        }
    }

    if (btn == BTN_0) {
        if (is_click) {
            motor.moveTo(0);
        }
        else {
            motor.setCurrentPosition(0);
        }
    }

    if (btn == BTN_1) {
        if (is_click) {
            motor.move(1 * btnDirection);
        }
    }

    if (btn == BTN_2) {
        if (is_click) {
            motor.move(2 * btnDirection);
        }
    }

    if (btn == BTN_3) {
        if (is_click) {
            motor.move(3 * btnDirection);
        }
    }

    if (btn == BTN_4) {
        if (is_click) {
            motor.move(4 * btnDirection);
        }
    }

    if (btn == BTN_5) {
        if (is_click) {
            motor.move(5 * btnDirection);
        }
    }

    if (btn == BTN_6) {
        if (is_click) {
            motor.move(6 * btnDirection);
        }
    }

    if (btn == BTN_7) {
        if (is_click) {
            motor.move(7 * btnDirection);
        }
    }

    if (btn == BTN_8) {
        if (is_click) {
            motor.move(8 * btnDirection);
        }
    }

    if (btn == BTN_9) {
        if (is_click) {
            motor.move(9 * btnDirection);
        }
    }
}

void buttonRepeat(long btn, long downTime) {

}

bool buttonDown(long btn) {
    if (btn == BTN_RIGHT) {
        return true;
    }
    if (btn == BTN_LEFT) {
        return true;
    }
    if (btn == BTN_TOP) {
        return true;
    }
    if (btn == BTN_BOTTOM) {
        return true;
    }
    if (btn == BTN_0) {
        return true;
    }
    if (btn == BTN_1 || btn == BTN_2 || btn == BTN_3 || btn == BTN_4 || btn == BTN_5 || btn == BTN_6 || btn == BTN_7 || btn == BTN_8 || btn == BTN_9) {
        return true;
    }

    return false;
}

void buttonAction(long btn, long downTime) {
    if (btn == BTN_RIGHT && downTime > 700) {
        motor.moveTo(safe_max_position);
    }
    if (btn == BTN_LEFT && downTime > 700) {
        motor.moveTo(safe_min_position);
    }
    if (btn == BTN_TOP && downTime > 700) {
        motor.moveTo(10000);
    }
    if (btn == BTN_BOTTOM && downTime > 700) {
        motor.moveTo(-10000);
    }
}

void loop()
{
    is_mov = motor.distanceToGo() != 0;

    if (irrecv.decode(&results)) {
        if (activeButton > 0 && results.value == 0xFFFFFFFF) {
            lastButtonRepeatTime = millis();

            buttonRepeat(activeButton, millis() - activeButtonDownTime);
        }
        else if (buttonDown(results.value)) {
            activeButton = results.value;
            activeButtonDownTime = millis();
            lastButtonRepeatTime = activeButtonDownTime;
        }
        else if (activeButton > 0) {
            buttonUp(activeButton, millis() - activeButtonDownTime);

            activeButton = 0;
            activeButtonDownTime = 0;
            lastButtonRepeatTime = 0;
        }

        irrecv.resume();
    }

    if (activeButton > 0 && millis() - lastButtonRepeatTime > 200) {
        buttonUp(activeButton, millis() - activeButtonDownTime);

        activeButton = 0;
        activeButtonDownTime = 0;
        lastButtonRepeatTime = 0;
    }

    if (activeButton > 0) {
        buttonAction(activeButton, millis() - activeButtonDownTime);
    }

    serialCommand.readSerial();

    motor.run();

    if (!is_mov) {
        savePosition(motor.currentPosition());
    }
}