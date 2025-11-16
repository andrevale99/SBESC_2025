#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "Wifi.h"
#include "NTP.h"
#include "ble.h"

void vTaskNTP(void *pvArgs)
{
    // a.st1.ntp.br
    ntp_t *xNTPData = ntp_init("200.160.7.186", UTC_MINUS_3);

    ntp_request(xNTPData);

    while (1)
    {
        if (ntP_ntp_response())
            printf("got ntp response: %02d/%02d/%04d %02d:%02d:%02d\n", xNTPData->utc->tm_mday, xNTPData->utc->tm_mon + 1, xNTPData->utc->tm_year + 1900,
                   xNTPData->utc->tm_hour, xNTPData->utc->tm_min, xNTPData->utc->tm_sec);

        vTaskDelay(pdMS_TO_TICKS(500));
    }

    ntp_deinit(xNTPData);
}

void vTaskBLE(void *pvArgs)
{
    uint16_t value = 0;
    while (1)
    {
        if (ble_get_uint16_data(&value))
            printf("[BLE] CHEGOU O DADO: %d\n", value);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main()
{

    stdio_init_all();

    switch (Wifi_station_init("LASEM", "besourosuco"))
    {
    case WIFI_SUCCESS:
        printf("[WIFI] CONECTADO AO WIFI\n");
        break;

    case WIFI_ARCH_INIT_ERROR:
    case WIFI_CONNECTION_FAILED:
        printf("[WIFI] ERRO DJABO\n");
        break;

    default:
        break;
    }

    ble_init();

    xTaskCreate(vTaskNTP, "NTP_Task", 256, NULL, 1, NULL);
    xTaskCreate(vTaskBLE, "BLE_TASK", 256, NULL, 1, NULL);

    vTaskStartScheduler();
}
