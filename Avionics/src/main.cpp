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
    unsigned long become_rise_time;
}

/* プロトタイプ宣言書く場所 */
bool can_open();
bool open_by_timer();

void setup() {
    Wire.begin();
    Serial.begin(9600);

    global::mode = Mode::standby;
}

void loop() {
    switch (global::mode)
    {
        case Mode::standby:
            break;

        case Mode::flight:
            if(true){ //TODO: [Add] 離床検知条件
                global::mode       = Mode::rise;
                global::become_rise_time = millis();
            }
            break;

        case Mode::rise:
            if(can_open() == true){
                if(open_by_timer() == true){
                    Serial.println("OPENbyTIMER_[SUCCESS]");
                    global::mode = Mode::parachute;
                }
            }
            break;

        case Mode::parachute:
            break;
        
        default:
            break;
    }
}

bool can_open(){
    auto time = millis() - global::become_rise_time;
    if(time < 5000) return true;
    return false;
}

bool open_by_timer(){
    auto time = millis() - global::become_rise_time;
    if(time > 10000) return true;
    return false;
}