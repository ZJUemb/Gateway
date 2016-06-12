/*************************************************************************
 > File Name: init.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Thu 12 May 2016 08:22:07 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <thpool.h>
#include "md5.h"
#include "EmbGW.h"
#include <fcntl.h>

#define BAUDRATE B9600

void openDevice(Sensor *sensor) {
    char buf[128];
    int fd = open(sensor->file_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        sprintf(buf, "Error: Cannot open device '%s'.", sensor->file_path);
        perror(buf);
        exit(1);
    }
    sensor->fd = fd;
    sensor->old_seq = -1;
    FD_SET(sensor->fd, &myGateway.allfd);
    if (sensor->fd > myGateway.maxfd)
        myGateway.maxfd = sensor->fd;
    // update lookup table
    fdLookup[fd].type = SENSOR;
    fdLookup[fd].file_path = sensor->file_path;
    fdLookup[fd].peer = (Peer *)malloc(sizeof(Peer));
    fdLookup[fd].peer->id = sensor->id;
    fdLookup[fd].peer->prot = sensor->type;
    fdLookup[fd].peer->owner = &sensor;
    fdLookup[fd].peer->next = NULL;
    pthread_mutex_init(&fdLookup[fd].lock, NULL);
    tcgetattr(sensor->fd, &(sensor->oldtio));
    sensor->newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    sensor->newtio.c_iflag = IGNPAR;// | ICRNL; Uncomment this flag to stop Linux replacing '\r' by '\n'
    sensor->newtio.c_oflag = 0;
    sensor->newtio.c_lflag = 0;//ICANON; receive one byte
    tcflush(sensor->fd, TCIFLUSH);
    tcsetattr(sensor->fd, TCSANOW, &sensor->newtio);

}

void genAuthKey(int id, const char *salt, char auth_key[33]) {
    int i;
    MD5_CTX ctx;
    char id_str[16], buf[64];
    BYTE hash[16];
    sprintf(id_str, "%d", id);

    md5_init(&ctx);
    md5_update(&ctx, (BYTE *)id_str, strlen(id_str));
    md5_final(&ctx, hash);
    for (i = 0; i < 16; i++) {
        sprintf(buf+i*2, "%02x", hash[i]);
    }
    strcat(buf, salt);
    md5_init(&ctx);
    md5_update(&ctx, (BYTE *)buf, strlen(buf));
    md5_final(&ctx, (BYTE *)hash);
    for (i = 0; i < 16; i++) {
        sprintf(auth_key+i*2, "%02x", hash[i]);
    }
    auth_key[32] = 0;
}

void GTWY_Init() {
    printf("INIT:\n");
    myGateway.maxfd = 0;
    FD_ZERO(&myGateway.allfd);

    /* authorization */
    {
        int i;
        printf("  Generate authorization key...");
        genAuthKey(myGateway.id, myGateway.md5_salt, myGateway.auth_key);
        for (i = 0; i < myGateway.sensor_num; i++) {
            genAuthKey(sensor_set[i].id, myGateway.md5_salt, sensor_set[i].auth_key);
        }
        printf("DONE\n");
    }

    /* upload */
    // try connecting to server
    {
        printf("  Connect to specified servers...");
        unsigned int i;
        int sock, j;

        for (i = 0; i < myGateway.server_num; i++) {
            bzero(&server_set[i].sock_addr, sizeof(server_set[i].sock_addr));
            server_set[i].sock_addr.sin_family = AF_INET;
            server_set[i].sock_addr.sin_port = htons(server_set[i].port);
            if (inet_pton(AF_INET, server_set[i].ipv4_addr, &(server_set[i].sock_addr.sin_addr)) != 1) {
                fprintf(stderr, "Error: Illegal ip address '%s' for server '%d'.\n", server_set[i].ipv4_addr, server_set[i].id);
                exit(1);
            }
            sock = Socket(AF_INET, SOCK_STREAM, 0);
            if (connect(sock, (struct sockaddr *)&server_set[i].sock_addr, sizeof(server_set[i].sock_addr)) < 0) {
                fprintf(stderr, "Error: Cannot connect to remote server '%s'.\n", server_set[i].ipv4_addr);
                exit(1);
            }
            if (server_set[i].type == BIN) {
                if (binLogin(sock, myGateway.id, myGateway.auth_key) < 0) {
                    fprintf(stderr, "Error: Gateway login failed.\n");
                    exit(1);
                }
                else {
                    for (j = 0; j < myGateway.sensor_num; j++) {
                        if (binLogin(sock, sensor_set[j].id, sensor_set[j].auth_key) < 0) {
                            fprintf(stderr, "Error: Sensor %d login failed.\n", sensor_set[j].id);
                            exit(1);
                        }
                    }
                }
                server_set[i].sockfd = sock;
                FD_SET(server_set[i].sockfd, &myGateway.allfd);
                if (server_set[i].sockfd > myGateway.maxfd)
                    myGateway.maxfd = server_set[i].sockfd;
                // update lookup table
                fdLookup[sock].type = SERVER;
                fdLookup[sock].file_path = NULL;
                pthread_mutex_init(&fdLookup[sock].lock, NULL);
                fdLookup[sock].peer = (Peer *)malloc(sizeof(Peer));
                fdLookup[sock].peer->id = server_set[i].id;
                fdLookup[sock].peer->prot = server_set[i].type;
                fdLookup[sock].peer->owner = &server_set[i];
                /* fdLookup[sock].peer->handler = ServerBIN_Handler; */
                fdLookup[sock].peer->next = NULL;
            }
            else
                Close(sock);
        }
        printf("DONE\n");
    }

    /* download */
    {
        printf("  Open specified device files...");
        int i, j;
        char buf[64];
        for (i = 0; i < myGateway.sensor_num; i++) {
            for (j = 0; j <= myGateway.maxfd; j++)
                if (strcmp(fdLookup[j].file_path, sensor_set[i].file_path) == 0)
                    break;
            if (j <= myGateway.maxfd) { // device already opened
                sensor_set[i].fd = j;
                continue;
            }
            else // open device
                openDevice(&sensor_set[i]);
        }
        printf("DONE\n");
    }

    /* log */
    {
        printf("  Open log files...");
        const char log1[] = "error.log", log2[] = "access.log";
        char path[128];
        // error log
        sprintf(path, "%s/%s", myGateway.log_location, log1);
        myGateway.error_fd = open(path, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
        if (myGateway.error_fd < 0) {
            char buf[64];
            sprintf(buf, "Error: Cannot open log file '%s'.", path);
            perror(buf);
            exit(1);
        }
        // access log
        sprintf(path, "%s/%s", myGateway.log_location, log2);
        myGateway.access_fd = open(path, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
        if (myGateway.access_fd < 0) {
            char buf[64];
            sprintf(buf, "Error: Cannot open log file '%s'.", path);
            perror(buf);
            exit(1);
        }
        printf("DONE\n");
    }

    /* signal */
    {
        printf("  Install signal handler...");
        signal(SIGINT, Signal_Handler);
        signal(SIGTERM, Signal_Handler);
        signal(SIGQUIT, Signal_Handler);
        signal(SIGALRM, Signal_Handler);
        signal(SIGPIPE, Signal_Handler);
        printf("DONE\n");
    }

    /* thread pool */
    {
        int i;
        printf("  Start thread pool...");
        myGateway.thpool = thpool_init(MAXTHREADNUM);
        printf("DONE\n");
    }

    // succeed
    printf("Initialize gateway system...DONE\n");
}
