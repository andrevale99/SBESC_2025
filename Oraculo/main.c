#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "Wifi.h"
#include "NTP.h"

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

    ntp_t *xNTPData = ntp_init("a.st1.ntp.br", UTC_3);

    while (true)
    {
        ntp_request(xNTPData);
        sleep_ms(5000);
    }
}
