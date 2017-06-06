#ifndef _HTTP_IFACES_H
#define _HTTP_IFACES_H
//--------------------------------------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <WString.h>
//--------------------------------------------------------------------------------------------------------------------------------
/*
  Принцип работы провайдера HTTP-запросов:
  
  1. Сначала у него запрашивают, может ли он отработать запрос, вызовом CanMakeQuery;
  2. Если запрос может быть поставлен в очередь немедленно, провайдер возвращает true;
  3. Вызывающая сторона может вызвать MakeQuery, передав туда указатель на интерфейс HTTPRequestHandler;
  4. После передачи своего интерфейса вовне вызывающая сторона ОБЯЗАНА ждать вызова OnHTTPResult, сигнализирующего об окончании обработки запроса;
  5. Провайдер перед установкой соединения запрашивает OnAskForHost для получения адреса хоста, к которому надо законнектиться (порт - всегда 80);
  6. Если соединение не удалось, то вызывается OnHTTPResult с одним из кодов ошибок;
  7. При успешной установке соединения провайдер вызывает OnAskForData для получения данных к отправке;
  8. После отправки данных провайдер по приходу ответа от сервера вызывает OnAnswerLineReceived до тех пор, пока вызвавшая сторона согласна принимать данные;
  9. После завершения приёма данных соединение закрывается и вызывается OnHTTPResult с кодом HTTP_REQUEST_COMPLETED;
  10. Провайдер переходит в режим ожидания запроса на новую команду получения данных по HTTP.

*/
//--------------------------------------------------------------------------------------------------------------------------------
// статусные коды запроса по HTTP
//--------------------------------------------------------------------------------------------------------------------------------
#define HTTP_REQUEST_COMPLETED 999 // закончили обработку HTTP-запроса
#define ERROR_MODEM_NOT_ANSWERING 1000 // модем не отвечает, не удалось выполнить команду
#define ERROR_CANT_ESTABLISH_CONNECTION 1001 // не удалось установить соединение
#define ERROR_HTTP_REQUEST_CANCELLED 1002 // запрос отменили, неожиданным вызовом MakeQuery
#define ERROR_HTTP_REQUEST_FAILED 1003 // не удалось сделать запрос
//--------------------------------------------------------------------------------------------------------------------------------
// методы HTTP
//--------------------------------------------------------------------------------------------------------------------------------
// интерфейс перехватчика работы с HTTP-запросами
//--------------------------------------------------------------------------------------------------------------------------------
struct HTTPRequestHandler
{
  virtual void OnAskForHost(String& host) = 0; // вызывается для запроса имени хоста
  virtual void OnAskForData(String* data) = 0; // вызывается для запроса данных, которые надо отправить HTTP-запросом
  
  virtual void OnAnswerLineReceived(String& line, bool& enough) = 0; // вызывается по приходу строки ответа от сервера, вызываемая сторона должна сама определить, когда достаточно данных.
  
  virtual void OnHTTPResult(uint16_t statusCode) = 0; // вызывается по завершению HTTP-запроса и получению ответа от сервера
};
//--------------------------------------------------------------------------------------------------------------------------------
// интерфейс для доступа к модулю как к провайдеру HTTP-запросов
//--------------------------------------------------------------------------------------------------------------------------------
struct HTTPQueryProvider
{
  virtual bool CanMakeQuery() = 0; // тестирует, может ли модуль сейчас сделать запрос
  virtual void MakeQuery(HTTPRequestHandler* handler) = 0; // начинаем запрос по HTTP
};
//--------------------------------------------------------------------------------------------------------------------------------
#endif