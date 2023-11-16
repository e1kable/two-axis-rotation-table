
#include <Arduino.h>

#ifdef ARDUINO_AVR_UNO
#include <ArduinoSTL.h>
#include <sstream>
#endif

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#include "comm.h"
#include "common.h"
#include "reference.h"

using namespace std;

#ifdef ARDUINO_AVR_UNO
bool atob(string str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    istringstream is(str);
    bool b;
    is >> boolalpha >> b;
    return b;
}

#endif

#ifdef ARDUINO_ARCH_RENESAS_UNO

bool atob(string str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);

    if (str == "true")
    {
        return true;
    }
    return false;
}

#endif

void serializeAxisStatus(Axis *ax)
{
    StaticJsonDocument<200> doc;
    doc["StepPin"] = ax->StepPin;
    doc["DirectionPin"] = ax->DirectionPin;
    doc["HallPin"] = ax->HallPin;
    doc["ReferenceOffset"] = ax->ReferenceOffset;
    doc["TotalSteps"] = ax->TotalSteps;
    doc["MaxDPhiDt"] = ax->MaxDPhiDt;
    doc["IsInit"] = ax->IsInit;
    doc["Position"] = ax->Position;
    doc["LastStepTime"] = ax->LastStepTime;
    doc["IsReferenced"] = ax->IsReferenced;
    doc["ReferencePosition"] = ax->ReferencePosition;

    serializeJson(doc, Serial);
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
            Serial.println(CMD_OK.c_str());
        }
        else if (cmdParts[0] == "status")
        {
            if (cmdParts.size() != 2)
            {

                char buf[100];
                sprintf(buf, "%s Size(%u) not matching", ERR_UNKNOWN_CMD.c_str(), cmdParts.size());
                Serial.println(buf);

                return;
            }

            Axis *selectedAx = parseAxis(cmdParts[1]);
            if (selectedAx == NULL)
            {
                Serial.println(ERR_UNKNOWN_AX.c_str());
                return;
            }
            serializeAxisStatus(selectedAx);

            Serial.println();
        }
        else if (cmdParts[0] == "steps")
        {

            if (cmdParts.size() != 4)
            {

                char buf[100];
                sprintf(buf, "%s Size(%u) not matching", ERR_UNKNOWN_CMD.c_str(), cmdParts.size());
                Serial.println(buf);

                return;
            }

            Axis *selectedAx = parseAxis(cmdParts[1]);
            if (selectedAx == NULL)
            {
                Serial.println(ERR_UNKNOWN_AX.c_str());
                return;
            }

            int Nsteps = atoi(cmdParts[2].c_str());

            bool isReverse = atob(cmdParts[3]);

            steps(selectedAx, Nsteps, isReverse);

            Serial.println(CMD_OK.c_str());
        }
        else if (cmdParts[0] == "reference")
        {
            if (cmdParts.size() != 2)
            {

                char buf[100];
                sprintf(buf, "%s Size(%u) not matching", ERR_UNKNOWN_CMD.c_str(), cmdParts.size());
                Serial.println(buf);
                return;
            }
            Axis *selectedAx = parseAxis(cmdParts[1]);
            if (selectedAx == NULL)
            {
                Serial.println(ERR_UNKNOWN_AX.c_str());
                return;
            }

            referenceAxis(selectedAx);
            Serial.println(CMD_OK.c_str());
        }
        else if (cmdParts[0] == "readhall")
        {
            if (cmdParts.size() != 3)
            {

                char buf[100];
                sprintf(buf, "%s Size(%u) not matching", ERR_UNKNOWN_CMD.c_str(), cmdParts.size());
                Serial.println(buf);
                return;
            }
            Axis *selectedAx = parseAxis(cmdParts[1]);
            if (selectedAx == NULL)
            {
                Serial.println(ERR_UNKNOWN_AX.c_str());
                return;
            }

            int Nmean = atoi(cmdParts[2].c_str());

            Serial.println(readHall(selectedAx, Nmean));
        }
        else
        {
            char buf[100];
            sprintf(buf, "%s (%s) ", ERR_UNKNOWN_CMD.c_str(), cmdParts[0].c_str());
            Serial.println(buf);
        }
    }
}