#include <Arduino.h>
#include <ArduinoSTL.h>
#include <vector>
#include <string.h>
#include "common.h"

using namespace std;

void referenceAxis2(Axis *ax)
{
    // 1. first search the two precipices
    size_t LEN_DBDPHI_HIST = 5;
    float dbdphi_hist[LEN_DBDPHI_HIST];
    size_t dbdphi_hist_ptr = 0;

    bool foundFirst = false;
    position_t posFirstPrec = 0;

    float lastMeasurement = readHall(ax);

    // --- first precipice
    for (steps_t x = 0; x < N_STEPS_TOTAL; x++)
    {
        step(ax);
        float val_mean = readHall(ax);

        // fill up the history
        float dbdphi = abs(val_mean - lastMeasurement);
        dbdphi_hist[dbdphi_hist_ptr++] = dbdphi;
        dbdphi_hist_ptr = dbdphi_hist_ptr % LEN_DBDPHI_HIST;
        lastMeasurement = val_mean;

        cout << "Pos: " << ax->Position << " Val_mean: " << val_mean << " dB/dphi " << dbdphi << endl;

        // check if last entries are above threshold
        if (!foundFirst)
        {
            bool allTrue = assertAllTrue<float>(dbdphi_hist, LEN_DBDPHI_HIST, [](float h) -> bool
                                                { return h > DB_DPHI_THRES; });

            if (allTrue)
            {
                foundFirst = true,
                posFirstPrec = ax->Position;

                cout << endl
                     << "found first@ " << ax->Position << endl;
            }
        }

        // continue stepping until all values are below threshold
        if (foundFirst)
        {
            bool allTrue = assertAllTrue<float>(dbdphi_hist, LEN_DBDPHI_HIST, [](float h) -> bool
                                                { return h <= DB_DPHI_THRES; });
            if (allTrue)
            {
                cout << endl
                     << "out of region, stopping " << endl;
                break; // stop stepping
            }
        }
    }

    // --- second precipice
    steps_t Nsteps = (steps_t)((float)N_STEPS_TOTAL) / 8.0; // search only on max 45 deg (1/8)
    cout << "searching second @ " << ax->Position << " with max of (steps): " << Nsteps << endl;

    bool foundSecond = false;
    position_t posSecondPrec = 0;
    memset(&dbdphi_hist, 0, sizeof(dbdphi_hist));
    dbdphi_hist_ptr = 0;

    for (steps_t x = 0; x < Nsteps; x++)
    {
        step(ax);
        float val_mean = readHall(ax);

        // fill up the history
        float dbdphi = abs(val_mean - lastMeasurement);
        dbdphi_hist[dbdphi_hist_ptr++] = dbdphi;
        dbdphi_hist_ptr = dbdphi_hist_ptr % LEN_DBDPHI_HIST;
        lastMeasurement = val_mean;

        cout << "Pos: " << ax->Position << " Val_mean: " << val_mean << " dB/dphi " << dbdphi << endl;

        // check if last entries are above threshold
        bool allTrue = assertAllTrue<float>(dbdphi_hist, LEN_DBDPHI_HIST, [](float h) -> bool
                                            { return h > DB_DPHI_THRES; });

        if (allTrue)
        {
            posSecondPrec = ax->Position;
            foundSecond = true;

            cout << "found second @" << ax->Position << endl;
            break;
        }
    }

    // if not found try the other way round
    if (!foundSecond)
    {

        cout << "no luck this direction, try reverse @ " << ax->Position << endl;
        moveTo(ax, posFirstPrec);
        cout << "am now @ " << ax->Position << endl;

        for (steps_t x = 0; x < N_STEPS_TOTAL; x++)
        {
            step(ax, true);
            float val_mean = readHall(ax);

            // fill up the history
            dbdphi_hist[dbdphi_hist_ptr++] = abs(val_mean - lastMeasurement);
            dbdphi_hist_ptr = dbdphi_hist_ptr % LEN_DBDPHI_HIST;
            lastMeasurement = val_mean;

            // check if last entries are above threshold
            bool allTrue = true;
            for (uint8_t k = 0; k < LEN_DBDPHI_HIST; k++)
            {
                allTrue = allTrue && (dbdphi_hist[k] > DB_DPHI_THRES);
            }

            if (allTrue)
            {
                posSecondPrec = ax->Position;
                foundSecond = true;
                break;
            }
        }
    }

    cout << "Pos 1: " << posFirstPrec << " Pos 2: " << posSecondPrec << endl;

    // if the first one is larger -> swap them
    if (posFirstPrec > posSecondPrec)
    {
        position_t temp = posFirstPrec;
        posFirstPrec = posSecondPrec;
        posSecondPrec = temp;
    }

    Nsteps = (steps_t)(posSecondPrec - posFirstPrec);

    cout << "moving to first prec by (steps): " << Nsteps << endl;

    moveTo(ax, posFirstPrec);

    vector<float> hallValues(Nsteps);

    for (steps_t i = 0; i < Nsteps; i++)
    {
        delay(10); // wait for the system to settle down
        hallValues.at(i) = abs(readHall(ax) - HALL_ZERO_V_VALUE);
        step(ax);

        cout << hallValues.at(i) << " ";
    }
    cout << endl;

    vector<size_t> idx = sort_indexes(hallValues); // sort indices
    std::reverse(idx.begin(), idx.end());          // reverse vector so that large values come first
    size_t Nval = (size_t)((float)idx.size() / 4);

    float sum = (float)accumulate(idx.begin(), idx.begin() + Nval, 0.0) / Nval;
    position_t referencePos = (position_t)(sum + posFirstPrec);
    ax->ReferencePosition = referencePos;
    ax->IsReferenced = true;

    cout << "center step is @" << referencePos << " Stepping back by " << referencePos - ax->Position << endl;

    moveTo(ax, referencePos);

    cout << "done." << endl;
};

