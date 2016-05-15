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

    /* authorization */
    {
        char id_str[16], buf[64];
        BYTE hash[16];
        int i;
        sprintf(id_str, "%d", gateway_id);

        MD5_CTX ctx;
        md5_init(&ctx);
        md5_update(&ctx, (BYTE *)id_str, strlen(id_str));
        md5_final(&ctx, hash);
        for (i = 0; i < 16; i++) {
            sprintf(buf+i*2, "%02x", hash[i]);
        }
        strcat(buf, md5_salt);

        md5_init(&ctx);
        md5_update(&ctx, (BYTE *)buf, strlen(buf));
        md5_final(&ctx, (BYTE *)hash);
        for (i = 0; i < 16; i++) {
            sprintf(auth_key+i*2, "%02x", hash[i]);
        }
    }

    /* upload */
    {
        unsigned int i;
        int sock;

        for (i = 0; i < server_num; i++) {
            bzero(&sock_addr[i], sizeof(sock_addr[i]));
            sock_addr[i].sin_family = AF_INET;
            sock_addr[i].sin_port = htons(server_port[i]);
            if (inet_pton(AF_INET, server_addr[i], &(sock_addr[i].sin_addr)) != 1) {
                fprintf(stderr, "Error: Illegal ip address '%s' for server '%s'.\n", server_addr[i], server_name[i]);
                exit(1);
            }
            sock = socket(AF_INET, SOCK_STREAM, 0);
            // TODO
            // socket -> Socket
            if (connect(sock, (struct sockaddr *)&sock_addr[i], sizeof(sock_addr[i])) < 0) {
                fprintf(stderr, "Error: Cannot connect to remote server '%s'.\n", server_addr[i]);
                exit(1);
            }
            close(sock);
        }
    }

    /* download */
    {
        // TODO
        char buf[64];
        sensor_fd[0] = open(sensor_dev[0], O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (sensor_fd[0] < 0) {
            sprintf(buf, "Error: Cannot open device '%s'.", sensor_dev[0]);
            perror(buf);
            exit(1);
        }
        fcntl(sensor_fd[0], F_SETOWN, getpid());
        fcntl(sensor_fd[0], F_SETFL, FASYNC);
        tcgetattr(sensor_fd[0], &oldtio);
        newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;// | ICRNL;
	    newtio.c_oflag = 0;
	    newtio.c_lflag = 0;//ICANON; receive one byte
        newtio.c_cc[VMIN] = 1;
        newtio.c_cc[VTIME] = 0;
        tcflush(sensor_fd[0], TCIFLUSH);
        tcsetattr(sensor_fd[0], TCSANOW, &newtio);
    }

    /* log */
    {
        const char log1[] = "error.log", log2[] = "access.log";
        char path[128];
        // error log
        sprintf(path, "%s%s", log_location, log1);
        error_log = open(path, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
        if (error_log < 0) {
            char buf[64];
            sprintf(buf, "Error: Cannot open log file '%s'.", path);
            perror(buf);
            exit(1);
        }
        // access log
        sprintf(path, "%s%s", log_location, log2);
        access_log = open(path, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
        if (access_log < 0) {
            char buf[64];
            sprintf(buf, "Error: Cannot open log file '%s'.", path);
            perror(buf);
            exit(1);
        }
        write(access_log, "hello\n", 6);
        // TODO
    }

    /* signal */
    {
        signal(SIGINT, Signal_Handler);
        signal(SIGTERM, Signal_Handler);
        signal(SIGQUIT, Signal_Handler);
        signal(SIGALRM, Signal_Handler);
        signal(SIGIO, Signal_Handler);
    }

    /* thread pool */
    {
        thpool = thpool_init(MAXTHREADNUM);
    }

    /* mutex lock */
    {
        pthread_mutex_init(&IO_ready);
    }

    // succeed
    printf("Initialize gateway system --- DONE\n");
}
