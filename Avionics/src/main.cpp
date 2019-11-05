#include <Arduino.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include "SparkFunBME280.h"
#include "SparkFunBME280.h"
MPU6050 acsensor;

enum class Mode{
    standby,
    flight,
    rise,
    parachute,
};

namespace global{
    Mode mode;
}

void setup() {
    Wire.begin();
    Serial.begin(9600);

    global::mode = Mode::standby;

    acsensor.initialize();
    pinMode(13, OUTPUT);
    Serial.println("AC Initialize done!");
}

void loop() {
    int16_t accelx = acsensor.getAccelerationX(), accely = acsensor.getAccelerationY(), accelz = acsensor.getAccelerationZ();
    double accelres = sqrt(accelx*accelx + accely*accely + accelz*accelz);

    switch (global::mode)
    {
        case Mode::standby:
            break;

        case Mode::flight:
            break;

        case Mode::rise:
            break;

        case Mode::parachute:
            break;
        
        default:
            break;
    }
}


