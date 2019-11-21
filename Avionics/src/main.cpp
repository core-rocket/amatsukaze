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

/* プロトタイプ宣言書く場所 */
bool open_by_BME280();

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
    utility::moving_average<int32_t, 5> accel_res_average_buffer;

    /* BME280 */
    float altitude_average_old    = 100000000; //移動平均比較用の高度[m]
    utility::moving_average<float, 5> altitude_average_buffer;

    /* GNSS */
    double latitude_now   = 0.0;      //現在の緯度
    double longitude_now  = 0.0;      //現在の経度
    uint8_t ephemeris_hour_now   = 0; //現在の時間[UTC]
    uint8_t ephemeris_minute_now = 0; //現在の分[UTC]
    uint8_t ephemeris_second_now = 0; //現在の秒[UTC]

    /* VL53L0X */
    uint32_t distance_to_expander_now = 0; //ランチャ支柱までの距離[mm] 

    /* タイマー */
    size_t got_sensor_value_time_old = 0;  //センサが1つ前のloopで値を取得した時間[ms]
}

namespace counter{
    size_t launch_by_accel_success  = 0;
    size_t open_by_bme280_success   = 0;
    size_t logging_interval_counter = 0;
}

namespace constant{
    constexpr int RXPIN = 3, TXPIN = 2;
    constexpr uint32_t GNSSBAUD = 9600;

    //大島での海面気圧[Pa]
    constexpr float SEA_LEVEL_PRESSURE = 101325.0;

    //離床検知する加速度の閾値の2乗 (x 10^-8 [G])
    constexpr int32_t LAUNCH_BY_ACCEL_THRESHOLD = 900000000;
    //連続してLIMIT回閾値を超えたら離床判定する
    constexpr int LAUNCH_BY_ACCEL_LIMIT = 5; 

    //連続してLIMIT回高度が下がったら開放判定する
    constexpr int OPEN_BY_BME280_LIMIT = 5; 
    //過去の値との差がDIFF_RATE以上なら有効な値とみなす
    //DIFF_RATE x LIMITが最小検出高度差
    constexpr float OPEN_BY_BME280_DIFF_RATE = 0.3;
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
bool open_by_BME280();
bool launch_by_accel();
bool can_get_sensor_value(size_t millis_now);


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
    if(can_get_sensor_value(millis())){
        get_all_sensor_value();
    }else{
        return;
    }

    switch (global::mode)
    {
        case Mode::standby:
        {
            global::mode = Mode::flight;
            break;
        }

        case Mode::flight:
        {
            if(launch_by_accel() /*||  vl53l0x条件 */ ){
                Serial.println("LAUNCHbyACCEL_[SUCCESS]");
                global::mode = Mode::rise;
            }
            break;
        }

        case Mode::rise:
        {
            if(open_by_BME280()){
                Serial.println("OPENbyBME280_[SUCCESS]");
                global::mode = Mode::parachute;
            }
            break;
        }

        case Mode::parachute:
        {
            Serial.println("[PARACHUTE]"); Serial.flush();
            break;   
        }
        
        default:{
            break;
        }
    }
}

bool can_get_sensor_value(size_t millis_now){
    size_t elapsed_time = millis_now - global::got_sensor_value_time_old;
    if(elapsed_time >= 10){
        global::got_sensor_value_time_old = millis_now;
        return true;
    }
    return false;
}

bool launch_by_accel(){
    const auto accel_average_now = global::accel_res_average_buffer.filtered();
    if(accel_average_now > constant::LAUNCH_BY_ACCEL_THRESHOLD){
        counter::launch_by_accel_success++;
    }else{
        counter::launch_by_accel_success = 0;
    }
    if(counter::launch_by_accel_success >= constant::LAUNCH_BY_ACCEL_LIMIT) return true;
    return false;
}

bool open_by_BME280(){
    const auto altitude_average_now = global::altitude_average_buffer.filtered();
    const auto altitude_diff = global::altitude_average_old - altitude_average_now;
    if(altitude_average_now <  global::altitude_average_old
            && (altitude_diff >= constant::OPEN_BY_BME280_DIFF_RATE )){
        counter::open_by_bme280_success++;
    }else{
        counter::open_by_bme280_success = 0;
    }
    global::altitude_average_old = altitude_average_now;
    if(counter::open_by_bme280_success >= constant::OPEN_BY_BME280_LIMIT) return true;
    return false;
}


/* センサの値を取る and ログを取る のは一箇所にしたい */
void get_all_sensor_value(){
    //MPU6050:(x軸/y軸/z軸加速度),合成加速度( x 10^-4 [G] )
    const auto accel_x = sensor::mpu6050.getAccelerationX(), accel_y = sensor::mpu6050.getAccelerationY(), accel_z = sensor::mpu6050.getAccelerationZ();
    int32_t accel_res_now = pow(accel_x,2) + pow(accel_y,2) + pow(accel_z,2);
    global::accel_res_average_buffer.add_data(accel_res_now);

    //BME280:気圧[Pa],湿度[%],高度[m]
    // TODO : pressure_now, humidity, altitude_nowのlogを取る
    float pressure_now    = sensor::bme280.readFloatPressure();
    //float humidity_now  = sensor::bme280.readFloatHumidity();
    float temperature_now = sensor::bme280.readTempC();
    float altitude_now    = ((pow(constant::SEA_LEVEL_PRESSURE / pressure_now, 1/5.257) - 1) * (temperature_now + 273.15)) / 0.0065;
    global::altitude_average_buffer.add_data(altitude_now);

    //GNSS:緯度,経度,時,分,秒
    //sserial_GNSSのバッファが0になるまで処理をブロック
    while(sensor::sserial_GNSS.available() > 0){
        if(sensor::gnss.encode(sensor::sserial_GNSS.read())){
            if(sensor::gnss.location.isValid()){
                global::latitude_now  = sensor::gnss.location.lat();
                global::longitude_now = sensor::gnss.location.lng();
            }
            if(sensor::gnss.time.isValid()){
                global::ephemeris_hour_now   = sensor::gnss.time.hour();
                global::ephemeris_minute_now = sensor::gnss.time.minute();
                global::ephemeris_second_now = sensor::gnss.time.second();
            }
        }
    }
    //VL53L0X:支柱までの距離
    global::distance_to_expander_now = sensor::vl53l0x.readRangeContinuousMillimeters();


    //TODO: logを取る
    //TODO: log()
}
