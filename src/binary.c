#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define OS_LINUX
#include <stdio.h>

#include <string.h>
#include <stdlib.h>


#ifdef OS_LINUX
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
	#include <sys/shm.h>
#else
	#include <winsock2.h>
#endif
#include "cJSON.h"
#include "BinaryProtocol.h"

SensorConfig binaryReportConfigs[SensorMax] = {
	{ //Sensor_872
		DeviceID_872,
		5,
		"type",		DataType_Int,	4,
		"temp",		DataType_Float, 4,
		"wet",		DataType_Float, 4,
		"state",	DataType_Int,	4,
		"id",		DataType_Int,	4
	},
	{ //Sensor_875
		DeviceID_875,//device_id
		4,			//dataCount
		"type",		DataType_Int,	4,
		"pid",		DataType_Int,	4,
		"time",		DataType_Int,	4,
		"result",	DataType_Int,	4
	},
	{ //Sensor_870
		DeviceID_870,
		4,
		"Activate LED bit",		DataType_String,	1,
		"Sound",		DataType_String,	1,
		"Light",		DataType_String,	1,
		"LEDState",		DataType_String,	1,
	},
	{ //Sensor_883
		DeviceID_883,
		2,
		"data",	DataType_Int,	4,
		"value",	DataType_Int,	4,
	},
};

SensorConfig binaryCtrlConfigs[SensorMax] = {
	{ //Sensor_872
		DeviceID_872,
		1,
		"start",	DataType_Int,	4
	},
	{ //Sensor_875
		DeviceID_875,
		1,
		"result",	DataType_Int,	4
	},
	{ //Sensor_870
		DeviceID_870,
		1,
		"action",	DataType_Int,	4
	},
	{ //Sensor_883
		DeviceID_883,
		1,
		"action",	DataType_Int,	4
	},
};

SensorConfig* GuessReportSensor(int device_id){
    int i;
	for (i = 0; i < SensorMax; i++)
		if (binaryReportConfigs[i].device_id == device_id)
			return binaryReportConfigs + i;
	return NULL;
}
SensorConfig* GuessCtrlSensor(int device_id){
    int i;
	for (i = 0; i < SensorMax; i++)
		if (binaryCtrlConfigs[i].device_id == device_id)
			return binaryCtrlConfigs + i;
	return NULL;
}

int ParseJson(char *dataStr, SensorConfig *dataStructure, char *payload){
	if (dataStructure == NULL)
		return -1;
	cJSON *jsonRoot, *jsonSub;
	int i;
	if (dataStr == NULL)
		return -1;
	jsonRoot = cJSON_Parse(dataStr);
	if (jsonRoot == NULL)
		return -1;
	for (i = 0; i<dataStructure->dataCount; i++){
		jsonSub = cJSON_GetObjectItem(jsonRoot, dataStructure->dataConfigs[i].dataName);
		if (jsonSub == NULL)
			return -1;
		switch (dataStructure->dataConfigs[i].type){
		case DataType_Int:
			*(int *)payload = jsonSub->valueint;
			break;
		case DataType_Float:
			*(float *)payload = jsonSub->valuedouble;
			break;
		case DataType_String:
			strcpy(payload, jsonSub->valuestring);
			break;
		default:
			return -1;
		}
		payload += dataStructure->dataConfigs[i].length;
	}
	return 1;
}
int ParseBinary(char *payload, SensorConfig *dataStructure, char *dataJson){
    int i;
	if (dataStructure == NULL)
		return -1;
	cJSON *root;
	root = cJSON_CreateObject();
	for (i = 0; i < dataStructure->dataCount; i++){
		switch (dataStructure->dataConfigs[i].type){
		case DataType_Int:
			cJSON_AddNumberToObject(root, dataStructure->dataConfigs[i].dataName, *(int *)payload);
			break;
		case DataType_Float:
			cJSON_AddNumberToObject(root, dataStructure->dataConfigs[i].dataName, *(float *)payload);
			break;
		case DataType_String:
			cJSON_AddStringToObject(root, dataStructure->dataConfigs[i].dataName, payload);
			break;
		default:
			return -1;
		}
		payload += dataStructure->dataConfigs[i].length;
	}
	strcpy(dataJson,cJSON_Print(root));
	return 1;
}


int binLogin(int sockfd, int device_id, char *auth_key){
	LoginPacket login;
	AckPacket	ack;
	int			ret;
	login.msgType = MSG_LOGIN;
	login.deviceId = device_id;
	memset(login.key, 0, sizeof(login.key));
	strcpy(login.key, auth_key);
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

int Ctrl(int sockfd, int *device_id, char *dataJson){
	CtrlPacket ctrl;
	int ret;
	//ReceivePacket(sockfd, (char*)&ctrl, sizeof(CtrlPacket));

	ret = recv(sockfd, (char*)&ctrl, sizeof(CtrlPacket), 0);
	//printf("%d\n", ret);
	//ret = recv(sockfd, (char*)&ctrl, sizeof(CtrlPacket), 0);
	if (ret <= 0 || ctrl.msgType != MSG_CTRL)
		return -1;
	*device_id = ctrl.device_id;
	ret = ParseBinary(ctrl.payload, GuessCtrlSensor(ctrl.device_id), dataJson);
	if (ret < 0)
		return -1;
	return 1;
}
