/*************************************************************************
 > File Name: EmbGW.h
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Thu 12 May 2016 09:26:50 AM CST
 ************************************************************************/
#ifndef EMBGW_H
#define EMBGW_H

#include <arpa/inet.h>
#include <sys/select.h>
#include <thpool.h>
#include <termios.h>

/*
 * Macros
 */
#define MAXSERVERNUM 4
#define MAXSENSORNUM 4
#define MAXTHREADNUM 8

#define BT          1
#define R430        2
#define BIN         3
#define HTTP        4

#define TRUE    1;
#define FALSE   0;

/*
 * struct
 */
typedef unsigned char bool;
typedef struct Gateway {
    int id;
    char name[16];
    char log_location[64];
    char md5_salt[32];

    char auth_key[33];
} Gateway;

typedef struct Server {
    bool isOK;

    char name[16];
    char ipv4_addr[64];
    int port;
    int type;

    int sockfd;
    struct sockaddr_in sock_addr;
} Server;

typedef struct Sensor {
    bool isOK;

    char name[16];
    char file_path[64];
    int type;

    int fd;
} Sensor;

/*
 * function
 */
void Signal_Handler(int sigid);
void BIN_Send(int id, char *data);
void HTTP_Send(int id, char *data);
void Save_Exit();
// Library
int Socket(int family, int type, int protocol);
int Connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen);
int Bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);
int Select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,\
        struct timeval *timeout);
int Close(int sockfd);

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap);
void err_quit(const char *fmt, ...);
void err_msg(const char *fmt, ...);
void err_dump(const char *fmt, ...);
void err_exit(int error, const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_ret(const char *fmt, ...);

ssize_t Readn(int fd, void *buff, size_t nbytes);
ssize_t Read(int fd, void *buff, size_t nbytes);
ssize_t Written(int fd, const void *buff, size_t nbytes);


/*
 * Configuration and Initialization
 */
Gateway myGateway;
Server server_set[MAXSERVERNUM];
Sensor sensor_set[MAXSENSORNUM];

unsigned int gateway_id;
char md5_salt[32];

unsigned int server_num; char server_name[MAXSERVERNUM][16];
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
char auth_key[33];

// upload
struct sockaddr_in sock_addr[MAXSERVERNUM];

// download
int sensor_fd[MAXSENSORNUM];
int maxfd;
struct termios oldtio, newtio;

// log
int access_log, error_log;

// thread pool
threadpool thpool;

// mutex lock
pthread_mutex_t IO_ready;

/*
 * Kernel
 */
unsigned char wait_flag;

#endif
