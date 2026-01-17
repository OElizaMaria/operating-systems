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

typedef struct ServiceArgs {
    pthread_mutex_t mutex;
    int install_req_fd;
    int install_fd;
    int callpipe_fd;
    int returnpipe_fd;
    char service_fifo[64];
    char buffer[64];
} ServiceArgs;



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
        memset(args->buffer, 0, 64);
        printf("service parsed\n");
    }
}

typedef struct ClientArgs {
    pthread_mutex_t mutex;
    int install_req_fd;
    int install_fd;
    int callpipe_fd;
    int returnpipe_fd;
    char client_fifo[64];
    char buffer[64];
} ClientArgs;


void client_thread(ClientArgs *args) {
    struct ConnectionRequestHeader CRH = {0};
    struct ConnectHeader CH = {0};
    strcpy(args->client_fifo, "../tests/.dispatcher/connection_req_pipe");
    char path[128];
    while (true) {
        memset(args->buffer, 0, 64);
        mkfifo(args->client_fifo, 0666);
        args->install_req_fd = open(args->client_fifo, O_RDONLY);
        read(args->install_req_fd, &CRH, sizeof(struct ConnectionRequestHeader));
        read(args->install_req_fd, args->buffer, CRH.m_RpnLen);
        read(args->install_req_fd, args->buffer, CRH.m_ApLen);

        memset(path, 0, 128);
        strcpy(path, "../tests/");
        strcat(path, args->buffer);
        mkfifo(path, 0666);
        args->install_fd = open(path, O_RDONLY);
        read(args->install_fd, &CH, sizeof(struct ConnectHeader));
        memset(args->buffer, 0, 64);
        read(args->install_fd, args->buffer, CH.m_VersionLen);
        memset(args->buffer, 0, 64);
        read(args->install_fd, args->buffer, be16toh(CH.m_CpnLen));
        memset(path, 0, 128);
        strcpy(path, "../tests/");
        strcat(path, args->buffer);
        mkfifo(path, 0666);
        args->callpipe_fd = open(path, O_WRONLY);
        memset(args->buffer, 0, 64);
        read(args->install_fd, args->buffer, be16toh(CH.m_RpnLen));
        memset(path, 0, 128);
        strcpy(path, "../tests/");
        strcat(path, args->buffer);
        args->returnpipe_fd = open(path, O_RDONLY);
        mkfifo(path, 0666);
        memset(args->buffer, 0, 64);
        read(args->install_fd, args->buffer, be16toh(CH.m_CpnLen));
        memset(args->buffer, 0, 64);
        printf("service parsed\n");
    }
}


int main(void)
{
    ServiceArgs *sevice_args = malloc(sizeof(ServiceArgs));
    pthread_mutex_init(&sevice_args->mutex, 0);
    pthread_t *service_thread = malloc(sizeof(pthread_t));
    pthread_create(service_thread, NULL, (void *)server_thread, sevice_args);
    pthread_join(*service_thread, NULL);

    ClientArgs *client_args = malloc(sizeof(ClientArgs));
    pthread_mutex_init(&client_args->mutex, 0);
    pthread_t *client_thread = malloc(sizeof(pthread_t));
    pthread_create(client_thread, NULL, (void *)client_thread, client_args);
    
    
    pthread_join(*client_thread, NULL);

	return 0;
}