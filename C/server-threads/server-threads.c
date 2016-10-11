#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

enum {
    PORTNUM = 5010,
    BACKLOG = 500,
    BUFLEN = 128
};

void get_rand_str(char *str)
{
    static const char table[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    int i;
    for (int i = 0; i < BUFLEN; i++) {
        str[i] = table[rand() % (sizeof(table) - 1)];
    }
}

void *connection_handler(void *arg)
{
    int *sockfd = arg;
    char buf[BUFLEN];
    get_rand_str(buf);
    
    ssize_t byte_wrote, pos = 0;
    while ((byte_wrote = send(*sockfd, buf + pos, BUFLEN - pos, 0)) > 0) {
        pos += byte_wrote;
    }
    if (byte_wrote < 0) {
        fprintf(stderr, "Error: can't send message.\n");
    }

    close(*sockfd);
    free(sockfd);
    
    return NULL;
}

int main(void)
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: can't create socket.\n");
        return 1;
    }

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        fprintf(stderr, "Error: can't set socket option.\n");
        return 1;
    }

    struct sockaddr_in own_addr = {0};
    own_addr.sin_family = AF_INET;
    own_addr.sin_port = htons(PORTNUM);
    own_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)(&own_addr), sizeof(own_addr)) < 0) {
        fprintf(stderr, "Error: can't bind socket.\n");
        return 1;
    }

    if (listen(sockfd, BACKLOG) < 0) {
        fprintf(stderr, "Error: can't listen socket.\n");
        return 1;
    }

    while (1) {
        struct sockaddr_in client_addr = {0};
        socklen_t client_addr_len = sizeof(client_addr);

        int *newsockfd = malloc(sizeof(*newsockfd));
        if ((*newsockfd = accept(sockfd, (struct sockaddr *)(&client_addr),
                &client_addr_len)) < 0) {
            fprintf(stderr, "Error: can't accept connection.\n");
            return 1;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, connection_handler, newsockfd);
    }
}
