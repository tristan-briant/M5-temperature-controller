#include "PID_v1.h"
#include <M5Stack.h>

#define __TC_VERSION__ "V0.1"

void save_value(const char *name, const char *key, double data);
void save_tuning(PID pid1, PID pid2);
void load_tuning(PID *pid1, PID *pid2);

extern PID pid1, pid2;
extern double temp1, temp2, tset1, tset2;

void loopComunication(void *param)
{
    while (true)
    {
        if (Serial.available())
        {
            int error = 1;
            do
            {
                const int16_t BUFFER_SIZE = 100;
                String str(BUFFER_SIZE);
                str = Serial.readStringUntil('\n');
                str.toUpperCase();
                // Serial.printf("Received: %s\n", str);

                if (str == "TEMP1?")
                    Serial.printf("%.2f\n", temp1);
                else if (str == "TEMP2?")
                    Serial.printf("%.2f\n", temp2);
                else if (str == "*IDN?")
                    Serial.printf("Temperature Controller %s\n", __TC_VERSION__);
                else if (str.startsWith("PID"))
                {
                    PID *pidselect = NULL;

                    if (str.startsWith("PID1:"))
                        pidselect = &pid1;
                    else if (str.startsWith("PID2:"))
                        pidselect = &pid2;
                    else
                        break;
                    str = str.substring(5);
                    if (str == "GP?")
                        Serial.printf("%.5f\n", pidselect->GetKp());
                    else if (str == "GD?")
                        Serial.printf("%.5f\n", pidselect->GetKd());
                    else if (str == "GI?")
                        Serial.printf("%.5f\n", pidselect->GetKi());
                    else if (str == "TSET?")
                        Serial.printf("%.2f\n", pidselect->GetSet()); // pidselect == &pid1 ? tset1 : tset2);

                    else if (str.startsWith("GP "))
                    {
                        float x = str.substring(3).toFloat();
                        pidselect->SetTunings(x, pidselect->GetKi(), pidselect->GetKd());
                        save_tuning(pid1, pid2);
                    }
                    else if (str.startsWith("GI "))
                    {
                        float x = str.substring(3).toFloat();
                        pidselect->SetTunings(pidselect->GetKp(), x, pidselect->GetKd());
                        save_tuning(pid1, pid2);
                    }
                    else if (str.startsWith("GD "))
                    {
                        float x = str.substring(3).toFloat();
                        pidselect->SetTunings(pidselect->GetKp(), pidselect->GetKi(), x);
                        save_tuning(pid1, pid2);
                    }
                    else if (str.startsWith("TSET "))
                    {
                        float x = str.substring(5).toFloat();
                        if (pidselect == &pid1)
                        {
                            tset1 = x;
                            save_value("PID1", "TSET", tset1);
                        }
                        else
                        {
                            tset2 = x;
                            save_value("PID2", "TSET", tset2);
                        }
                    }
                }

                error = 0;
            } while (false);

            if (error)
                Serial.println("syntaxe error");
        }

        delay(10);
    }
}