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

#include <time.h>
static unsigned int getTime(){
    return (unsigned int)time(NULL);
}

char *Sensor870_Handler(fdLUT *lut, int fd, char *buf) {
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
    int device_id;
    int seq_number;
    char data;
    int timestamp;

    // decode
    {
        int i;
        for (i = 0; i < 14; i++)
            printf("%02x ", *((char *)buf + i));
        printf("\n");
        device_id = *((int *)buf);
        seq_number = *((int *)buf + 1);
        data = *((char *)buf + 9);
        timestamp = *((int *)((char *)buf + 10));
    }

    char flag = 1;
    // check package correctness
    {
        // check whether is old package
        int old = ((Sensor *)lut->peer->owner)->old_seq;
        if (old != -1 || old != seq_number + 1)
            flag = 0;
        else if (data & 0xF0 != 0)
            flag = 0;
        else{
            int now = getTime();    
            if (now > timestamp + 100 || now < timestamp - 100)
                flag = 0;
        }
    }

    // reply
    {
        char reply_buf[32];
        int ack_number = seq_number + 1;
        char reply_type = 0x00, reply_data = flag; // data: 0x01 for ACk, 0x00 for NAK

        *(int *)reply_buf = device_id;
        *((int *)reply_buf + 1) = ack_number;
        *((char *)reply_buf + 8) = reply_type;
        *((char *)reply_buf + 9) = reply_data;
        *((int *)((char *)reply_buf + 10)) = timestamp;

        Written(fd, reply_buf, 14);
        pthread_mutex_unlock(&lut->lock);
    }

    if (flag == 0)
        return NULL;

    // pack into JSON
    {
        cJSON *root, *payload;
        root = cJSON_CreateObject();
        payload = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "auth_id", myGateway.id);
        cJSON_AddStringToObject(root, "auth_key", myGateway.auth_key);
        cJSON_AddNumberToObject(root, "device_id", device_id);
        cJSON_AddItemToObject(root, "payload", payload);
        {
            cJSON_AddNumberToObject(payload, "timestamp", timestamp);
            if ((data & 0x8) == 0)
                cJSON_AddFalseToObject(payload, "Activate LED bit");
                        else
                cJSON_AddTrueToObject(payload, "Activate LED bit");
            if ((data & 0x4) == 0)
                cJSON_AddFalseToObject(payload, "Sound");
            else
                cJSON_AddTrueToObject(payload, "Sound");
            if ((data & 0x2) == 0)
                cJSON_AddFalseToObject(payload, "Light");
            else
                cJSON_AddTrueToObject(payload, "Light");
            if ((data & 0x1) == 0)
                cJSON_AddFalseToObject(payload, "LEDState");
            else
                cJSON_AddTrueToObject(payload, "LESState");
        }
        char *buf_tmp = cJSON_Print(root);
	bzero(buf, 512);
        strcpy(buf, buf_tmp);
        // TODO Error
    }
    return buf;
}

char *Sensor883_Handler(fdLUT *lut, int fd, char *buf) {
    /*
     * package:
     * +---------------+----------------+---------+---------+-----------+---------------+
     * | device_id(32) | seq_number(32) | type(8) | data(8) | value(32) | timestamp(32) |
     * +---------------+----------------+---------+---------+-----------+---------------+
     *
     * type:
     * +---+---+---+---+---+---+---+---+
     * | 0 | 0 | 0 | 0 | 0 | 0 | x | x |
     * +---+---+---+---+---+---+---+---+
     * 0x2 for data, 0x1 for command, 0x0 for response(ACK/NAK)
     *
     * data:
     * +---+---+---+---+-------+
     * | 0 | 0 | 0 | 0 | magic |
     * +---+---+---+---+-------+
     *
     */
    int device_id;
    int seq_number;
    char data;
    int value, timestamp;

    // decode
    {
        int i;
        for (i = 0; i < 18; i++)
            printf("%02x ", *((char *)buf + i));
        printf("\n");
        device_id = *((int *)buf);
        seq_number = *((int *)buf + 1);
        data = *((char *)buf + 9);
        value = *((int *)((char *)buf + 10));
        timestamp = *((int *)((char *)buf + 14));
    }

    char flag = 1;
    // check package correctness
    {
        // check whether is old package
        int old = ((Sensor *)lut->peer->owner)->old_seq;
        if (old != -1 || old != seq_number + 1)
            flag = 0;
        else if (data & 0xF0 != 0)
            flag = 0;
        else{
            int now = getTime();    
            if (now > timestamp + 100 || now < timestamp - 100)
                flag = 0;
        }
    }

    // reply
    {
        char reply_buf[32];
        int ack_number = seq_number + 1;
        char reply_type = 0x00, reply_data = flag; // data: 0x01 for ACk, 0x00 for NAK

        *(int *)reply_buf = device_id;
        *((int *)reply_buf + 1) = ack_number;
        *((char *)reply_buf + 8) = reply_type;
        *((char *)reply_buf + 9) = reply_data;
        *((int *)((char *)reply_buf + 10)) = timestamp;

        Written(fd, reply_buf, 14);
        pthread_mutex_unlock(&lut->lock);
    }

    if (flag == 0)
        return NULL;

    // pack into JSON
    {
        cJSON *root, *payload;
        root = cJSON_CreateObject();
        payload = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "auth_id", myGateway.id);
        cJSON_AddStringToObject(root, "auth_key", myGateway.auth_key);
        cJSON_AddNumberToObject(root, "device_id", device_id);
        cJSON_AddItemToObject(root, "payload", payload);
        {
            cJSON_AddNumberToObject(payload, "timestamp", timestamp);
            cJSON_AddNumberToObject(payload, "data", (int)data);
            cJSON_AddNumberToObject(payload, "value", value);
        }
        char *buf_tmp = cJSON_Print(root);
        strncpy(buf, buf_tmp, strlen(buf_tmp));
    }
    return buf;
}
void ServerBIN_Handler(Server *server) {
    char buf[255];
    bzero(buf, 255);
    Read(server->sockfd, buf, sizeof(buf));  
    printf("Server: %s\n", buf); 
    if (strcmp(buf, "on\n") == 0) {
        *(int *)buf = 883;
        *((int *)buf + 1) = 12;
        *((char *)buf + 8) = 0x01;
        *((char *)buf + 9) = 0x1;
        *((int *)((char *)buf + 10)) = 0x12345678;
    }
    else if (strcmp(buf, "off\n") == 0) {
        *(int *)buf = 883;
        *((int *)buf + 1) = 12;
        *((char *)buf + 8) = 0x01;
        *((char *)buf + 9) = 0x0;
        *((int *)((char *)buf + 10)) = 0x12345678;
    }
    Written(sensor_set[0].fd, buf, 14);
}
void ServerHTTP_Handler(Server *server) {
}

