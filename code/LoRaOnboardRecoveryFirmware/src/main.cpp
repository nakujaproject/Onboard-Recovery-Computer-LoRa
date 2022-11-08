#include "../lib/definitions/defs.h"
#include <Sensors.h>
#include <Ejection.h>
#include <NKJLoRa.h>
#include <Tasks.h>
#include <FlightStatus.h>
#include <Interrupts.h>

const BaseType_t pro_cpu = 0;
const BaseType_t app_cpu = 1;

TaskHandle_t SendGPSLoRaTaskHandle = NULL;
TaskHandle_t SendFlightStatusLoRaTaskHandle = NULL;
TaskHandle_t OnReceiveLoRaTaskHandle = NULL;

TimerHandle_t sendFlightStatusTimerHandle = NULL;
TimerHandle_t sendGPSTimerHandle = NULL;

// TODO: optimize so that LCM is very large so that they never collide
const int GPS_TIMER_PERIOD = 300;
const int FLIGHT_STATUS_TIMER_PERIOD = 200;

void setup()
{
  initHeltecLoRa();

  setEjectionPinModes();

  setInterruptPins();

  init_gps();

  // Create tasks on core 1
  xTaskCreatePinnedToCore(OnReceiveLoRaTask, "OnReceiveLoRaTask", 1500, NULL, 1, &OnReceiveLoRaTaskHandle, app_cpu);

  // Create tasks on core 0
  xTaskCreatePinnedToCore(sendGPSLoRaTask, "LoRaGPSTask", 2500, NULL, 1, &SendGPSLoRaTaskHandle, pro_cpu);
  xTaskCreatePinnedToCore(sendStatusLoRaTask, "SendFlightStatusTask", 2500, NULL, 1, &SendFlightStatusLoRaTaskHandle, pro_cpu);

  // create periodic timer to send flight status data to Ground Station
  sendFlightStatusTimerHandle = xTimerCreate("FlightStatusTimer", FLIGHT_STATUS_TIMER_PERIOD / portTICK_PERIOD_MS, pdFALSE, (void *)0, SendFlightStatusTimerCallback);

  // create periodic timer to send GPS data to Ground Station
  sendGPSTimerHandle = xTimerCreate("GPSTimer", GPS_TIMER_PERIOD / portTICK_PERIOD_MS, pdTRUE, (void *)1, SendGPSTimerCallback);

  // start sending Flight Status Data periodically
  xTimerStart(sendFlightStatusTimerHandle, portMAX_DELAY);

  vTaskDelete(NULL);
}

void loop()
{
  vTaskDelete(NULL);
}