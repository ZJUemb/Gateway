/*************************************************************************
 > File Name: httpd_v2.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Tue 29 Mar 2016 09:47:52 PM CST
 ************************************************************************/

#include "stdio.h"
#include "http_service.h"
#include <ctype.h>
/* #include <sys/select.h> */
/* #include <sys/time.h> */
/* #include <sys/types.h> */
#include <sys/stat.h>
#include <strings.h>
#include <string.h>

#define DEBUG

typedef struct request_field req_head;
struct request_field {
    char Host[64];
    char Referer[64];
    char User_Agent[64];
    char Date[64];
    char Connection[64];

    char Accept[64];
    char Accept_Lang[8][16];
    char Accept_Enc[8][16];
};


int read_line(int sockfd, char *buf, size_t len) {
    char c;
    unsigned int i = 0;

    while (recv(sockfd, &c, 1, 0) > 0 && i < len - 1) {
        if (c == '\r') {
            char d;
            if (recv(sockfd, &d, 1, MSG_PEEK) > 0 && d == '\n')
                recv(sockfd, &d, 1, 0);
            c = '\n';
        }
        buf[i++] = c;
        if (c == '\n')
            break;
    }
    buf[i] = '\0';
    return i;
}


int accept_request(int clifd) {
    char buf[255];
    char method[10], uri[255], version[10];
    char query_args[255], path[255];
    unsigned int i = 0;
    unsigned char isCGI = 0;
    struct stat st;

    // read first line
#ifdef DEBUG
    printf("\033[49;31mstart read_line()\033[0m\n");
#endif
    read_line(clifd, buf, sizeof(buf));
#ifdef DEBUG
    printf("\033[49;31mafter read_line()\033[0m\n");
#endif
    printf ("buf = %s", buf);
    while (!isspace(buf[i])) {
        if (i >= sizeof(method) - 1) { // too long for method field, bad request
            read_all(clifd); // read all rest message
            send_response(clifd, NULL, BAD_REQUEST);
            return -1;
        }
        method[i] = buf[i];
        i++;
    }
    method[i] = '\0';

    memset(uri, 0, sizeof(uri));
    while (isspace(buf[++i]));
    int j = 0;
    while (!isspace(buf[i])) {
        if (i >= sizeof(uri) - 1) {
            read_all(clifd);
            send_response(clifd, NULL, BAD_REQUEST);
            return -1;
        }
        if (buf[i] == '?') {
            j = 0;
            uri[i++] = '\0';
            while (!isspace(buf[i]))
                query_args[j++] = buf[i++];
            break;
        }
        uri[j] = buf[i];
        i++; j++;
    }
#ifdef DEBUG
    printf("\033[49;32muri = %s\n\033[0m", uri);
#endif

    /* we don't care http version here */
    memset(path, 0, sizeof(path));
    sprintf(path, "data%s", uri); // we store file in directory 'data'
#ifdef DEBUG
    printf("\033[49;32mpath = %s\n\033[0m", path);
#endif

    if (!strcasecmp(method, "POST")) {
        isCGI = 1;
    }
    else if (!strcasecmp(method, "GET")) {
        if (path[strlen(path) - 1] == '/')
            strcat(path, "history.dat");
        if (stat(path, &st) == -1) {
#ifdef DEBUG
            printf("\033[49;32mpath = %s\n\033[0m", path);
#endif
            read_all(clifd);
            send_response(clifd, NULL, NOT_FOUND);
            return -1;
        }
        else if (S_ISDIR(st.st_mode)) {
            strcat(path, "/history.dat");
        }
        read_all(clifd);
        send_response(clifd, (const char *)path, OK);
        return 0;
    }
    else { // only support 'GET' and 'POST'
        read_all(clifd);
        send_response(clifd, NULL, NOT_IMPLEMENTED);
        return -1;
    }
}

void read_all(int clifd) {
    char buf[255];
    while (read_line(clifd, buf, sizeof(buf)) > 0 && strcmp(buf, "\n"))
        printf ("buf = %s", buf);
}

const char *SERVER_STRING = "Ember/1.0";
/* const char *CHARSET = "us-ascii"; */

