#ifndef MQTT_H
#define MQTT_H

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/unique_id.h"

#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h" // needed to set hostname
#include "lwip/dns.h"
#include "lwip/altcp_tls.h"

#define MQTT_SERVER "gph.imd.ufrn.br"
#define PORT 8884

// qos passed to mqtt_subscribe
// At most once (QoS 0)
// At least once (QoS 1)
// Exactly once (QoS 2)
#define MQTT_SUBSCRIBE_QOS 1
#define MQTT_PUBLISH_QOS 1
#define MQTT_PUBLISH_RETAIN 0
#define MQTT_TOPIC_LEN 100
#define MQTT_DEVICE_NAME "ORACULO_SBESC"

// topic used for last will and testament
#define MQTT_WILL_TOPIC "/ORACULO/will"
#define MQTT_WILL_MSG "0"
#define MQTT_WILL_QOS 1

// keep alive in seconds
#define MQTT_KEEP_ALIVE_S 60

#define TOPIC "/ORACULO/versao_0.1"

typedef struct {
    mqtt_client_t* mqtt_client_inst;
    struct mqtt_connect_client_info_t mqtt_client_info;
    char data[MQTT_OUTPUT_RINGBUF_SIZE];
    char topic[MQTT_TOPIC_LEN];
    uint32_t len;
    ip_addr_t mqtt_server_address;
    bool connect_done;
    int subscribe_count;
    bool stop_client;
} MQTT_CLIENT_DATA_T;

void Mqtt_init(void);

void dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);

const char *full_topic(MQTT_CLIENT_DATA_T *state, const char *name);

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
void pub_request_cb(__unused void *arg, err_t err);

void start_client(MQTT_CLIENT_DATA_T *state);

#endif