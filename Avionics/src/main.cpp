#include <Arduino.h>
#include <utility/filter.hpp>
#include <FlexiTimer2.h>
#include <SoftwareSerial.h>

#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include "SparkFunBME280.h"
#include <TinyGPS++.h>
#include <VL53L0X.h>

enum class Mode{
    standby,
    flight,
    rise,
    parachute,
};

/* グローバル**変数** */
namespace global{
    Mode mode;

    /* MPU6050 */
    int16_t accel_res_now = 0; //現在の合成加速度

    utility::moving_average<int16_t, 5> accel_res_average_buffer;
}

namespace sensor{
    /* センサは1種類つき1つずつしか無いので安直な名前で */
    MPU6050 mpu6050;
}

/* プロトタイプ宣言書く場所 */
void get_all_sensor_value();
bool launch_by_accel(int16_t accel_res);


void setup() {
    Wire.begin();
    Serial.begin(9600);

    global::mode = Mode::standby;

    //センサ初期化:開始
    sensor::mpu6050.initialize();
    Serial.println("[Init]mpu6050_[SUCCESS]");

    FlexiTimer2::set(10, get_all_sensor_value);
    FlexiTimer2::start();
}

void loop() {
    switch (global::mode)
    {
        case Mode::standby:
        {
            //TODO: remove
            //TEST CODE
            Serial.print("[ACC]"); Serial.println(global::accel_res_now);

            global::mode = Mode::flight;
        }

        case Mode::flight:
        {
            int launch_decide_counter = 0;
            while(1){
                if(launch_by_accel(global::accel_res_now)){
                    launch_decide_counter++;
                }else{
                    launch_decide_counter = 0;
                } 

                if(launch_decide_counter > 5 /* || vl53l0x 条件 */){
                    global::mode = Mode::rise;
                    break;
                }
            }
            break;
        }

        case Mode::rise:
        {

        }
            break;

        case Mode::parachute:
        {
            
        }
            break;
        
        default:
            break;
    }
}

bool launch_by_accel(int16_t accel_res){
    int limit_counter = 0;
    global::accel_res_average_buffer.add_data(accel_res);

    const auto accel_average_new = global::accel_res_average_buffer.filtered();
    if(accel_average_new > 3000){
        limit_counter++;
    }else{
        limit_counter = 0;
    }
    if(limit_counter >= 5) return true;
    return false;
}


/* センサの値を取る and ログを取る のは一箇所にしたい */
void get_all_sensor_value(){
    //MPU6050:(x軸/y軸/z軸加速度),合成加速度
    const auto accel_x = sensor::mpu6050.getAccelerationX(), accel_y = sensor::mpu6050.getAccelerationY(), accel_z = sensor::mpu6050.getAccelerationZ();
    global::accel_res_now = sqrt(accel_x*accel_x + accel_y*accel_y + accel_z*accel_z);

    //TODO: logを取る
    //TODO: log()
}
