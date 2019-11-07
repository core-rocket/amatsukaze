#include <Arduino.h>
#include <Wire.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

enum class Mode{
    standby,
    flight,
    rise,
    parachute,
};

namespace global{
    Mode mode;
    static const int RXPin = 3, TXPin = 2;
    static const uint32_t GPSBaud = 4800;
    TinyGPSPlus gps;
    SoftwareSerial ss(RXPin, TXPin);

    double latitude = 0.0, longitude = 0.0;
    uint8_t ephemeris_hour = 0, ephemeris_minute = 0, ephemeris_second = 0;
}

/* プロトタイプ宣言書く場所 */

void setup() {
    Wire.begin();
    Serial.begin(9600);
    global::ss.begin(global::GPSBaud);

    global::mode = Mode::standby;
}

void loop() {
    if(global::gps.location.isValid()){
        global::latitude = global::gps.location.lat();
        global::longitude = global::gps.location.lng();
    }
    if(global::gps.time.isValid()){
        global::ephemeris_hour = global::gps.time.hour();
        global::ephemeris_minute = global::gps.time.minute();
        global::ephemeris_second = global::gps.time.second();
    }

    Serial.print(global::latitude); Serial.print("/"); Serial.print(global::longitude);
    Serial.print("\n");
    Serial.print(global::ephemeris_hour); Serial.print(":");
    Serial.print(global::ephemeris_minute); Serial.print(":");
    Serial.print(global::ephemeris_second); 
    Serial.print("\n");

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