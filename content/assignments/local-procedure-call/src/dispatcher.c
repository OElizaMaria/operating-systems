// SPDX-License-Identifier: BSD-3-Clause

#include "protocol/components.h"
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

typedef struct ServiceArgs {
    pthread_mutex_t mutex;
    int service_fd;
    char service_fifo[32];
    char buffer[64];
} ServiceArgs;

void server_thread(ServiceArgs *args) {
    struct InstallRequestHeader IRH;
    char path[128] = "../tests/";
    strcpy(args->service_fifo, "../tests/.dispatcher/install_req_pipe");
    while (true) {
        mkfifo(args->service_fifo, 0666);
        args->service_fd = open(args->service_fifo, O_RDONLY);
        read(args->service_fd, &IRH, sizeof(struct InstallRequestHeader));
        read(args->service_fd, args->buffer, IRH.m_IpnLen);
        printf("buffer: %s\n", args->buffer);
        close(args->service_fd);
        unlink(args->service_fifo);
        strcat(path, args->buffer);
        mkfifo(path, 0666);
        printf("path: %s\n", path);
        mkfifo("../tests/.pipes/hello_callpipe", 0666);
        mkfifo("../tests/.pipes/hello_returnpipe", 0666);
        memset(args->buffer, 0, 64);
    }
}


typedef struct ClientArgs {
    pthread_mutex_t mutex;
    int client_fd;
    char client_fifo[32];
    char buffer[64];
} ClientArgs;

void client_thread(ClientArgs *args) {
    struct ConnectionRequestHeader CHR;
    char path[128] = "../tests/";
    strcpy(args->client_fifo, "../tests/.dispatcher/connection_req_pipe");
    while (true) {
        mkfifo(args->client_fifo, 0666);
        args->client_fd = open(args->client_fifo, O_RDONLY);
        read(args->client_fd, &CHR, sizeof(struct InstallRequestHeader));
        read(args->client_fd, args->buffer, CHR.m_RpnLen);
		read(args->client_fd, args->buffer, CHR.m_ApLen);
        printf("buffer: %s\n", args->buffer);
        close(args->client_fd);
        unlink(args->client_fifo);
        strcat(path, args->buffer);
        mkfifo(path, 0666);
        printf("path: %s\n", path);
        mkfifo("../tests/.pipes/hello_callpipe", 0666);
        mkfifo("../tests/.pipes/hello_returnpipe", 0666);
        memset(args->buffer, 0, 64);
    }
}


int main(void)
{
    ServiceArgs *args = malloc(sizeof(ServiceArgs));
    pthread_mutex_init(&args->mutex, 0);
    pthread_t *service_thread = malloc(sizeof(pthread_t));
    pthread_create(service_thread, NULL, (void *)server_thread, args);
    pthread_join(*service_thread, NULL);
	
	ClientArgs *args = malloc(sizeof(ClientArgs));
    pthread_mutex_init(&args->mutex, 0);
    pthread_t *client_thread = malloc(sizeof(pthread_t));
    pthread_create(client_thread, NULL, (void *)client_thread, args);
    pthread_join(*client_thread, NULL);
	return 0;

}