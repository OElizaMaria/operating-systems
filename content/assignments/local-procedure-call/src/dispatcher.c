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
} ServiceArgs;

void server_thread(ServiceArgs *args) {
    struct InstallRequestHeader IRH;
    pthread_mutex_lock(&args->mutex);
    strcpy(args->service_fifo, "../tests/.dispatcher/install_req_pipe");
    mkfifo(args->service_fifo, 0666);
    args->service_fd = open(args->service_fifo, O_RDONLY);
    read(args->service_fd, &IRH, sizeof(struct InstallRequestHeader));
    IRH.m_IpnLen = be32toh(IRH.m_IpnLen);
    printf("%s\n", args->service_fifo);
    pthread_mutex_unlock(&args->mutex);

}

int main(void)
{
    // threads are just memory
    ServiceArgs *args = malloc(sizeof(ServiceArgs));
    pthread_mutex_init(&args->mutex, 0);
    pthread_t *service_thread = malloc(sizeof(pthread_t));
    pthread_create(service_thread, NULL, (void *)server_thread, args);
    pthread_join(*service_thread, NULL);
	return 0;
}
