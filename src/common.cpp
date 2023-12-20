#include <Arduino.h>

#ifdef ARDUINO_AVR_UNO
#include <ArduinoSTL.h>
#endif

#include "common.h"
#include "sloping.h"

struct Axis AX_AZ = {2, 5, A5,
                     REFERENCE_OFFSET_CORRECTION_AZ,
                     N_STEPS_AZ *N_MICRO_STEPS_AZ,
                     MAX_DPHI_DT_AZ,
                     false};
struct Axis AX_EL = {3, 6, A4,
                     REFERENCE_OFFSET_CORRECTION_EL,
                     N_STEPS_EL *N_MICRO_STEPS_EL,
                     MAX_DPHI_DT_EL,
                     false};

void initAxis(Axis *ax)
{
    // if axis is not initialized
    if (!ax->IsInit)
    {
        ax->IsReferenced = false;

        pinMode(ax->StepPin, OUTPUT);
        pinMode(ax->DirectionPin, OUTPUT);
        ax->IsInit = true;
    }
}

void step(Axis *axis, bool reverse)
{
    initAxis(axis);

    if (reverse)
    {
        digitalWrite(axis->DirectionPin, HIGH);
        axis->Position--; // decrement position
    }
    else
    {
        digitalWrite(axis->DirectionPin, LOW);
        axis->Position++; // increment position
    }

    // limit the angular speed with MAX_DPHI_DT
    float phiPerstep = 360 / (float)axis->TotalSteps;
    float tminperstep = phiPerstep / axis->MaxDPhiDt * 1000; // in ms
    float waitTime = (float)tminperstep - ((float)millis() - (float)axis->LastStepTime);

    if (waitTime > 0 && axis->LastStepTime != 0)
    {
        delay((uint64_t)waitTime);
    }

    digitalWrite(axis->StepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(axis->StepPin, LOW);
    delayMicroseconds(500);
    axis->LastStepTime = millis();
}

void step(Axis *axis)
{
    step(axis, false);
}

void steps(Axis *ax, steps_t nsteps, bool reverse)
{
    if (IS_ENABLE_SLOPING)
    {
        float phi = (360.0 / ax->TotalSteps) * nsteps; // convert to an angle
        steps_t NstepsSlope;                           // not really needed here
        for (size_t i = 0; i < nsteps; i++)
        {
            step(ax, reverse);

            uint32_t sleepTime = generateSlopeSteps(i, phi, MAX_DPHI_DT_AZ, ax->TotalSteps, NstepsSlope);

            if (sleepTime > 200)
            {
                Serial.print("Large sleep time detected ");
                Serial.print(sleepTime);
                Serial.print(" ms ");
                Serial.print(i);
                Serial.print(" ");
                Serial.print(phi);
                Serial.print(" ");
                Serial.print(ax->TotalSteps);
                Serial.print(" ");
                Serial.println(NstepsSlope);
            }
            // ensure that the sleep time does not exceed SLOPING_MAX_SLEEP_TIME
            sleepTime = (sleepTime > SLOPING_MAX_SLEEP_TIME) ? SLOPING_MAX_SLEEP_TIME : sleepTime;

            delay(sleepTime);
        }
    }
    else
    {
        for (uint32_t i = 0; i < nsteps; i++)
        {
            step(ax, reverse);
        }
    }
}
void steps(Axis *ax, steps_t nsteps)
{
    steps(ax, nsteps, false);
}

void moveTo(Axis *ax, position_t position)
{
    position_t diff = position - ax->Position;

    steps(ax, (steps_t)abs(diff), diff < 0);
}

float readHall(Axis *ax, size_t Nmean)
{

    float val_mean = 0;

    for (size_t k = 0; k < Nmean; k++)
    {
        val_mean += (1 / (float)Nmean) * (float)analogRead(ax->HallPin);
    }

    return val_mean;
}

float readHall(Axis *ax)
{

    return readHall(ax, N_MEAN_DEFAULT_HALL_READOUT);
}