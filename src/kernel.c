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
#include <unistd.h>
#include <asm/errno.h>
#include <cJSON.h>
#include "EmbGW.h"
#include <time.h>
#include <BinaryProtocol.h>

extern SensorConfig binaryReportConfigs[SensorMax];
extern SensorConfig binaryCtrlConfigs[SensorMax];

static unsigned int getTime() {
    return (unsigned int)time(NULL);
}
static float getFloat(char* buffer) {
	return (buffer[0]-'0')*10 + (buffer[1]-'0') + 0.1*(buffer[3]-'0') + 0.01*(buffer[4]-'0');
}

int Sensor870_Handler(fdLUT *lut, int fd, char *buf, ReportPacket *BINbuf) {
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
        device_id = *((int *)buf);
        seq_number = *((int *)buf + 1);
        data = *((char *)buf + 9);
        timestamp = *((int *)((char *)buf + 10));
    }

    char flag = 1;
    // check package correctness
    /* { */
        /* // check whether is old package */
        /* int old = ((Sensor *)lut->peer->owner)->old_seq; */
        /* if (old != -1 || old != seq_number + 1) */
            /* flag = 0; */
        /* else if (data & 0xF0 != 0) */
            /* flag = 0; */
        /* else{ */
            /* int now = getTime(); */
            /* if (now > timestamp + 100 || now < timestamp - 100) */
                /* flag = 0; */
        /* } */
    /* } */

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
        return -1;

    cJSON *root, *payload;
    // pack into JSON
    {
        root = cJSON_CreateObject();
        payload = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "auth_id", myGateway.id);
        cJSON_AddStringToObject(root, "auth_key", myGateway.auth_key);
        cJSON_AddNumberToObject(root, "device_id", device_id);
        cJSON_AddNumberToObject(root, "report_id", 37);
        cJSON_AddItemToObject(root, "payload", payload);
        {
            if ((data & 0x8) == 0)
                cJSON_AddNumberToObject(payload, "ActivateLEDbit", 0);
                        else
                cJSON_AddNumberToObject(payload, "ActivateLEDbit", 1);
            if ((data & 0x4) == 0)
                cJSON_AddNumberToObject(payload, "Sound", 0);
            else
                cJSON_AddNumberToObject(payload, "Sound", 1);
            if ((data & 0x2) == 0)
                cJSON_AddNumberToObject(payload, "Light", 0);
            else
                cJSON_AddNumberToObject(payload, "Light", 1);
            if ((data & 0x1) == 0)
                cJSON_AddNumberToObject(payload, "LEDState", 0);
            else
                cJSON_AddNumberToObject(payload, "LEDState", 1);
        }
        char *buf_tmp = cJSON_Print(root);
	    bzero(buf, 512);
        strcpy(buf, buf_tmp);
        // TODO Error
    }

    // pack into BIN
    {
    	BINbuf->msgType = MSG_REPORT;
    	BINbuf->device_id = device_id;
    	bzero(BINbuf->payload, sizeof(BINbuf->payload));
    	ParseJson(cJSON_Print(payload), GuessReportSensor(device_id), BINbuf->payload);
    }
    return 0;
}
int Sensor883_Handler(fdLUT *lut, int fd, char *buf, ReportPacket *BINbuf) {
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
        device_id = *((int *)buf);
        seq_number = *((int *)buf + 1);
        data = *((char *)buf + 9);
        value = *((int *)((char *)buf + 10));
        timestamp = *((int *)((char *)buf + 14));
    }

    char flag = 1;
    // check package correctness
    /* { */
        /* // check whether is old package */
        /* int old = ((Sensor *)lut->peer->owner)->old_seq; */
        /* if (old != -1 || old != seq_number + 1) */
            /* flag = 0; */
        /* else if (data & 0xF0 != 0) */
            /* flag = 0; */
        /* else{ */
            /* int now = getTime(); */
            /* if (now > timestamp + 100 || now < timestamp - 100) */
                /* flag = 0; */
        /* } */
    /* } */

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
        return -1;

    cJSON *root, *payload;
    // pack into JSON
    {
        root = cJSON_CreateObject();
        payload = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "auth_id", myGateway.id);
        cJSON_AddStringToObject(root, "auth_key", myGateway.auth_key);
        cJSON_AddNumberToObject(root, "device_id", device_id);
        cJSON_AddNumberToObject(root, "report_id", 37);
        cJSON_AddItemToObject(root, "payload", payload);
        {
            cJSON_AddNumberToObject(payload, "data", (int)data);
            cJSON_AddNumberToObject(payload, "value", value);
        }
        char *buf_tmp = cJSON_Print(root);
        strncpy(buf, buf_tmp, strlen(buf_tmp));
    }

    // pack into BIN
    {
    	BINbuf->msgType = MSG_REPORT;
    	BINbuf->device_id = device_id;
    	bzero(BINbuf->payload, sizeof(BINbuf->payload));
    	ParseJson(cJSON_Print(payload), GuessReportSensor(device_id), BINbuf->payload);
    }
    return 0;
}
int Sensor872_Handler(char *buf, ReportPacket *BINbuf) {
    int device_id;
    char *buffer = buf + 1;
    int len = strlen(buffer);
	if (len != 16) {
		return -1;
	}
	float temp = getFloat(buffer+1);
	printf("temp = %f\n", temp);
	float humid = getFloat(buffer+6);
	char state[2] = {buffer[11], '\0'};
	char id[3] = {buffer[12], buffer[13], '\0'};
	unsigned char check = buffer[14];
	int sum = 0, i;
	for (i = 0; i < len-2; i++) {
		sum += buffer[i];
	}
	sum %= 128;
	/* printf("Len: %d, Temp: %f, Humid: %f State: %c id: %s, Check: %d, sum: %d\n", len, temp, humid, state, id, check, sum); */
	if (sum != check) {
		/* printf("Data and Check bit not match!\n"); */
		return -1;
	}

    /* device_id = atoi(id); */
    device_id = Sensor872_ID;
    cJSON *root, *payload;
    // pack into JSON
    {
        root = cJSON_CreateObject();
        payload = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "auth_id", myGateway.id);
        cJSON_AddStringToObject(root, "auth_key", myGateway.auth_key);
        cJSON_AddNumberToObject(root, "device_id", device_id);
        cJSON_AddNumberToObject(root, "report_id", 21);
        cJSON_AddItemToObject(root, "payload", payload);
        {
            cJSON_AddNumberToObject(payload, "temperature", temp);
            cJSON_AddNumberToObject(payload, "humidity", humid);
            cJSON_AddStringToObject(payload, "state", state);
        }
        char *buf_tmp = cJSON_Print(root);
	    bzero(buf, 512);
        strcpy(buf, buf_tmp);
        // TODO Error
    }

    // pack into BIN
    {
    	BINbuf->msgType = MSG_REPORT;
    	BINbuf->device_id = device_id;
    	bzero(BINbuf->payload, sizeof(BINbuf->payload));
    	ParseJson(cJSON_Print(payload), GuessReportSensor(device_id), BINbuf->payload);
    }
    return 0;
}
int Sensor875_Handler(char *buf, ReportPacket *BINbuf) {
    int device_id;
    char *buffer = buf + 1;
    int len = strlen(buffer);
	if (len != 16) {
		return -1;
	}
	char type[] = {buffer[0], 0};
	char result[] = {buffer[13], 0};
	char pid[5], time[9];
	memcpy(pid, buffer+1, 4);
	memcpy(time, buffer+5, 8);
	pid[4] = 0; time[8] = 0;
	unsigned char check = buffer[14];
	int sum = 0, i;
	for (i = 0; i < len-2; i++) {
		sum += buffer[i];
	}
	sum %= 128;
	if (sum != check) {
		/* printf("Data and Check bit not match!\n"); */
		return -1;
	}

    device_id = Sensor875_ID;
    cJSON *root, *payload;
    // pack into JSON
    {
        root = cJSON_CreateObject();
        payload = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "auth_id", myGateway.id);
        cJSON_AddStringToObject(root, "auth_key", myGateway.auth_key);
        cJSON_AddNumberToObject(root, "device_id", device_id);
        cJSON_AddNumberToObject(root, "report_id", 31);
        cJSON_AddItemToObject(root, "payload", payload);
        {
            cJSON_AddStringToObject(payload, "type", type);
            cJSON_AddStringToObject(payload, "pid", pid);
            cJSON_AddStringToObject(payload, "time", time);
            cJSON_AddStringToObject(payload, "result", result);
        }
        char *buf_tmp = cJSON_Print(root);
	    bzero(buf, 512);
        strcpy(buf, buf_tmp);
        // TODO Error
    }

    // pack into BIN
    {
    	BINbuf->msgType = MSG_REPORT;
    	BINbuf->device_id = device_id;
    	bzero(BINbuf->payload, sizeof(BINbuf->payload));
    	ParseJson(cJSON_Print(payload), GuessReportSensor(device_id), BINbuf->payload);
    }
    return 0;
}
int ServerBIN_Handler(Server *server) {
    int i, device_id, count;
    char buf[255];
    bzero(buf, 255);
    count = Read(server->sockfd, buf, sizeof(buf));
    if (count == 0) { // server is down
        server->isOK = FALSE;
        close(server->sockfd);
	FD_CLR(server->sockfd, &myGateway.allfd);
        printf("\033[1;31;40mServer '%d'-%s:%d is down\n\033[0m",
                server->id, server->ipv4_addr, server->port);
        return -1;
    }
    if (buf[0] == 0x00 || buf[0] == 0x01) { // ACK or NACK
	printf("\033[1;32;40m\tServer reply: %02x\n\033[0m", buf[0]);
    }
    else if (buf[0] == 0x04) { // Control
	printf("\033[1;34;40mServer: \033[0m");
	for (i = 0; i < count; i++)
	    printf("%02x ", buf[i]);
	printf("\n");
	device_id = *(int *)(buf + 1);
	if (device_id == Sensor883_ID) {
	    int comm = *(buf+5) - '0';
	    *(int *)buf = Sensor883_ID;
	    *((int *)buf + 1) = 12;
	    *((char *)buf + 8) = 0x01;
	    *((char *)buf + 9) = comm;
	    *((int *)((char *)buf + 10)) = 0x12345678;
	    Written(sensor_set[0].fd, buf, 14);
	    printf("\033[1;32;40m\tSend to sensor 883-%d: \033[0m", device_id);
	    for (i = 0; i < 14; i++)
		printf("%02x ", buf[i]);
	    printf("\n");
	}
	else if (device_id == Sensor870_ID) {
	    int comm = *(buf+5) - '0';
	    *(int *)buf = Sensor870_ID;
	    *((int *)buf + 1) = 12;
	    *((char *)buf + 8) = 0x01;
	    *((char *)buf + 9) = comm;
	    *((int *)((char *)buf + 10)) = 0x12345678;
	    Written(sensor_set[0].fd, buf, 14);
	    printf("\033[1;32;40m\tSend to sensor 870-%d: \033[0m", device_id);
	    for (i = 0; i < 14; i++)
		printf("%02x ", buf[i]);
	    printf("\n");
	}
	else if (device_id == Sensor872_ID) {
	    char data = *(buf + 5);
	    char cmd_buf[4] = {'{', data, '}', '\0'};
	    Written(sensor_set[2].fd, cmd_buf, 3);
	    printf("\033[1;32;40m\tSend to sensor 872-%d: %s\033[0m\n", device_id, cmd_buf);
	}
    }
}

