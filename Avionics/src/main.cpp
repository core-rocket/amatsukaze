#include <Arduino.h>
#include <utility/filter.hpp>
#include <SoftwareSerial.h>

#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include "SparkFunBME280.h"
#include <Servo.h>

// input:	sec
// output:	millisec
constexpr size_t seconds(const float s_f){
	return static_cast<size_t>(s_f * 1000.0f);
}

enum class Mode : uint8_t{
    standby,
    flight,
    rise,
    parachute,
};


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

    /*open_by_timer*/
    constexpr size_t FIRING_TIME = seconds(5.0f);	//TODO: remove
	constexpr size_t OPEN_TIMEOUT = seconds(10.0f);

     /* サーボ用ピン */
    constexpr int UPPER_SERVO_PIN = 8;
    constexpr int LOWER_SERVO_PIN = 9;

    // パラシュート非開放位置
    constexpr int SERVO_ANGLE_CLOSE = 90;
    // パラシュート開放位置
    constexpr int UPPER_SERVO_ANGLE_OPEN = 180;
    constexpr int LOWER_SERVO_ANGLE_OPEN = 0;

    //ES920LRに接続されているピン
    constexpr int ES920LR_RX    = 6;
    constexpr int ES920LR_TX    = 7;
    constexpr int ES920LR_Reset = 3;
    constexpr uint32_t ES920LRBraud = 9600; //115200bpsだとSoftwareSerialできない
}

/* グローバル**変数** */
namespace global{
    Mode mode;

    /* MPU6050 */
    int32_t accel_res_now = 0;
    utility::moving_average<int32_t, 5> accel_res_average_buffer;

    /* BME280 */
    float altitude_average_old    = 100000000.0; //移動平均比較用の高度[m]
    float altitude_now = 0.0;
    utility::moving_average<float, 5> altitude_average_buffer;

    /* タイマー */
    size_t got_sensor_value_time_old = 0;  //センサが1つ前のloopで値を取得した時刻[ms]
    unsigned long become_rise_time; //離床判定された時刻[ms]

    /* サーボ */
    Servo upper_servo;
    Servo lower_servo;

    /* ES920LR */
    SoftwareSerial telemeter(constant::ES920LR_RX, constant::ES920LR_TX); //回路上でTX->RX,RX->TXに接続していないのでソフト的にする
}

namespace counter{
    size_t launch_by_accel_success  = 0;
    size_t open_by_bme280_success   = 0;
    size_t logging_interval_counter = 0;
}

namespace sensor{
    /* センサは1種類つき1つずつしか無いので安直な名前で */
    MPU6050 mpu6050;
    BME280 bme280;
}

/* プロトタイプ宣言書く場所 */
void get_all_sensor_value();
void telemeter_reset();
void open_parachute();
void close_parachute();
bool launch_by_accel();
bool can_get_sensor_value(size_t millis_now);
bool can_open();
bool open_by_BME280();
bool open_by_timer();
bool send_telemeter_data(String payload);
bool analyze_command(String received_cmd);
String receive_telemeter_command();
String shape_send_now_data();

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
    //センサ初期化:終了

    //サーボ初期化:開始
    global::upper_servo.attach(constant::UPPER_SERVO_PIN);
    global::lower_servo.attach(constant::LOWER_SERVO_PIN);

    open_parachute(); //電源投入時はサーボをパラシュート開放位置に設定する
    //サーボ初期化:終了

    //テレメータ初期化:開始
    Serial.print("[Init]ES920LR...");
    global::telemeter.begin(constant::ES920LRBraud);
    pinMode(constant::ES920LR_Reset, OUTPUT);
    digitalWrite(constant::ES920LR_Reset, HIGH);
    telemeter_reset();
    delay(3000);                    //ES920LR完全起動まで待つ

    while(!global::telemeter){}; 
    Serial.println("done");
    //テレメータ初期化:終了

}

void loop() {
    if(can_get_sensor_value(millis())){
        get_all_sensor_value();
    }else{
        return;
    }

    Serial.print("[ANALYZE]_analyzing_command...");
    if(analyze_command(receive_telemeter_command())){
        Serial.print("COMMAND");
    }
    Serial.println("");

    switch (global::mode)
    {
        case Mode::standby:
        {
            //standbyでは何もしない
            break;
        }

        case Mode::flight:
        {
            if(launch_by_accel()){
                Serial.println("LAUNCHbyACCEL_[SUCCESS]");
                global::become_rise_time = millis();
                global::mode = Mode::rise;
            }
            break;
        }

        case Mode::rise:
        {
            if(can_open() && (open_by_timer() || open_by_BME280())){
                Serial.println("OPEN_[SUCCESS]");
                global::mode = Mode::parachute;
            }
            break;
        }

        case Mode::parachute:
        {
            Serial.println("[PARACHUTE]");
            open_parachute();
            Serial.println("[PARACUTE]_SERVO_WROTE");
            break;
        }
        
        default:{
            break;
        }
    }
}

