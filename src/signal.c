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

void mysig(int sigid) {
    switch (sigid) {
        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
            printf("\nThe system is going down to maintenance mode NOW!\n");
            usleep(500000);
            Save_Exit();
            break;
        case SIGALRM:
            // TODO
            break;
        case SIGPIPE:
            // TODO
            break;
    }
}

