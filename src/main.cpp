#include <Arduino.h>
#include <ArduinoSTL.h>
// #include <EEPROM.h>

#include "common.h"
#include "reference.h"
#include "comm.h"
#include "sloping.h"

void setup()
{

  Serial.begin(SERIAL_BAUD);
  cout << CMD_OK << endl;
}

void loop()
{
  parseSerialCommands();
}
