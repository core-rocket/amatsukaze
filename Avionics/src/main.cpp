#include <Arduino.h>
#include <Wire.h>

enum class Mode{
    standby,
    flight,
    rise,
    parachute,
};

namespace global{
    Mode mode;
}

/* プロトタイプ宣言書く場所 */

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
}

void setup() {
<<<<<<< HEAD
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

=======
    Wire.begin();
    Serial.begin(9600);

    global::mode = Mode::standby;
}

void loop() {
>>>>>>> develop
    switch (global::mode)
    {
        case Mode::standby:
            break;

        case Mode::flight:
            break;

        case Mode::rise:
<<<<<<< HEAD
            if(open_by_BME280()) Serial.println("OPENbyBME280_[SUCCESS]");
=======
>>>>>>> develop
            break;

        case Mode::parachute:
            break;
        
        default:
            break;
    }
<<<<<<< HEAD
}

bool open_by_BME280(){
    int limit_counter = 0;
    int average_rate = 5;
    float average_buff[5] = {0.0};
    float average;
    float alt_old = 10000000000.0;
    for(int limit_index = 0; limit_index < 5; limit_index++){
        for(int i = 0; i < average_rate - 1; i++){
            average_buff[i] = average_buff[i+1]; //左にずらす
        }
        average_buff[average_rate - 1] = global::altsensor.readFloatAltitudeMeters(); //バッファの右端だけ新しい値を入れる

        for(int i = 0; i < average_rate; i++){
            float sum = average_buff[i];
            average = sum / 5.0;
        }

        if(average <  alt_old){ //TODO: 本当に3G?
            limit_counter++;
        }else{
            limit_counter = 0;
        }
        alt_old = average;
    }
    if(limit_counter >= 5) return true;
    return false;
=======
>>>>>>> develop
}