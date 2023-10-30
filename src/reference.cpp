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
    float dbdphi_hist[LEN_DBDPHI_HIST];
    size_t dbdphi_hist_ptr = 0;

    vector<position_t> areaMeasurementsPositions;
    vector<float> areaMesurementValues;

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
        bool allTrue = assertAllTrue<float>(dbdphi_hist, LEN_DBDPHI_HIST, [](float h) -> bool
                                            { return h > DB_DPHI_THRES; });

        if (allTrue) // if is in region
        {

            if (!isInArea)
            {
                cout << "in ->" << endl;
            }
            isInArea = true;

            areaMeasurementsPositions.push_back(ax->Position);
            areaMesurementValues.push_back(dbdphi);
        }

        if (!allTrue && isInArea)
        {
            cout << "<- out" << endl;
            break;
        }
    }

    // find max index
    size_t Idx = std::distance(areaMesurementValues.begin(), std::max_element(areaMesurementValues.begin(), areaMesurementValues.end()));

    // offset to steps
    position_t offset = (position_t)((ax->ReferenceOffset / 360) * (float)ax->TotalSteps);
    position_t correctedRef = areaMeasurementsPositions[Idx] + offset;

    cout << "refpos: " << areaMeasurementsPositions[Idx] << " offset correction (steps): " << offset << " corrected pos: " << correctedRef << endl;
    ax->IsReferenced = true;
    ax->ReferencePosition = correctedRef;

    moveTo(ax, ax->ReferencePosition);
}