void referenceAxis3(Axis *ax)
{
    cout << "start referencing" << endl;
    // serach for max posiive change

    float lastMeasurement = readHall(ax);

    size_t LEN_DBDPHI_HIST = 3;
    float dbdphi_hist[LEN_DBDPHI_HIST];
    size_t dbdphi_hist_ptr = 0;

    vector<position_t> areaMeasurementsPositions;
    vector<float> areaMesurementValues;

    bool isInArea = false;
    for (steps_t x = 0; x < N_STEPS_TOTAL; x++)
    {

        step(ax);

        float val_mean = 0.0;
        if (isInArea)
        {
            //delay(100); // delay some time, to let system settle down
            val_mean = readHall(ax, N_MEAN_DEFAULT_HALL_FINE_READOUT);
        }
        else
        {
            val_mean = readHall(ax);
        }

        float dbdphi = val_mean - lastMeasurement;
        dbdphi_hist[dbdphi_hist_ptr++] = dbdphi;
        dbdphi_hist_ptr = dbdphi_hist_ptr % LEN_DBDPHI_HIST;
        lastMeasurement = val_mean;

        cout << "Pos: " << ax->Position << " Val_mean: " << val_mean << " dB/dphi " << dbdphi << endl;

        // check if last entries are above threshold
        bool allTrue = assertAllTrue<float>(dbdphi_hist, LEN_DBDPHI_HIST, [](float h) -> bool
                                            { return h > DB_DPHI_THRES; });

        if (allTrue) // if is in region
        {

            if (!isInArea)
            {
                cout << "In Area now" << endl;
            }
            isInArea = true;

            areaMeasurementsPositions.push_back(ax->Position);
            areaMesurementValues.push_back(dbdphi);
        }

        if (!allTrue && isInArea)
        {
            cout << "Out of Area now" << endl;
            break;
        }
    }

    // find max index
    size_t Idx = std::distance(areaMesurementValues.begin(), std::max_element(areaMesurementValues.begin(), areaMesurementValues.end()));

    // offset to steps
    position_t offset = (position_t)((ax->ReferenceOffset / 360) * (float)N_STEPS_TOTAL);
    position_t correctedRef = areaMeasurementsPositions[Idx] + offset;

    cout << "refpos: " << areaMeasurementsPositions[Idx] << " offset correction (steps): " << offset << " corrected pos: " << correctedRef << endl;
    ax->IsReferenced = true;
    ax->ReferencePosition = correctedRef;

    moveTo(ax, ax->ReferencePosition);
    cout
        << "done." << endl;
}

// void findPrecipice()