int send_response(int clifd, const char *path, int mode) {
#ifdef DEBUG
    printf("\033[49;32mmode = %d\n\033[0m", mode);
#endif
    char buf[255];

    switch (mode) {
        case OK: {
                    strcpy(buf, "HTTP/1.1 200 OK\r\n");
                    /* strcpy(buf, "HTTP/1.1 401 Unauthorized\r\n"); */
                    Written(clifd, buf, strlen(buf));
                    /* strcpy(buf, "WWW-Authenticate: Basic realm=\"We need to verify your identity.\"\r\n"); */
                    /* Written(clifd, buf, strlen(buf)); */
                    sprintf(buf, "Server: %s\r\n", SERVER_STRING);
                    Written(clifd, buf, strlen(buf));
                    sprintf(buf, "Content-Type: text/html\r\n");
                    Written(clifd, buf, strlen(buf));
                    strcpy(buf, "Connection: close\r\n");
                    Written(clifd, buf, strlen(buf));
                    strcpy(buf, "\r\n");
                    Written(clifd, buf, strlen(buf));

                    FILE *fd = fopen(path, "r");
                    while (fgets(buf, sizeof(buf), fd)) {
                        Written(clifd, buf, strlen(buf));
                    }
                    return 0;
                }
        case BAD_REQUEST: {
                    strcpy(buf, "HTTP/1.1 400 Bad Request\r\n");
                    Written(clifd, buf, strlen(buf));
                    sprintf(buf, "Server: %s\r\n", SERVER_STRING);
                    Written(clifd, buf, strlen(buf));
                    sprintf(buf, "Content-Type: text/html\r\n");
                    Written(clifd, buf, strlen(buf));
                    strcpy(buf, "Connection: close\r\n");
                    Written(clifd, buf, strlen(buf));
                    strcpy(buf, "\r\n");
                    Written(clifd, buf, strlen(buf));

                    FILE *fd = fopen("response/bad_request.html", "r");
                    while (fgets(buf, sizeof(buf), fd))
                        Written(clifd, buf, strlen(buf));
                    return 0;
                }
        case NOT_FOUND: {
                    strcpy(buf, "HTTP/1.1 404 Not Found\r\n");
                    /* strcpy(buf, "HTTP/1.1 401 Unauthorized\r\n"); */
                    Written(clifd, buf, strlen(buf));
                    sprintf(buf, "Server: %s\r\n", SERVER_STRING);
                    Written(clifd, buf, strlen(buf));
                    sprintf(buf, "Content-Type: text/html\r\n");
                    Written(clifd, buf, strlen(buf));
                    strcpy(buf, "Connection: close\r\n");
                    Written(clifd, buf, strlen(buf));
                    strcpy(buf, "\r\n");
                    Written(clifd, buf, strlen(buf));

                    FILE *fd = fopen("response/not_found.html", "r");
                    while (fgets(buf, sizeof(buf), fd))
                        Written(clifd, buf, strlen(buf));
                    return 0;
                }
        case NOT_IMPLEMENTED: {
                    strcpy(buf, "HTTP/1.1 501 Method Not Implemented\r\n");
                    Written(clifd, buf, strlen(buf));
                    sprintf(buf, "Server: %s\r\n", SERVER_STRING);
                    Written(clifd, buf, strlen(buf));
                    sprintf(buf, "Content-Type: text/html\r\n");
                    Written(clifd, buf, strlen(buf));
                    strcpy(buf, "Connection: close\r\n");
                    Written(clifd, buf, strlen(buf));
                    strcpy(buf, "\r\n");
                    Written(clifd, buf, strlen(buf));

                    FILE *fd = fopen("response/not_implemented.html", "r");
                    while (fgets(buf, sizeof(buf), fd))
                        Written(clifd, buf, strlen(buf));
                    return 0;
                }
    }
}
int main(int argc, char *argv[])
{
    /*
     * socket variables
     */
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len = sizeof(cliaddr);
    int servfd, clifd;
    /*
     * file descriptor set
     */
    int maxfd;
    fd_set all_set;
    FD_ZERO(&all_set); // initial setting

    /*
     * set server socket address
     * bind and listen
     */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10000);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    servfd = Socket(AF_INET, SOCK_STREAM, 0);
    Bind(servfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    Listen(servfd, 5);

    /*
     * add servfd into read_set
     * set maxfd as the biggest file descriptor
     */
    FD_SET(servfd, &all_set);
    maxfd = servfd + 1;

    while (1) {
        fd_set read_set = all_set;
        int fd;
        Select(maxfd + 1, &read_set, NULL, NULL, NULL);
        for (fd = 0; fd <= maxfd; fd++) {
            if (!FD_ISSET(fd, &read_set)) // fd is not ready now
                continue;
            if (fd == servfd) { // server socket is ready; new client connects
                clifd = Accept(servfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
#ifdef DEBUG
                printf("\033[49;32maccept clifd = %d\n\033[0m", clifd);
#endif
                FD_SET(clifd, &read_set);
                if (clifd > maxfd)
                    maxfd = clifd;
            }
            else { // client socket is ready; client tries to say something
                int status = accept_request(clifd);
                if (status == 0) { // connection closed
                    FD_CLR(clifd, &all_set); // remove clifd from read_set
                    if (clifd == maxfd) // update maxfd if maxfd == clifd
                        while (!FD_ISSET(maxfd - 1, &all_set))
                            maxfd--;
                    Close(clifd);
                }
            }
        }
    }
    return 0;
}


