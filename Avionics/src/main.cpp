#include <Arduino.h>
#include <Wire.h>
#include "SparkFunBME280.h"

BME280 altsensor;

void setup() {
  Wire.begin();
  pinMode(13,OUTPUT);
  Serial.begin(9600);

  if(altsensor.beginI2C() == false){
    Serial.println("[ERROR]BME280 init failed...");
    while(1);
  }
}

void loop() {
  digitalWrite(13,HIGH);

  float pressure, humidity, altitude;
  pressure = altsensor.readFloatPressure();
  humidity = altsensor.readFloatHumidity();
  altitude = altsensor.readFloatAltitudeMeters();
  Serial.print(" P:"); Serial.print(pressure,3);
  Serial.print(" H:"); Serial.print(humidity,3);
  Serial.print(" A:"); Serial.print(altitude,3);
  Serial.print("\n");

  delay(100);
  digitalWrite(13,LOW);
  delay(500);
}