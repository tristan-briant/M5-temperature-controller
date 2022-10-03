#include <Preferences.h>
#include "PID_v1.h"

Preferences preferencesdata;

bool load_value(const char *name, const char *key, double *target)
{
    double value = 0;
    bool error = false;

    if (preferencesdata.begin(name, true))
    {
        if (preferencesdata.isKey(key))
            preferencesdata.getBytes(key, (uint8_t *)target, sizeof(double));
        else
            error = true;

        preferencesdata.end();
    }
    else
        error = false;

    return error;
}

void save_value(const char *name, const char *key, double data)
{
    if (preferencesdata.begin(name, false))
    {
        preferencesdata.putBytes(key, (uint8_t *)&data, sizeof(double));
        preferencesdata.end();
    }
}

void save_tuning(PID pid1, PID pid2)
{
    if (preferencesdata.begin("TUNING", false))
    {
        double data[] = {pid1.GetKp(), pid1.GetKi(), pid1.GetKd(), (double)pid1.GetDirection()};
        preferencesdata.putBytes("PID1", (uint8_t *)data, 4 * sizeof(double));

        Serial.println(data[0]);

        double data2[] = {pid2.GetKp(), pid2.GetKi(), pid2.GetKd(), (double)pid2.GetDirection()};
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
        pid1->SetTunings(data[0], data[1], data[2], true);
        preferencesdata.getBytes("PID2", (uint8_t *)data, 4 * sizeof(double));
        pid2->SetTunings(data[0], data[1], data[2], true);
        preferencesdata.end();
    }
}
