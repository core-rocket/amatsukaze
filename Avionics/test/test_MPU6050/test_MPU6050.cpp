/* FOR EMBEDDED */

#include <Arduino.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <unity.h>

MPU6050 accelgyro;

void test_function_test_connection(void){
    TEST_ASSERT_TRUE(accelgyro.testConnection());
}

void test_function_get_ax(void){
    TEST_ASSERT_INT16_WITHIN(500, 0, accelgyro.getAccelerationX());
}

void test_function_get_yx(void){
    TEST_ASSERT_INT16_WITHIN(500, 0, accelgyro.getAccelerationY());
}

void test_function_get_zx(void){
    TEST_ASSERT_INT16_WITHIN(500, 0, accelgyro.getAccelerationZ());
}

void process(){
    UNITY_BEGIN();
    RUN_TEST(test_function_get_ax);
    RUN_TEST(test_function_get_yx);
    RUN_TEST(test_function_get_zx);
    UNITY_END();
}

void setup(){
    delay(2000);

    Wire.begin();
    Serial.begin(9600);
    accelgyro.initialize();
    pinMode(13, OUTPUT);

    process();
}

void loop(){
    digitalWrite(13,HIGH);

    Serial.print("ax:");
    Serial.print(accelgyro.getAccelerationX());
    Serial.print(" ay:");
    Serial.print(accelgyro.getAccelerationY());
    Serial.print(" az:");
    Serial.println(accelgyro.getAccelerationZ());

    delay(100);
    digitalWrite(13,LOW);
    delay(500);
}
