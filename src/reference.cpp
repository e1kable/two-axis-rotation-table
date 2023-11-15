#include <Arduino.h>
#include <ArduinoSTL.h>
#include <vector>
#include <string.h>
#include "common.h"

using namespace std;

void referenceAxis(Axis *ax)
{
    cout << "start referencing" << endl;
    // serach for max posiive change

    float lastMeasurement = readHall(ax);

    size_t LEN_DBDPHI_HIST = 3;

    size_t dbdphi_hist_ptr = 0;
    vector<float> dbdphi_hist(LEN_DBDPHI_HIST);

    position_t maxPos = 0;
    float maxPosValue = 0;

    bool isInArea = false;
    for (steps_t x = 0; x < ax->TotalSteps; x++)
    {

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

        float dbdphi = val_mean - lastMeasurement;
        dbdphi_hist[dbdphi_hist_ptr++] = dbdphi;
        dbdphi_hist_ptr = dbdphi_hist_ptr % LEN_DBDPHI_HIST;
        lastMeasurement = val_mean;

        cout << "Pos: " << ax->Position << " Val_mean: " << val_mean << " dB/dphi " << dbdphi << endl;

        // check if last entries are above threshold

        bool allTrue = all_of(dbdphi_hist.begin(), dbdphi_hist.end(), [](float f)
                              { return f > DB_DPHI_THRES; });

        if (allTrue) // if is in region
        {

            if (!isInArea)
            {
                cout << "in ->" << endl;
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
            cout << "<- out" << endl;
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
