#include <HIDPowerDevice.h>

#define MINUPDATEINTERVAL   26
#define CHGDCHPIN           4
#define RUNSTATUSPIN        5
#define COMMLOSTPIN         10
#define BATTSOCPIN          A7

int iIntTimer=0;


// String constants 
const char STRING_DEVICECHEMISTRY[] PROGMEM = "PbAc";
const char STRING_OEMVENDOR[] PROGMEM = "MyCoolUPS";
const char STRING_SERIAL[] PROGMEM = "UPS10"; 

const byte bDeviceChemistry = IDEVICECHEMISTRY;
const byte bOEMVendor = IOEMVENDOR;

PresentStatus iPresentStatus = {}, iPreviousStatus = {};

byte bRechargable = 1;
byte bCapacityMode = 1;  // unit: 0=mAh, 1=mWh, 2=%

// Physical parameters
const uint16_t iConfigVoltage = 1380; // centiVolt
uint16_t iVoltage =1300; // centiVolt
uint16_t iRunTimeToEmpty = 0, iPrevRunTimeToEmpty = 0;
uint16_t iAvgTimeToFull = 7200;
uint16_t iAvgTimeToEmpty = 7200;
uint16_t iRemainTimeLimit = 600;
uint16_t iManufacturerDate = 0; // initialized in setup function

// Parameters for ACPI compliancy
const byte iDesignCapacity = 100;
byte iRemnCapacityLimit = 5; // low at 5% 
const byte bCapacityGranularity1 = 1;
const byte bCapacityGranularity2 = 1;
byte iFullChargeCapacity = 100;

byte iRemaining =0, iPrevRemaining=0;
bool bCharging = false;

int iRes=0;


void setup() {

  Serial.begin(57600);
  
  // Serial No is set in a special way as it forms Arduino port name
  PowerDevice.setSerial(STRING_SERIAL); 
  
  // Used for debugging purposes. 
  PowerDevice.setOutput(Serial);
  
  pinMode(CHGDCHPIN, INPUT_PULLUP); // ground this pin to simulate power failure. 
  pinMode(RUNSTATUSPIN, OUTPUT);  // output flushing 1 sec indicating that the arduino cycle is running. 
  pinMode(COMMLOSTPIN, OUTPUT); // output is on once communication is lost with the host, otherwise off.


  PowerDevice.setFeature(HID_PD_PRESENTSTATUS, &iPresentStatus, sizeof(iPresentStatus));
  
  PowerDevice.setFeature(HID_PD_RUNTIMETOEMPTY, &iRunTimeToEmpty, sizeof(iRunTimeToEmpty));
  PowerDevice.setFeature(HID_PD_AVERAGETIME2FULL, &iAvgTimeToFull, sizeof(iAvgTimeToFull));
  PowerDevice.setFeature(HID_PD_AVERAGETIME2EMPTY, &iAvgTimeToEmpty, sizeof(iAvgTimeToEmpty));
  PowerDevice.setFeature(HID_PD_REMAINTIMELIMIT, &iRemainTimeLimit, sizeof(iRemainTimeLimit));
  
  PowerDevice.setFeature(HID_PD_RECHARGEABLE, &bRechargable, sizeof(bRechargable));
  PowerDevice.setFeature(HID_PD_CAPACITYMODE, &bCapacityMode, sizeof(bCapacityMode));
  PowerDevice.setFeature(HID_PD_CONFIGVOLTAGE, &iConfigVoltage, sizeof(iConfigVoltage));
  PowerDevice.setFeature(HID_PD_VOLTAGE, &iVoltage, sizeof(iVoltage));

  PowerDevice.setStringFeature(HID_PD_IDEVICECHEMISTRY, &bDeviceChemistry, STRING_DEVICECHEMISTRY);
  PowerDevice.setStringFeature(HID_PD_IOEMINFORMATION, &bOEMVendor, STRING_OEMVENDOR);

  PowerDevice.setFeature(HID_PD_DESIGNCAPACITY, &iDesignCapacity, sizeof(iDesignCapacity));
  PowerDevice.setFeature(HID_PD_FULLCHRGECAPACITY, &iFullChargeCapacity, sizeof(iFullChargeCapacity));
  PowerDevice.setFeature(HID_PD_REMAININGCAPACITY, &iRemaining, sizeof(iRemaining));
  PowerDevice.setFeature(HID_PD_REMNCAPACITYLIMIT, &iRemnCapacityLimit, sizeof(iRemnCapacityLimit));
  PowerDevice.setFeature(HID_PD_CPCTYGRANULARITY1, &bCapacityGranularity1, sizeof(bCapacityGranularity1));
  PowerDevice.setFeature(HID_PD_CPCTYGRANULARITY2, &bCapacityGranularity2, sizeof(bCapacityGranularity2));

  uint16_t year = 2024, month = 10, day = 12;
  iManufacturerDate = (year - 1980)*512 + month*32 + day; // from 4.2.6 Battery Settings in "Universal Serial Bus Usage Tables for HID Power Devices"
  PowerDevice.setFeature(HID_PD_MANUFACTUREDATE, &iManufacturerDate, sizeof(iManufacturerDate));
}

