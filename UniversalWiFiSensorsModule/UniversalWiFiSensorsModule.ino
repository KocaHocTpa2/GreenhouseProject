//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "CONFIG.h"
#include "DS18B20Query.h"
#include "BH1750.h"
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Прошивка универсального модуля с датчиками, отсылающего данные на контроллер через ESP.
// Прошивка предназначена для закачки в ESP8266, например, в плату NodeMCU
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// настройки прошивки
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define _DEBUG // закомментировать для выключения отладочного режима (плюётся в Serial отладочной информацией на скорости 57600)
#define UPDATE_INTERVAL 10000 // интервал обновления показаний с датчиков, мс
#define CONTROLLER_SSID "TEPLICA" // имя сети Wi-Fi контроллера
#define CONTROLLER_PASSWORD "12345678" // пароль к сети Wi-Fi контроллера
#define SEND_PACKET_DELAY 1000 // задержка после записи данных одного датчика на контроллер, для того, чтобы быть уверенными, что контроллер обработал данные (мс)
#define CLIENT_FLUSH_DELAY 100 // задержка после закрытия клиента, который отсылал очередной пакет данных, мс
#define CONNECT_TIMEOUT 30000 // таймаут коннекта, мс
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// режим глубокого сна (максимальный промежуток между пробуждениями - 35 минут)
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// для поддержки режима глубокого сна надо соединить GPIO16 (на NodeMCU - пин D0) с выводом RST на плате.
// при включенной подержке глубокого сна ESP после отправки данных засыпает, потом - рестартует сначала
//#define USE_DEEP_SLEEP // закомментировать, если не надо использовать режим глубокого сна
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// настройки датчиков
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// датчики прописываются через запятую, одна запись имеет вид
// { тип, индексВконтроллере, параметр1, параметр2, параметр3 }, где
// тип - тип датчика, индексВконтроллере - индекс датчика в контроллере, параметрN - необязательные параметры, зависящие от типа датчика.
// Для датчика DS18B20:
// { sensor_DS18B20, НОМЕР_ПИНА, РАЗРЕШЕНИЕ }, где РАЗРЕШЕНИЕ - одно из значений
// temp9bit, temp10bit, temp11bit, temp12bit
// ПРИМЕРЫ:
// { sensor_DS18B20, 3, D5, temp12bit } // датчик DS18B20 с индексом в контроллере 3, на пине D5, с разрешением 12 бит
// {sensor_BH1750, 3, BH1750Address1 } // датчик освещённости BH1750 с адресом 1 на шине I2C, индекс в контроллере - 3
// {sensor_BH1750, 5, BH1750Address2 } // датчик освещённости BH1750 с адресом 2 на шине I2C, индекс в контроллере - 5

