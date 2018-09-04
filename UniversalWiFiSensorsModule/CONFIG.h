#pragma once
#include <Arduino.h>
//--------------------------------------------------------------------------------------------------------------------------------------
#define NO_TEMPERATURE_DATA -128
#define NO_LUMINOSITY_DATA -1
//--------------------------------------------------------------------------------------------------------------------------------------
typedef enum
{
  sensor_NoSensor,
  sensor_DS18B20,
  sensor_DS18S20,
  sensor_BH1750,
  
} SensorType;
//--------------------------------------------------------------------------------------------------------------------------------------
struct SensorSettings
{
  SensorType type;
  uint8_t targetIndex;
  uint8_t param1;
  uint8_t param2;
  uint8_t param3;
  void* data;

  SensorSettings()
  {
    type = sensor_NoSensor;
    param1 = param2 = param3 = 0;
    targetIndex = 0;
    data = NULL;
  }

  SensorSettings(SensorType t = sensor_NoSensor, uint8_t ti = 0, uint8_t p1 = 0, uint8_t p2 = 0, uint8_t p3 = 0, void* d = NULL)
  {
    type = t;
    targetIndex = ti;
    param1 = p1;
    param2 =  p2;
    param3 = p3;
    data = d;
    
  }
  
};
//--------------------------------------------------------------------------------------------------------------------------------------

