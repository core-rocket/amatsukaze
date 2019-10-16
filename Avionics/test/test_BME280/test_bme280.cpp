/* FOR EMBEDDED */

#include <Arduino.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include <unity.h>

BME280 sensor1;

void test_function_begin_i2c(void){
    TEST_ASSERT_TRUE(sensor1.beginI2C());
}

void test_function_read_pressure(void){
    TEST_ASSERT_FLOAT_WITHIN(100.0, 1000.0, sensor1.readFloatPressure());
}

void test_function_read_humidity(void){
    TEST_ASSERT_FLOAT_WITHIN(50.0, 50.0, sensor1.readFloatHumidity());
}

void test_function_read_temperature(void){
    TEST_ASSERT_FLOAT_WITHIN(50.0, 50.0, sensor1.readTempC());
}

void test_function_read_altitude(void){
    TEST_ASSERT_FLOAT_WITHIN(500.0, 500.0, sensor1.readFloatAltitudeMeters());
}

void process(){
    UNITY_BEGIN();
    RUN_TEST(test_function_begin_i2c);
    RUN_TEST(test_function_read_pressure);
    RUN_TEST(test_function_read_humidity);
    RUN_TEST(test_function_read_temperature);
    RUN_TEST(test_function_read_altitude);
    UNITY_END();
}

void setup(){
    delay(2000);

    Wire.begin();
    pinMode(13,OUTPUT);

    process();
}

void loop(){
    digitalWrite(13,HIGH);
    delay(100);
    digitalWrite(13,LOW);
    delay(500);
}
