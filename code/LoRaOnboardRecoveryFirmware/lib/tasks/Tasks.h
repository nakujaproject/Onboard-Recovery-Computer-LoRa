#ifndef TASKS_H
#define TASKS_H
#include "../definitions/defs.h"

void sendGPSLoRaTask(void *parameter);
void sendStatusLoRaTask(void *parameter);
void OnReceiveLoRaTask(void *param);
void resumeGPSTask();
void SendGPSTimerCallback(TimerHandle_t sendGPSTimerHandle);
void SendFlightStatusTimerCallback(TimerHandle_t sendFlightStatusTimerHandle);

#endif // TASKS_H