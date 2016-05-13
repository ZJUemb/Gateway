/*************************************************************************
 > File Name: httpCli.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Tue 08 Mar 2016 08:53:39 PM CST
 ************************************************************************/

#include "http_service.h"
#include "string.h"

const char *cgi_path = "/cgi/recv_data.py";
const char *user_agent = "Emba/1.0";
const char *content_type = "text/sensor";

void send_data(FILE *fd, const char *ip, unsigned int port) {
    int serv_fd, ret_code;
    long int file_size;
    struct sockaddr_in serv_addr;
    char buf[255];

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    ret_code = inet_pton(AF_INET, ip, (void *)(&serv_addr.sin_addr.s_addr));
    if (ret_code <= 0) {
        fprintf(stderr, "Illegal ip address: %s\nError code: %d\n", ip, ret_code);
        exit(-1);
    }

    serv_fd = Socket(AF_INET, SOCK_STREAM, 0);
    Connect(serv_fd, (const struct sockaddr *)(&serv_addr), sizeof(serv_addr));

    fseek(fd, 0L, SEEK_END);
    file_size = ftell(fd);
    fseek(fd, 0L, SEEK_SET);

    sprintf(buf, "POST %s HTTP/1.1\r\n", cgi_path);
    Written(serv_fd, buf, strlen(buf));
    sprintf(buf, "User-Agent: %s\r\n", user_agent);
    Written(serv_fd, buf, strlen(buf));
    sprintf(buf, "Content-Type: %s\r\n", content_type);
    Written(serv_fd, buf, strlen(buf));
    sprintf(buf, "Content-Length: %ld\r\n", file_size);
    Written(serv_fd, buf, strlen(buf));
    sprintf(buf, "\r\n");
    Written(serv_fd, buf, strlen(buf));

    while (fgets(buf, sizeof(buf), fd))
        Written(serv_fd, buf, strlen(buf));

    Close(serv_fd);
}
int main(int argc, char *argv[])
{
    FILE *fd;
    fd = fopen("sensor1.dat", "r+");

    send_data(fd, "10.214.128.100", 8080);
    return 0;
}
