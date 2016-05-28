/*************************************************************************
 > File Name: src/http.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Sat 14 May 2016 02:18:38 PM CST
 ************************************************************************/

#include "stdio.h"
#include <EmbGW.h>
#include <string.h>
/* #include <arpa/inet.h> */

void HTTP_Send(int serv_fd, const char *host, char *data) {
    char buf[1023];
    char buf_temp[64];
    {
        sprintf(buf, "POST /api/report HTTP/1.1\r\n");
        sprintf(buf_temp, "Host: %s\r\n", host);
        strcat(buf, buf_temp);
        sprintf(buf_temp, "User-Agent: EmberGTWY/1.0\r\n");
        strcat(buf, buf_temp);
        sprintf(buf_temp, "Content-Type: application/x-www-form-urlencoded\r\n");
        strcat(buf, buf_temp);
        sprintf(buf_temp, "\r\n");
        strcat(buf, buf_temp);
        {
            char data_str[128];
            int prev = 0, curr = 0;
            while (1) {
                while (data[curr] != '\n' && data[curr] != 0)
                    curr++;
                strncpy(data_str, data+prev, curr-prev);
                data_str[curr-prev] = 0;
                sprintf(buf_temp, "%s\r\n", data_str);
                strcat(buf, buf_temp);
                if (data[curr] == 0)
                    break;
                prev = ++curr;
            }
        }
        Written(serv_fd, buf, strlen(buf));
    }

    // wait for response
    {
        Close(serv_fd);
    }
}
