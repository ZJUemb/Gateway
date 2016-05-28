/*************************************************************************
 > File Name: src/kernel.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Fri 13 May 2016 07:28:27 PM CST
 ************************************************************************/

#include "stdio.h"
#include <stdlib.h>
#include "string.h"
#include "sys/syscall.h"
#include <pthread.h>
#include <asm/errno.h>
#include <cJSON.h>
#include "EmbGW.h"

/* #include <EmbGW.h> */
/* void Sensor870_Handler(void *); */
/* void Sensor870_Recv() { */
    /* if (cnt < 14) { */
        /* cnt += read(sensor_fd[0], recv_buf+cnt, 14-cnt); */
    /* } */
    /* printf("cnt = %d\n", cnt); */
    /* wait_flag = 1; */

    /* if (cnt >= 14) { */
        /* int i; */
        /* cnt = 0; */

        /* [> handle data <] */
        /* Sensor870_Handler(recv_buf); */
        /* bzero(recv_buf, sizeof(recv_buf)); */
    /* } */

/* } */
/* void IO_detect() { */
    /* printf("Thread %d is waiting for I/O...\n", syscall(SYS_gettid)); */
    /* int cnt = 0; */
    /* char buf[255]; */
    /* while (1) { */
        /* if (cnt < 14) { */
            /* cnt += read(sensor_fd[0], buf+cnt, 14 - cnt); */
        /* } */
        /* if (cnt >= 14) { */
            /* cnt = 0; */
            /* [> handle data <] */
            /* Sensor870_Handler(buf); */
            /* bzero(buf, sizeof(buf)); */
        /* } */
    /* } */
/* } */

/* void Report_Data(void *param) { */
    /* unsigned int i; */
    /* [> char *sensor_data = (char *)param; <] */
    /* char sensor_data[128]; */
    /* { */
        /* *(unsigned int *)sensor_data = 870; */
        /* *((unsigned int *)sensor_data + 1) = 1020; */
        /* *((unsigned char *)sensor_data + 8) = 2; */
        /* *((unsigned char *)sensor_data + 9) = 15; */
        /* *((unsigned long *)((char *)sensor_data + 10)) = 13412353; */
    /* } */
    /* Sensor870_Handler(sensor_data); */
    /* [> BIN or HTTP <] */
    /* for (i = 0; i < server_num; i++) { */
        /* if (server_type[i] == BIN) { */
            /* send by binary protocol */
            /* [> BIN_Send(0, sensor_data); <] */
        /* } */
        /* else if (server_type[i] == HTTP) { */
            /* send by http server */
            /* [> HTTP_Send(0, sensor_data); <] */
        /* } */
    /* } */
/* } */

void Sensor870_Handler(void *peer) {
    Sensor *sensor = (Sensor *)peer;

    pthread_mutex_lock(&sensor->lock);

    pthread_mutex_unlock(&sensor->lock);
}
void Sensor883_Handler(void *peer) {
}
void ServerBIN_Handler(void *peer) {
}

bool isLocked(int fd) {
    if (pthread_mutex_trylock(fdLookup[fd].lock) == EBUSY) // locked
        return TRUE;
    pthread_mutex_unlock(fdLookup[fd].lock);
    return FALSE;
}



void GTWY_Work() {
    // TODO
    // select --> Select
    alarm(10); // timed task
    while (1) {
        int sel_fd;
        fd_set read_set = myGateway.allfd;
        Select(myGateway.maxfd+1, &read_set, NULL, NULL, NULL);
        for (sel_fd = 0; sel_fd <= myGateway.maxfd; sel_fd++) {
            // if ready?
            if (!FD_ISSET(sel_fd, &read_set))
                continue;
            // if locked?
            if (isLocked(fd))
                continue;
            // ok now
            pthread_mutex_lock(fdLookup[i].lock);
            if (fdLookup[fd].type == SERVER)
                thpool_add_work(thpool, fdLookup[fd].peer->handler, (void *)fdLookup[fd].peer);
            else if (fdLookup[fd].type == SENSOR)
                thpool_add_work(thpool, Sensor_detect, (void *)&fdLookup[fd]);
        }
    }
}
void Save_Exit() {
    // TODO
    tcsetattr(sensor_set[0].fd, TCSANOW, &sensor_set[0].oldtio);
    Close(sensor_set[0].fd);
    exit(0);
}
