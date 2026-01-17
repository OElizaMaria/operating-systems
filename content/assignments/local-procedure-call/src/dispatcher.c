// SPDX-License-Identifier: BSD-3-Clause

#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include "protocol/components.h"



typedef struct ServiceArgs {
    pthread_mutex_t mutex;
    int service_fd;
    char service_fifo[32];
} ServiceArgs;

void server_thread(ServiceArgs *args) {
    struct InstallRequestHeader IRH;
    printf("test");

    while (1) {
        pthread_mutex_lock(&args->mutex);
        strcpy(args->service_fifo, ".dispatcher/install_req_pipe");
        mkfifo(args->service_fifo, 0666);
        args->service_fd = open(args->service_fifo, O_RDONLY);
        read(args->service_fd, &IRH, sizeof(struct InstallRequestHeader));
        IRH.m_IpnLen = be32toh(IRH.m_IpnLen);
    }
}


typedef struct ClientArgs {
    pthread_mutex_t mutex;
    int client_fd;
    char client_fifo[32];
} ClientArgs;


void client_thread(ServiceArgs *args) {
    struct ConnectionRequestHeader CRH;
	while(1){
		strcpy(args->service_fifo, ".dispatcher/connection_req_pipe");
        mkfifo(args->service_fifo, 0666);
		args->service_fd = open(args->service_fifo, O_RDONLY);
		read(args->service_fd, &CRH,sizeof(struct ConnectionRequestHeader));
		CRH.m_RpnLen = be32toh(CRH.m_RpnLen);
	}
}


int main(void)
{
 
    ServiceArgs *args = malloc(sizeof(ServiceArgs));
    pthread_mutex_init(&args->mutex, 0);
    pthread_t *service_thread = malloc(sizeof(pthread_t));
    pthread_create(service_thread, NULL, (void *)server_thread, args);
	return 0;
}