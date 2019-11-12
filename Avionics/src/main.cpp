#include <Arduino.h>
#include <Wire.h>

// input:	sec
// output:	millisec
constexpr size_t seconds(const float s_f){
	return static_cast<size_t>(s_f * 1000.0f);
}

enum class Mode{
    standby,
    flight,
    rise,
    parachute,
};

namespace constant {
	constexpr size_t open_timeout_tmp = seconds(5.0f);	// TODO remove
	constexpr size_t open_timeout = seconds(10.0f);
}

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
            if(true){ //TODO: [Add] コマンド操作
                global::mode = Mode::flight;
            }
            break;

        case Mode::flight:
            if(true){ //TODO: [Add] 離床検知条件
                global::mode       = Mode::rise;
                global::become_rise_time = millis();
            }
            break;

        case Mode::rise:
            if(can_open() || open_by_timer()){
                Serial.println("OPENbyTIMER_[SUCCESS]");
                global::mode = Mode::parachute;
            }
            break;

        case Mode::parachute:
            break;
        
        default:
            break;
    }
}

size_t flight_time() {
	return (millis() - global::become_rise_time);
}

bool can_open(){
    if(flight_time() > constant::open_timeout_tmp) return true;
    return false;
}

bool open_by_timer(){
    if(flight_time() > constant::open_timeout) return true;
    return false;
}
