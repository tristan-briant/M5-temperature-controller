#define __TC_VERSION__ "V0.1"


#include <Arduino.h>
#include <M5Stack.h>
#include "PID_v1.h"

double load_value(const char *name, const char *key);
void save_value(const char *name, const char *key, float data);
void save_tuning(PID pid1, PID pid2);
void load_tuning(PID *pid1, PID *pid2);


double alpha = 0.005;
double temp1, temp2, tset1 = 19, tset2;
double Gp1, Gp2;
double Gd1, Gd2;
double Gi1, Gi2;
double output1, output2;

const double T_factor = 108.8 / 4096.0;

// int pid_enable = 1;

PID pid1(&temp1, &output1, &tset1, 0, 0, 0, DIRECT);
PID pid2(&temp2, &output2, &tset2, 0, 0, 0, DIRECT);

void loopComunication(void *param)
{
  while (true)
  {
    if (Serial.available())
    {
      const int16_t BUFFER_SIZE = 100;
      String str(BUFFER_SIZE);
      str = Serial.readStringUntil('\n');
      str.toUpperCase();
      Serial.printf("Received: %s\n", str);

      if (str == "TEMP1?")
        Serial.printf("%.2f\n", temp1);
      else if (str == "TEMP2?")
        Serial.printf("%.2f\n", temp2);
      else if (str == "*IDN?")
        Serial.printf("Temperature Controller %s\n", __TC_VERSION__);
      else if (str == "PID1:GP?")
        Serial.printf("%.5f\n", pid1.GetKp());
      else if (str == "PID1:GD?")
        Serial.printf("%.5f\n", pid1.GetKd());
      else if (str == "PID1:GI?")
        Serial.printf("%.5f\n", pid1.GetKi());
      else if (str == "PID2:GP?")
        Serial.printf("%.5f\n", pid2.GetKp());
      else if (str == "PID2:GD?")
        Serial.printf("%.5f\n", pid2.GetKd());
      else if (str == "PID2:GI?")
        Serial.printf("%.5f\n", pid2.GetKi());
      else if (str == "TSET1?")
        Serial.printf("%.2f\n", temp1);
      else if (str == "TSET2?")
        Serial.printf("%.2f\n", temp2);

      else if (str.startsWith("PID1:GP"))
      {
        float x = str.substring(7).toFloat();
        pid1.SetTunings(x, pid1.GetKi(), pid1.GetKp());
        save_tuning(pid1, pid2);
      }
    }
    delay(10);
  }
}

void loopGUI(void *param)
{
  while (true)
  {
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("  T = %7.2fC", temp1);

    M5.Lcd.setCursor(0, 30);
    M5.Lcd.printf("Set = %7.2fC", tset1);

    M5.Lcd.setCursor(0, 60);
    M5.Lcd.printf("Out = %7.2f", output1);

    delay(100);
  }
}

void loopMeasure(void *param)
{
  temp1 = analogRead(35) * T_factor;

  while (true)
  {
    int x = analogRead(35) * T_factor;
    temp1 = alpha * x + (1 - alpha) * temp1;

    pid1.Compute();
    // delay(1);
  }
}

void setup()
{
  M5.begin();
  Serial.begin(115200);
  analogSetAttenuation(ADC_0db);

  pid1.SetOutputLimits(-255, 255);

  xTaskCreatePinnedToCore(loopMeasure, "", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(loopGUI, "", 4000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(loopComunication, "", 4000, NULL, 1, NULL, 0);

  // Gd1 = load_value("PID1", "GP");
  load_tuning(&pid1, &pid2);

  Serial.println(pid1.GetKp());
  Serial.println(pid1.GetKi());
  Serial.println(pid1.GetKd());
  Serial.println(pid1.GetDirection());
  Serial.println(pid1.GetMode());

  pid1.SetMode(AUTOMATIC);
}

void loop() // not used
{
}
