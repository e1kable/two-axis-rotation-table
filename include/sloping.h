#ifndef ROT_TABLE_SLOPING_H
#define ROT_TABLE_SLOPING_H

#include <ArduinoSTL.h>

using namespace std;

float linearSlope(float phi_i, float ts, float phiDotMax, float phi);
uint32_t generateSlopeSteps(steps_t i, float phi, float phiDotMax, steps_t totalSteps, steps_t &Nsteps);

#endif