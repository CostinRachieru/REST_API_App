#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include "helpers.h"
#include "requests.h"

char* send_and_receive(char *message, char* ip_host) {
    int sockfd = open_connection(ip_host, 8080, AF_INET, SOCK_STREAM, 0);
    
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    close_connection(sockfd);
    return response;
}

int main(int argc, char *argv[]) {

    char inputs_buffer[INPUTS_BUFLEN];
    char username[MAX_USERNM_LEN];
    char password[MAX_PASSWD_LEN];
    int username_len, password_len;
    bool is_logged_in = false;
    char *cookie;
    char *token;
    cookie = NULL;
    token = NULL;
    bool has_token = false;

    char *message;
    char *response;

    char *ip_host = inet_ntoa((*((struct in_addr **)gethostbyname(HOST)
        -> h_addr_list)[0]));

    while (true) {
        memset(inputs_buffer, 0, INPUTS_BUFLEN);
        read(STDIN_FILENO, inputs_buffer, INPUTS_BUFLEN);

        if (strncmp(inputs_buffer, "register\n", 9) == 0) {
            char new_username[MAX_USERNM_LEN];
            char new_password[MAX_PASSWD_LEN];

            // Prompt for getting user credentials.
            std::cout << "username=" << std::flush;
            int new_username_len = read(STDIN_FILENO, new_username, MAX_USERNM_LEN);
            new_username[new_username_len - 1] = 0;
            std::cout << "password=" << std::flush;
            int new_password_len = read(STDIN_FILENO, new_password, MAX_PASSWD_LEN);
            new_password[new_password_len - 1] = 0;

            char *body_data[4];
            body_data[0] = USERNM_CHARS;
            body_data[1] = new_username;
            body_data[2] = PASSWD_CHARS;
            body_data[3] = new_password;

            message = compute_post_request(HOST, REGISTER, PAYLOAD_TYPE, body_data,
                REGISTER_FIELDS, NULL, NULL);
            
            response = send_and_receive(message, ip_host);

            char *error = extract_json_error(response);

            if (error == NULL) {
                std::cout << "=> Registration completed successfully.\n\n";
            } else {
                std::cout << "=>" << error << "\n\n";
            }

            free(response);
            free(message);
            continue;
        }

        if (strncmp(inputs_buffer, "login\n", 6) == 0) {
            if (is_logged_in == true) {
                std::cout << "=> Already logged in.\n\n";
                continue;
            }

            // Prompt for getting user credentials.
            std::cout << "username=" << std::flush;
            username_len = read(STDIN_FILENO, username, MAX_USERNM_LEN);
            username[username_len - 1] = 0;
            std::cout << "password=" << std::flush;
            password_len = read(STDIN_FILENO, password, MAX_PASSWD_LEN);
            password[password_len - 1] = 0;

            char *body_data[4];
            body_data[0] = USERNM_CHARS;
            body_data[1] = username;
            body_data[2] = PASSWD_CHARS;
            body_data[3] = password;
            message = compute_post_request(HOST, LOGIN, PAYLOAD_TYPE, body_data,
                LOGIN_FIELDS, NULL, NULL);
            
            response = send_and_receive(message, ip_host);
            
            char *error = extract_json_error(response);

            if (error == NULL) {
                std::cout << "=> Login succeeded\n\n";
                // Extract and save cookie.
                char* extracted = extract_cookie(response);
                uint32_t size = strlen(extracted);
                cookie = (char*)calloc(size + 1, sizeof(char));
                memcpy(cookie, extracted, size);
                is_logged_in = true;
            } else {
                std::cout << "=> " << error << "\n\n";
            }

            free(message);
            free(response);
            continue;
        }

        if (strncmp(inputs_buffer, "enter_library\n", 14) == 0) {
            if (is_logged_in == false) {
                std::cout << "=> You need to log in first.\n\n";
                continue;
            }
            if (has_token) {
                free(token);
                token = NULL;
            }

            message = compute_get_request(HOST, ENTER_LIBRARY, NULL, cookie, NULL, false);
            
            response = send_and_receive(message, ip_host);

            char *error = extract_json_error(response);

            if (error == NULL) {
                char *extracted = extract_json_token(response);
                uint32_t size = strlen(extracted);
                token = (char*)calloc(size + 1, sizeof(char));
                memcpy(token, extracted, size);
                has_token = true;
                std::cout << "=> Access granted to library.\n\n";
            } else {
                // Logs the user out because he needs to get the cookie again.
                std::cout << "=> Entering the library failed: " << error << "\n";
                std::cout << "=> Please log in again.\n\n";
                memset(username, 0, MAX_USERNM_LEN);
                memset(password, 0, MAX_PASSWD_LEN);
                username_len = 0;
                password_len = 0;
                is_logged_in = false;
                has_token = false;
                free(cookie);
                cookie = NULL;
            }

            free(message);
            free(response);
            continue;
        }


        if (strncmp(inputs_buffer, "get_books\n", 10) == 0) {
            if (is_logged_in == false) {
                std::cout << "=> You need to log in first.\n\n";
                continue;
            }
            if (has_token == false) {
                std::cout << "=> You have to get access to library first.\n\n";
                continue;
            }

            message = compute_get_request(HOST, GET_BOOKS, NULL,
                NULL, token, false);
            
            response = send_and_receive(message, ip_host);

            char *error = extract_json_error(response);
            if (error != NULL) {
                std::cout << "=> " << error << "\n\n";
            } else {
                std::cout << "=> " << extract_books(response) << "\n\n";
            }

            free(message);
            free(response);
            continue;
        }

        if (strncmp(inputs_buffer, "get_book\n", 9) == 0) {
            if (is_logged_in == false) {
                std::cout << "=> You need to log in first.\n\n";
                continue;
            }
            if (has_token == false) {
                std::cout << "=> You have to get access to library first.\n\n";
                continue;
            }

            // Prompt for getting the id of the book.
            char id[MAX_ID_LEN];
            memset(id, 0, MAX_ID_LEN);
            
            std::cout << "id=" << std::flush;
            id[read(STDIN_FILENO, id, MAX_ID_LEN) - 1] = 0;

            message = compute_get_request(HOST, GET_BOOK, id,
                NULL, token, false);
            
            response = send_and_receive(message, ip_host);

            char *error = extract_json_error(response);
            if (error != NULL) {
                std::cout << "=> " <<  error << "\n\n";
            } else {
                std::cout << "=> " << basic_extract_json_response(response) << "\n\n";
            }

            free(message);
            free(response);
            continue;
        }

        if (strncmp(inputs_buffer, "add_book\n", 9) == 0) {
            if (is_logged_in == false) {
                std::cout << "=> You need to log in first.\n\n";
                continue;
            }
            if (has_token == false) {
                std::cout << "=> You have to enter the library first.\n\n";
                continue;
            }

            char title[MAX_BOOK_PARAM_LEN];
            char author[MAX_BOOK_PARAM_LEN];
            char genre[MAX_BOOK_PARAM_LEN];
            char publisher[MAX_BOOK_PARAM_LEN];
            char page_count[MAX_PAGES_LEN];
            memset(title, 0, MAX_BOOK_PARAM_LEN);
            memset(author, 0, MAX_BOOK_PARAM_LEN);
            memset(genre, 0, MAX_BOOK_PARAM_LEN);
            memset(publisher, 0, MAX_BOOK_PARAM_LEN);
            memset(page_count, 0, MAX_PAGES_LEN);

            // Prompt for getting the information about the book to be added.
            std::cout << "title=" << std::flush;
            title[read(STDIN_FILENO, title, MAX_BOOK_PARAM_LEN) - 1] = 0;
            std::cout << "author=" << std::flush;
            author[read(STDIN_FILENO, author, MAX_BOOK_PARAM_LEN) - 1] = 0;
            std::cout << "genre=" << std::flush;
            genre[read(STDIN_FILENO, genre, MAX_BOOK_PARAM_LEN) - 1] = 0;
            std::cout << "publisher=" << std::flush;
            publisher[read(STDIN_FILENO, publisher, MAX_BOOK_PARAM_LEN) - 1] = 0;
            std::cout << "page_count=" << std::flush;
            page_count[read(STDIN_FILENO, page_count, MAX_BOOK_PARAM_LEN) - 1] = 0;
            bool is_number = true;
            // Tests if the information is valid.
            for (uint16_t i = 0; i < strlen(page_count); ++i) {
                if (page_count[i] < '0' || page_count[i] > '9') {
                    std::cout << "=> Page_count must be a positive number.\n\n";
                    is_number = false;
                    break;
                }
            }
            if (!is_number) {
                continue;
            }

            char *body_data[BODY_DATA_ADD_BOOK_SIZE];
            body_data[0] = TITLE_CHARS;
            body_data[1] = title;
            body_data[2] = AUTHOR_CHARS;
            body_data[3] = author;
            body_data[4] = GENRE_CHARS;
            body_data[5] = genre;
            body_data[6] = PAGE_COUNT_CHARS;
            body_data[7] = page_count;
            body_data[8] = PUBLISHER_CHARS;
            body_data[9] = publisher;

            message = compute_post_request(HOST, ADD_BOOK, PAYLOAD_TYPE, body_data,
                BODY_DATA_ADD_BOOK_SIZE, NULL, token);
            
            response = send_and_receive(message, ip_host);

            char* error = extract_json_error(response);
            if (error != NULL) {
                std::cout << "=> " << error << "\n\n";
            } else {
                std::cout << "=> Book added.\n\n";
            }

            free(message);
            free(response);
            continue;
        }

        if (strncmp(inputs_buffer, "delete_book\n", 12) == 0) {
            if (is_logged_in == false) {
                std::cout << "=> You need to log in first.\n\n";
                continue;
            }
            if (has_token == false) {
                std::cout << "=> You have to enter the library first.\n\n";
                continue;
            }

            char id[MAX_ID_LEN];
            memset(id, 0, MAX_ID_LEN);
            
            // Prompt to get the id for the book to be deleted.            
            std::cout << "id=" << std::flush;
            id[read(STDIN_FILENO, id, MAX_ID_LEN) - 1] = 0;

            message = compute_get_request(HOST, DELETE_BOOK, id,
                NULL, token, true);
            
            response = send_and_receive(message, ip_host);
            
            char *error = extract_json_error(response);
            if (error != NULL) {
                std::cout << "=> " << error << "\n\n";
            } else {
                std::cout << "=> Book deleted.\n\n";
            }

            free(message);
            free(response);
            continue;
        }

        if (strncmp(inputs_buffer, "logout\n", 7) == 0) {
            if (is_logged_in == false) {
                std::cout << "=> You need to log in first.\n\n";
                continue;
            }

            message = compute_get_request(HOST, LOGOUT, NULL, cookie, NULL, false);
            
            response = send_and_receive(message, ip_host);

            std::cout << "=> Logged out successfully.\n\n";

            memset(username, 0, MAX_USERNM_LEN);
            memset(password, 0, MAX_PASSWD_LEN);
            username_len = 0;
            password_len = 0;
            is_logged_in = false;
            if (has_token == true) {
                free(token);
                token = NULL;
            }
            has_token = false;
            free(cookie);
            cookie = NULL;

            free(message);
            free(response);
            continue;
        }

        if (strncmp(inputs_buffer, "exit\n", 5) == 0) {
            std::cout << "=> Exited successfully.\n";
            std::cout << "~~~~~~~ Closing ~~~~~~~\n";
            break;
        }

        std::cout << "=> Invalid command.\n\n";
    }

    if (token != NULL) {
        free(token);
    }
    if (cookie != NULL) {
        free(cookie);
    }

    return 0;
}
