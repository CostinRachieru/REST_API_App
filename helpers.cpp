#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "buffer.h"

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line) {
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag) {
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    DIE(sockfd < 0, "Error opening socket.");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    int r = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    DIE(r < 0, "Error connecting.");

    return sockfd;
}

void close_connection(int sockfd) {
    close(sockfd);
}

void send_to_server(int sockfd, char *message) {
    int bytes, sent = 0;
    int total = strlen(message);
    do {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing message to socket");
        }
        if (bytes == 0) {
            break;
        }
        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd) {
    char response[MSGLEN];
    buffer buffer = buffer_init();
    int header_end = 0;
    int content_length = 0;

    do {
        int bytes = read(sockfd, response, MSGLEN);
        if (bytes < 0){
            error("ERROR reading response from socket");
        }
        if (bytes == 0) {
            break;
        }
        buffer_add(&buffer, response, (size_t) bytes);
        header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);
        if (header_end >= 0) {
            header_end += HEADER_TERMINATOR_SIZE;
            
            int content_length_start = buffer_find_insensitive(&buffer,
                CONTENT_LENGTH, CONTENT_LENGTH_SIZE);
            
            if (content_length_start < 0) {
                continue;           
            }
            content_length_start += CONTENT_LENGTH_SIZE;
            content_length = strtol(buffer.data + content_length_start, NULL, 10);
            break;
        }
    } while (1);

    size_t total = content_length + (size_t) header_end;

    while (buffer.size < total) {
        int bytes = read(sockfd, response, MSGLEN);
        if (bytes < 0) {
            error("ERROR reading response from socket");
        }
        if (bytes == 0) {
            break;
        }
        buffer_add(&buffer, response, (size_t) bytes);
    }
    buffer_add(&buffer, "", 1);

    char *to_return = (char*) malloc(buffer.size);
    memcpy(to_return, buffer.data, buffer.size);
    buffer_destroy(&buffer);
    return to_return;
}

char *basic_extract_json_response(char *str) {
    return strstr(str, "{\"");
}

char *extract_json_error(char *str) {
    char *new_str = strstr(str, "{\"error\"");
    if (new_str == NULL) {
        return NULL;
    }
    new_str += 10;
    new_str[strlen(new_str) - 2] = 0;
    return new_str;
}

char *extract_json_token(char *str) {
    char *new_str = strstr(str, "{\"token\":");
    new_str += 10;
    new_str[strlen(new_str) - 2] = 0;
    return new_str;
}

char *extract_cookie(char *str) {
    char *new_str = strstr(str, "connect.sid=");
    new_str += 12;
    int i = 1;
    while (new_str[i] != ' ') {
        i++;
    }
    new_str[i - 1] = 0;

    return new_str;
}

char *extract_books(char *str) {
    return strstr(str, "[");
}

