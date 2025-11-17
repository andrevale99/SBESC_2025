#include "pico/stdlib.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "ds3231.h"
#include "pico_logs.h"
#include "wifi.h"
#include "ntp.h"
#include "ble.h"

#include "mqtt_lib.h"
#include "st7735.h"
#include "hw.h"
#include "ssd1306.h"

//====================================
//  DEFINES
//====================================

#define I2C_PORT i2c0
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1

#define OLED_I2C_SDA 14
#define OLED_I2C_SCL 15

#define DELAY_TIME_MS 500
#define DELAY_TIME_CONF_MS 500

// #define WIFI_SSID "LARS-301-2.4GHz"
// #define WIFI_PASSWORD "LARS@ROBOTICA"

#define WIFI_SSID "brisa-2532295"
#define WIFI_PASSWORD "zlgy1ssc"

#define MQTT_BROKER "10.7.224.54"
#define MQTT_BROKER_PORT 8884
#define MQTT_CLIENT_ID "Pico_W_RTC_Client"
#define MQTT_TOPIC_PUB "pico/rtc/time"
#define MQTT_TOPIC_SUB "pico/config"

// Macro para colocar a escrita no inicoio do OLED
#define RETURN_HOME_SSD(_ssd) memset(_ssd, 0, ssd1306_buffer_length)

#define MAX_LEN_BUFFER 256

//====================================
//  VARIAVEIS
//====================================

// Preparar área de renderização para o display
// (ssd1306_width pixels por ssd1306_n_pages páginas)
static struct render_area frame_area = {
    start_column : 0,
    end_column : ssd1306_width - 1,
    start_page : 0,
    end_page : ssd1306_n_pages - 1
};

char buffer[MAX_LEN_BUFFER];
uint8_t ssd[ssd1306_buffer_length];

volatile bool mqtt_is_connected = false;

const static char *TAG = "MAIN";

pico_mqtt_client_t mqtt_client;

//====================================
//  FUNCOES
//====================================

void on_mqtt_message(const char *topic, const uint8_t *payload, uint16_t len);
void on_mqtt_connection(mqtt_connection_status_t status);

void update_oled(void);

//====================================
//  MAIN
//====================================

