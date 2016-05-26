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
#define MAXSERVERNUM        4
#define MAXSENSORNUM        4
#define MAXFILEDESCRIPTOR   512
#define MAXTHREADNUM        8

#define BT                  0x0801
#define R430                0x0802
#define BIN                 0x0401
#define HTTP                0x0402

#define SENSOR              0x1001
#define SERVER              0x1002

typedef unsigned char bool;
#define TRUE                1
#define FALSE               0

/*
 * struct
 */
typedef struct Dictionary {
    int type;
    void *owner;
} Dict;

typedef struct Gateway {
    int id;
    char name[16];
    char log_location[64];
    char md5_salt[32];
    int server_num;
    int sensor_num;

    char auth_key[33];
    Dict dict[MAXFILEDESCRIPTOR];
    int maxfd;
    int access_fd, error_fd;
    fd_set allfd;
    threadpool thpool;
} Gateway;

typedef struct Server {
    bool isOK;

    char name[16];
    char ipv4_addr[64];
    int port;
    int type;

    int sockfd;
    struct sockaddr_in sock_addr;
    pthread_mutex_t lock;
} Server;

typedef struct Sensor {
    bool isOK;

    char name[16];
    char file_path[64];
    int type;

    int fd;
    pthread_mutex_t lock;
    struct termios oldtio, newtio;
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

#endif
