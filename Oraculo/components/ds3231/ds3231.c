#include "ds3231.h"
#include <stdio.h>
#include <stdlib.h>
#include "pico_logs.h"

// --- Registradores do DS3231 ---
#define REG_SECONDS 0x00
#define REG_MINUTES 0x01
#define REG_HOURS 0x02
#define REG_DAY 0x03
#define REG_DATE 0x04
#define REG_MONTH 0x05
#define REG_YEAR 0x06
#define REG_CONTROL 0x0E

#define BAUD_RATE_HZ 200000

const static char *TAG = "DS3231";

static uint8_t dec_to_bcd(int val)
{
    return (uint8_t)((val / 10 * 16) + (val % 10));
}

static int bcd_to_dec(uint8_t val)
{
    return (int)((val / 16 * 10) + (val % 16));
}

// --- Funções Públicas ---

void ds3231_init(DS3231 *rtc, i2c_inst_t *i2c_instance, uint sda, uint scl)
{

    PICO_LOGI(TAG, "Iniciando setup de I2C em SDA:%d, SCL:%d.", sda, scl);

    rtc->i2c = i2c_instance;
    rtc->sda_pin = sda;
    rtc->scl_pin = scl;

    i2c_init(rtc->i2c, BAUD_RATE_HZ);

    gpio_set_function(rtc->sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(rtc->scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(rtc->sda_pin);
    gpio_pull_up(rtc->scl_pin);

    PICO_LOGI(TAG, "Tentando comunicação com DS3231 (0x%X).", DS3231_ADDR);

    uint8_t buf[] = {REG_CONTROL, 0x00};
    int ret = i2c_write_blocking(rtc->i2c, DS3231_ADDR, buf, 2, false);

    if (ret == PICO_ERROR_GENERIC)
    {
        PICO_LOGE(TAG, "DS3231 não respondeu em 0x%X. Verifique conexões!", DS3231_ADDR);

        rtc->initialized = false;
    }
    else
    {
        PICO_LOGI(TAG, "Inicializado em I2C%d (SDA:%d, SCL:%d).",
                  (rtc->i2c == i2c0 ? 0 : 1), rtc->sda_pin, rtc->scl_pin);
        rtc->initialized = true;
    }
}

bool ds3231_set_datetime(DS3231 *rtc, const DateTime *dt)
{
    if (!rtc->initialized)
        return false;

    uint8_t buf[8];

    buf[0] = REG_SECONDS;
    buf[1] = dec_to_bcd(dt->second);
    buf[2] = dec_to_bcd(dt->minute);
    buf[3] = dec_to_bcd(dt->hour);
    buf[4] = dec_to_bcd(0);
    buf[5] = dec_to_bcd(dt->day);
    buf[6] = dec_to_bcd(dt->month);
    buf[7] = dec_to_bcd(dt->year % 100);

    int ret = i2c_write_blocking(rtc->i2c, DS3231_ADDR, buf, 8, false);

    if (ret == 8)
    {
        PICO_LOGI(TAG, "Horário de partida configurado: %02d/%02d/20%02d %02d:%02d:%02d.",
                  dt->day, dt->month, dt->year % 100, dt->hour, dt->minute, dt->second);
        return true;
    }
    else
    {
        PICO_LOGI(TAG, "Erro ao escrever a data. Bytes escritos: %d", ret);
        return false;
    }
}

bool ds3231_get_datetime(DS3231 *rtc, DateTime *dt)
{
    if (!rtc->initialized)
        return false;

    uint8_t reg_addr = REG_SECONDS;
    uint8_t data[7];

    int ret_w = i2c_write_blocking(rtc->i2c, DS3231_ADDR, &reg_addr, 1, true);
    if (ret_w == PICO_ERROR_GENERIC)
        return false;

    int ret_r = i2c_read_blocking(rtc->i2c, DS3231_ADDR, data, 7, false);
    if (ret_r != 7)
        return false;

    dt->second = bcd_to_dec(data[0]);
    dt->minute = bcd_to_dec(data[1]);
    dt->hour = bcd_to_dec(data[2] & 0x3F);
    dt->day = bcd_to_dec(data[4]);
    dt->month = bcd_to_dec(data[5] & 0x1F);
    dt->year = bcd_to_dec(data[6]) + 2000;

    return true;
}

bool ds3231_is_date_valid(DS3231 *rtc, time_t *ntp_time, int tol_seconds)
{
    DateTime current_dt;
    if (!ds3231_get_datetime(rtc, &current_dt))
    {
        PICO_LOGE(TAG, "Falha ao ler o horário para validação.");
        return false;
    }

    if (current_dt.year < 2025 || current_dt.month < 1 || current_dt.month > 12 || current_dt.day < 1 || current_dt.day > 31)
    {
        PICO_LOGW(TAG, "Data lida do RTC é inválida: %02d/%02d/%04d.",
                  current_dt.day, current_dt.month, current_dt.year);
        return false;
    }

    time_t rtc_time = ds3231_datetime_to_tm(&current_dt);

    int diff = (int)difftime(*ntp_time, rtc_time);
    if (abs(diff) <= tol_seconds)
    {
        PICO_LOGI(TAG, "Validação OK. Diferença entre RTC e NTP: %d segundos.", diff);
        return true;
    }
    else
    {
        PICO_LOGW(TAG, "Diferença entre RTC e NTP muito grande: %d segundos. Necessita configuração.", diff);
        return false;
    }
}

time_t ds3231_datetime_to_tm(const DateTime *dt)
{
    struct tm t;
    t.tm_year = dt->year - 1900;
    t.tm_mon = dt->month - 1;
    t.tm_mday = dt->day;
    t.tm_hour = dt->hour;
    t.tm_min = dt->minute;
    t.tm_sec = dt->second;
    t.tm_isdst = -1;

    return mktime(&t);
}