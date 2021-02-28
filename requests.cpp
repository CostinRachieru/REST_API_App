#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char *cookie, char *token, bool isDelete) {
    char *message = (char*)calloc(MSGLEN, sizeof(char));
    char *line = (char*)calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        if (!isDelete) {
            sprintf(line, "GET %s%s HTTP/1.1", url, query_params);
        } else {
            sprintf(line, "DELETE %s%s HTTP/1.1", url, query_params);
        }
    } else {
        if (!isDelete) {
            sprintf(line, "GET %s HTTP/1.1", url);
        } else {
            sprintf(line, "DELETE %s HTTP/1.1", url);
        }
    }

    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    memset(line, 0, LINELEN);
    if (cookie != NULL) {
        strcat(line, "Cookie: ");
        strcat(line, COOKIE_HEADER);
        strncat(line, cookie, strlen(cookie));
        compute_message(message, line);
    }
    if (token != NULL) {
        strcat(line, TOKEN_HEADER);
        strncat(line, token, strlen(token));
        compute_message(message, line);
    }

    compute_message(message, "");
    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char *cookie, char *token) {
    char *message = (char*)calloc(MSGLEN, sizeof(char));
    char *line = (char*)calloc(LINELEN, sizeof(char));
    char *body_data_buffer = (char*)calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    sprintf(line, "Host: %s", host);
    compute_message(message,line);

    if (token != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, TOKEN_HEADER);
        strncat(line, token, strlen(token));
        compute_message(message, line);
    }
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    if (body_data != NULL) {
        body_data_buffer[0] = '{';
        for (int i = 0; i < body_data_fields_count - 2; ++i) {
            strcat(body_data_buffer, (char*)"\"");
            strcat(body_data_buffer, body_data[i]);
            strcat(body_data_buffer, (char*)"\": \"");
            strcat(body_data_buffer, body_data[++i]);
            strcat(body_data_buffer, (char*)"\",");
        }
        strcat(body_data_buffer, (char*)"\"");
        strcat(body_data_buffer, body_data[body_data_fields_count - 2]);
        strcat(body_data_buffer, (char*)"\": \"");
        strcat(body_data_buffer, body_data[body_data_fields_count - 1]);
        strcat(body_data_buffer, (char*)"\"}");
    }
    sprintf(line, "Content-Length: %ld", strlen(body_data_buffer));
    compute_message(message, line);

    compute_message(message, "");

    compute_message(message, body_data_buffer);

    free(line);
    free(body_data_buffer);
    return message;
}