size_t flight_time() {
	return (millis() - global::become_rise_time);
}

bool can_open(){
    if(flight_time() > constant::FIRING_TIME) return true;
    return false;
}

bool open_by_timer(){
    if(flight_time() > constant::OPEN_TIMEOUT) return true;
    return false;
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
    Serial.print("[ACC]"); Serial.println(accel_average_now);
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

void open_parachute(){
    global::upper_servo.write(constant::UPPER_SERVO_ANGLE_OPEN);
    global::lower_servo.write(constant::LOWER_SERVO_ANGLE_OPEN);
}

void close_parachute(){
    global::upper_servo.write(constant::SERVO_ANGLE_CLOSE);
    global::lower_servo.write(constant::SERVO_ANGLE_CLOSE);
}

void telemeter_reset(){
    digitalWrite(constant::ES920LR_Reset, LOW);
    delay(100);
    digitalWrite(constant::ES920LR_Reset, HIGH);
}

String receive_telemeter_command(){
    if (global::telemeter.available() > 0){
        String command_with_rssi = global::telemeter.readString();
        String command_only = command_with_rssi.substring(4); //minimum構成ではRSSI値は利用しない
        return command_only;
    }
    return "";
}

bool send_telemeter_data(String payload){
    global::telemeter.println(payload);
    delay(1000); //ack待ち
    String receivemsg = global::telemeter.readString();
    String ack = receivemsg.substring(0,2); //ackの応答のみを切り出す
    if(ack.equals("OK")) return true;
    if(ack.equals("NG")) return false;
    return false;
}

bool analyze_command(String received_cmd){
    //キーボードのキーを同時押しして間違い防止したいので大文字アルファベット

    //パラシュート強制開放コマンド
    //離床判定されない時の緊急開放用としても作動する(=燃焼中でも作動できる)ため、誤操作防止として5文字としている
    //テストでパラシュートを開放させたい時/離床後OPEN_TIMEOUT[s]経っても開放されない時
    if(received_cmd.indexOf("OPENP") != -1){
        open_parachute();
        Serial.println("[ANALYZE]_OPEN_parachute");
        return send_telemeter_data(shape_send_now_data());
    }
    //パラシュート強制ロックコマンド
    //テストでパラシュートをロックしたい時
    else if(received_cmd.indexOf("LO") != -1){
        close_parachute();
        Serial.println("[ANALYZE]_CLOSE_parachute");
        return send_telemeter_data(shape_send_now_data());
    }
    //フライトモード移行コマンド
    //フライトモードに移行する時
    else if(received_cmd.indexOf("FL") != -1){
        global::mode = Mode::flight;
        Serial.println("[ANALYZE]_FLIGHT_mode");
        return send_telemeter_data(shape_send_now_data());
    }
    //スタンバイモード移行コマンド
    //スタンバイモードに移行させたい時/NOGOが出てロケットを射点から撤去する時
    else if(received_cmd.indexOf("ST") != -1){
        global::mode = Mode::standby;
        Serial.println("[ANALYZE]_STANDBY_mode");
        return send_telemeter_data(shape_send_now_data());
    }
    //現在情報取得コマンド
    //本当に所定のモードになっているか確認したい時/制御電装との応答確認がしたい時
    else if(received_cmd.indexOf("NW") != -1){
        Serial.println("[ANALYZE]_NOW_info");
        return send_telemeter_data(shape_send_now_data());
    }
    return false;

}

/* センサの値を取る */
void get_all_sensor_value(){
    //MPU6050:(x軸/y軸/z軸加速度),合成加速度( x 10^-4 [G] )
    const auto accel_x = sensor::mpu6050.getAccelerationX(), accel_y = sensor::mpu6050.getAccelerationY(), accel_z = sensor::mpu6050.getAccelerationZ();
    global::accel_res_now = pow(accel_x,2) + pow(accel_y,2) + pow(accel_z,2);
    global::accel_res_average_buffer.add_data(global::accel_res_now);

    //BME280:気圧[Pa],湿度[%],高度[m]
    float pressure_now    = sensor::bme280.readFloatPressure();
    //float humidity_now  = sensor::bme280.readFloatHumidity();
    float temperature_now = sensor::bme280.readTempC();
    global::altitude_now    = ((pow(constant::SEA_LEVEL_PRESSURE / pressure_now, 1/5.257) - 1) * (temperature_now + 273.15)) / 0.0065;
    global::altitude_average_buffer.add_data(global::altitude_now);
}

String shape_send_now_data(){
    //intとかをbinaryにしてUARTで送信する方法が(上手く行くかを含めて)よくわからんので
    //とりあえずString()で変換する
    String payload = "";

    payload += static_cast<uint8_t>(global::mode);
    payload += ":";

    String accel = String(global::accel_res_now);
    payload += accel.substring(0,6);
    payload += ":";

    String altitude = String(global::altitude_now);
    payload += altitude.substring(0,6);

    return payload;
}
