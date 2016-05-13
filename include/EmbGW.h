/*************************************************************************
 > File Name: EmbGW.h
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Thu 12 May 2016 09:26:50 AM CST
 ************************************************************************/
#ifndef EMBGW_H
#define EMBGW_H

#include <arpa/inet.h>

/*
 * Macros
 */
#define MAXSERVERNUM 4
#define MAXSENSORNUM 4

#define BT          1
#define R430        2
#define BIN         3
#define HTTP        4

/*
 * function
 */
void mysig(int sigid);


/*
 * Configuration
 */
unsigned int gateway_id;
char md5_salt[32];

unsigned int server_num;
char server_name[MAXSERVERNUM][16];
char server_addr[MAXSERVERNUM][64]; // ip address or domain name of web server
unsigned int server_port[MAXSERVERNUM]; // port
unsigned int server_type[MAXSERVERNUM]; // BIN or HTTP

unsigned int sensor_num;
char sensor_name[MAXSENSORNUM][16];
char sensor_dev[MAXSENSORNUM][32]; // path of serial devices
unsigned int sensor_type[MAXSENSORNUM]; // BlueTooth or Radio430

char log_location[128];

/*
 * Initialization
 */
// authorization
char auth_key[32];

// upload
int sock[MAXSERVERNUM];
struct sockaddr_in sock_addr[MAXSERVERNUM];

// download
int fd[MAXSENSORNUM];

// log
int access_log, error_log;

#endif
