#include "wifi.h"
#include "pico_logs.h"

const static char *TAG = "WIFI";

extern cyw43_t cyw43_state;

int8_t wifi_station_init(const char *ssid, const char *password)
{
    PICO_LOGI(TAG, "Iniciando o módulo Wi-Fi...");
    if (cyw43_arch_init())
    {
        PICO_LOGE(TAG, "Falha ao inicializar o CYW43.");
        return WIFI_ARCH_INIT_ERROR;
    }

    cyw43_arch_enable_sta_mode();

    PICO_LOGI(TAG, "Tentando conectar à rede Wi-Fi: %s...", ssid);

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        PICO_LOGE(TAG, "Falha ao conectar à rede Wi-Fi: %s", ssid);
        return WIFI_CONNECTION_FAILED;
    }

    PICO_LOGI(TAG, "Conectado à rede Wi-Fi: %s", ssid);

    return WIFI_SUCCESS;
}

bool wifi_is_connected()
{
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}