void *Server_Handler(void *arg) {
    fdLUT *lut = (fdLUT *)arg;
    pthread_mutex_lock(&lut->lock);
    int sockfd = (lut - fdLookup) / (sizeof(fdLUT));
    Server *server = (Server *)(lut->peer->owner);
    char buf[512];

    ServerBIN_Handler(server);
    pthread_mutex_unlock(&lut->lock);
}

void SensorR430_Handler(fdLUT *lut) {
    int fd = lut - &fdLookup[0];
    int cnt = 0, device_id, i;
    char type;
    char buf[512];
    ReportPacket BINbuf;
    cnt += read(fd, buf+cnt, 9-cnt);
    if (cnt < 9) { // Uncomplete packet
	    usleep(50000);
	    cnt += read(fd, buf+cnt, 9-cnt);
    	if (cnt < 9) {
    	   // fprintf(stderr, "...Uncomplete packet.\n");
            /* pthread_mutex_unlock(&lut->lock); */
            return; // log TODO
    	}
    }
    /*
     * Packet Identification
     */
    device_id = *(int *)buf;
    type = *((char *)buf + 8);
    // TODO add device_id in configuration
    if (device_id == Sensor870_ID) {
        switch (type) {
            case 0x00: // ACK or NAK
                break;
            case 0x01:
                // TODO
                // Error: Gateway to Sensor
                break;
            case 0x02: // data
                /* while (cnt < 14) // package length = 14 */
                    /* cnt += read(fd, buf+cnt, 14 - cnt); */
                cnt += read(fd, buf+cnt, 14-cnt);
                if (cnt < 14) { // Uncomplete package
		            usleep(50000);
                    cnt += read(fd, buf+cnt, 14-cnt);
		            if (cnt < 14) {
	    		        // fprintf(stderr, "...Uncomplete packet.\n");
                        return; // log TODO
		            }
		        }
        		cnt += read(fd, buf+cnt, sizeof(buf)-cnt);
        		if (cnt > 14)
        		    return;
        		else {
                	int i;
        	    	printf("\033[1;32;40mSensor: \033[0m");
                	for (i = 0; i < 14; i++)
                    	printf("%02x ", *((char *)buf + i));
                	printf("\n");
        		}
                if (Sensor870_Handler(lut, fd, buf, &BINbuf) < 0)
                    return; // log TODO
                for (i = 0; i < myGateway.server_num; i++) {
        		    if (!server_set[i].isOK)
                        continue;
                	if (server_set[i].type == HTTP)
                		HTTP_Report(server_set[i].sock_addr, server_set[i].ipv4_addr, buf);
                	else if (server_set[i].type == BIN)
                        Written(server_set[i].sockfd, (const char *)&BINbuf, 25);
                }
                break;
        }
    }
    else if (device_id == Sensor883_ID) { // TODO
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
                /* while (cnt < 18) // package length = 18 */
                    /* cnt += read(fd, buf+cnt, 18 - cnt); */
                cnt += read(fd, buf+cnt, 18-cnt);
                if (cnt < 18) { // Uncomplete package
		            usleep(50000);
                    cnt += read(fd, buf+cnt, 18-cnt);
		            if (cnt < 18) {
	    		        // fprintf(stderr, "...Uncomplete packet.\n");
                        return; // log TODO
		            }
		        }
		        cnt += read(fd, buf+cnt, sizeof(buf)-cnt);
		        if (cnt > 18)
		            return;
		        else {
        	    int i;
		        printf("\033[1;32;40mSensor: \033[0m");
        	    for (i = 0; i < 18; i++)
            	 	printf("%02x ", *((char *)buf + i));
        	    printf("\n");
		        }
                if (Sensor883_Handler(lut, fd, buf, &BINbuf) < 0)
                    return; // log TODO
                for (i = 0; i < myGateway.server_num; i++) {
	       	        if (!server_set[i].isOK)
                        continue;
                    if (server_set[i].type == HTTP)
                        HTTP_Report(server_set[i].sock_addr, server_set[i].ipv4_addr, buf);
                    else if (server_set[i].type == BIN)
                        Written(server_set[i].sockfd, (const char *)&BINbuf, 17);
                }
                break;
        }
    }
    else {
        // TODO
        // Error
    }

}
void SensorBT_Handler(fdLUT *lut) {
    int fd = lut - &fdLookup[0];
    int cnt = 0;
    char buf[512];
    ReportPacket BINbuf;

    cnt += read(fd, buf+cnt, sizeof(buf));
    if (buf[0] != '{') {
        return;
    }
    if (buf[cnt - 1] != '}') {
        usleep(200000);
        cnt += read(fd, buf+cnt, sizeof(buf));
        if (buf[cnt - 1] != '}') {
            return; // log TODO
        }
    }
    buf[cnt] = 0;
    /*
     * Packet Identification
     */
    printf("\033[1;34;40mSensor: \033[0m%s\n", buf);
    int type = buf[1];
    int ret, i;
    if (type == '0') // 872
        ret = Sensor872_Handler(buf, &BINbuf);
    else if (type == '1') // 875
        ret = Sensor875_Handler(buf, &BINbuf);

    if (ret < 0)
        return; // log TODO
    for (i = 0; i < myGateway.server_num; i++) {
        if (!server_set[i].isOK)
            continue;
	printf("hello, world\n");
        if (server_set[i].type == HTTP)
            HTTP_Report(server_set[i].sock_addr, server_set[i].ipv4_addr, buf);
        else if (server_set[i].type == BIN)
            Written(server_set[i].sockfd, (const char *)&BINbuf, 25);
    }
}

void *Sensor_Handler(void *arg) {
    fdLUT *lut = (fdLUT *)arg;
    pthread_mutex_lock(&lut->lock);
    if (lut->type == R430)
        SensorR430_Handler(lut);
    else if (lut->type == BT)
        SensorBT_Handler(lut);
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
    alarm(100); // timed task
    while (1) {
        int sel_fd, err;
        fd_set read_set = myGateway.allfd;
        err = select(myGateway.maxfd+1, &read_set, NULL, NULL, NULL);
	if (err < 0) {
	    perror("select(): ");
	    continue;
	}
        for (sel_fd = 0; sel_fd <= myGateway.maxfd; sel_fd++) {
            // if ready?
            if (!FD_ISSET(sel_fd, &read_set))
                continue;
            // if locked?
            if (isLocked(sel_fd))
                continue;
            // ok now
            if (fdLookup[sel_fd].type == SERVER)
        //        thpool_add_work(myGateway.thpool, Server_Handler, (void *)&fdLookup[sel_fd]);
		Server_Handler(&fdLookup[sel_fd]);
            else // sensor
       //         thpool_add_work(myGateway.thpool, Sensor_Handler, (void *)&fdLookup[sel_fd]);
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
