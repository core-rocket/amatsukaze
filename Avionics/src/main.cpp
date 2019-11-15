#include <Arduino.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include "SparkFunBME280.h"
#include <utility/filter.hpp>

enum class Mode{
    standby,
    flight,
    rise,
    parachute,
};

namespace global{
    Mode mode;
    MPU6050 acsensor;
}

bool launch_by_accel();

void setup() {
    Wire.begin();
    Serial.begin(9600);

    global::mode = Mode::standby;

    global::acsensor.initialize();
    pinMode(13, OUTPUT);
    Serial.println("AC Initialize done!");
}

void loop() {
    //int16_t accelx = acsensor.getAccelerationX(), accely = acsensor.getAccelerationY(), accelz = acsensor.getAccelerationZ();
    //double accelres = sqrt(accelx*accelx + accely*accely + accelz*accelz);

    switch (global::mode)
    {
        case Mode::standby:
            break;

        case Mode::flight:
            if(launch_by_accel()) Serial.println("LaunchByACC_[SUCCESS]");
            break;

        case Mode::rise:
            break;

        case Mode::parachute:
            break;
        
        default:
            break;
    }
}

bool launch_by_accel(){
    int limit_counter = 0;
    int average_rate = 5;
    double average_buff[5] = {0.0};
    double average;
    for(int limit_index = 0; limit_index < 10; limit_index++){
        for(int i = 0; i < average_rate - 1; i++){
            average_buff[i] = average_buff[i+1]; //左にずらす
        }
        int16_t accelx = global::acsensor.getAccelerationX(), accely = global::acsensor.getAccelerationY(), accelz = global::acsensor.getAccelerationZ();
        double accelres = sqrt(accelx*accelx + accely*accely + accelz*accelz);
        average_buff[average_rate - 1] = accelres; //バッファの右端だけ新しい値を入れる

        for(int i = 0; i < average_rate; i++){
            double sum = average_buff[i];
            average = sum / 5.0;
        }

        if(average >  30000){ //TODO: 本当に3G?
            limit_counter++;
        }else{
            limit_counter = 0;
        }
    }
    if(limit_counter >= 10) return true;
    return false;
}


