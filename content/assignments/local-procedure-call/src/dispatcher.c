// SPDX-License-Identifier: BSD-3-Clause

#include "protocol/components.h"
#include <endian.h>
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

typedef struct ServiceArgs {
    int install_req_fd;
    int install_fd;
    int callpipe_fd;
    int returnpipe_fd;
    char access_path[64];
    char service_fifo[64];
    char buffer[64];
} ServiceArgs;

typedef struct ClientArgs {
    int connect_req_fd;
    int connect_fd;
    int callpipe_fd;
    int returnpipe_fd;
    char access_path[64];
    char client_fifo[64];
    char buffer[64];
} ClientArgs;

void server_thread(ServiceArgs *args) {
    struct InstallRequestHeader IRH = {0};
    struct InstallHeader IH = {0};
    strcpy(args->service_fifo, "../tests/.dispatcher/install_req_pipe");
    char path[128];
    // memset(path, 0, 128);
    // strcpy(path, "../tests/");
    while (true) {
        memset(args->buffer, 0, 64);
        mkfifo(args->service_fifo, 0666);
        args->install_req_fd = open(args->service_fifo, O_RDONLY);
        read(args->install_req_fd, &IRH, sizeof(struct InstallRequestHeader));
        read(args->install_req_fd, args->buffer, IRH.m_IpnLen);
        memset(path, 0, 128);
        strcpy(path, "../tests/");
        strcat(path, args->buffer);
        mkfifo(path, 0666);
        args->install_fd = open(path, O_RDONLY);
        read(args->install_fd, &IH, sizeof(struct InstallHeader));
        memset(args->buffer, 0, 64);
        read(args->install_fd, args->buffer, IH.m_VersionLen);
        memset(args->buffer, 0, 64);
        read(args->install_fd, args->buffer, be16toh(IH.m_CpnLen));
        memset(path, 0, 128);
        strcpy(path, "../tests/");
        strcat(path, args->buffer);
        mkfifo(path, 0666);
        args->callpipe_fd = open(path, O_WRONLY);
        memset(args->buffer, 0, 64);
        read(args->install_fd, args->buffer, be16toh(IH.m_RpnLen));
        memset(path, 0, 128);
        strcpy(path, "../tests/");
        strcat(path, args->buffer);
        args->returnpipe_fd = open(path, O_RDONLY);
        mkfifo(path, 0666);
        memset(args->buffer, 0, 64);
        read(args->install_fd, args->buffer, be16toh(IH.m_ApLen));
        strcpy(args->access_path, args->buffer);
        memset(args->buffer, 0, 64);
        printf("service parsed\n");
        close(args->install_fd);
        close(args->install_req_fd);
    }
}

void client_thread(ClientArgs *args) {
    struct ConnectionRequestHeader CRH = {0};
    struct ConnectHeader CH = {0};
    strcpy(args->client_fifo, "../tests/.dispatcher/connection_req_pipe");
    char path[128];
    // memset(path, 0, 128);
    // strcpy(path, "../tests/");
    while (true) {
        memset(args->buffer, 0, 64);
        mkfifo(args->client_fifo, 0666);
        args->connect_req_fd = open(args->client_fifo, O_RDONLY);
        read(args->connect_req_fd, &CRH, sizeof(struct ConnectionRequestHeader));
        read(args->connect_req_fd, args->buffer, be32toh(CRH.m_RpnLen));
        memset(path, 0, 128);
        strcpy(path, "../tests/");
        strcat(path, args->buffer);
        // printf("test1\n");
        printf("path: %s\n", path);
        mkfifo(path, 0666);
        args->connect_fd = open(".pipes/connect_pipe0", O_WRONLY);
        read(args->connect_req_fd, args->buffer, be32toh(CRH.m_ApLen));
    }
}

int main(void)
{
    pthread_t *threads = calloc(2, sizeof(pthread_t));

    ServiceArgs *sevice_args = malloc(sizeof(ServiceArgs));
    pthread_create(&threads[0], NULL, (void *)server_thread, sevice_args);

    ClientArgs *client_args = malloc(sizeof(ClientArgs));
    pthread_create(&threads[1], NULL, (void *)client_thread, client_args);

    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

	return 0;
}
