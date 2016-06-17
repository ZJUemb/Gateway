/*************************************************************************
 > File Name: src/schedule.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Sun 12 Jun 2016 01:07:08 AM CST
 ************************************************************************/

#include <EmbGW.h>

void Sensor870_sched(int timer) {
    int i;
    for (i = 0; i < myGateway.server_num; i++) {
	if (server_set[i].type == HTTP) {
	    
	}
    }
}
