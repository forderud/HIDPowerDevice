/*
  HIDPowerDevice.h

  Copyright (c) 2020, Aleksandr Bratchik
  Original code (pre-library): Copyright (c) 2020, Aleksandr Bratchik

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef HIDPOWERDEVICE_h
#define HIDPOWERDEVICE_h

#define _USING_HID



#include "HID/HID.h"
#include "WString.h"


#if !defined(_USING_HID)

#warning "Using legacy HID core (non pluggable)"

#else

//================================================================================
//================================================================================

#define HID_PD_IPRODUCT              0x01 // FEATURE ONLY
#define HID_PD_MANUFACTURER          0x03 // FEATURE ONLY

#define HID_PD_PRESENTSTATUS         0x07 // INPUT OR FEATURE(required by Windows)
#define HID_PD_VOLTAGE               0x0B // 11 INPUT (NA) OR FEATURE(implemented)
#define HID_PD_REMAININGCAPACITY     0x0C // 12 INPUT OR FEATURE(required by Windows)
#define HID_PD_FULLCHRGECAPACITY     0x0E // 14 FEATURE ONLY. Last Full Charge Capacity 
#define HID_PD_CAPACITYMODE          0x16
#define HID_PD_DESIGNCAPACITY        0x17

// PresentStatus dynamic flags
struct PresentStatus {
  uint8_t Charging : 1;                   // bit 0x00
  uint8_t Discharging : 1;                // bit 0x01
  uint8_t ACPresent : 1;                  // bit 0x02
  
  operator uint8_t () {
      return *(uint8_t*)(this); // switch to std::bit_cast after migrating to C++20
  }
};
static_assert(sizeof(PresentStatus) == sizeof(uint8_t));





class HIDPowerDevice_  {
    
private:
    
    const byte bProduct = IPRODUCT;
    const byte bManufacturer = IMANUFACTURER;
    
public:
  HIDPowerDevice_(void);
  
  void setOutput(Serial_&);
  
  void setSerial(const char*);
  
  int sendDate(uint16_t id, uint16_t year, uint8_t month, uint8_t day);
  int sendReport(uint16_t id, const void* bval, int len);
  
  int setFeature(uint16_t id, const void* data, int len);
  
  int setStringFeature(uint8_t id, const uint8_t* index, const char* data);

};

extern HIDPowerDevice_ PowerDevice;


#endif
#endif

