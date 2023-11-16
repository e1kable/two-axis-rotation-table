#ifndef ROT_TABLE_COMMON_H
#define ROT_TABLE_COMMON_H

#include <Arduino.h>
#include <vector>
#include <numeric>   // std::iota
#include <algorithm> // std::sort, std::stable_sort

#include "config.h"

using namespace std;

typedef int32_t position_t;
typedef uint32_t steps_t;

struct Axis
{
    const uint8_t StepPin;
    const uint8_t DirectionPin;
    const uint8_t HallPin;
    const float ReferenceOffset;

    const steps_t TotalSteps;

    const float MaxDPhiDt;

    bool IsInit;
    position_t Position;
    uint64_t LastStepTime;

    bool IsReferenced;
    position_t ReferencePosition;
};

extern struct Axis AX_AZ;
extern struct Axis AX_EL;

void initAxis(Axis *ax);
void step(Axis *axis, bool reverse);
void step(Axis *axis);
void steps(Axis *ax, steps_t steps);
void steps(Axis *ax, steps_t steps, bool reverse);

void moveTo(Axis *ax, position_t steps);
float readHall(Axis *ax, size_t Nmean);
float readHall(Axis *ax);

#ifdef ARDUINO_AVR_UNO

template <class InputIt, class UnaryPredicate>
InputIt find_if_not(InputIt first, InputIt last, UnaryPredicate q)
{
    for (; first != last; ++first)
        if (!q(*first))
            return first;

    return last;
}
template <class InputIt, class UnaryPredicate>
constexpr bool all_of(InputIt first, InputIt last, UnaryPredicate p)
{
    return find_if_not(first, last, p) == last;
}
#endif

#endif