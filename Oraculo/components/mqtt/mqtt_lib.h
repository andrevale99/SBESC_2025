#ifndef PICO_MQTT_CLIENT_H
#define PICO_MQTT_CLIENT_H

#include <stdint.h>
// #include <cstdint>
#include <stdbool.h>        // Incluído para garantir 'bool' está definido
#include "lwip/apps/mqtt.h" // Inclui os tipos de dados básicos do lwIP MQTT

// --- Tipos de Dados ---

// 1. Definições dos Callbacks de Alto Nível (Simplificados)

/**
 * @brief Callback chamado ao receber uma mensagem MQTT.
 * @param topic O tópico da mensagem recebida (terminado em null).
 * @param payload O conteúdo binário da mensagem.
 * @param len O comprimento do payload.
 */
typedef void (*pico_mqtt_message_cb_t)(const char *topic, const uint8_t *payload, uint16_t len);

/**
 * @brief Callback chamado para notificar a mudança de status da conexão.
 * @param status Status da conexão (e.g., MQTT_CONNECT_ACCEPTED, MQTT_CONNECT_DISCONNECTED).
 */
typedef void (*pico_mqtt_connection_cb_t)(mqtt_connection_status_t status);

// 2. Estrutura de Configuração (Wrapper sobre lwIP)

/**
 * @brief Estrutura que representa e configura o cliente MQTT do Pico.
 * Utiliza estruturas aninhadas para agrupar parâmetros.
 */
// Renomeado de pico_mqtt_t para pico_mqtt_client_t
typedef struct
{
    // Grupo: Credenciais e Identificação
    struct
    {
        const char *client_id;
        const char *user;
        const char *pass;
    } credentials;

    // Grupo: Last Will and Testament (LWT)
    struct
    {
        const char *topic;
        const void *msg;
        uint16_t msg_len;
        uint8_t qos;
        uint8_t retain;
    } lwt;

    // Parâmetros de Conexão Geral
    uint16_t keep_alive_sec; // Tempo de Keep-Alive em segundos
    uint8_t clean_session;   // 1 para nova sessão, 0 para retomar

    // Callbacks da Aplicação
    pico_mqtt_message_cb_t message_cb;
    pico_mqtt_connection_cb_t connection_cb;

    // Campo Interno para Gerenciar o Cliente lwIP
    mqtt_client_t *internal_client;

} pico_mqtt_client_t; // Renomeado de pico_mqtt_config_t

// --- Funções da API ---

/**
 * @brief Inicializa o cliente MQTT e tenta conectar-se ao broker.
 * @param client A estrutura do cliente MQTT (tipo: pico_mqtt_client_t).
 * @param broker_ip_addr O endereço IP do broker (pode ser obtido por DNS ou IP fixo).
 * @param port A porta do broker (e.g., 1883).
 * @return true se a inicialização e tentativa de conexão foram bem-sucedidas.
 */
bool pico_mqtt_init(pico_mqtt_client_t *client, const char *broker_ip_addr, uint16_t port);

/**
 * @brief Publica uma mensagem em um tópico.
 * @param client A estrutura do cliente MQTT.
 * @param topic O tópico para publicar.
 * @param payload O conteúdo da mensagem.
 * @param len O comprimento do payload.
 * @param qos Quality of Service (0, 1 ou 2).
 * @param retain Flag Retain (1 para reter, 0 para não).
 * @return true se a publicação foi enviada com sucesso para o buffer.
 */
bool pico_mqtt_publish(pico_mqtt_client_t *client, const char *topic,
                       const void *payload, uint16_t len, uint8_t qos, uint8_t retain);

/**
 * @brief Assina um tópico no broker.
 * @param client A estrutura do cliente MQTT.
 * @param topic O tópico para assinar.
 * @param qos Quality of Service desejado.
 * @return true se a requisição de assinatura foi enviada.
 */
bool pico_mqtt_subscribe(pico_mqtt_client_t *client, const char *topic, uint8_t qos);

/**
 * @brief Cancela a assinatura de um tópico no broker.
 * @param client A estrutura do cliente MQTT.
 * @param topic O tópico para cancelar a assinatura.
 * @return true se a requisição de desinscrição foi enviada.
 */
bool pico_mqtt_unsubscribe(pico_mqtt_client_t *client, const char *topic);

/**
 * @brief Desconecta o cliente do broker MQTT de forma limpa.
 * @param client A estrutura do cliente MQTT.
 */
void pico_mqtt_disconnect(pico_mqtt_client_t *client);

bool pico_mqtt_is_connected(pico_mqtt_client_t *client);

// void start_mqtt();

#endif // PICO_MQTT_CLIENT_H