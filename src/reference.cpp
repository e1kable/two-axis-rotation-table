#include <Arduino.h>

#ifdef ARDUINO_AVR_UNO
#include <ArduinoSTL.h>
#endif

#include <vector>
#include <string.h>
#include "common.h"

using namespace std;

void referenceAxis(Axis *ax)
{
    // serach for max positive change
    float lastMeasurement = readHall(ax);

    size_t LEN_DBDPHI_HIST = 3;

    size_t dbdphi_hist_ptr = 0;
    vector<float> dbdphi_hist(LEN_DBDPHI_HIST);

    position_t maxPos = 0;
    float maxPosValue = 0;

    bool isInArea = false;

    // limit the angular speed with MAX_DPHI_DT
    float phiPerstep = 360 / (float)ax->TotalSteps;
    float tminperstep = phiPerstep / REFERENCE_ANGULAR_VELOCITY * 1000; // in ms
    uint64_t lastStep = millis();

    for (steps_t x = 0; x < ax->TotalSteps; x++)
    {

        float waitTime = (float)tminperstep - ((float)millis() - (float)lastStep);
        if (waitTime > 0)
        {
            delay((uint64_t)waitTime);
        }
        lastStep = millis();

        step(ax);

        float val_mean = 0.0;
        if (isInArea)
        {
            // delay(100); // delay some time, to let system settle down
            val_mean = readHall(ax, N_MEAN_DEFAULT_HALL_FINE_READOUT);
        }
        else
        {
            val_mean = readHall(ax);
        }

        float dbdphi = (val_mean - lastMeasurement) / phiPerstep;
        dbdphi_hist[dbdphi_hist_ptr++] = dbdphi;
        dbdphi_hist_ptr = dbdphi_hist_ptr % LEN_DBDPHI_HIST;
        lastMeasurement = val_mean;

        char buf[200];
        sprintf(buf, "Pos: %i B: %f dB/dphi: %f ", (int)ax->Position, (double)val_mean, (double)dbdphi);
        Serial.println(buf);

        // check if last entries are above threshold

        bool allTrue = all_of(dbdphi_hist.begin(), dbdphi_hist.end(), [](float f)
                              { return f > DB_DPHI_THRES; });

        if (allTrue) // if is in region
        {

            if (!isInArea)
            {
                Serial.println("in ->");
            }
            isInArea = true;

            if (dbdphi > maxPosValue)
            {
                maxPosValue = dbdphi;
                maxPos = ax->Position;
            }
        }

        if (!allTrue && isInArea)
        {
            Serial.println("<- out");
            break;
        }
    }

    // offset to steps
    position_t offset = (position_t)((ax->ReferenceOffset / 360) * (float)ax->TotalSteps);
    position_t correctedRef = maxPos + offset;

    ax->IsReferenced = true;
    ax->ReferencePosition = correctedRef;

    moveTo(ax, ax->ReferencePosition);
}
