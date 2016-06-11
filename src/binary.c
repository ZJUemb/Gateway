#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/shm.h>
#include "cJSON.h"
#include "BinaryProtocol.h"
/* #include "BinaryConfig.h" */




int binLogin(int sockfd, int device_id, char *auth_key){
	LoginPacket login;
	AckPacket	ack;
	int			ret;
	login.msgType = MSG_LOGIN;
	login.deviceId = device_id;
	memset(login.key, 0, sizeof(login.key));
	strncpy(login.key, auth_key, 32);
	send(sockfd, (char*)&login, sizeof(LoginPacket), 0);
	ret = recv(sockfd, (char*)&ack, sizeof(AckPacket), 0);
	if (ret < 0 || ack.msgType == MSG_NAK)
		return -1;
	return 1;
}

void binLogout(int sockfd){
	LogoutPacket logout;
	logout.msgType = MSG_LOGOUT;
	send(sockfd, (char*)&logout, sizeof(LogoutPacket), 0);
}

