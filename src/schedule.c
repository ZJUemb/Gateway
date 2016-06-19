/*************************************************************************
 > File Name: src/schedule.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Sun 12 Jun 2016 01:07:08 AM CST
 ************************************************************************/

#include <EmbGW.h>
#include <stdio.h>
#include <string.h>
#include <cJSON.h>

static void Sensor_sched(int device_id, int control_id) {
    int i;
    char buf[512];
    printf("device_id: %d, control_id: %d\n", device_id, control_id);
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "auth_id", myGateway.id);
    cJSON_AddStringToObject(root, "auth_key", myGateway.auth_key);
    cJSON_AddNumberToObject(root, "device_id", device_id);
    cJSON_AddNumberToObject(root, "control_id", control_id);
    strcpy(buf, cJSON_Print(root));
    for (i = 0; i < myGateway.server_num; i++) {
        Server server = server_set[i];
        if (server.type == HTTP) {
	    if (device_id == Sensor870_ID)
		HTTP_Poll(server.sock_addr, server.ipv4_addr, buf, &sensor_set[0]);
	    else if (device_id == Sensor883_ID)
		HTTP_Poll(server.sock_addr, server.ipv4_addr, buf, &sensor_set[1]);
	    else if (device_id == Sensor872_ID)
		HTTP_Poll(server.sock_addr, server.ipv4_addr, buf, &sensor_set[2]);
        }
    }
}

void *Sensor870_sched(void *arg) {
    Sensor_sched(Sensor870_ID, 10);
}
void *Sensor872_sched(void *arg) {
    Sensor_sched(Sensor872_ID, 6);
}
void *Sensor883_sched(void *arg) {
    Sensor_sched(Sensor883_ID, 11);
}
