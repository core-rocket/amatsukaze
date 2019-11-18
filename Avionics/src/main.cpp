#include <Arduino.h>
#include <utility/filter.hpp>
//#include <FlexiTimer2.h>
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
    int32_t accel_res_now = 0; //現在の合成加速度
    utility::moving_average<int32_t, 5> accel_res_average_buffer;

    /* BME280 */
    float pressure_now    = 0.0; //現在の気圧[Pa]
    float humidity_now    = 0.0; //現在の湿度[%]
    float altitude_now    = 0.0; //現在の高度[m]

    /* GNSS */
    double latitude_now   = 0.0; //現在の緯度
    double longitude_now  = 0.0; //現在の経度
    uint8_t ephemeris_hour_now   = 0; //現在の時間[UTC]
    uint8_t ephemeris_minute_now = 0; //現在の分[UTC]
    uint8_t ephemeris_second_now = 0; //現在の秒[UTC]

    /* VL53L0X */
    uint32_t distance_to_expander_now = 0; //ランチャ支柱までの距離[mm] 
}

namespace counter{
    size_t launch_by_accel_success = 0;
}

namespace constant{
    constexpr int RXPIN = 3, TXPIN = 2;
    constexpr uint32_t GNSSBAUD = 4800;

    /*閾値*/
    constexpr int LAUNCH_BY_ACCEL_THRESHOLD = 30000;
}

namespace sensor{
    /* センサは1種類つき1つずつしか無いので安直な名前で */
    MPU6050 mpu6050;
    BME280 bme280;
    TinyGPSPlus gnss;
    VL53L0X vl53l0x;
    SoftwareSerial sserial_GNSS(constant::TXPIN, constant::RXPIN); //GNSS用
}

/* プロトタイプ宣言書く場所 */
void get_all_sensor_value();
bool launch_by_accel(int32_t accel_res);


void setup() {
    Wire.begin();
    Serial.begin(9600);

    global::mode = Mode::standby;

    //センサ初期化:開始
    sensor::mpu6050.initialize();
    Serial.println("[Init]mpu6050_[SUCCESS]");

    if(sensor::bme280.beginI2C()){
        Serial.println("[Init]bme280_[SUCCESS]");
    }else{
        Serial.println("[Init]bme280_[FAILED]");
        while(1);
    }

    sensor::sserial_GNSS.begin(constant::GNSSBAUD);

    sensor::vl53l0x.setTimeout(500);
    if(sensor::vl53l0x.init()){
        Serial.println("[Init]vl53l0x_[SUCCESS]");
    }else{
        Serial.println("[Init]vl53l0x_[FAILED]");
        //while(1);
    }
    //センサ初期化:終了

    //FlexiTimer2::set(10, get_all_sensor_value);
    //FlexiTimer2::start();
}

void loop() {
    get_all_sensor_value();
    switch (global::mode)
    {
        case Mode::standby:
        {
            global::mode = Mode::flight;
        }

        case Mode::flight:
        {
            Serial.print("[ACC]"); Serial.println(global::accel_res_now); Serial.flush();
            Serial.print("[FLIGHT]counter:"); Serial.println(counter::launch_by_accel_success); Serial.flush();
            if(launch_by_accel(global::accel_res_now) /*||  vl53l0x条件 */ ){
                global::mode = Mode::rise;
            }
            break;
        }

        case Mode::rise:
        {
            Serial.println("[RISE]"); Serial.flush();

        }
            break;

        case Mode::parachute:
        {
            
        }
            break;
        
        default:
            break;
    }
    delay(15);
}

bool launch_by_accel(int32_t accel_res){
    global::accel_res_average_buffer.add_data(accel_res);

    const auto accel_average_now = global::accel_res_average_buffer.filtered();
    Serial.print("[AVG]:"); Serial.println(accel_average_now); Serial.flush(); //TODO: remove
    if(accel_average_now > constant::LAUNCH_BY_ACCEL_THRESHOLD){
        counter::launch_by_accel_success++;
    }else{
        counter::launch_by_accel_success = 0;
    }
    if(counter::launch_by_accel_success >= 5) return true;
    return false;
}


/* センサの値を取る and ログを取る のは一箇所にしたい */
void get_all_sensor_value(){
    //MPU6050:(x軸/y軸/z軸加速度),合成加速度
    const auto accel_x = sensor::mpu6050.getAccelerationX(), accel_y = sensor::mpu6050.getAccelerationY(), accel_z = sensor::mpu6050.getAccelerationZ();
    global::accel_res_now = sqrt(pow(accel_x,2) + pow(accel_y,2) + pow(accel_z,2));

    //BME280:気圧,湿度,高度
    global::pressure_now = sensor::bme280.readFloatPressure();
    global::humidity_now = sensor::bme280.readFloatHumidity();
    global::altitude_now = sensor::bme280.readFloatAltitudeMeters();

    //GNSS:緯度,経度,時,分,秒
    if(sensor::sserial_GNSS.available() > 0 && sensor::gnss.encode(sensor::sserial_GNSS.read())){
        if(sensor::gnss.location.isValid() && sensor::gnss.location.isUpdated()){
            global::latitude_now  = sensor::gnss.location.lat();
            global::longitude_now = sensor::gnss.location.lng();
        }
        if(sensor::gnss.time.isValid() && sensor::gnss.time.isUpdated()){
            global::ephemeris_hour_now   = sensor::gnss.time.hour();
            global::ephemeris_minute_now = sensor::gnss.time.minute();
            global::ephemeris_second_now = sensor::gnss.time.second();
        }
    }
    //VL53L0X:支柱までの距離
    global::distance_to_expander_now = sensor::vl53l0x.readRangeContinuousMillimeters();


    //TODO: logを取る
    //TODO: log()
}
