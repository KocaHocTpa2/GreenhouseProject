#ifndef _COMMON_H
#define _COMMON_H
//----------------------------------------------------------------------------------------------------------------
#include "CoreArray.h"
//----------------------------------------------------------------------------------------------------------------
//#define _DEBUG // раскомментировать для отладочного режима (плюётся в Serial, если RS-485 сидит тоже на Serial - то RS-485 в этом режиме работать не будет)
//----------------------------------------------------------------------------------------------------------------
#define DISPLAY_WINDOWS_PAGE // отображать или нет страницы управления окнами
#define SUPPORTED_WINDOWS 4 // кол-во поддерживаемых окон, максимум 16
#define DISPLAY_WATERING_PAGE // отображать или нет страницы управления каналами полива
#define WATER_RELAYS_COUNT 4 // кол-во поддерживаемых каналов полива, максимум 16
#define DISPLAY_LIGHT_PAGE // отображать или нет страницу управления досветкой
#define RESET_SENSORS_DATA_DELAY 60000 // через сколько мс сбрасывать показания датчиков, если долго ничего не приходило по RS-485
#define FRACT_DELIMITER ',' // разделитель целой и дробной части числа
//----------------------------------------------------------------------------------------------------------------
// настройки RS-485
//----------------------------------------------------------------------------------------------------------------
#define RS485_SPEED 57600 // скорость работы по RS-485
#define RS485_DE_PIN 4 // номер пина для управления переключением приём/передача RS485
#define RS485_SERIAL Serial // какой аппаратный UART использовать для RS-485
//----------------------------------------------------------------------------------------------------------------
// Настройки информационного диода. Диод мигает после того, как получено подтверждение от контроллера о выполнении команды.
//----------------------------------------------------------------------------------------------------------------
#define USE_INFO_DIODE // закомментировать, если не надо использовать информационный диод
#define INFO_DIODE_PIN 7 // номер пина информационного диода
#define INFO_DIODE_ON_LEVEL HIGH // уровень включения диода
//----------------------------------------------------------------------------------------------------------------
//#define USE_NEXTION_HARDWARE_UART // закомментировать, если надо использовать программный UART (дополнительные 2 пина) вместо аппаратного (аппаратный остаётся свободен)
#define NEXTION_SERIAL Serial3 // какой Serial используем для Nextion (настройка актуальна только при включенной USE_NEXTION_HARDWARE_UART)

// следующие две настройки актуальны только при выключенной USE_NEXTION_HARDWARE_UART
#define NEXTION_SOFTWARE_UART_RX_PIN 10 // пин RX для SoftwareSerial при общении с Nextion через программный UART
#define NEXTION_SOFTWARE_UART_TX_PIN 11 // пин ТX для SoftwareSerial при общении с Nextion через программный UART

#define NEXTION_SLEEP_DELAY 60 // через сколько секунд переходить в сон, если ничего не сделано на экране
#define NEXTION_WAIT_TIMER 10000 // интервал таймера переключения на экран ожидания, мс
#define NEXTION_ROTATION_INTERVAL 7000 // через сколько мс менять на экране ожидания показания с датчиков
//----------------------------------------------------------------------------------------------------------------
// настройки заголовков датчиков, строки через запятую. По порядку совпадает с датчиками, показания с которых 
// отсылаются по сети главным контроллером и прописаны в настройке REMOTE_DISPLAY_SENSORS прошивки контроллера
//----------------------------------------------------------------------------------------------------------------
#define SENSORS_CAPTIONS "Т внутри", "Влажность DHT", "Свет BH1750"




















