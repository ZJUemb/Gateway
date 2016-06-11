#ifndef BINARY_PROTOCOL_H
#define BINARY_PROTOCOL_H

#define TCP_PORT		10659
#define	KEY_SIZE		32
#define PAYLOAD_SIZE	256
typedef enum {
	MSG_ACK = 0,
	MSG_NAK = 1,
	MSG_LOGIN = 2,
	MSG_REPORT = 3,
	MSG_CTRL = 4,
	MSG_LOGOUT = 5
} useless;
typedef char MsgType;

typedef struct {
	MsgType		msgType;
	int			deviceId;
	char		key[KEY_SIZE];
} __attribute__((packed)) LoginPacket;

typedef struct {
	MsgType		msgType;
} __attribute__((packed)) AckPacket;
typedef struct {
	MsgType		msgType;
} __attribute__((packed)) NakPacket;

typedef struct {
	MsgType		msgType;
	int			device_id;
	int			reserved;
	char		payload[PAYLOAD_SIZE];
} __attribute__((packed)) ReportPacket;



typedef struct {
	MsgType		msgType;
	int			device_id;
	char		payload[PAYLOAD_SIZE];
} __attribute__((packed)) CtrlPacket;
typedef struct {
	MsgType		msgType;
} __attribute__((packed)) LogoutPacket;

#endif // !BINARY_PROTOCOL_H
