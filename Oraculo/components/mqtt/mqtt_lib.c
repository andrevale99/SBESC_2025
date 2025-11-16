
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"
#include "lwip/init.h"

#include "pico_logs.h"
#include "mqtt_lib.h"

static const char *TAG = "MQTT";

static void lwip_mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    pico_mqtt_client_t *pico_client = (pico_mqtt_client_t *)arg;

    if (pico_client && pico_client->connection_cb)
    {
        pico_client->connection_cb(status);
    }

    //     if (status == MQTT_CONNECT_ACCEPTED)
    //     {
    //         PICO_LOGI(TAG, "Conectado ao broker MQTT com sucesso.");

    //         // O código de subscrição deve ser adicionado aqui, se houver tópicos padrão
    //     }
    //     else
    //     {
    //         PICO_LOGE(TAG, "Falha na conexão ao broker MQTT, status: %d", status);
    //     }
    // }
}

static void lwip_mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    pico_mqtt_client_t *pico_client = (pico_mqtt_client_t *)arg;

    if (pico_client)
    {
        PICO_LOGI(TAG, "incoming_publish_cb chamado para tópico: %s, tamanho total: %u", topic, (unsigned)tot_len);
    }
}
static void lwip_mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    pico_mqtt_client_t *pico_client = (pico_mqtt_client_t *)arg;

    if (pico_client && pico_client->message_cb)
    {

        PICO_LOGI(TAG, "incoming_data_cb chamado com len: %d, flags: %d", len, flags);
    }
}

static void lwip_mqtt_request_cb(void *arg, err_t err)
{
    if (err != ERR_OK)
    {
        PICO_LOGE(TAG, "Requisição (Sub/Unsub/Pub QoS1+) falhou: %d", err);
    }
}

static void dns_simple_cb(const char *name, const ip_addr_t *ipaddr, void *arg)
{
    ip_addr_t *out = (ip_addr_t *)arg;

    if (ipaddr == NULL)
    {
        PICO_LOGE(TAG, "DNS: Falha ao resolver '%s' (callback: ipaddr NULL)", name);
        return;
    }

    *out = *ipaddr;

    char buf[32];
    ipaddr_ntoa_r(ipaddr, buf, sizeof(buf));

    PICO_LOGI(TAG, "DNS: '%s' resolvido para %s", name, buf);
}

static bool resolve_broker_address(const char *broker_ip_addr, ip_addr_t *addr)
{
    if (ip4addr_aton(broker_ip_addr, addr))
    {
        PICO_LOGI(TAG, "Endereço IP fornecido diretamente: %s", broker_ip_addr);
        return true;
    }

    PICO_LOGI(TAG, "Resolvendo DNS para %s...", broker_ip_addr);

    err_t err = dns_gethostbyname(broker_ip_addr, addr, dns_simple_cb, addr);

    if (err == ERR_OK)
    {
        PICO_LOGI(TAG, "DNS imediato: %s -> %s", broker_ip_addr, ipaddr_ntoa(addr));
        return true;
    }

    if (err == ERR_INPROGRESS)
    {
        for (int i = 0; i < 100; i++)
        {
            cyw43_arch_poll();
            sleep_ms(10);

            if (!ip_addr_isany_val(*addr))
            {
                PICO_LOGI(TAG, "DNS assíncrono OK: %s -> %s", broker_ip_addr, ipaddr_ntoa(addr));
                return true;
            }
        }

        PICO_LOGE(TAG, "Timeout aguardando DNS para %s", broker_ip_addr);
        return false;
    }

    PICO_LOGE(TAG, "dns_gethostbyname falhou com erro %d", err);
    return false;
}

bool pico_mqtt_init(pico_mqtt_client_t *client, const char *broker_ip_addr, uint16_t port)
{
    static ip_addr_t broker_addr;

    if (!resolve_broker_address(broker_ip_addr, &broker_addr))
    {
        PICO_LOGE(TAG, "Falha ao resolver o endereço do broker MQTT: %s", broker_ip_addr);
        return false;
    }

    if (!client->internal_client)
    {
        client->internal_client = mqtt_client_new();
    }

    if (!client->internal_client)
    {
        PICO_LOGE(TAG, "Falha ao criar novo cliente lwIP.");
        return false;
    }

    struct mqtt_connect_client_info_t lwip_info = {
        .client_id = client->credentials.client_id,
        .client_user = client->credentials.user,
        .client_pass = client->credentials.pass,
        .keep_alive = client->keep_alive_sec,
        .will_topic = client->lwt.topic,
        .will_msg = client->lwt.msg,
        .will_msg_len = client->lwt.msg_len,
        .will_qos = client->lwt.qos,
        .will_retain = client->lwt.retain};

    mqtt_set_inpub_callback(client->internal_client,
                            lwip_mqtt_incoming_publish_cb,
                            lwip_mqtt_incoming_data_cb,
                            (void *)client);

    PICO_LOGI(TAG, "Tentando conectar ao broker MQTT em %s:%d...", broker_ip_addr, port);

    PICO_LOGI(TAG, "IP server: %s", ipaddr_ntoa(&broker_addr));

    err_t err = mqtt_client_connect(client->internal_client,
                                    &broker_addr,
                                    port,
                                    lwip_mqtt_connection_cb,
                                    (void *)client,
                                    &lwip_info);

    if (err != ERR_OK)
    {
        PICO_LOGE(TAG, "Erro ao iniciar a conexão MQTT: %d", err);
        return false;
    }

    return true;
}

bool pico_mqtt_publish(pico_mqtt_client_t *client, const char *topic,
                       const void *payload, uint16_t len, uint8_t qos, uint8_t retain)
{
    if (!client || !client->internal_client)
        return false;

    err_t err = mqtt_publish(client->internal_client,
                             topic,
                             payload,
                             len,
                             qos,
                             retain,
                             lwip_mqtt_request_cb,
                             (void *)client);

    return (err == ERR_OK);
}

bool pico_mqtt_subscribe(pico_mqtt_client_t *client, const char *topic, uint8_t qos)
{
    if (!client || !client->internal_client)
        return false;

    err_t err = mqtt_subscribe(client->internal_client,
                               topic,
                               qos,
                               lwip_mqtt_request_cb,
                               (void *)client);

    return (err == ERR_OK);
}

bool pico_mqtt_unsubscribe(pico_mqtt_client_t *client, const char *topic)
{
    if (!client || !client->internal_client)
        return false;

    err_t err = mqtt_unsubscribe(client->internal_client,
                                 topic,
                                 lwip_mqtt_request_cb,
                                 (void *)client);

    return (err == ERR_OK);
}

void pico_mqtt_disconnect(pico_mqtt_client_t *client)
{
    if (!client || !client->internal_client)
        return;

    if (mqtt_client_is_connected(client->internal_client))
    {
        mqtt_disconnect(client->internal_client);
        PICO_LOGI(TAG, "Desconectado do broker MQTT.");
    }
}

bool pico_mqtt_is_connected(pico_mqtt_client_t *client)
{
    if (!client || !client->internal_client)
        return false;

    return mqtt_client_is_connected(client->internal_client);
}