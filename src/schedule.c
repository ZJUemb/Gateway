/*************************************************************************
 > File Name: src/schedule.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Sun 12 Jun 2016 01:07:08 AM CST
 ************************************************************************/

#include <EmbGW.h>
#include <cJSON.h>

static void Sensor_sched(int device_id) {
    int i;
    char buf[512];
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "auth_id", myGateway.id);
    cJSON_AddStringToObject(root, "auth_key", myGateway.auth_key);
    cJSON_AddNumberToObject(root, "device_id", device_id);
    strcpy(buf, cJSON_Print(root));
    for (i = 0; i < myGateway.server_num; i++) {
        Server server = server_set[i];
        if (server.type == HTTP) {
            HTTP_Poll(server.sock_addr, server.ipv4_addr, buf);
        }
    }
}

inline void Sensor870_Handler() {
    Sensor_sched(Sensor870_ID);
}
inline void Sensor872_Handler() {
    Sensor_sched(Sensor872_ID);
}
inline void Sensor883_Handler() {
    Sensor_sched(Sensor883_ID);
}
