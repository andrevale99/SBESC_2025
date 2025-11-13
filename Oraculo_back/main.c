#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "Wifi.h"
#include "Mqtt.h"
#include "NTP.h"


QueueHandle_t handleQueue_to_Mqtt = NULL;

void vTaskNTP(void *pvArgs)
{
    NTP_T *state = ntp_init();
    if (!state)
        return;

    hard_assert(async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &state->request_worker, 0)); // make the first request
    while (true)
    {

        cyw43_arch_poll();

        cyw43_arch_wait_for_work_until(at_the_end_of_time);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    free(state);
}

static void vTaskMqtt(void *pvArgs)
{
    uint16_t temp = 0;
    MQTT_CLIENT_DATA_T state = *(MQTT_CLIENT_DATA_T *)pvArgs;
    while (1)
    {
        if (xQueueReceive(handleQueue_to_Mqtt, &temp, pdMS_TO_TICKS(20)) == pdTRUE)
            printf("TEMP: %d\n", temp);

        if (!state.connect_done || mqtt_client_is_connected(state.mqtt_client_inst))
        {
            cyw43_arch_poll();
            // cyw43_arch_wait_for_work_until(make_timeout_time_ms(10000));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main()
{

    MQTT_CLIENT_DATA_T xMqttData;

    stdio_init_all();

    Wifi_station_init("LASEUM", "besourosuco");

    Mqtt_init(&xMqttData);

    while (true)
    {
    }
}
