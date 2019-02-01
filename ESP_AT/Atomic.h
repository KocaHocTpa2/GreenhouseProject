#pragma once
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//#define _DEBUG // закомментировать для выключения отладочного режима!!!
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "TinyVector.h"
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define MAX_CLIENTS 4
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 1337
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class CriticalSection
{
  public:
    CriticalSection();
    ~CriticalSection();

   static bool Triggered();
};
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  char* data;
  size_t dataLength;
  
} EventData;
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
typedef Vector<EventData> MessagesList;
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class EventsList
{
  public:
    EventsList();
    void update();
    void begin();

    void clear();
    void raise(const char* evt);
    void raise(const char* data, size_t dataLength);

  private:
    MessagesList messages;
};
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
extern EventsList Events;
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
  uint8_t linkID;
  size_t dataLength;
  
} CipsendData;
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class CipsendHandler
{
  public:

    CipsendHandler();
   ~CipsendHandler();

   void clear();
   void update();

   void add(size_t dataLength,uint8_t linkID);

  private:

    Vector<CipsendData> data;
    uint32_t segmentID;
  
};
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
extern CipsendHandler Cipsend;
extern WiFiClient Clients[MAX_CLIENTS]; // наши клиенты
extern uint8_t ClientConnectStatus[MAX_CLIENTS]; // статусы соединения клиентов
extern WiFiUDP ntpUDP;
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define CRITICAL_SECTION CriticalSection __thisCriticalSection;
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <typename T> inline Stream& operator << (Stream &s, T n) { s.print(n); return s; }
#define ENDLINE "\r\n"
//----------------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef _DEBUG
  #define DBG(s) Serial << (s)
  #define DBGLN(s) Serial << (s) << ENDLINE
#else
  #define DBG(s) (void) 0
  #define DBGLN(s) (void) 0
#endif
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

