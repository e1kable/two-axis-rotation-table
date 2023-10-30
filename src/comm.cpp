
#include <Arduino.h>
#include <ArduinoSTL.h>
#include <sstream>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#include "comm.h"
#include "common.h"
#include "reference.h"

using namespace std;

void serializeAxisStatus(Axis *ax)
{
    StaticJsonDocument<200> doc;
    doc["StepPin"] = ax->StepPin;
    doc["DirectionPin"] = ax->DirectionPin;
    doc["HallPin"] = ax->HallPin;
    doc["ReferenceOffset"] = ax->ReferenceOffset;
    doc["IsInit"] = ax->IsInit;
    doc["Position"] = ax->Position;
    doc["LastStepTime"] = ax->LastStepTime;
    doc["IsReferenced"] = ax->IsReferenced;
    doc["ReferencePosition"] = ax->ReferencePosition;

    serializeJson(doc, cout);
}

Axis *parseAxis(string ax)
{

    Axis *selectedAx;
    if (ax == "AZ")
    {
        selectedAx = &AX_AZ;
    }
    else if (ax == "EL")
    {
        selectedAx = &AX_EL;
    }
    else
    {
        selectedAx = NULL;
    }
    return selectedAx;
}

bool atob(string str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    istringstream is(str);
    bool b;
    is >> boolalpha >> b;
    return b;
}

void parseSerialCommands()
{
    // if there is datat to read
    if (Serial.available() > 0)
    {

        String buf = Serial.readStringUntil('\n');

        string cmd(buf.begin(), buf.length()); // convert to std::string

        // get the first command
        vector<string> cmdParts;
        size_t pos = 0;
        string delimiter = " ";
        while ((pos = cmd.find(delimiter)) != std::string::npos)
        {
            cmdParts.push_back(cmd.substr(0, pos));
            cmd.erase(0, pos + delimiter.length());
        }
        cmdParts.push_back(cmd);

        // if the cmd is empty
        if (cmdParts.size() == 0)
        {
            return;
        }

        if (cmdParts[0] == "test")
        {
            cout << CMD_OK << endl;
        }
        else if (cmdParts[0] == "status")
        {
            if (cmdParts.size() != 2)
            {

                cout << ERR_UNKNOWN_CMD << " Size (" << cmdParts.size() << ") not matching" << endl;
                return;
            }

            Axis *selectedAx = parseAxis(cmdParts[1]);
            if (selectedAx == NULL)
            {
                cout << ERR_UNKNOWN_AX << endl;
                return;
            }
            serializeAxisStatus(selectedAx);
            cout << endl;
        }
        else if (cmdParts[0] == "steps")
        {

            if (cmdParts.size() != 4)
            {

                cout << ERR_UNKNOWN_CMD << " Size (" << cmdParts.size() << ")" << endl;
                return;
            }

            Axis *selectedAx = parseAxis(cmdParts[1]);
            if (selectedAx == NULL)
            {
                cout << ERR_UNKNOWN_AX << endl;
                return;
            }

            int Nsteps = atoi(cmdParts[2].c_str());

            bool isReverse = atob(cmdParts[3]);

            steps(selectedAx, Nsteps, isReverse);

            cout << CMD_OK << endl;
        }
        else if (cmdParts[0] == "reference")
        {
            if (cmdParts.size() != 2)
            {

                cout << ERR_UNKNOWN_CMD << " Size (" << cmdParts.size() << ")" << endl;
                return;
            }
            Axis *selectedAx = parseAxis(cmdParts[1]);
            if (selectedAx == NULL)
            {
                cout << ERR_UNKNOWN_AX << endl;
                return;
            }

            referenceAxis(selectedAx);
            cout << CMD_OK << endl;
        }
        else
        {
            cout << ERR_UNKNOWN_CMD << " (" << cmdParts[0] << ")" << endl;
        }
    }
}