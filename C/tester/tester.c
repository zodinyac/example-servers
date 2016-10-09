#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

enum {
    ARGV_ADDRESS = 1,
    ARGV_PORT = 2,
    ARGV_THREADS = 3,
    ARGV_REQUESTS = 4
};

long requests;
pthread_mutex_t requests_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *thread_worker(void *arg)
{
    struct addrinfo *r_ptr = NULL;
    while (1) {
        pthread_mutex_lock(&requests_mutex);
        long new_requests = (requests > 0 ? requests-- : 0);
        pthread_mutex_unlock(&requests_mutex);
        
        if (new_requests == 0) {
            break;
        }

        for (r_ptr = (r_ptr ? r_ptr : arg); r_ptr != NULL; r_ptr = r_ptr->ai_next) {
            int sfd = socket(r_ptr->ai_family, r_ptr->ai_socktype, r_ptr->ai_protocol);
            if (sfd < 0) {
                continue;
            }
            if (connect(sfd, r_ptr->ai_addr, r_ptr->ai_addrlen) < 0) {
                close(sfd);
                continue;
            }
            
            // buffer (128) + null terminator (1)
            char buf[128 + 1] = {0};
            ssize_t byte_read, pos = 0;
            while ((byte_read = recv(sfd, buf + pos, (sizeof(buf) - 1) - pos, 0)) > 0) {
                pos += byte_read;
            }
            printf("%s\n", buf);
            
            close(sfd);
            break;
        }
        if (r_ptr == NULL) {
            fprintf(stderr, "Error: could not connect.\n");
            exit(1);
        }
    }
    
    return NULL;
}

int main(int argc, const char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Usage: ./tester <address> <port> <count of threads> <count of requests>.\n");
        return 1;
    }
    
    struct addrinfo hints = {0};
    hints.ai_socktype = SOCK_STREAM;
    
    struct addrinfo *r;
    int ret = getaddrinfo(argv[ARGV_ADDRESS], argv[ARGV_PORT], &hints, &r);
    if (ret < 0) {
        fprintf(stderr, "Error: %s.\n", gai_strerror(ret));
        return 1;
    }
    
    long threads = strtol(argv[ARGV_THREADS], NULL, 10);
    requests = strtol(argv[ARGV_REQUESTS], NULL, 10);
    if (threads < 1 || requests < 1) {
        fprintf(stderr, "Error: count of threads and requests must be > 0.\n");
        return 1;
    }

    pthread_t tid[threads];
    for (long i = 0; i < threads; i++) {
        pthread_create(&tid[i], NULL, thread_worker, r);
    }
    for (long i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }
    
    freeaddrinfo(r);
    
    return 0;
}
