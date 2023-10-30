#include <Arduino.h>
#include <ArduinoSTL.h>
// #include <EEPROM.h>

#include "common.h"
#include "reference.h"

void setup()
{

  Serial.begin(SERIAL_BAUD);

  cout << "Total steps: " << N_STEPS_TOTAL << endl;

  // steps(&AX_AZ, 100, true);
  // referenceAxis3(&AX_AZ);
  // cout << "place sticker now" << endl;
  // delay(5000); // delay for 1 second

  // moveTo(&AX_AZ, AX_AZ.ReferencePosition + 100);
  // moveTo(&AX_AZ, AX_AZ.ReferencePosition - 300);
  // moveTo(&AX_AZ, AX_AZ.ReferencePosition);

  // delay(1000); // delay for 1 second
  // // do full rotation
  // steps(&AX_AZ, N_STEPS_TOTAL);

  // delay(1000); // delay for 1 second

  // moveTo(&AX_AZ, AX_AZ.ReferencePosition);

  // delay(2000); // delay for 1 second

  // moveTo(&AX_AZ, AX_AZ.ReferencePosition - 400);

  // referenceAxis3(&AX_AZ);

  // delay(2000); // delay for 1 second

  // step(&AX_AZ, false);
  // delay(500); // delay for 1 second
  // step(&AX_AZ, true);
  // delay(500); // delay for 1 second
  // step(&AX_AZ, false);
  // delay(500); // delay for 1 second
  // step(&AX_AZ, true);
  // delay(500); // delay for 1 second

  delay(2000);
  steps(&AX_EL, 50, true);
  referenceAxis3(&AX_EL);

  steps(&AX_EL, N_STEPS_TOTAL);

  position_t p = AX_EL.Position;
  cout << "pos at " << p << endl;

  // referenceAxis3(&AX_EL);

  // position_t diff = p - AX_EL.Position;
  // cout << "diff at " << diff << endl;

  //   delay(2000);
  // steps(&AX_EL, 50, true);
  // referenceAxis3(&AX_EL);
}

void loop()
{

  if (Serial.available())
  {
  }
}
