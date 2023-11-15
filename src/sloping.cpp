#include <ArduinoSTL.h>
#include "config.h"
#include "common.h"
#include "sloping.h"

using namespace std;

float linearSlope(float phi_i, float ts, float phiDotMax, float phi)
{
    /// refer to ../docs/slope.py for more detailed explanation

    float phiSlope = phiDotMax * ts; // phi after the slope
    // if not a full slope is required
    if (phiSlope > phi)
    {
        float t0 = sqrt(phi * ts / phiDotMax);

        float phiDotEnd = phiDotMax / ts * t0;

        float t1 = phiDotEnd * ts / phiDotMax + t0;

        if (phi_i < phi / 2.0)
        {
            return sqrt(2 * phi_i * ts / phiDotMax);
        }
        else if (phi_i >= phi / 2 && phi_i < phi)
        {
            float ti = ts * phiDotEnd / phiDotMax - sqrt(pow((ts * phiDotEnd / phiDotMax), 2) - 2 * ts * (phi_i - phi / 2.0) / phiDotMax);

            return t0 + ti;
        }
        else
        {
            return t0 + t1;
        }
    }
    // if we can to full sloping
    else
    {
        float phiRest = phi - phiSlope;
        float tr = phiRest / phiDotMax;
        if (phi_i < phiDotMax * (ts / 2))
        {
            return sqrt(phi_i * 2 * ts / phiDotMax);
        }
        else if (phi_i >= phiDotMax * (ts / 2) && phi_i <= phiDotMax * (ts / 2 + tr))
        {
            return ts + (phi_i - phiDotMax * (ts / 2)) / phiDotMax;
        }
        else if (phi_i > phiDotMax * (ts / 2 + tr) && phi <= phiDotMax * (ts + tr))
        {
            return -sqrt((-phi_i + phiDotMax * (ts + tr)) / ((1 / 2.0) * phiDotMax / ts)) + tr + 2 * ts;
        }
        else
        {
            return 2 * ts + tr;
        }
    }
}
uint32_t generateSlopeSteps(steps_t i, float phi, float phiDotMax, steps_t totalSteps, steps_t &Nsteps)
{
    float phiPerStep = 360.0 / (float)totalSteps;

    Nsteps = (steps_t)round(phi / phiPerStep) - 1; // approximate number of steps

    float ti = linearSlope((i + 1) * phiPerStep, T_SLOPE, phiDotMax, phi);
    float t0 = linearSlope(i * phiPerStep, T_SLOPE, phiDotMax, phi);

    return (ti - t0) * 1000;
}
