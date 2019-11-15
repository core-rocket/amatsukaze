#include <Arduino.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include <utility/filter.hpp>

/* プロトタイプ宣言書く場所 */
bool open_by_BME280();

enum class Mode{
    standby,
    flight,
    rise,
    parachute,
};

bool open_by_BME280();

namespace global{
    Mode mode;
    BME280 altsensor;

    //バッファ類
    utility::moving_average<float, 5> altitude_average_filter;
}

void setup() {
  Wire.begin();
  Serial.begin(9600);

  global::mode = Mode::standby;

  if(global::altsensor.beginI2C() == false){
    Serial.println("[ERROR]BME280 init failed...");
    while(1);
  }
  pinMode(13, OUTPUT);
}

void loop() {
    digitalWrite(13,HIGH);

    float pressure = global::altsensor.readFloatPressure();
    float humidity = global::altsensor.readFloatHumidity();
    float altitude = global::altsensor.readFloatAltitudeMeters();
    Serial.print(" P:"); Serial.print(pressure,3);
    Serial.print(" H:"); Serial.print(humidity,3);
    Serial.print(" A:"); Serial.print(altitude,3);
    Serial.print("\n");

    switch (global::mode)
    {
        case Mode::standby:
        if(true){ //TODO コマンド操作
            global::mode = Mode::flight;
        }
            break;

        case Mode::flight:
        if(true){ //TODO 離床検知条件
            global::mode = Mode::rise;
        }
            break;

        case Mode::rise:
            if(open_by_BME280()) Serial.println("OPENbyBME280_[SUCCESS]");
            break;

        case Mode::parachute:
            break;
        
        default:
            break;
    }
}

bool open_by_BME280(){
    int limit_counter = 0;
    float altitude_old = 10000000000.0;
    for(int i = 0; i < 5; i++){
        const auto al = global::altsensor.readFloatAltitudeMeters();
        global::altitude_average_filter.add_data(al);

        const auto altitude_new = global::altitude_average_filter.filtered();
        if(global::altitude_average_filter.filtered() <  altitude_old){
            limit_counter++;
        }else{
            limit_counter = 0;
        }
        altitude_old = altitude_new;
    }
    if(limit_counter >= 5) return true;
    return false;
}