void loop() {
  //*********** Measurements Unit ****************************
  int iBattSoc = analogRead(BATTSOCPIN); // potensiometer value in [0,1024)

  iRemaining = (byte)(round((float)iFullChargeCapacity*iBattSoc/1024));
  iRunTimeToEmpty = (uint16_t)round((float)iAvgTimeToEmpty*iRemaining/iFullChargeCapacity);

  if (iRemaining > iPrevRemaining)
    bCharging = true;
  else if (iRemaining < iPrevRemaining)
    bCharging = false;
  
  // Charging
  iPresentStatus.Charging = bCharging;
  iPresentStatus.ACPresent = bCharging; // assume charging implies AC present
  iPresentStatus.FullyCharged = (iRemaining == iFullChargeCapacity);
    
  // Discharging
  if(!bCharging) { // assume not charging implies discharging
    iPresentStatus.Discharging = 1;
    // if(iRemaining < iRemnCapacityLimit) iPresentStatus.BelowRemainingCapacityLimit = 1;
    
    iPresentStatus.RemainingTimeLimitExpired = (iRunTimeToEmpty < iRemainTimeLimit);

  }
  else {
    iPresentStatus.Discharging = 0;
    iPresentStatus.RemainingTimeLimitExpired = 0;
  }

  // Shutdown imminent
  if((iPresentStatus.ShutdownRequested) || 
     (iPresentStatus.RemainingTimeLimitExpired)) {
    iPresentStatus.ShutdownImminent = 1;
    Serial.println("shutdown imminent");
  }
  else
    iPresentStatus.ShutdownImminent = 0;


  
  iPresentStatus.BatteryPresent = 1;

  

  //************ Delay ****************************************  
  delay(1000);
  iIntTimer++;
  digitalWrite(RUNSTATUSPIN, HIGH);   // turn the LED on (HIGH is the voltage level);
  delay(1000);
  iIntTimer++;
  digitalWrite(RUNSTATUSPIN, LOW);   // turn the LED off;

  //************ Check if we are still online ******************

  

  //************ Bulk send or interrupt ***********************

  if((iPresentStatus != iPreviousStatus) || (iRemaining != iPrevRemaining) || (iRunTimeToEmpty != iPrevRunTimeToEmpty) || (iIntTimer>MINUPDATEINTERVAL) ) {

    PowerDevice.sendReport(HID_PD_REMAININGCAPACITY, &iRemaining, sizeof(iRemaining));
    if(!bCharging) PowerDevice.sendReport(HID_PD_RUNTIMETOEMPTY, &iRunTimeToEmpty, sizeof(iRunTimeToEmpty));
    iRes = PowerDevice.sendReport(HID_PD_PRESENTSTATUS, &iPresentStatus, sizeof(iPresentStatus));

    if(iRes <0 ) {
      digitalWrite(COMMLOSTPIN, HIGH);
    }
    else
      digitalWrite(COMMLOSTPIN, LOW);
        
    iIntTimer = 0;
    iPreviousStatus = iPresentStatus;
    if (abs(iPrevRemaining - iRemaining) > 1) // add a bit of hysteresis
      iPrevRemaining = iRemaining;
    iPrevRunTimeToEmpty = iRunTimeToEmpty;
  }
  

  Serial.println(iRemaining);
  Serial.println(iRunTimeToEmpty);
  Serial.println(iRes);
  
}
