#ifndef NTP_H
#define NTP_H

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

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
} ntp_t;

/// @brief Inicializacao do objeto ntp_t
/// @param[in] ntp_url_ URL do servidor NTP
/// @param[in] UTC_offset_seconds Offset do fuso horario da regiao
/// @return Objeto ntp_t 
ntp_t *ntp_init(const char ntp_url_[], const int UTC_offset_seconds);

/// @brief Funcao para realizar a chamada para o servidor NTP
/// @param[in] ntp Estrutura de dados ntp_t
void ntp_request(ntp_t *ntp);
#endif