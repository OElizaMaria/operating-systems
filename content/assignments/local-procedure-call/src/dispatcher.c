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
    strcpy(args->service_fifo, "../tests/dispatcher/install_req_pipe");
    char path[128];
    // memset(path, 0, 128);
    // strcpy(path, "../tests/");
    while (true) {
        mkfifo(args->service_fifo, 0666);
        args->install_req_fd = open(args->service_fifo, O_RDONLY);
        read(args->install_req_fd, &IRH, sizeof(struct InstallRequestHeader));
        read(args->install_req_fd, args->buffer, IRH.m_IpnLen);

        mkfifo(args->buffer, 0666);
        memset(path, 0, 128);
        strcpy(path, "../tests/");
        strcpy(path, args->buffer);
        args->install_fd = open(args->buffer, O_RDONLY);
        read(args->install_fd, &IH, sizeof(struct InstallHeader));
        read(args->install_fd, args->buffer, IH.m_VersionLen);
        read(args->install_fd, args->buffer, IH.m_CpnLen);
        read(args->install_fd, args->buffer, IH.m_RpnLen);
        read(args->install_fd, args->buffer, IH.m_ApLen);
        printf("IH.m_VersionLen : %d", IH.m_VersionLen);
        printf("IH.m_CpnLen : %d", IH.m_CpnLen);
        printf("IH.m_RpnLen : %d", IH.m_RpnLen);
        printf("IH.m_ApLen : %d", IH.m_ApLen);
        mkfifo("../tests/.pipes/hello_pipe_in", 0666);
        mkfifo("../tests/.pipes/hello_pipe_out", 0666);
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
	return 0;
}
