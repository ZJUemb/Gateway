/*************************************************************************
 > File Name: controller.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Thu 12 May 2016 08:59:15 AM CST
 ************************************************************************/

#include "stdio.h"
#include "EmbGW.h"

void Welcome() {
    printf("*************************************\n\
"          "*          EmbGW version 1.0        *\n\
"          "*    Copyright (c) 2016 EmbedFun    *\n\
"          "*************************************\n");
}

int main(int argc, char *argv[])
{

    Welcome();

    /*
     * load configuration
     * from user-defined config file
     *
     * 1. gateway id
     * 2. md5 salt
     * 3. upload(web server)
     * 4. download(sensor)
     */
    Load_Conf();

    /*
     * Initialization:
     * open serial devices if exist
     * new a socket
     * open log file
     */

    GTWY_Init();

    /*
     * Kernel:
     * listen from serial devices
     * send to web server
     * record in log file
     */
    GTWY_Work();

    /*
     * prepare to exit
     */
    /* GTWY_Exit(); */
    return 0;
}
