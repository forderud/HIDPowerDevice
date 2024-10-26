#include <HIDPowerDevice.h>

void setup() {
  Serial.begin(57600);
  
  // Used for debugging purposes. 
  PowerDevice.setOutput(Serial);
}

void loop() {
  delay(1000);
}
