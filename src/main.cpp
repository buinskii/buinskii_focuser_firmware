#include <AccelStepper.h>
#include <SerialCommand.h>
#include "Arduino.h"
#include <EEPROM.h>

//AccelStepper motor(AccelStepper::HALF4WIRE, 10, 11, 13, 12);
//AccelStepper motor(AccelStepper::HALF4WIRE, pc3, pc1, pc2, pc0);
AccelStepper motor(AccelStepper::HALF4WIRE, 17, 15, 16, 14);
SerialCommand serialCommand;

#define DEVICE_RESPONSE "BAA.Focuser"

int position = 0;

bool is_mov	= false;
bool revers = false;

double temperature1 = 36.61;
double temperature2 = 36.62;
double temperature3 = 36.63;

const int DEFAULT_SPEED = 80;
const int DEFAULT_ACCELERATION = 40;

unsigned int speed = 80;
unsigned int acceleration = 40;

const int ADDR_SPEED = 0;
const int ADDR_ACCELERATION = ADDR_SPEED + sizeof(speed);
const int ADDR_POSITION = ADDR_ACCELERATION + sizeof(acceleration);

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
    position = atoi(arg);
    motor.moveTo(position);

    EEPROM.put(ADDR_POSITION, position);

    Serial.print("A:M ");
    Serial.print(position);
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
    motor.moveTo(motor.currentPosition());

    position = motor.currentPosition();
    EEPROM.put(ADDR_POSITION, position);

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
    position = atoi(arg);
    motor.setCurrentPosition(position);
    motor.moveTo(position);

    EEPROM.put(ADDR_POSITION, position);

    Serial.print("A:SP ");
    Serial.print(position);
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

    EEPROM.get(ADDR_POSITION, position);
    if (position < -30000 || position > 30000) {
        position = 0;
    }

    motor.setMaxSpeed(speed * 10);
    motor.setAcceleration(acceleration * 10);
    motor.setCurrentPosition(position);
    motor.moveTo(position);

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
}

void loop()
{
    is_mov = motor.distanceToGo() != 0;

    serialCommand.readSerial();
    motor.run();

}