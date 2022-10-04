#include <M5Stack.h>
#include "PID_v1.h"

///////  OutPout PWM  //////////
int out1Pin[] = {16, 17};
int out2Pin[] = {2, 5};
const int freqPWM = 10000;
// const int MaxPWM = 4095;
const int NbitPWM = 12;

enum OutputMode
{
  BIPOLAR,
  UNIPOLAR,
  UNIPOLAR_CENTER
};
OutputMode outputMode1 = OutputMode::UNIPOLAR, outputMode2 = OutputMode::UNIPOLAR;

bool output1Enable = true;
bool output2Enable = true;

/// PID variables //////
double temp1, temp2, tset1, tset2;
double output1, output2;

const double T_factor = 125.0 / 4096.0;

// int pid_enable = 1;

PID pid1(&temp1, &output1, &tset1, 0, 0, 0, DIRECT);
PID pid2(&temp2, &output2, &tset2, 0, 0, 0, DIRECT);

///////////////// Function declaration ///////////////
void loopComunication(void *param);
bool load_value(const char *name, const char *key, double *target);
void save_value(const char *name, const char *key, double data);
void save_tuning(PID pid1, PID pid2);
void load_tuning(PID *pid1, PID *pid2);
///////////////// \Function declaration ///////////////

void loopGUI(void *param)
{
  while (true)
  {
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(PINK, BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("  T1 = %7.2fC", temp1);

    M5.Lcd.setCursor(0, 30);
    M5.Lcd.printf("Set1 = %7.2fC", tset1);

    M5.Lcd.setCursor(0, 60);
    M5.Lcd.printf("Out1 = %7.2f", output1);

    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.printf("  T2 = %7.2fC", temp2);

    M5.Lcd.setCursor(0, 130);
    M5.Lcd.printf("Set2 = %7.2fC", tset2);

    M5.Lcd.setCursor(0, 160);
    M5.Lcd.printf("Out2 = %7.2f", output2);

    delay(100);
  }
}

void loopServo(void *param)
{
  const double alpha = 0.005; // smooth temp measurment

  temp1 = analogRead(35) * T_factor;

  while (true)
  {
    int x = analogRead(35) * T_factor;
    temp1 = alpha * x + (1 - alpha) * temp1;

    x = analogRead(36) * T_factor;
    temp2 = alpha * x + (1 - alpha) * temp2;

    pid1.Compute();
    pid2.Compute();

    if (output1Enable)
    {
      if (outputMode1 == OutputMode::UNIPOLAR)
      {
        if (output1 > 0)
          ledcWrite(0, (output1 / 100.0) * 2047);
        else
          ledcWrite(0, 0);
        ledcWrite(1, 0);
      }
      else if (outputMode1 == OutputMode::BIPOLAR)
      {
        if (output1 >= 0)
        {
          ledcWrite(0, (output1 / 100.0) * 2047);
          ledcWrite(1, 0);
        }
        else
        {
          ledcWrite(0, 0);
          ledcWrite(1, (-output1 / 100.0) * 2047);
        }
      }
    }
    else
    {
      ledcWrite(0, 0);
      ledcWrite(1, 0);
    }

    // delay(1);
  }
}

void setup()
{
  M5.begin();
  Serial.begin(115200);
  analogSetAttenuation(ADC_0db);
  pinMode(out1Pin[0], OUTPUT);
  pinMode(out1Pin[1], OUTPUT);
  pinMode(out2Pin[0], OUTPUT);
  pinMode(out2Pin[1], OUTPUT);

  ledcSetup(0, freqPWM, NbitPWM);
  ledcSetup(1, freqPWM, NbitPWM);
  ledcSetup(2, freqPWM, NbitPWM);
  ledcSetup(3, freqPWM, NbitPWM);

  ledcAttachPin(out1Pin[0], 0);
  ledcAttachPin(out1Pin[1], 1);
  ledcAttachPin(out2Pin[0], 2);
  ledcAttachPin(out2Pin[1], 3);

  pid1.SetOutputLimits(-100, 100);

  xTaskCreatePinnedToCore(loopServo, "", 4000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(loopGUI, "", 4000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(loopComunication, "", 4000, NULL, 1, NULL, 0);

  // Gd1 = load_value("PID1", "GP");
  load_tuning(&pid1, &pid2);
  load_value("PID1", "TSET", &tset1);
  load_value("PID2", "TSET", &tset2);

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
