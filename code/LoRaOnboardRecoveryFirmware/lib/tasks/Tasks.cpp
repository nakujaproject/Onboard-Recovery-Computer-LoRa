#include "Sensors.h"
#include <NKJLoRa.h>
#include <FlightStatus.h>
#include <Tasks.h>
#include <Ejection.h>

extern TimerHandle_t sendGPSTimerHandle;

extern TaskHandle_t SendGPSLoRaTaskHandle;
extern TaskHandle_t SendFlightStatusLoRaTaskHandle;

// Pins to start ejection charge
extern uint8_t MAIN_EJECTION_PIN;
extern uint8_t DROGUE_EJECTION_PIN;

extern char DROGUE_MESSAGE[];
extern char MAIN_MESSAGE[];

void SendGPSTimerCallback(TimerHandle_t sendGPSTimerHandle)
{
    BaseType_t checkIfYieldRequired;
    checkIfYieldRequired = xTaskResumeFromISR(SendGPSLoRaTaskHandle);
    portYIELD_FROM_ISR(checkIfYieldRequired);
}
void SendFlightStatusTimerCallback(TimerHandle_t sendFlightStatusTimerHandle)
{
    BaseType_t checkIfYieldRequired;
    checkIfYieldRequired = xTaskResumeFromISR(SendFlightStatusLoRaTaskHandle);
    portYIELD_FROM_ISR(checkIfYieldRequired);
}

void sendGPSLoRaTask(void *parameter)
{
    struct GPSReadings gpsReadings = {0};
    static float latitude = 0;
    static float longitude = 0;

    for (;;)
    {
        vTaskSuspend(NULL);
        gpsReadings = get_gps_readings();

        if (gpsReadings.longitude != 0 && gpsReadings.latitude != 0)
        {
            latitude = gpsReadings.latitude;
            longitude = gpsReadings.longitude;
        }
        else
        {
            gpsReadings.latitude = latitude;
            gpsReadings.longitude = longitude;
        }
        sendLora(gpsReadings);
        LoRa.receive();

        // yield to other task
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void sendStatusLoRaTask(void *parameter)
{
    struct FlightStatus flightStatus = {0};
    for (;;)
    {
        vTaskSuspend(NULL);
        flightStatus = get_flight_status();
        sendLora(flightStatus);
        LoRa.receive();

        // yield to other task
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void OnReceiveLoRaTask(void *param)
{
    for (;;)
    {
        LoRa.receive();
        int packetSize = LoRa.parsePacket();
        char command[2];
        for (int i = 0; i < packetSize; i++)
        {
            command[i] = (char)LoRa.read();
        }
        if (strcmp(command, DROGUE_MESSAGE) == 0)
        {
            ejection(DROGUE_EJECTION_PIN);

            // start sending GPS after ejection
            xTimerStart(sendGPSTimerHandle, portMAX_DELAY);
        }
        else if (strcmp(command, MAIN_MESSAGE) == 0)
        {
            ejection(MAIN_EJECTION_PIN);
        }
        debugln(LoRa.packetRssi());
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
