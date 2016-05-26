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
/* [> void Sensor870_Recv() { <] */
    /* [> if (cnt < 14) { <] */
        /* [> cnt += read(sensor_fd[0], recv_buf+cnt, 14-cnt); <] */
    /* [> } <] */
    /* [> printf("cnt = %d\n", cnt); <] */
    /* [> wait_flag = 1; <] */

    /* [> if (cnt >= 14) { <] */
		/* [> int i; <] */
		/* [> cnt = 0; <] */

        /* [> [> handle data <] <] */
        /* [> Sensor870_Handler(recv_buf); <] */
		/* [> bzero(recv_buf, sizeof(recv_buf)); <] */
    /* [> } <] */

/* [> } <] */
/* [> void IO_detect() { <] */
    /* [> printf("Thread %d is waiting for I/O...\n", syscall(SYS_gettid)); <] */
    /* [> int cnt = 0; <] */
    /* [> char buf[255]; <] */
    /* [> while (1) { <] */
        /* [> if (cnt < 14) { <] */
            /* [> cnt += read(sensor_fd[0], buf+cnt, 14 - cnt); <] */
        /* [> } <] */
        /* [> if (cnt >= 14) { <] */
            /* [> cnt = 0; <] */
            /* [> [> handle data <] <] */
            /* [> Sensor870_Handler(buf); <] */
            /* [> bzero(buf, sizeof(buf)); <] */
        /* [> } <] */
    /* [> } <] */
/* [> } <] */

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
            /* // send by binary protocol */
            /* [> BIN_Send(0, sensor_data); <] */
        /* } */
        /* else if (server_type[i] == HTTP) { */
            /* // send by http server */
            /* [> HTTP_Send(0, sensor_data); <] */
        /* } */
    /* } */
/* } */

/* void Sensor870_Handler(void *message) { */
    /*
     * package:
     * +---------------+----------------+---------+---------+---------------+
     * | device_id(32) | seq_number(32) | type(8) | data(8) | timestamp(32) |
     * +---------------+----------------+---------+---------+---------------+
     *
     * type:
     * +---+---+---+---+---+---+---+---+
     * | 0 | 0 | 0 | 0 | 0 | 0 | x | x |
     * +---+---+---+---+---+---+---+---+
     * 0x2 for data, 0x1 for command, 0x0 for response(ACK/NAK)
     *
     * data:
     * +---+---+---+---+------+------+------+------+
     * | 0 | 0 | 0 | 0 | LED1 | LED2 | LED3 | LED4 |
     * +---+---+---+---+------+------+------+------+
     *
     */
    /* unsigned int device_id; */
    /* unsigned int seq_number; */
    /* unsigned char data; */
    /* unsigned int timestamp; */

    /* // decode */
    /* { */
		/* [> int i; <] */
		/* for (i = 0; i < 14; i++) */
			/* printf("%02x ", *((unsigned char *)message + i)); */
        /* printf("\n"); */
        /* device_id = *((unsigned int *)message); */
        /* seq_number = *((unsigned int *)message + 1); */
        /* unsigned char type = *((unsigned char *)message + 8); */
        /* if (type == 0x2) { // data */
            /* data = *((unsigned char *)message + 9); */
            /* timestamp = *((unsigned int *)((char *)message + 10)); */
        /* } */
        /* else { */
            /* // TODO */
            /* // error data */
        /* } */
    /* } */

    /* cJSON *root, *payload; */
    /* // pack into JSON */
    /* { */
        /* root = cJSON_CreateObject(); */
        /* payload = cJSON_CreateObject(); */
        /* cJSON_AddNumberToObject(root, "auth_id", gateway_id); */
        /* cJSON_AddStringToObject(root, "auth_key", auth_key); */
        /* cJSON_AddNumberToObject(root, "device_id", device_id); */
        /* cJSON_AddItemToObject(root, "payload", payload); */
        /* { */
            /* cJSON_AddNumberToObject(payload, "timestamp", timestamp); */
            /* if ((data & 0x8) == 0) */
                /* cJSON_AddFalseToObject(payload, "LED1"); */
            /* else */
                /* cJSON_AddTrueToObject(payload, "LED1"); */
            /* if ((data & 0x4) == 0) */
                /* cJSON_AddFalseToObject(payload, "LED2"); */
            /* else */
                /* cJSON_AddTrueToObject(payload, "LED2"); */
            /* if ((data & 0x2) == 0) */
                /* cJSON_AddFalseToObject(payload, "LED3"); */
            /* else */
                /* cJSON_AddTrueToObject(payload, "LED3"); */
            /* if ((data & 0x1) == 0) */
                /* cJSON_AddFalseToObject(payload, "LED4"); */
            /* else */
                /* cJSON_AddTrueToObject(payload, "LED4"); */
        /* } */
        /* char *buf = cJSON_Print(root); */
        /* HTTP_Send(0, buf); */
    /* } */
/* } */

bool isLocked(int fd) {
    pthread_mutex_t *lock_tmp;
    Dict dict_tmp = myGateway.dict[fd];
    if (dict_tmp.type == SERVER)
        lock_tmp = &(((Server *)(dict_tmp.owner))->lock);
    else if (dict_tmp.type == SENSOR)
        lock_tmp = &(((Sensor *)(dict_tmp.owner))->lock);
    else
        ; // TODO error log

    if (pthread_mutex_trylock(lock_tmp) == EBUSY) // locked
        return FALSE;
    else
        pthread_mutex_unlock(lock_tmp);
    return TRUE;
}

void IO_Handler(int fd) {

}

void GTWY_Work() {
    // TODO
    // select --> Select
    alarm(10); // timed task
    /* thpool_add_work(thpool, (void *)IO_detect, NULL); */
    while (1) {
        int sel_fd;
        fd_set read_set = myGateway.allfd;
        Select(myGateway.maxfd+1, &read_set, NULL, NULL, NULL);
        for (sel_fd = 0; sel_fd <= myGateway.maxfd; sel_fd++) {
            // if ready?
            if (!FD_ISSET(sel_fd, &read_set))
                continue;
            // if locked?
            if (isLocked(sel_fd))
                continue;
            // ok now
            IO_Handler(sel_fd);
        }
    }
}
void Save_Exit() {
    // TODO
    tcsetattr(sensor_set[0].fd, TCSANOW, &sensor_set[0].oldtio);
    Close(sensor_set[0].fd);
    exit(0);
}
