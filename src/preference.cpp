#include <Preferences.h>
#include "PID_v1.h"

Preferences preferencesdata;

double load_value(const char *name, const char *key)
{
    double value = 0;

    if (preferencesdata.begin(name, true))
    {
        if (preferencesdata.isKey(key))
            preferencesdata.getBytes(key, (uint8_t *)&value, sizeof(float));
        preferencesdata.end();
    }
    return value;
}

void save_value(const char *name, const char *key, float data)
{
    if (preferencesdata.begin(name, false))
    {
        preferencesdata.putBytes(key, (uint8_t *)&data, sizeof(float));
        preferencesdata.end();
    }
}

void save_tuning(PID pid1, PID pid2)
{
    if (preferencesdata.begin("TUNING", false))
    {
        double data[] = {pid1.GetKp(), pid1.GetKi(), pid1.GetKd(), pid1.GetDirection()};
        preferencesdata.putBytes("PID1", (uint8_t *)data, 4 * sizeof(double));

        Serial.println(data[0]);

        double data2[] = {pid2.GetKp(), pid2.GetKi(), pid2.GetKd(), pid2.GetDirection()};
        preferencesdata.putBytes("PID2", (uint8_t *)data2, 4 * sizeof(double));
        preferencesdata.end();
    }
}

void load_tuning(PID *pid1, PID *pid2)
{
    if (preferencesdata.begin("TUNING", true))
    {
        double data[4];
        preferencesdata.getBytes("PID1", (uint8_t *)data, 4 * sizeof(double));
        pid1->SetTunings(data[0], data[1], data[2], data[3]);
        preferencesdata.getBytes("PID2", (uint8_t *)data, 4 * sizeof(double));
        pid2->SetTunings(data[0], data[1], data[2], data[3]);
        preferencesdata.end();
    }
}
