#include "lzzvr.h"
#include <cstring>
#include <string>
#include <sstream>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"
#include <windows.h>
#include "driver_hydra.h"
#include <iostream>
#include <fstream>	//文件读写
using namespace std;
using std::string;
#define WIN64
#if defined(WIN64)
#define sleep Sleep
#else
#include <time.h>
#include <unistd.h>
#endif
sixenseAllControllerData cc;
//参数传递中的全局变量

//参数传递中的全局变量

//异步通讯的初始化信息和函数

volatile int finished = 0;
//char* topic = NULL;
int subscribed = 0;
int disconnected = 0;
char topic[40] = "shangweiji";
char lefthand[40] = "shangweiji";
char righthand[40] = "shangweiji";
void cfinish(int sig)
{
	signal(SIGINT, NULL);
	finished = 1;
}


struct
{
	char* clientid;
	int nodelimiter;
	char delimiter;
	int qos;
	char* username;
	char* password;
	char* host;
	char* port;
	int showtopics;
	int keepalive;
} opts =
{
	"steamvr", 1, '\n', 2, "admin", "password", "localhost", "1883", 0, 10
};
MQTTAsync client;
int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{	//在线程中不停接收信息，顺便进行字符串处理分配给各个变量
	char* msg = (char*)message->payload;
	float axis_x;
	float axis_y;
	float axis_z;
	float acc_x;
	float acc_y;
	float acc_z;
	float Quaternion1;
	float Quaternion2;
	float Quaternion3;
	float Quaternion4;
	float joystick_x;
	float joystick_y;
	char button[5];
	char name[40];
	int datatime;
	stringstream flow(msg);
	flow >> datatime;
	flow >> name;
	flow >> Quaternion1;
	flow >> Quaternion2;
	flow >> Quaternion3;
	flow >> Quaternion4;
	flow >> acc_x;
	flow >> acc_y;
	flow >> acc_z;
	flow >> button;
	flow >> joystick_x;
	flow >> joystick_y;
	flow >> axis_x;
	flow >> axis_y;
	flow >> axis_z;
	
	if (strcmp(lefthand, name) == 0) {
		(cc).controllers[0].pos[0] = axis_x;
		(cc).controllers[0].pos[1] = axis_y;
		(cc).controllers[0].pos[2] = axis_z;
		(cc).controllers[0].joystick_x = joystick_x;
		(cc).controllers[0].joystick_y = joystick_y;
		(cc).controllers[0].trigger = button[0];
		(cc).controllers[0].buttons1 = button[1];
		(cc).controllers[0].buttons2 = button[2];
		(cc).controllers[0].buttons3 = button[3];
		(cc).controllers[0].buttons4 = button[4];
		(cc).controllers[0].sequence_number = datatime;
		(cc).controllers[0].rot_quat[0] = Quaternion1;
		(cc).controllers[0].rot_quat[1] = Quaternion2;
		(cc).controllers[0].rot_quat[2] = Quaternion3;
		(cc).controllers[0].rot_quat[3] = Quaternion4;
		(cc).controllers[0].firmware_revision = 1;
		(cc).controllers[0].hardware_revision = 1;
		(cc).controllers[0].packet_type = 1;
		(cc).controllers[0].magnetic_frequency = 9;
		(cc).controllers[0].enabled = 1;
		(cc).controllers[0].controller_index = 0;
		(cc).controllers[0].is_docked = 0;
		(cc).controllers[0].which_hand = 1;
		(cc).controllers[0].hemi_tracking_enabled = 0;
		DriverLog("Left Hand received %f %f %f %f %f %f %f \n", axis_x, axis_y, axis_z, Quaternion1, Quaternion2, Quaternion3, Quaternion4);
	}
	if (strcmp(righthand, name) == 0) {
		(cc).controllers[1].pos[0] = axis_x - 0.9;
		(cc).controllers[1].pos[1] = axis_y;
		(cc).controllers[1].pos[2] = axis_z;
		(cc).controllers[1].joystick_x = 0;
		(cc).controllers[1].joystick_y = 0;
		(cc).controllers[1].trigger = 0;
		(cc).controllers[1].buttons1 = 0;
		(cc).controllers[1].buttons2 = 0;
		(cc).controllers[1].buttons3 = 0;
		(cc).controllers[1].buttons4 = 0;
		(cc).controllers[1].sequence_number = datatime;
		(cc).controllers[1].rot_quat[0] = 0;
		(cc).controllers[1].rot_quat[1] = 0;
		(cc).controllers[1].rot_quat[2] = 0;
		(cc).controllers[1].rot_quat[3] = 1;
		(cc).controllers[1].firmware_revision = 1;
		(cc).controllers[1].hardware_revision = 1;
		(cc).controllers[1].packet_type = 1;
		(cc).controllers[1].magnetic_frequency = 9;
		(cc).controllers[1].enabled = 1;
		(cc).controllers[1].controller_index = 0;
		(cc).controllers[1].is_docked = 0;
		(cc).controllers[1].which_hand = 2;
		(cc).controllers[1].hemi_tracking_enabled = 0;
		DriverLog("Right Hand received %f %f %f %f %f %f %f \n", axis_x, axis_y, axis_z, Quaternion1, Quaternion2, Quaternion3, Quaternion4);
	}
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return 1;
}


