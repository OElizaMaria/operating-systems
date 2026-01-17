// SPDX-License-Identifier: BSD-3-Clause

#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include "protocol/components.h"


void* foo(void* arg) {
    printf("Thread de asteptare creat.");
    return NULL;
 }

int main(void)
{
    pthread_t asteapta_thread_req;
	int fd;
	struct InstallRequestHeader hdr;
	
	// cream pipe ul pt req unde e scris

	 char * myfifo = "./dispatcher/install_req_pipe";
    mkfifo(myfifo, 0666);

    pthread_create(&asteapta_thread_req, NULL, foo, NULL);
	while(1){
		// pune un mutex sa nu fie busy waiting
		fd = open(myfifo, O_RDONLY);
		uint16_t cat_citim;
		read(fd, cat_citim, sizeof(uint16_t));
		hdr.m_IpnLen = cat_citim;
		
		// citeste primu int16 si pune l in structura 

	}

	return 0;
}
