#ifndef _HELPERS_
#define _HELPERS_

#define LINELEN 1024
#define MSGLEN 8192
#define INPUTS_BUFLEN	128

#define MAX_USERNM_LEN  64
#define MAX_PASSWD_LEN   64

#define MAX_BOOK_PARAM_LEN   128
#define MAX_PAGES_LEN   6
#define MAX_ID_LEN  16

#define BODY_DATA_LOGIN_SIZE	4
#define BODY_DATA_ADD_BOOK_SIZE	10

#define HOST 	(char*)"ec2-3-8-116-10.eu-west-2.compute.amazonaws.com"
#define REGISTER 	(char*)"/api/v1/tema/auth/register"
#define REGISTER_FIELDS	4
#define LOGIN	(char*)"/api/v1/tema/auth/login"
#define LOGIN_FIELDS	4
#define LOGOUT	(char*)"/api/v1/tema/auth/logout"
#define ENTER_LIBRARY	(char*)"/api/v1/tema/library/access"
#define GET_BOOKS	(char*)"/api/v1/tema/library/books"
#define ADD_BOOK	(char*)"/api/v1/tema/library/books"
#define GET_BOOK 	(char*)"/api/v1/tema/library/books/"
#define DELETE_BOOK	(char*)"/api/v1/tema/library/books/"
#define PAYLOAD_TYPE 	(char*)"application/json"

#define USERNM_CHARS	(char*)"username"
#define PASSWD_CHARS	(char*)"password"
#define TITLE_CHARS	(char*)"title"
#define AUTHOR_CHARS	(char*)"author"
#define GENRE_CHARS	(char*)"genre"
#define PAGE_COUNT_CHARS	(char*)"page_count"
#define PUBLISHER_CHARS	(char*)"publisher"
#define TOKEN_HEADER	(char*)"Authorization: Bearer "
#define COOKIE_HEADER	(char*)"connect.sid="

#define DIE(assertion, call_description)    \
    do {                                    \
        if (assertion) {                    \
            fprintf(stderr, "(%s, %d): ",   \
                    __FILE__, __LINE__);    \
            perror(call_description);       \
            exit(EXIT_FAILURE);             \
        }                                   \
    } while(0)

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

// extracts and returns only the error form a JSON from a server response
char *extract_json_error(char *str);

// extracts cookie from a server response
char *extract_cookie(char *str);

// extracts token from a server response
char *extract_json_token(char *str);

// extracts the summary about books from a server response
char *extract_books(char *str);

#endif
