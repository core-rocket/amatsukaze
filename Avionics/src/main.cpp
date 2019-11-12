#include <Arduino.h>
#include <Wire.h>
#include <utility/filter.hpp>

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
            break;

        case Mode::rise:
            break;

        case Mode::parachute:
            break;
        
        default:
            break;
    }
}