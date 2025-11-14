#ifndef MQTT_H
#define MQTT_H

#include <string.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/dns.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h" // needed to set hostname

#define MQTT_SERVER "test.mosquitto.org"

#ifndef WIFI_SSID
#define WIFI_SSID "WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "123"
#endif

// qos passed to mqtt_subscribe
// At most once (QoS 0)
// At least once (QoS 1)
// Exactly once (QoS 2)
#define MQTT_SUBSCRIBE_QOS 1
#define MQTT_PUBLISH_QOS 1
#define MQTT_PUBLISH_RETAIN 0
#define MQTT_TOPIC_LEN 100
#define MQTT_DEVICE_NAME "picoSBESC"

// topic used for last will and testament
#define MQTT_WILL_TOPIC "/ORACULO"
#define MQTT_WILL_MSG "0"
#define MQTT_WILL_QOS 1

// keep alive in seconds
#define MQTT_KEEP_ALIVE_S 60

extern float glicose[50];
extern uint8_t idxGlicose;

static uint16_t glicoseSensor[50] = {
178.99999999999997, 183.0, 188.0, 193.0, 200.0, 208.0, 183.0, 188.0, 193.0, 
200.0, 208.0, 215.0, 188.0, 193.0, 200.0, 208.0, 215.0, 
219.0, 193.0, 200.0, 208.0,215.0,219.0,234.0,200.0,208.0,215.0,219.0,
234.0,247.0,208.0,215.0,219.0,234.0,247.0,257.0,215.0,
219.0,234.0,247.0,257.0,270.0,219.0,234.0,247.0,257.0,270.0,283.0,234.0, 247.0, 
};

typedef struct
{
    mqtt_client_t *mqtt_client_inst;
    struct mqtt_connect_client_info_t mqtt_client_info;
    char data[MQTT_OUTPUT_RINGBUF_SIZE];
    char topic[MQTT_TOPIC_LEN];
    uint32_t len;
    ip_addr_t mqtt_server_address;
    bool connect_done;
    int subscribe_count;
    bool stop_client;
} MQTT_CLIENT_DATA_T;

static const char *full_topic(MQTT_CLIENT_DATA_T *state, const char *name)
{
#if MQTT_UNIQUE_TOPIC
    static char full_topic_[MQTT_TOPIC_LEN];
    snprintf(full_topic, sizeof(full_topic), "/%s%s", state->mqtt_client_info.client_id, name);
    return full_topic;
#else
    return name;
#endif
}

//=================================================
//  PUBLIC FUNCTIONS
//=================================================

/// @brief  Inicia o cliente MQTT
/// @param state Estrutura MQTT
void start_client(MQTT_CLIENT_DATA_T *state);

// Call back with a DNS result
void dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);

void hello();

#endif