#define SENSORS { sensor_DS18B20, 3, D5, temp12bit },  { sensor_DS18B20, 4, D6, temp12bit }, {sensor_BH1750, 5, BH1750Address2 }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// дальше лазить - неосмотрительно :)
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// ||
// \/
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint32_t lastMillis = 0;
uint32_t updateInterval = UPDATE_INTERVAL;
bool isWireInited = false;
SensorSettings sensors[] = {
  SENSORS
};
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void wifiOff()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
String readFromSensor(SensorSettings* sett)
{
  String result;
  switch(sett->type)
  {
      case sensor_NoSensor:
      break;

     case sensor_BH1750:
     {
        BH1750Support* bh = (BH1750Support*) sett->data;
        long lum = bh->GetCurrentLuminosity();
        result = F("CTSET=LIGHT|DATA|");
        result += sett->targetIndex;
        result += '|';
        result += lum;
     }
     return result;
    
     case sensor_DS18B20:
     case sensor_DS18S20:
     {
        DS18B20Temperature t;
        DS18B20Support* ds = (DS18B20Support*) sett->data;
        DSSensorType st = DS18B20;
        if(sett->type == sensor_DS18S20)
          st = DS18S20;


        result = F("CTSET=STATE|TEMP|");
        result += sett->targetIndex;
        result += '|';
          
        if(ds->readTemperature(&t,st))
        {
          if(t.Negative)
            result += '-';
          uint16_t tt = abs(t.Whole);
          tt *= 100;
          tt += abs(t.Fract);

          result += tt;
          
        }
        else
        {
          // не удалось прочитать
          result += NO_TEMPERATURE_DATA;
          result += "00";
          
        }
     }
     return result;
  } // switch

  return result;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void doSendData()
{
  #ifdef _DEBUG
    Serial.println(F("Start sending data..."));
  #endif 

  const size_t sensorsCount = sizeof(sensors)/sizeof(sensors[0]);

  for(size_t i=0;i<sensorsCount;i++)
  {
      String strToSend = readFromSensor(&(sensors[i]));
      if(!strToSend.length())
      {
        #ifdef _DEBUG
          Serial.println("No data from sensor, continue...");
        #endif        
        continue;
      }

    
      WiFiClient client;
      if (!client.connect(WiFi.gatewayIP(), 1975)) 
      {
        #ifdef _DEBUG
          Serial.println(F("Connection failed!"));
        #endif
        delay(50);
        continue;
      }  
        
      #ifdef _DEBUG
        Serial.print(F("Command to send: "));
        Serial.println(strToSend);
      #endif
    
      client.println(strToSend);
      client.flush();
    
      delay(SEND_PACKET_DELAY + random(100));
    
      while(client.connected() && client.available())
      {
        char ch = client.read();
        #ifdef _DEBUG
          Serial.print(ch);
        #endif
      }
    
      client.stop();
      delay(CLIENT_FLUSH_DELAY + random(100));

  } // for

  #ifdef _DEBUG
    Serial.println(F("All data was sent!"));
  #endif  

}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void sendSensorsData()
{
  #ifdef _DEBUG
    Serial.println(F("Attempt to send sensors data..."));
  #endif

  WiFi.mode(WIFI_STA);
    
  #ifdef _DEBUG
    Serial.print(F("Connecting to "));
    Serial.print(CONTROLLER_SSID);
  #endif
  
  WiFi.begin(CONTROLLER_SSID, CONTROLLER_PASSWORD);

  static uint32_t startConnectTimer = 0;
  startConnectTimer = millis();

  while(1)
  {
    if(millis() - startConnectTimer > CONNECT_TIMEOUT)
    {
        #ifdef _DEBUG
          Serial.println();
          Serial.println(F("Connect timeout!"));
        #endif

       wifiOff();
      return;
    }
    int status = WiFi.status();
    switch(status)
    {
      case WL_IDLE_STATUS:
      break;

      case WL_NO_SSID_AVAIL:
      {
        #ifdef _DEBUG
          Serial.println();
          Serial.println(F("No SSID found!"));
        #endif

        wifiOff();
      }
      return;

      case WL_CONNECTED:
      {
        #ifdef _DEBUG
          Serial.println();
          Serial.println(F("Connected, send data!"));
        #endif

        doSendData();

        wifiOff(); 
            
      }
      return;

      case WL_CONNECT_FAILED:
      {
        #ifdef _DEBUG
          Serial.println();
          Serial.println(F("Password incorrect!"));
        #endif

        wifiOff();    
      }
      return;   
   
    } // switch
    #ifdef _DEBUG
      Serial.print('.');
    #endif    
    delay(500);
  } // while

  
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void clearSensorsData()
{
  isWireInited = false;
  lastMillis = 0;
  
  const size_t sensorsCount = sizeof(sensors)/sizeof(sensors[0]); 

 for(size_t i=0;i<sensorsCount;i++)
  {
    switch(sensors[i].type)
    {
      case sensor_NoSensor:
      break;

      case sensor_BH1750:
      {
        BH1750Support* bh = (BH1750Support*) sensors[i].data;
        delete bh;
      }
      break;
      
      case sensor_DS18B20:
      case sensor_DS18S20:
      {
        DS18B20Support* ds = (DS18B20Support*) sensors[i].data;
        delete ds;
      }
      break;
      
    } // switch

    sensors[i].data = NULL;
  } // for  
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void initSensors()
{
  #ifdef _DEBUG
    Serial.println(F("Init sensors..."));
  #endif

  const size_t sensorsCount = sizeof(sensors)/sizeof(sensors[0]);

  #ifdef _DEBUG
    Serial.print(F("Sensors found: "));
    Serial.println(sensorsCount);
  #endif

  for(size_t i=0;i<sensorsCount;i++)
  {
    switch(sensors[i].type)
    {
      case sensor_NoSensor:
      break;

      case sensor_BH1750:
      {
        #ifdef _DEBUG
          Serial.println(F("BH1750 found, init..."));
        #endif        
        if(!isWireInited)
        {
          isWireInited = true;
          Wire.begin();
        }

        BH1750Support* bh = new BH1750Support();
        sensors[i].data = bh;
        bh->begin((BH1750Address) sensors[i].param1);

        #ifdef _DEBUG
          Serial.print(F("BH1750 inited on address "));
          Serial.print(sensors[i].param1);
          Serial.print(F(" with data="));
          Serial.println((int)sensors[i].data);
        #endif        
      }
      break;
      
      case sensor_DS18B20:
      case sensor_DS18S20:
      {
        #ifdef _DEBUG
          Serial.println(F("DS18*20 found, init..."));
        #endif
        DS18B20Support* ds = new DS18B20Support();
        sensors[i].data = ds;
        ds->begin(sensors[i].param1);
        ds->setResolution((DS18B20Resolution) sensors[i].param2);
        
        #ifdef _DEBUG
          Serial.print(F("DS18*20 inited on pin "));
          Serial.print(sensors[i].param1);
          Serial.print(F(" with data="));
          Serial.println((int)sensors[i].data);
        #endif
      }
      break;
      
    } // switch
  } // for

  #ifdef _DEBUG
    Serial.println(F("Sensors inited."));
  #endif  
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() 
{
  #ifdef _DEBUG
    Serial.begin(57600);
    Serial.println();
    Serial.println(F("Starting..."));
    Serial.setDebugOutput(true);
  #endif

  randomSeed(analogRead(0));
  updateInterval = UPDATE_INTERVAL + random(100);
  
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);

  initSensors();

  #ifdef _DEBUG
    Serial.println(F("Started."));
  #endif



}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() 
{
#ifndef USE_DEEP_SLEEP
  if(millis() - lastMillis > updateInterval)
  {
#endif    

    sendSensorsData();

#ifndef USE_DEEP_SLEEP    
    lastMillis = millis();
    updateInterval = UPDATE_INTERVAL + random(100);
  }
#endif

#ifdef USE_DEEP_SLEEP
  clearSensorsData();
  uint32_t dsPeriod = UPDATE_INTERVAL;
  dsPeriod *= 1000; // в микросекунды
  ESP.deepSleep(dsPeriod);
#endif

  delay(1);
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

