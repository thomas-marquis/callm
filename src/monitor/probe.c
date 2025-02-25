#include "probe.h"
#include "../core/errors.h"
#include "../core/logging.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct http_client_t
{
    int sockfd;
    struct sockaddr_in server_addr;
    pthread_mutex_t lock;
};

static struct http_client_t *client_instance = NULL;

CallmStatusCode
Probe_init(const char *host, int server_port)
{
    if (client_instance == NULL)
    {
        client_instance = (struct http_client_t *) malloc(sizeof(struct http_client_t));
        CHECK_MALLOC_PANIC(client_instance, "Error init prob http client");

        if (pthread_mutex_init(&client_instance->lock, NULL) != 0)
        {
            LOG_ERROR("Mutex init failed");
            exit(EXIT_FAILURE);
        }

        client_instance->sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_instance->sockfd < 0)
        {
            LOG_ERROR("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        client_instance->server_addr.sin_family = AF_INET;
        client_instance->server_addr.sin_port = htons(server_port);
        if (inet_pton(AF_INET, host, &client_instance->server_addr.sin_addr) <= 0)
        {
            LOG_ERROR("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }

        if (connect(client_instance->sockfd, (struct sockaddr *) &client_instance->server_addr,
                    sizeof(client_instance->server_addr))
            < 0)
        {
            LOG_ERROR("Connection failed");
            exit(EXIT_FAILURE);
        }
    }

    return OK;
}

CallmStatusCode
Probe_send_matrix(Matrix *M, const char *msg)
{
    LOG_INFO("Sending matrix to probe server");
    if (M == NULL || msg == NULL)
    {
        LOG_ERROR("Invalid or null input");
        return ERROR;
    }

    if (pthread_mutex_lock(&client_instance->lock) != 0)
    {
        LOG_ERROR("Mutex lock failed");
        return ERROR;
    }

    char *json_matrix = Matrix_to_json(M);
    if (json_matrix == NULL)
    {
        LOG_ERROR("Failed to convert matrix to json");
        pthread_mutex_unlock(&client_instance->lock);
        return ERROR;
    }

    size_t json_payload_size = snprintf(NULL, 0, "{\"message\": \"%s\", \"matrix\": %s}", msg, json_matrix) + 1;

    size_t request_size
        = snprintf(NULL, 0,
                   "POST /data HTTP/1.1\r\n"
                   "Host: %s\r\n"
                   "Content-Type: application/json\r\n"
                   "Content-Length: %zu\r\n\r\n"
                   "{\"message\": \"%s\", \"matrix\": %s}",
                   inet_ntoa(client_instance->server_addr.sin_addr), json_payload_size - 1, msg, json_matrix)
          + 1;

    char *request = (char *) malloc(request_size);
    if (request == NULL)
    {
        LOG_ERROR("Memory allocation failed for request");
        free(json_matrix);
        pthread_mutex_unlock(&client_instance->lock);
        return ERROR;
    }

    char *body = (char *) malloc(strlen(json_matrix) + strlen(msg) + strlen("{\"message\": \"\", \"matrix\": }") + 1);
    if (body == NULL)
    {
        LOG_ERROR("Memory allocation failed for body");
        free(request);
        free(json_matrix);
        pthread_mutex_unlock(&client_instance->lock);
        return ERROR;
    }
    sprintf(body, "{\"message\": \"%s\", \"matrix\": %s}", msg, json_matrix);

    snprintf(request, request_size,
             "POST /data HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "\r\n"
             "%s",
             inet_ntoa(client_instance->server_addr.sin_addr), strlen(body), body);

    ssize_t bytes_sent = send(client_instance->sockfd, request, strlen(request), 0);
    if (bytes_sent == -1)
    {
        LOG_ERROR("Failed to send data");
        free(request);
        free(json_matrix);
        pthread_mutex_unlock(&client_instance->lock);
        return ERROR;
    }
    LOGF_DEBUG("Sent %ld bytes, body length: %zu", bytes_sent, strlen(json_matrix));

    char response[4096];
    ssize_t bytes_received = recv(client_instance->sockfd, response, sizeof(response) - 1, 0);
    if (bytes_received == -1)
    {
        LOG_ERROR("Failed to receive data");
        free(request);
        free(json_matrix);
        pthread_mutex_unlock(&client_instance->lock);
        return ERROR;
    }
    response[bytes_received] = '\0';
    LOGF_INFO("Received response: %s", response);

    free(request);
    free(json_matrix);

    if (pthread_mutex_unlock(&client_instance->lock) != 0)
    {
        LOG_ERROR("Mutex unlock failed");
        return ERROR;
    }

    return OK;
}
