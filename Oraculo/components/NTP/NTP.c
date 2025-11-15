/**
 * @note MUDAR de uma variavel bool (bFlag) para um evento de bits,
 * ou algo parecido para verificar se houve comunicacao com o servidor
 * NTP
 */

#include "NTP.h"

#define LEN_URL 45
static char ntp_url[LEN_URL];
static int UTC_offset = 0;
static bool bFlag = false;

//=========================================
//  STATIC
//=========================================

// Called with results of operation
static void ntp_result(ntp_t *state, int status, time_t *result)
{
    if (status == 0 && result)
    {
        state->utc = gmtime(result);
        bFlag = true;
        // struct tm *utc = gmtime(result);
        // printf("got ntp response: %02d/%02d/%04d %02d:%02d:%02d\n", state->utc->tm_mday, state->utc->tm_mon + 1, state->utc->tm_year + 1900,
        //        state->utc->tm_hour, state->utc->tm_min, state->utc->tm_sec);
    }
    // async_context_remove_at_time_worker(cyw43_arch_async_context(), &state->resend_worker);
    // hard_assert(async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(),  &state->request_worker, NTP_TEST_TIME_MS)); // repeat the request in future
    // printf("Next request in %ds\n", NTP_TEST_TIME_MS / 1000);
}

// Call back with a DNS result
static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
    ntp_t *state = (ntp_t *)arg;
    if (ipaddr)
    {
        state->ntp_server_address = *ipaddr;
        printf("ntp address %s\n", ipaddr_ntoa(ipaddr));
        ntp_request(state);
    }
    else
    {
        printf("ntp dns request failed\n");
        ntp_result(state, -1, NULL);
    }
}

// NTP data received
static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    ntp_t *state = (ntp_t *)arg;
    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    // Check the result
    if (ip_addr_cmp(addr, &state->ntp_server_address) && port == NTP_PORT && p->tot_len == NTP_MSG_LEN &&
        mode == 0x4 && stratum != 0)
    {
        uint8_t seconds_buf[4] = {0};
        pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
        uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];
        uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA + UTC_offset;
        time_t epoch = seconds_since_1970;
        ntp_result(state, 0, &epoch);
    }
    else
    {
        printf("invalid ntp response\n");
        ntp_result(state, -1, NULL);
    }
    pbuf_free(p);
}

// Called to make a NTP request
static void request_worker_fn(__unused async_context_t *context, async_at_time_worker_t *worker)
{
    ntp_t *state = (ntp_t *)worker->user_data;
    hard_assert(async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &state->resend_worker, NTP_RESEND_TIME_MS)); // in case UDP request is lost
    int err = dns_gethostbyname(ntp_url, &state->ntp_server_address, ntp_dns_found, state);
    if (err == ERR_OK)
    {
        ntp_request(state); // Cached DNS result, make NTP request
    }
    else if (err != ERR_INPROGRESS)
    { // ERR_INPROGRESS means expect a callback
        printf("dns request failed\n");
        ntp_result(state, -1, NULL);
    }
}

// Called to resend an NTP request if it appears to get lost
static void resend_worker_fn(__unused async_context_t *context, async_at_time_worker_t *worker)
{
    ntp_t *state = (ntp_t *)worker->user_data;
    // printf("ntp request failed\n");
    // PICO_LOGW
    ntp_result(state, -1, NULL);
}

//=========================================
//  PUBLIC
//=========================================
ntp_t *ntp_init(const char ntp_url_[], const int UTC_offset_seconds)
{
    memcpy(ntp_url, ntp_url_, LEN_URL);

    ntp_set_utc_offset(UTC_offset_seconds);

    ntp_t *state = (ntp_t *)calloc(1, sizeof(ntp_t));
    if (!state)
    {
        // printf("failed to allocate state\n");
        // PICO_LOGE
        return NULL;
    }
    state->ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!state->ntp_pcb)
    {
        // printf("failed to create pcb\n");
        // PICO_LOGE
        free(state);
        return NULL;
    }

    udp_recv(state->ntp_pcb, ntp_recv, state);
    state->request_worker.do_work = request_worker_fn;
    state->request_worker.user_data = state;
    state->resend_worker.do_work = resend_worker_fn;
    state->resend_worker.user_data = state;

    hard_assert(async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &state->request_worker, 0)); // make the first request

    return state;
}

void ntp_deinit(ntp_t *ntp)
{
    free(ntp);
}

// Make an NTP request
void ntp_request(ntp_t *state)
{
    bFlag = false;
    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    uint8_t *req = (uint8_t *)p->payload;
    memset(req, 0, NTP_MSG_LEN);
    req[0] = 0x1b;
    udp_sendto(state->ntp_pcb, p, &state->ntp_server_address, NTP_PORT);
    pbuf_free(p);
    cyw43_arch_lwip_end();
}

bool ntP_ntp_response(void)
{
    return bFlag;
}

void ntp_set_utc_offset(const int UTC_offset_seconds)
{
    UTC_offset = UTC_offset_seconds;
}

//=============================================
//  TASKS
//=============================================