void onDisconnect(void* context, MQTTAsync_successData* response)
{
	
	disconnected = 1;
}


void onSubscribe(void* context, MQTTAsync_successData* response)
{
	DriverLog("lzzvr:订阅成功\n");
	subscribed = 1;
}


void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	//printf("Subscribe failed, rc %d\n", response->code);
	finished = 1;
}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	//printf("Connect failed, rc %d\n", response->code);
	finished = 1;
}

void onConnect(void* context, MQTTAsync_successData* response)
{
	DriverLog("lzzvr:连接上了\n");
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	//if (opts.showtopics)
	//printf("Subscribing to topic %s with client %s at QoS %d\n", topic, opts.clientid, opts.qos);

	ropts.onSuccess = onSubscribe;
	ropts.onFailure = onSubscribeFailure;
	ropts.context = client;
	if ((rc = MQTTAsync_subscribe(client, topic, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		//printf("Failed to start subscribe, return code %d\n", rc);
		finished = 1;
		DriverLog("lzzvr 订阅失败\n");
	}
}


MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;


void connectionLost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync)context;
	int rc;

	//printf("connectionLost called\n");
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		//printf("Failed to start reconnect, return code %d\n", rc);
		finished = 1;
	}
}
//异步通讯的初始化信息和函数

int lzzvrInit(void) {	//初始化函数，由hydra调用
						//异步接收部分_在初始化函数中建立客户端、连接服务器、建立setCallBacks线程
	int rc = 0;
	char url[100];

	char username[20];
	char password[20];
	char host[20];
	char port[20];
	
	ifstream fin("C:\\lzzvrconfig.txt");
	if (fin.fail())
	{
		DriverLog("文件打开失败\n");
	}
	else {
		fin >> username;
		fin >> password;
		fin >> host;
		fin >> port;
		fin >> topic;
		fin >> lefthand;
		fin >> righthand;
	}
	opts.username = username;
	opts.password = password;
	opts.host = host;
	opts.port = port;

	sprintf(url, "%s:%s", opts.host, opts.port);
	rc = MQTTAsync_create(&client, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTAsync_setCallbacks(client, client, connectionLost, messageArrived, NULL);

	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);
	conn_opts.keepAliveInterval = opts.keepalive;
	conn_opts.cleansession = 1;
	conn_opts.username = opts.username;
	conn_opts.password = opts.password;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		//printf("Failed to start connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	while (!subscribed)
	#if defined(WIN64)
		Sleep(100);
	#else
		usleep(10000L);
	#endif
	DriverLog("LZZVR: 连接上啦\n");

	return SIXENSE_SUCCESS;
};
int lzzvrExit(void) {
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	disc_opts.onSuccess = onDisconnect;
	int rc = 0;
	if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
	{
		//printf("Failed to start disconnect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	while (!disconnected)
#if defined(WIN64)
		Sleep(100);
#else
		usleep(10000L);
#endif


	MQTTAsync_destroy(&client);

	//return EXIT_SUCCESS;
	//异步接收部分——在初始化函数中建立客户端、连接服务器、建立setCallBacks线程
	return SIXENSE_SUCCESS;
};//

int sixenseGetMaxBases() {
	return 1;
};
int sixenseSetActiveBase(int i) {
	return SIXENSE_SUCCESS;
};
int sixenseIsBaseConnected(int i) {
	return 1;
};

int sixenseGetMaxControllers(void) {
	return 2;
};
int sixenseIsControllerEnabled(int which) {
	return 1;
};

int sixenseGetAllNewestData(sixenseAllControllerData *out) {
	
	memcpy(out, &cc, sizeof(sixenseAllControllerData));
	return SIXENSE_SUCCESS;
};
int sixenseAutoEnableHemisphereTracking(int which_controller) {
	return SIXENSE_SUCCESS;
}
int sixenseSetHighPriorityBindingEnabled(int on_or_off)
{
	return 0;
};
int sixenseTriggerVibration(int controller_id, int duration_100ms, int pattern_id) {
	return SIXENSE_SUCCESS;
};