//----------------------------------------------------------------------------------------------------------------
// далее - служебная информация
//----------------------------------------------------------------------------------------------------------------
#define UNUSED(expr) do { (void)(expr); } while (0)
//----------------------------------------------------------------------------------------------------------------
//Структура передаваемая мастеру и обратно
//----------------------------------------------------------------------------------------------------------------
enum {RS485FromMaster = 1, RS485FromSlave = 2};
enum 
{
  RS485ControllerStatePacket = 1, 
  RS485SensorDataPacket = 2, 
  RS485WindowsPositionPacket = 3,
  RS485RequestCommandsPacket = 4,
  RS485CommandsToExecuteReceipt = 5,
  RS485SensorDataForRemoteDisplay = 6,
  RS485SettingsForRemoteDisplay = 7,
};
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte header1;
  byte header2;

  byte direction; // направление: 1 - от меги, 2 - от слейва
  byte type; // тип: 1 - пакет исполнительного модуля, 2 - пакет модуля с датчиками

  byte data[23]; // N байт данных, для исполнительного модуля в этих данных содержится состояние контроллера
  // для модуля с датчиками: первый байт - тип датчика, 2 байт - его индекс в системе. В обратку модуль с датчиками должен заполнить показания (4 байта следом за индексом 
  // датчика в системе и отправить пакет назад, выставив direction и type.

  byte tail1;
  byte tail2;
  byte crc8;
  
} RS485Packet; // пакет, гоняющийся по RS-485 туда/сюда (30 байт)
//----------------------------------------------------------------------------------------------------------------
enum // команды с модуля управления
{
  emCommandNone = 0,          // нет команды
  emCommandOpenWindows,       // открыть все окна
  emCommandCloseWindows,      // закрыть все осна
  emCommandOpenWindow,        // открыть определённое окно
  emCommandCloseWindow,       // закрыть определённое окно
  emCommandWaterOn,           // включить волив
  emCommandWaterOff,          // выключить полив
  emCommandWaterChannelOn,    // включить канал полива
  emCommandWaterChannelOff,   // выключить канал полива
  emCommandLightOn,           // включить досветку
  emCommandLigntOff,          // выключить досветку
  emCommandPinOn,             // включить пин
  emCommandPinOff,            // выключить пин
  emCommandAutoMode,          // перейти в автоматический режим работы
  emCommandWindowsAutoMode,   // автоматический режим работы окон
  emCommandWindowsManualMode,   // ручной режим работы окон
  emCommandWaterAutoMode,   // автоматический режим работы полива
  emCommandWaterManualMode,   // ручной режим работы полива
  emCommandLightAutoMode,   // автоматический режим работы досветки
  emCommandLightManualMode,   // ручной режим работы досветки
  emCommandSetOpenTemp, // установить температуру открытия
  emCommandSetCloseTemp, // установить температуру закрытия
  emCommandSetMotorsInterval, // установить интервал работы моторов
};
//----------------------------------------------------------------------------------------------------------------
typedef struct
{ 
  byte whichCommand;
  byte param1;
  byte param2;
  
} CommandToExecute;
//----------------------------------------------------------------------------------------------------------------
typedef Vector<CommandToExecute> CommandsPool; // пул команд от модуля к контроллеру
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  byte moduleID; // ID модуля, от которого пришла команда
  CommandToExecute commands[7]; // 21 байт
  byte reserved; // добитие до 23 байт
  
} CommandsToExecutePacket; // пакет с командами на выполнение
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  uint32_t WindowsState; // состояние каналов окон, 4 байта = 32 бита = 16 окон)
  uint16_t WaterChannelsState; // состояние каналов полива, 2 байта, (16 каналов)
  uint8_t LightChannelsState; // состояние каналов досветки, 1 байт (8 каналов)
  uint8_t PinsState[16]; // состояние пинов, 16 байт, 128 пинов
  
} ControllerState; // состояние контроллера
//----------------------------------------------------------------------------------------------------------------
typedef enum
{
  StateUnknown = 0, // неизвестное состояние
  StateTemperature = 1, // есть температурные датчики
 // StateDummyState = 2, // запас :)
  StateLuminosity = 4, // есть датчики освещенности
  StateHumidity = 8, // есть датчики влажности
  StateWaterFlowInstant = 16, // есть датчик мгновенного расхода воды
  StateWaterFlowIncremental = 32, // есть датчик постоянного расхода воды
  StateSoilMoisture = 64, // есть датчик влажности почвы
  StatePH = 128 // есть датчики pH

} ModuleStates; // вид состояния
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  uint8_t type;
  bool hasData;
  uint8_t data[4];
  
} SensorDisplayData;
//----------------------------------------------------------------------------------------------------------------
typedef Vector<SensorDisplayData> SensorsList;
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  uint8_t type; // тип датчика
  uint8_t data[4]; // данные датчика
  
} RemoteDisplaySensorData; // данные одного датчика для выносного дисплея
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  uint8_t firstOrLastPacket; // признак начала/окончания всех данных (0 - нет данных, 1 - начало данных, 2 - окончание данных)
  uint8_t sensorsInPacket; // кол-во датчиков в пакете
  uint8_t hasDataFlags; // флаги, с каких датчиков в пакете есть показания
  RemoteDisplaySensorData data[4]; // данные датчиков
  
} RemoteDisplaySensorsPacket; // пакет показаний датчиков для выносного дисплея
//----------------------------------------------------------------------------------------------------------------
#define REMOTE_DISPLAY_FIRST_SENSORS_PACKET 1
#define REMOTE_DISPLAY_LAST_SENSORS_PACKET 2
//----------------------------------------------------------------------------------------------------------------
typedef struct
{
  uint8_t openTemp; // температура открытия
  uint8_t closeTemp; // температура закрытия
  uint16_t interval; // интервал работы моторов
  uint8_t isWindowsOpen; // открыты ли окна
  uint8_t isWindowAutoMode; // автоматический режим работы окон?
  uint16_t windowsStatus; // статус окон по каналам (1 - открыто, 0 - закрыто)
  uint8_t isWaterOn; // включен ли полив?
  uint8_t isWaterAutoMode; // автоматический режим работы полива?
  uint8_t isLightOn; // включена ли досветка
  uint8_t isLightAutoMode; // автоматический режим работы досветки?
  uint8_t reserved[11]; // добитие до 23 байт
  
} RemoteDisplaySettingsPacket; // данные настроек для выносного дисплея
//----------------------------------------------------------------------------------------------------------------
#endif
