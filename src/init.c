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

void GTWY_Init() {
    printf("INIT:\n");
    /* authorization */
    {
        printf("  Generate authorization key...");

        char id_str[16], buf[64];
        BYTE hash[16];
        int i;
        sprintf(id_str, "%d", myGateway.id);

        MD5_CTX ctx;
        md5_init(&ctx);
        md5_update(&ctx, (BYTE *)id_str, strlen(id_str));
        md5_final(&ctx, hash);
        for (i = 0; i < 16; i++) {
            sprintf(buf+i*2, "%02x", hash[i]);
        }
        strcat(buf, myGateway.md5_salt);

        md5_init(&ctx);
        md5_update(&ctx, (BYTE *)buf, strlen(buf));
        md5_final(&ctx, (BYTE *)hash);
        for (i = 0; i < 16; i++) {
            sprintf(myGateway.auth_key+i*2, "%02x", hash[i]);
        }
        printf("DONE\n");
    }

    /* upload */
    // try connecting to server
    {
        printf("  Connect to specified servers...");
        unsigned int i;
        int sock;

        for (i = 0; i < server_num; i++) {
            bzero(&server_set[i].sock_addr, sizeof(server_set[i].sock_addr));
            server_set[i].sock_addr.sin_family = AF_INET;
            server_set[i].sock_addr.sin_port = htons(server_set[i].port);
            if (inet_pton(AF_INET, server_set[i].ipv4_addr, &(server_set[i].sock_addr.sin_addr)) != 1) {
                fprintf(stderr, "Error: Illegal ip address '%s' for server '%s'.\n", server_set[i].ipv4_addr, server_set[i].name);
                exit(1);
            }
            sock = Socket(AF_INET, SOCK_STREAM, 0);
            if (connect(sock, (struct sockaddr *)&sock_addr[i], sizeof(sock_addr[i])) < 0) {
                fprintf(stderr, "Error: Cannot connect to remote server '%s'.\n", server_addr[i]);
                exit(1);
            }
            Close(sock);
        }
        printf("DONE\n");
    }

    /* download */
    {
        printf("  Open specified device files...");
        // TODO
        char buf[64];
        sensor_set[0].fd = open(sensor_set[0].file_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (sensor_fd[0] < 0) {
            sprintf(buf, "Error: Cannot open device '%s'.", sensor_set[0].file_path);
            perror(buf);
            exit(1);
        }
        fcntl(sensor_fd[0], F_SETOWN, getpid());
        fcntl(sensor_fd[0], F_SETFL, FASYNC);
        tcgetattr(sensor_fd[0], &oldtio);
        newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;// | ICRNL; Uncomment this flag to stop Linux replacing '\r' by '\n'
	    newtio.c_oflag = 0;
	    newtio.c_lflag = 0;//ICANON; receive one byte
        newtio.c_cc[VMIN] = 1;
        newtio.c_cc[VTIME] = 0;
        tcflush(sensor_set[0].fd, TCIFLUSH);
        tcsetattr(sensor_set[0].fd, TCSANOW, &newtio);
        printf("DONE\n");
    }

    /* log */
    {
        printf("  Open log files...");
        const char log1[] = "error.log", log2[] = "access.log";
        char path[128];
        // error log
        sprintf(path, "%s/%s", myGateway.log_location, log1);
        error_log = open(path, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
        if (error_log < 0) {
            char buf[64];
            sprintf(buf, "Error: Cannot open log file '%s'.", path);
            perror(buf);
            exit(1);
        }
        // access log
        sprintf(path, "%s/%s", log_location, log2);
        access_log = open(path, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
        if (access_log < 0) {
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
        signal(SIGIO, Signal_Handler);
        printf("DONE\n");
    }

    /* thread pool */
    {
        printf("  Start thread pool and mutex lock...");
        thpool = thpool_init(MAXTHREADNUM);
        pthread_mutex_init(&IO_ready);
        printf("DONE\n");
    }

    // succeed
    printf("Initialize gateway system...DONE\n");
}
