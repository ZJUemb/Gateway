#ifndef BINARY_CONFIG_H
#define BINARY_CONFIG_H


#define DataLenMax	16
#define SensorMax	10
typedef enum{
	DataType_NONE,
	DataType_Int,
	DataType_Float,
	DataType_String,
}DataType;
typedef struct{
	const char* dataName;
	DataType type;
	int length;
}DataItem;
typedef struct{
	int device_id;
	int dataCount;
	DataItem dataConfigs[DataLenMax];
}SensorConfig;
typedef enum{
	Sensor_872 = 0,
	Sensor_875 = 1,
	Sensor_870 = 2,
	Sensor_883 = 3
}SensorID;
//Copy From Server
typedef enum{
	DeviceID_872 = 872,
	DeviceID_875 = 875,
	DeviceID_870 = 24,
	DeviceID_883 = 25,
}DeviceID;

SensorConfig* GuessReportSensor(int device_id);
SensorConfig* GuessCtrlSensor(int device_id);

#endif
