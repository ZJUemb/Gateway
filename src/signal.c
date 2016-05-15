/*************************************************************************
 > File Name: src/signal.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Fri 13 May 2016 08:38:21 PM CST
 ************************************************************************/

#include "stdio.h"
#include <EmbGW.h>
#include <unistd.h>
#include <signal.h>

void Signal_Handler(int sigid) {
    switch (sigid) {
        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
            printf("\nThe system is going down to maintenance mode NOW!\n");
            Save_Exit();
            break;
        case SIGALRM:
            // TODO
            // poll
            alarm(10);
            /* printf("10 second triggered.\n"); */
            break;
        case SIGPIPE:
            // TODO
            break;
        case SIGIO:
            /* wait_flag = 0; */
            break;
    }
}