void *Server_Handler(void *arg) {
    fdLUT *lut = (fdLUT *)arg;
    int sockfd = (lut - fdLookup) / (sizeof(fdLUT));
    Server *server = (Server *)(lut->peer->owner);
    char buf[512];

    if (server->type == HTTP) {
	
	
    }
    else { // BIN
	ServerBIN_Handler(server);
    }
    pthread_mutex_unlock(&lut->lock);
}

void *Sensor_Handler(void *arg) {
    printf("Sensor Data Coming...\n");
    fdLUT *lut = (fdLUT *)arg;
    int fd = lut - &fdLookup[0];

    int cnt = 0, device_id;
    int i;
    char type;
    char buf[512];
    while (cnt < 9)
        cnt += read(fd, buf+cnt, 9 - cnt);
    /*
     * Package Identification
     */
    device_id = *(int *)buf;
    type = *((char *)buf + 8);
    // TODO add device_id in configuration
    if (device_id == 0x870) {
        switch (type) {
            case 0x00: // ACK or NAK
                break;
            case 0x01:
                // TODO
                // Error: Gateway to Sensor
                break;
            case 0x02: // data
                while (cnt < 14) // package length = 14
                    cnt += read(fd, buf+cnt, 14 - cnt);
                Sensor870_Handler(lut, fd, buf);
		for (i = 0; i < myGateway.server_num; i++) {
		    if (server_set[i].type == HTTP)
			HTTP_Send(server_set[i].sock_addr, server_set[i].ipv4_addr, buf);
		    else if (server_set[i].type == BIN); // TODO
		}
                break;
        }
    }
    else if (device_id == 883) { // TODO
        switch (type) {
            case 0x00: // ACK or NAK
		while (cnt < 14) {
		    cnt+= read(fd, buf+cnt, 14 - cnt);
		} 
		int i = 0;
		for (; i < 14; i++)
		    printf("%02x ", buf[i]);
		printf("\n");
                break;
            case 0x01:
                // TODO
                // Error: Gateway to Sensor
                break;
            case 0x02: // data
                while (cnt < 18) // package length = 18
                    cnt += read(fd, buf+cnt, 18 - cnt);
                Sensor883_Handler(lut, fd, buf);
/*		for (i = 0; i < myGateway.server_num; i++) {
		    if (server_set[i].type == HTTP)
			HTTP_Send(server_set[i].sock_addr, server_set[i].ipv4_addr, buf);
		    else if (server_set[i].type == BIN); // TODO
		}*/
                break;
        }
    }
    else {
        // TODO
        // Error
    }
    pthread_mutex_unlock(&lut->lock);
}

bool isLocked(int fd) {
    if (pthread_mutex_trylock(&fdLookup[fd].lock) == EBUSY) // locked
        return TRUE;
    pthread_mutex_unlock(&fdLookup[fd].lock);
    return FALSE;
}



void GTWY_Work() {
    // TODO
    // select --> Select
    //alarm(10); // timed task
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
            pthread_mutex_lock(&fdLookup[sel_fd].lock);
            if (fdLookup[sel_fd].type == SERVER)
//                thpool_add_work(myGateway.thpool, Server_Handler, (void *)&fdLookup[sel_fd]);
		Server_Handler(&fdLookup[sel_fd]);
            else if (fdLookup[sel_fd].type == SENSOR)
//                thpool_add_work(myGateway.thpool, Sensor_Handler, (void *)&fdLookup[sel_fd]);
		  Sensor_Handler(&fdLookup[sel_fd]);
        }
    }
}
void Save_Exit() {
    // TODO
    tcsetattr(sensor_set[0].fd, TCSANOW, &sensor_set[0].oldtio);
    Close(sensor_set[0].fd);
    exit(0);
}