int main()
{

    PICO_LOGI(TAG, "Iniciando o sistema...");
    stdio_init_all();
    sleep_ms(DELAY_TIME_CONF_MS);

    PICO_LOGI(TAG, "Conectando ao Wi-Fi...");
    wifi_station_init(WIFI_SSID, WIFI_PASSWORD);
    sleep_ms(DELAY_TIME_CONF_MS);

    PICO_LOGI(TAG, "Inicializando BLE...");
    ble_init();
    sleep_ms(DELAY_TIME_CONF_MS);

    PICO_LOGI(TAG, "Configurando cliente MQTT...");

    mqtt_client.credentials.client_id = "ORAC";
    mqtt_client.credentials.user = NULL;
    mqtt_client.credentials.pass = NULL;
    mqtt_client.keep_alive_sec = 60;
    mqtt_client.message_cb = on_mqtt_message;
    mqtt_client.connection_cb = on_mqtt_connection;
    mqtt_client.lwt.topic = "pico/status";
    mqtt_client.lwt.msg = "offline";
    mqtt_client.lwt.msg_len = strlen((const char *)mqtt_client.lwt.msg);
    mqtt_client.lwt.qos = 1;
    mqtt_client.lwt.retain = 0;

    PICO_LOGI(TAG, "Iniciando conexão MQTT...");

    if (pico_mqtt_init(&mqtt_client, MQTT_BROKER, MQTT_BROKER_PORT))
    {
        PICO_LOGI(TAG, "Tentativa de conexão MQTT iniciada...");
    }
    else
    {
        PICO_LOGE(TAG, "Falha ao iniciar cliente MQTT.");
    }
    sleep_ms(DELAY_TIME_CONF_MS);

    PICO_LOGI(TAG, "Inicializando módulo RTC DS3231...");
    DS3231 rtc_module;
    ds3231_init(&rtc_module, I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN);
    sleep_ms(DELAY_TIME_CONF_MS);

    PICO_LOGI(TAG, "Obtendo data e hora do NTP...");
    ntp_t *ntp_client = ntp_init("a.st1.ntp.br", UTC_MINUS_3);
    ntp_request(ntp_client);
    sleep_ms(DELAY_TIME_CONF_MS);

    static time_t ntp_time = {0};
    if (ntp_response())
    {
        struct tm *utc_time = ntp_client->utc;

        DateTime dt = {
            .year = utc_time->tm_year + 1900,
            .month = utc_time->tm_mon + 1,
            .day = utc_time->tm_mday,
            .hour = utc_time->tm_hour,
            .minute = utc_time->tm_min,
            .second = utc_time->tm_sec};

        PICO_LOGI(TAG, "Data e hora obtidas do NTP: %02d/%02d/%04d %02d:%02d:%02d",
                  dt.day, dt.month, dt.year,
                  dt.hour, dt.minute, dt.second);
        ntp_time = ds3231_datetime_to_tm(&dt);
        if (!ds3231_is_date_valid(&rtc_module, &ntp_time, 60))
        {
            PICO_LOGW(TAG, "Data inválida no RTC. Configurando data inicial...");
            ds3231_set_datetime(&rtc_module, &dt);
            PICO_LOGI(TAG, "Data inicial configurada.");
        }
    }
    else
    {
        PICO_LOGE(TAG, "Falha ao obter data e hora do NTP.");
    }

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(OLED_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(OLED_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_I2C_SDA);
    gpio_pull_up(OLED_I2C_SCL);

    // Inicia o display oled
    ssd1306_init();
    calculate_render_area_buffer_length(&frame_area);

    // coloca no home do display
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    uint16_t u16BLEData = 0;
    struct tm *temporario = localtime(&ntp_time);
    while (true)
    {

        sleep_ms(2 * DELAY_TIME_MS);
    }

    return 0;
}

//====================================
//  FUNCOES
//====================================

void on_mqtt_message(const char *topic, const uint8_t *payload, uint16_t len)
{
    PICO_LOGI(TAG, "MQTT RECEBIDO: Tópico: %s", topic);
    char payload_str[len + 1];
    memcpy(payload_str, payload, len);
    payload_str[len] = '\0';
    PICO_LOGI(TAG, "MQTT RECEBIDO: Payload: %s", payload_str);

    if (strcmp(topic, MQTT_TOPIC_SUB) == 0)
    {
        PICO_LOGI(TAG, "Processando comando de configuração...");
    }
}

void on_mqtt_connection(mqtt_connection_status_t status)
{
    if (status == MQTT_CONNECT_ACCEPTED)
    {
        mqtt_is_connected = true;
        PICO_LOGI(TAG, "MQTT Conectado! Tentando se inscrever...");

        if (pico_mqtt_subscribe(&mqtt_client, MQTT_TOPIC_SUB, 1))
        {
            PICO_LOGI(TAG, "Inscrição no tópico '%s' enviada.", MQTT_TOPIC_SUB);
        }
        else
        {
            PICO_LOGE(TAG, "Falha ao enviar requisição de inscrição.");
        }

        if (pico_mqtt_publish(&mqtt_client,
                              "pico/status",
                              "online",
                              strlen("online"),
                              1,
                              0))
        {
            PICO_LOGI(TAG, "Publicado status online.");
        }
        else
        {
            PICO_LOGE(TAG, "Falha ao publicar status online.");
        }
    }
    else
    {
        mqtt_is_connected = false;
        PICO_LOGW(TAG, "MQTT Desconectado ou falha na conexão: %d", status);
    }
}

void update_oled(void)
{
    strftime(buffer, MAX_LEN_BUFFER, "Time %T", temporario);
    ssd1306_draw_string(ssd, 5, 5, buffer);

    sprintf(buffer, "Wifi: %s",
            (wifi_is_connected() ? "OK" : "NO WIFI"));
    ssd1306_draw_string(ssd, 5, 20, buffer);

    sprintf(buffer, "Mqtt: %s",
            (pico_mqtt_is_connected(&mqtt_client) ? "OK" : "NO"));
    ssd1306_draw_string(ssd, 5, 35, buffer);

    sprintf(buffer, "BLE: %s",
            (ble_get_situation() ? "OK" : "NO"));
    ssd1306_draw_string(ssd, 5, 50, buffer);

    render_on_display(ssd, &frame_area);
}

// void tft_example_init(void)
// {
//     tft_spi_init(); // Inicializa SPI e pinos
// #ifdef TFT_ENABLE_BLACK
//     TFT_BlackTab_Initialize();
// #elif defined(TFT_ENABLE_GREEN)
//     TFT_GreenTab_Initialize();
// #elif defined(TFT_ENABLE_RED)
//     TFT_RedTab_Initialize();
// #elif defined(TFT_ENABLE_GENERIC)
//     TFT_ST7735B_Initialize();
// #endif
//     sleep_ms(100);
//     PICO_LOGI(TAG, "TFT: preenchendo a tela de vermelho...");
//     fillScreen(ST7735_BLACK); // Limpa a tela

//     setTextWrap(true);

//     setRotation(3);
//     fillScreen(ST7735_BLACK); // Limpa a tela

//     const char *txt = " hello";

//     drawText(0, 0, txt, ST7735_WHITE, ST7735_BLACK, 2);
//     sleep_ms(1000);

//     drawText(100, 0, txt, ST7735_WHITE, ST7735_BLACK, 2);
//     sleep_ms(5000);

//     drawText(200, 0, txt, ST7735_WHITE, ST7735_BLACK, 2);
//     sleep_ms(100);

//     sleep_ms(1000);
// }