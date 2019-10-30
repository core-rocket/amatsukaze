#include <Arduino.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>

MPU6050 acsensor;

void setup() {
    Wire.begin();
    Serial.begin(9600);

    acsensor.initialize();
    pinMode(13, OUTPUT);
    Serial.println("AC Initialize done!");
}

void loop() {
    int16_t accelx = getAccelerationX(), accely = getAccelerationY(), accelz = getAccelerationZ();
    double accelres = sqrt(accelx*accelx + accely*accely + accelz*accelz);
    Serial.print(" ACX:"); Serial.print(accelx);
    Serial.print(" ACY:"); Serial.print(accely);
    Serial.print(" ACZ:"); Serial.print(accelz);
    Serial.print(" RES:"); Serial.println(accelres);
}
