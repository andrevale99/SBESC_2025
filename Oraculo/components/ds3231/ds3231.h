#ifndef DS3231_H
#define DS3231_H

#include "hardware/i2c.h"
#include "pico/stdlib.h"

// #include "time.h"
#include "sys/time.h"

// Endereço I2C do DS3231
#define DS3231_ADDR 0x68

// Estrutura para Data/Hora (formato decimal)
typedef struct
{
    int year; // Ex: 2025
    int month;
    int day;
    int hour; // 24h format
    int minute;
    int second;
} DateTime;

// Estrutura do Módulo RTC
typedef struct
{
    bool initialized;
    i2c_inst_t *i2c;
    uint sda_pin;
    uint scl_pin;
} DS3231;

/**
 * @brief Inicializa o I2C e o módulo DS3231.
 * @param rtc Ponteiro para a estrutura DS3231.
 * @param i2c_instance Instância I2C (i2c0 ou i2c1).
 * @param sda Pino SDA.
 * @param scl Pino SCL.
 */
void ds3231_init(DS3231 *rtc, i2c_inst_t *i2c_instance, uint sda, uint scl);

/**
 * @brief Configura a data e hora no DS3231.
 * @param rtc Ponteiro para a estrutura DS3231.
 * @param dt Ponteiro para a estrutura DateTime contendo o novo horário.
 * @return true se a escrita foi bem-sucedida, false caso contrário.
 */
bool ds3231_set_datetime(DS3231 *rtc, const DateTime *dt);

/**
 * @brief Lê a data e hora atual do DS3231.
 * @param rtc Ponteiro para a estrutura DS3231.
 * @param dt Ponteiro para a estrutura DateTime onde o horário lido será armazenado.
 * @return true se a leitura foi bem-sucedida, false caso contrário.
 */
bool ds3231_get_datetime(DS3231 *rtc, DateTime *dt);

/**
 * @brief Verifica se o ano no RTC é plausível (ex: maior que 2024).
 * @param rtc Ponteiro para a estrutura DS3231.
 * @return true se a data for considerada válida, false caso contrário.
 */
bool ds3231_is_date_valid(DS3231 *rtc, time_t *ntp_time, int tol_seconds);

time_t ds3231_datetime_to_tm(const DateTime *dt);

#endif // DS3231_H