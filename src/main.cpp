#include <Arduino.h>

#ifdef ARDUINO_AVR_UNO
#include <ArduinoSTL.h>
#endif

#include "common.h"
#include "comm.h"
#include "reference.h"

using namespace std;

void setup()
{
  while (!Serial)
    ;

  Serial.begin(SERIAL_BAUD);
  Serial.println(CMD_OK.c_str());
}

void loop()
{
  parseSerialCommands();
}
