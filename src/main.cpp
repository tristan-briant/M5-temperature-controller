#include <M5Stack.h>
#include "PID_v1.h"

///////  OutPout PWM  //////////
int out1Pin[] = {16, 17};
int out2Pin[] = {2, 5};
const int freqPWM = 10000;
// const int MaxPWM = 4095;
const int NbitPWM = 12;

bool output1Enable = true;
bool output2Enable = true;

/// PID variables //////
double temp1, temp2, tset1 = 20, tset2 = 20;
double output1, output2;

const double T_factor = 143.0 / 4095.0;

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
    M5.update();

    M5.Lcd.setTextSize(4);
    M5.Lcd.setTextColor(pid1.GetMode() ? GREEN : DARKGREY, BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("  T: %5.2fC", temp1);

    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.printf("Set: %5.2fC", tset1);
    M5.Lcd.setTextSize(2);
    int k = 100;
    M5.Lcd.setCursor(0, k);
    M5.Lcd.printf("%4.0f%% < %5.1f%% < %4.0f%%", pid1.GetOutMin(), output1, pid1.GetOutMax());
    k += 30;
    M5.Lcd.setCursor(0, k);
    M5.Lcd.printf("GP: %7.2g", pid1.GetKp());
    k += 30;
    M5.Lcd.setCursor(0, k);
    M5.Lcd.printf("GI: %7.2g", pid1.GetKi());
    k += 30;
    M5.Lcd.setCursor(0, k);
    M5.Lcd.printf("GD: %7.2g", pid1.GetKd());

    /*
        M5.Lcd.setTextColor(PINK, BLACK);
        M5.Lcd.setCursor(0, 100);
        M5.Lcd.printf("  T2 = %7.2fC", temp2);

        M5.Lcd.setCursor(0, 130);
        M5.Lcd.printf("Set2 = %7.2fC", tset2);

        M5.Lcd.setCursor(0, 160);
        M5.Lcd.printf("Out2 = %7.1f%", output2);
    */
    /// Handle Butons

    if (M5.BtnA.wasPressed())
      pid1.SetMode(!pid1.GetMode()); //

    if (M5.BtnB.wasPressed() || M5.BtnB.pressedFor(200))
      tset1 -= 0.1;
    if (M5.BtnC.wasPressed() || M5.BtnC.pressedFor(200))
      tset1 += 0.1;

    if (M5.BtnB.wasReleased() || M5.BtnC.wasReleased())
      save_value("PID1", "TSET", tset1);

    delay(50);
  }
}

void loopServo(void *param)
{
  const double alpha = 0.001; // smooth temp measurment

  temp1 = analogRead(35) * T_factor;

  while (true)
  {
    double x = analogRead(35) * T_factor;
    temp1 = alpha * x + (1 - alpha) * temp1;

    x = analogRead(36) * T_factor;
    temp2 = alpha * x + (1 - alpha) * temp2;

    pid1.Compute();
    pid2.Compute();

    if (output1Enable)
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
  double max = 100, min = -100;
  load_value("PID1", "OUTMAX", &max);
  load_value("PID1", "OUTMIN", &min);
  pid1.SetOutputLimits(min, max);
  max = 100, min = -100;
  load_value("PID2", "OUTMAX", &max);
  load_value("PID2", "OUTMIN", &min);
  pid2.SetOutputLimits(min, max);

  Serial.println(pid1.GetKp());
  Serial.println(pid1.GetKi());
  Serial.println(pid1.GetKd());
  Serial.println(pid1.GetDirection());
  Serial.println(pid1.GetMode());

  pid1.SetMode(MANUAL);
  pid2.SetMode(MANUAL);
}

void loop() // not used
{
}
