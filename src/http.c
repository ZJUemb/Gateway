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

void HTTP_Report(struct sockaddr_in sock_addr, const char *host, char *data) {
    int serv_fd = Socket(AF_INET, SOCK_STREAM, 0);
    Connect(serv_fd, (const struct sockaddr *)&sock_addr, sizeof(sock_addr));
    char buf[1023];
    memset(buf, 0, sizeof(buf));
    char buf_temp[64], buf_data[512];
    {
        sprintf(buf, "POST /api/report HTTP/1.1\r\n");
        sprintf(buf_temp, "Host: %s:80\r\n", host);
        strcat(buf, buf_temp);
        sprintf(buf_temp, "User-Agent: EmberGTWY/1.0\r\n");
        strcat(buf, buf_temp);
        sprintf(buf_temp, "Content-Type: application/x-www-form-urlencoded\r\n");
        strcat(buf, buf_temp);
	    sprintf(buf_temp, "Content-Length: %d\r\n", (int)strlen(data));
	    strcat(buf, buf_temp);
        sprintf(buf_temp, "\r\n");
        strcat(buf, buf_temp);
	strcat(buf, data);
	printf("buf = %s\n", buf);
        Written(serv_fd, buf, strlen(buf));
    }

    // wait for response
    {
    	int i, count;
    	char *tok;
    	bzero(buf, sizeof(buf));
    	Read(serv_fd, buf, sizeof(buf));
    	printf("\t\033[0;32;40mServer reply: \033[0m");
    	tok = strtok(buf, " ");
    	if ((tok = strtok(NULL, " \t")) != NULL)
    	    printf("%s ", tok);
    	if ((tok = strtok(NULL, " \t\r\n")) != NULL)
    	    printf("%s\n", tok);
        Close(serv_fd);
    }
}

void HTTP_Poll(struct sockaddr_in sock_addr, const char *host, char *data, Sensor *sensor) {
    int serv_fd = Socket(AF_INET, SOCK_STREAM, 0);
    Connect(serv_fd, (const struct sockaddr *)&sock_addr, sizeof(sock_addr));
    char buf[1023];
    char buf_temp[64], buf_data[512];
    {
        sprintf(buf, "POST /api/poll HTTP/1.1\r\n");
        sprintf(buf_temp, "Host: %s:80\r\n", host);
        strcat(buf, buf_temp);
        sprintf(buf_temp, "User-Agent: EmberGTWY/1.0\r\n");
        strcat(buf, buf_temp);
        sprintf(buf_temp, "Content-Type: application/x-www-form-urlencoded\r\n");
        strcat(buf, buf_temp);
	    sprintf(buf_temp, "Content-Length: %d\r\n", (int)strlen(data));
	    strcat(buf, buf_temp);
        sprintf(buf_temp, "\r\n");
        strcat(buf, buf_temp);
	strcat(buf, data);
        Written(serv_fd, buf, strlen(buf));
    }

    // wait for response
    {
    	int i, count;
    	char *tok;
    	bzero(buf, sizeof(buf));
    	Read(serv_fd, buf, sizeof(buf));
	tok = buf;
	int code, data;
 	//json = buf;
	while (*tok++ != '{');
	while (*tok++ != ':');
	while (*tok != '0' && *tok != '-')tok++;
	if (*tok == '0')
	    code = 0;
	else
	    code = -1;
	if (code == -1) {
	    close(serv_fd);
	    return;
	}
	while (*tok++ != ':');
	while (*tok++ != ':');
	while (*tok != '0' && *tok != '1' && *tok != '2')tok++;
	data = *tok - '0';
	if (sensor->id == Sensor870_ID) {
	    int i;
	    char buf[256];
            *(int *)buf = Sensor870_ID;
            *((int *)buf + 1) = 12;
            *((char *)buf + 8) = 0x01;
            *((char *)buf + 9) = data;
	    *((int *)((char *)buf + 10)) = 0x12345678;
	    Written(sensor->fd, buf, 14);    
	    printf("\033[1;32;40m\tSend to sensor 870-%d: \033[0m", Sensor870_ID);
	    for (i = 0; i < 14; i++)
		printf("%02x ", buf[i]);
	    printf("\n");
	}
	else if (sensor->id == Sensor883_ID) {
	    char buf[256];
	    int i;
            *(int *)buf = Sensor883_ID;
            *((int *)buf + 1) = 12;
            *((char *)buf + 8) = 0x01;
            *((char *)buf + 9) = data;
	    *((int *)((char *)buf + 10)) = 0x12345678;
	    Written(sensor->fd, buf, 14);    
	    printf("\033[1;32;40m\tSend to sensor 883-%d: \033[0m", Sensor883_ID);
	    for (i = 0; i < 14; i++)
		printf("%02x ", buf[i]);
	    printf("\n");
	}
	else if (sensor->id == Sensor872_ID) {
	    char cmd_buf[4] = {'{', data+'0', '}', '\0'};
	    Written(sensor->fd, cmd_buf, 3);    
	    printf("\033[1;32;40m\tSend to sensor 872-%d: \033[0m%s\n", Sensor872_ID, cmd_buf);
	}
        close(serv_fd);
    }
}
