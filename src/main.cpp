#include <Arduino.h>
#include <M5Stack.h>
#include "PID_v1.h"

float alpha = 0.005;
float t;

PID pid();

void loopMeasure(void *param)
{
  t = analogRead(35);

  while (true)
  {
    int x = analogRead(35);
    t = alpha * x + (1 - alpha) * t;
    //delay(1);
  }
}

void setup()
{
  M5.begin();
  analogSetAttenuation(ADC_0db);
  xTaskCreatePinnedToCore(loopMeasure,"",4000,NULL,1,NULL,1);
}

void loop()
{
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(3);
  M5.Lcd.printf("%7.2fC", t/4096.0*108.8);
  delay(100);
}