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
extern const steps_t N_STEPS_TOTAL;

struct Axis
{
    const uint8_t StepPin;
    const uint8_t DirectionPin;
    const uint8_t HallPin;
    const float ReferenceOffset;

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

template <class T>
bool assertAllTrue(T array[], size_t size, bool (*func)(T))
{
    for (size_t k = 0; k < size; k++)
    {
        if (!func(array[k]))
            return false;
    }
    return true;
}

template <class ForwardIt, class T>
constexpr // since C++20
    void
    iota(ForwardIt first, ForwardIt last, T value)
{
    while (first != last)
    {
        *first++ = value;
        ++value;
    }
}

template <typename T>
vector<size_t> sort_indexes(const vector<T> &v)
{

    // initialize original index locations
    vector<size_t> idx(v.size());
    iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    // using std::stable_sort instead of std::sort
    // to avoid unnecessary index re-orderings
    // when v contains elements of equal values
    stable_sort(idx.begin(), idx.end(),
                [&v](size_t i1, size_t i2)
                { return v[i1] < v[i2]; });

    return idx;
}
#endif