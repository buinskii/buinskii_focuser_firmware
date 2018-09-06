#include <AccelStepper.h>
#include <SerialCommand.h>
#include "Arduino.h"

AccelStepper motor(AccelStepper::HALF4WIRE, 10, 11, 13, 12);
SerialCommand serialCommand;

#define DEVICE_RESPONSE "BAA.Focuser"

long mov = 0;
long set_current_pos;

bool is_mov	= false;
bool revers = false;

double temperature1 = 36.61;                    // Sensor #1 Temp
double temperature2 = 36.62;                    // Sensor #2 Temp
double temperature3 = 36.63;                    // Sensor #3 Temp

unsigned int speed = 800;
unsigned int acceleration = 400;

void handshake()
{
    Serial.print("A:T:");
    Serial.print(F(DEVICE_RESPONSE));
    Serial.print("#\n");
}

void setAcceleration()
{
    char *arg = serialCommand.next();
    acceleration = atoi (arg);
    motor.setAcceleration(acceleration);

    Serial.print("A:A ");
    Serial.print(acceleration);
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
    mov = atol(arg);
    motor.moveTo(mov);

    Serial.print("A:M ");
    Serial.print(mov);
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
    set_current_pos = atol(arg);
    motor.setCurrentPosition(set_current_pos);
    motor.moveTo(set_current_pos);

    Serial.print("A:SP ");
    Serial.print(set_current_pos);
    Serial.print("#\n");
}

void getPosition()
{
    Serial.print("A:GP:");
    Serial.print(motor.currentPosition());
    Serial.print("#\n");
}

void setSpeed()
{
    char *arg = serialCommand.next();
    speed = atoi (arg);
    motor.setMaxSpeed(speed);

    Serial.print("A:SS ");
    Serial.print(speed);
    Serial.print("#\n");
}

void getSpeed()
{
    Serial.print("A:GS:");
    Serial.print((int) motor.maxSpeed());
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

    motor.setMaxSpeed(speed);
    motor.setAcceleration(acceleration);

    serialCommand.addCommand("T", handshake);

    serialCommand.addCommand("A", setAcceleration);
    serialCommand.addCommand("Z", setRevers);

    serialCommand.addCommand("M", moveTo);
    serialCommand.addCommand("T1", getTemperature1);
    serialCommand.addCommand("T2", getTemperature2);
    serialCommand.addCommand("T3", getTemperature3);
    serialCommand.addCommand("H", halt);
    serialCommand.addCommand("I", isMoving);

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