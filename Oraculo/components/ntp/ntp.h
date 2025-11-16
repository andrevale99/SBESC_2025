#ifndef NTP_H
#define NTP_H

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "pico_logs.h"

#define NTP_MSG_LEN 48
#define NTP_PORT 123
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970
#define NTP_RESEND_TIME_MS (10 * 1000)

typedef struct ntp_t_
{
    ip_addr_t ntp_server_address;
    struct udp_pcb *ntp_pcb;
    async_at_time_worker_t request_worker;
    async_at_time_worker_t resend_worker;

    struct tm *utc;

    int8_t resultNTP;
} ntp_t;

enum UTC_OFFSET
{
    UTC_PLUS_12 = (3600 * 12),
    UTC_PLUS_11 = (3600 * 11),
    UTC_PLUS_10 = (3600 * 10),
    UTC_PLUS_9 = (3600 * 9),
    UTC_PLUS_8 = (3600 * 8),
    UTC_PLUS_7 = (3600 * 7),
    UTC_PLUS_6 = (3600 * 6),
    UTC_PLUS_5 = (3600 * 5),
    UTC_PLUS_4 = (3600 * 4),
    UTC_PLUS_3 = (3600 * 3),
    UTC_PLUS_2 = (3600 * 2),
    UTC_PLUS_1 = (3600 * 1),
    UTC_0 = (3600 * 0),
    UTC_MINUS_1 = (3600 * -1),
    UTC_MINUS_2 = (3600 * -2),
    UTC_MINUS_3 = (3600 * -3),
    UTC_MINUS_4 = (3600 * -4),
    UTC_MINUS_5 = (3600 * -5),
    UTC_MINUS_6 = (3600 * -6),
    UTC_MINUS_7 = (3600 * -7),
    UTC_MINUS_8 = (3600 * -8),
    UTC_MINUS_9 = (3600 * -9),
    UTC_MINUS_10 = (3600 * -10),
    UTC_MINUS_11 = (3600 * -11),
    UTC_MINUS_12 = (3600 * -12),
};

/// @brief Inicializacao do objeto ntp_t
/// @param[in] ntp_url_ URL do servidor NTP
/// @param[in] UTC_offset_seconds Offset do fuso horario da regiao
/// @return Objeto ntp_t
ntp_t *ntp_init(const char ntp_url_[], const int UTC_offset_seconds);

/// @brief Desaloca o estrutura
/// @param ntp Estrutura
void ntp_deinit(ntp_t *ntp);

/// @brief Funcao para realizar a chamada para o servidor NTP
/// @param[in] ntp Estrutura de dados ntp_t
void ntp_request(ntp_t *ntp);

/// @brief Funcao para verificar se houve resposta do
/// do servidor NTP
/// @param
/// @return true se houve, false caso contrário
bool ntP_ntp_response(void);

/// @brief Funcao para configurar o UTC do sistema
/// @param[in] UTC_offset_seconds Novo UTC da regiao
void ntp_set_utc_offset(const int UTC_offset_seconds);

/// @brief Funcao para obter o tempo unix do NTP
/// @param[in] state Estrutura de dados ntp_t
/// @param[out] unix_time Ponteiro para armazenar o tempo unix
void ntp_get_unix_time(ntp_t *state, time_t *unix_time);

#endif