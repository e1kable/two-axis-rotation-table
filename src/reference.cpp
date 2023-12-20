#include <Arduino.h>

#ifdef ARDUINO_AVR_UNO
#include <ArduinoSTL.h>
#endif

#include <vector>
#include <string.h>
#include <numeric>
#include "common.h"

using namespace std;

float stepAndMeasure(Axis *ax, size_t Nmean, bool isReverse)
{

    float phiPerstep = 360.0 / (float)ax->TotalSteps;
    float tminperstep = phiPerstep / REFERENCE_ANGULAR_VELOCITY * 1000; // in ms

    float waitTime = (float)tminperstep - ((float)millis() - (float)ax->LastStepTime);
    if (waitTime > 0)
    {
        delay((uint64_t)waitTime);
    }

    step(ax, isReverse);

    return readHall(ax, Nmean);
}

float stepAndMeasure(Axis *ax, size_t Nmean)
{
    return stepAndMeasure(ax, Nmean, false);
}
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

    for (steps_t x = 0; x < ax->TotalSteps; x++)
    {
        float val_mean = 0.0;
        if (isInArea)
        {
            // delay(100); // delay some time, to let system settle down
            val_mean = stepAndMeasure(ax, N_MEAN_DEFAULT_HALL_FINE_READOUT);
        }
        else
        {
            val_mean = stepAndMeasure(ax, N_MEAN_DEFAULT_HALL_READOUT);
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

struct Peak
{
    position_t position;
    float hallValue;
    float dphidtValue;

    float isPeakFound;

    position_t areaStart;
    position_t areaEnd;
};

void findNextPeak(Peak *maxPeak, Axis *ax, steps_t maxSteps, bool isReverse)
{
    vector<float> measurementHist(REFERENCE_MOVING_AVG_FILT_SIZE);
    size_t histPointer = 0;

    float phiPerstep = 360.0 / (float)ax->TotalSteps;

    float lastMeasurement = readHall(ax, N_MEAN_DEFAULT_HALL_READOUT);
    bool isInArea = false;
    for (steps_t x = 0; x < maxSteps; x++)
    {
        float val_mean = 0.0;
        if (isInArea)
        {
            val_mean = stepAndMeasure(ax, N_MEAN_DEFAULT_HALL_FINE_READOUT, isReverse);
        }
        else
        {
            val_mean = stepAndMeasure(ax, N_MEAN_DEFAULT_HALL_READOUT, isReverse);
        }

        float dbdphi = (val_mean - lastMeasurement) / phiPerstep;
        measurementHist.at(histPointer++) = dbdphi;
        histPointer = histPointer % measurementHist.size();

        lastMeasurement = val_mean;

        float dbdphiMA = std::reduce(measurementHist.begin(), measurementHist.end()) / measurementHist.size();

        char buf[200];
        sprintf(buf, "Pos: %i B: %f dB/dphi: %f dB/dphi[MA]: %f", (int)ax->Position, (double)val_mean, (double)dbdphi, (double)dbdphiMA);
        Serial.println(buf);

        bool isOverThreas = dbdphiMA > DB_DPHI_THRES;
        if (isOverThreas) // if is in region, only use the MA value to its faster inside the zone
        {
            if (!isInArea)
            {
                maxPeak->areaStart = ax->Position + ((!isReverse) ? -4 * measurementHist.size() : +4 * measurementHist.size());
                Serial.println("in (1) ->");
            }
            isInArea = true;
        }

        if (abs(dbdphiMA) > abs(maxPeak->dphidtValue) && isInArea)
        {
            maxPeak->dphidtValue = dbdphiMA;
            maxPeak->position = ax->Position;
            maxPeak->hallValue = val_mean;
            maxPeak->isPeakFound = true;
        }

        bool AreAllOfBelowThres = all_of(measurementHist.begin(), measurementHist.end(), [](float m)
                                         { return abs(m) < DB_DPHI_THRES; });
        if (!isOverThreas && isInArea && AreAllOfBelowThres) // use a stricter measrue to ensure one stays long inside the zone
        {
            Serial.println("<- out (1)");
            isInArea = false;
            maxPeak->areaEnd = ax->Position;
            break;
        }
    }
}

void referenceAxisTwoPoint(Axis *ax)
{
    Serial.println("Start referencing");
    steps_t NwidthMagnet = (steps_t)(REFERENCE_MAGNET_SIZE_DEG / 360.0 * (float)ax->TotalSteps);

    // serach for max positive change
    Peak firstPeak = {0, 0, 0, false};
    findNextPeak(&firstPeak, ax, ax->TotalSteps, false);

    Serial.println("Found first peak, searching for second now...");

    // 2. Search for the second precipice in same direction
    Peak secondPeak = {0, 0, 0, false};
    findNextPeak(&secondPeak, ax, NwidthMagnet, false);

    if (!secondPeak.isPeakFound)
    {
        Serial.println("Trying the other direction...");
        /////////////////////////
        // move to first pos
        /////////////////////////
        moveTo(ax, firstPeak.areaStart);

        secondPeak = {0, 0, 0, false};
        findNextPeak(&secondPeak, ax, NwidthMagnet, true);

        if (!secondPeak.isPeakFound)
        {
            Serial.println("No second peak found. Exit.");
            return;
        }
    }

    Serial.println("Found second peak");
    float refPosfloat = firstPeak.position + (secondPeak.position - firstPeak.position) / 2.0;
    position_t refPos = (position_t)refPosfloat;
    char buf[200];
    sprintf(buf, "Pos0: %i Pos1: %i RefPosf: %f RefPosi: %i", (int)firstPeak.position, (int)secondPeak.position, (double)refPosfloat, refPos);
    Serial.println(buf);

    ax->IsReferenced = true;
    ax->ReferencePosition = refPos;

    moveTo(ax, ax->ReferencePosition);
}
