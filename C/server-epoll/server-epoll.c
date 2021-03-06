#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

enum {
    PORTNUM = 5010,
    BACKLOG = 500,
    BUFLEN = 128,
    BUFEPOLLLEN = 100
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

void send_message(int sockfd)
{
    char buf[BUFLEN];
    get_rand_str(buf);
    
    ssize_t byte_wrote, pos = 0;
    while ((byte_wrote = send(sockfd, buf + pos, BUFLEN - pos, 0)) > 0) {
        pos += byte_wrote;
    }
    if (byte_wrote < 0) {
        fprintf(stderr, "Error: can't send message.\n");
    }
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
    
    int efd;
    if ((efd = epoll_create(1)) < 0) {
        fprintf(stderr, "Error: can't create epoll fd.\n");
        return 1;
    }

    struct epoll_event ev_set;
    ev_set.events = EPOLLIN;
    ev_set.data.fd = sockfd;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &ev_set) < 0) {
        fprintf(stderr, "Error: can't add epoll fd.\n");
        return 1;
    }
    
    while (1) {
        int events_pending;
        struct epoll_event events[BUFEPOLLLEN];

        if ((events_pending = epoll_wait(efd, events, BUFEPOLLLEN, -1)) < 0) {
            fprintf(stderr, "Error: can't receive number of events for pending.\n");
            return 1;
        }

        for (int i = 0; i < events_pending; i++) {
            int curfd = events[i].data.fd;

            if (curfd == sockfd) {
                int newsockfd;
                if ((newsockfd = accept(sockfd, NULL, NULL)) < 0) {
                    fprintf(stderr, "Error: can't accept connection.\n");
                    return 1;
                }

                ev_set.data.fd = newsockfd;
                ev_set.events = EPOLLOUT | EPOLLERR | EPOLLHUP;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, newsockfd, &ev_set) < 0) {
                    fprintf(stderr, "Error: can't add epoll fd.\n");
                    return 1;
                }
            } else if (events[i].events & EPOLLOUT) {
                send_message(curfd);
                close(curfd);
            } else if ((events[i].events & EPOLLHUP) || (events[i].events & EPOLLERR)) {
                close(curfd);
            }
        }
    }
    
    return 0;
}
