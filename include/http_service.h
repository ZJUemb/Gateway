/*************************************************************************
 > File Name: http_service.h
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Tue 08 Mar 2016 09:06:29 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdarg.h>
#include <sys/select.h>

#define OK                  200
#define BAD_REQUEST         400
#define NOT_FOUND           404
#define NOT_IMPLEMENTED     501


/* Library */
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

/* server */
int read_line(int sockfd, char *buf, size_t size);
int accept_request(int clifd);
int send_response(int clifd, const char *path, int mode);

void read_all(int sockfd);

/* client */
void send_data(FILE *fd, const char *ip, unsigned int port);
