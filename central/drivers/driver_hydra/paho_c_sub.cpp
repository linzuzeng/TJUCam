#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"

#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <sstream>
#include <windows.h>

using namespace std;
using std::string;

#define WIN64
#if defined(WIN64)
#define sleep Sleep
#else
#include <time.h>
#include <unistd.h>
#endif

float axis_x;
float axis_y;
float axis_z;
float Q1;
float Q2;
float Q3;
float Q4;
int joystick_x;
int joystick_y;
char button[3];
char name[20];

volatile int finished = 0;
char* topic = NULL;
int subscribed = 0;
int disconnected = 0;


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
	"steamvr", 1, '\n', 2, "admin", "password", "192.168.43.173", "1883", 0, 10
};

/*
void usage(void)
{
	printf("MQTT stdout subscriber\n");
	printf("Usage: stdoutsub topicname <options>, where options are:\n");
	printf("  --host <hostname> (default is %s)\n", opts.host);
	printf("  --port <port> (default is %s)\n", opts.port);
	printf("  --qos <qos> (default is %d)\n", opts.qos);
	printf("  --delimiter <delim> (default is no delimiter)\n");
	printf("  --clientid <clientid> (default is %s)\n", opts.clientid);
	printf("  --username none\n");
	printf("  --password none\n");
	printf("  --showtopics <on or off> (default is on if the topic has a wildcard, else off)\n");
	printf("  --keepalive <seconds> (default is 10 seconds)\n");
	exit(EXIT_FAILURE);
}


void getopts(int argc, char** argv)
{
	int count = 2;

	while (count < argc)
	{
		if (strcmp(argv[count], "--qos") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "0") == 0)
					opts.qos = 0;
				else if (strcmp(argv[count], "1") == 0)
					opts.qos = 1;
				else if (strcmp(argv[count], "2") == 0)
					opts.qos = 2;
				else
					usage();
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--host") == 0)
		{
			if (++count < argc)
				opts.host = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--port") == 0)
		{
			if (++count < argc)
				opts.port = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--clientid") == 0)
		{
			if (++count < argc)
				opts.clientid = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--username") == 0)
		{
			if (++count < argc)
				opts.username = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--password") == 0)
		{
			if (++count < argc)
				opts.password = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--delimiter") == 0)
		{
			if (++count < argc)
			{
				if (strcmp("newline", argv[count]) == 0)
					opts.delimiter = '\n';
				else
					opts.delimiter = argv[count][0];
				opts.nodelimiter = 0;
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--showtopics") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "on") == 0)
					opts.showtopics = 1;
				else if (strcmp(argv[count], "off") == 0)
					opts.showtopics = 0;
				else
					usage();
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--keepalive") == 0)
		{
			if (++count < argc)
				opts.keepalive = atoi(argv[count]);
			else
				usage();
		}
		count++;
	}

}
*/

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
	stringstream flow((char*)message->payload);
	if (opts.showtopics)
		printf("%s\t", topicName);
	if (opts.nodelimiter) {
		flow >> name;
		flow >> name;
		flow >> Q1;
		flow >> Q2;
		flow >> Q3;
		flow >> Q4;
		flow >> button;
		flow >> joystick_x;
		flow >> joystick_y;
	}
		//printf("%.*s", message->payloadlen, (char*)message->payload);
	else {
		flow >> name;
		flow >> name;
		flow >> Q1;
		flow >> Q2;
		flow >> Q3;
		flow >> Q4;
		flow >> button;
		flow >> joystick_x;
		flow >> joystick_y;
	}
		//printf("%.*s%c", message->payloadlen, (char*)message->payload, opts.delimiter);
	fflush(stdout);
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
	subscribed = 1;
}


void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Subscribe failed, rc %d\n", response->code);
	finished = 1;
}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Connect failed, rc %d\n", response->code);
	finished = 1;
}


void onConnect(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	if (opts.showtopics)
		printf("Subscribing to topic %s with client %s at QoS %d\n", topic, opts.clientid, opts.qos);

	ropts.onSuccess = onSubscribe;
	ropts.onFailure = onSubscribeFailure;
	ropts.context = client;
	if ((rc = MQTTAsync_subscribe(client, topic, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe, return code %d\n", rc);
		finished = 1;
	}
}


MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;


void connectionLost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync)context;
	int rc;

	printf("connectionLost called\n");
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start reconnect, return code %d\n", rc);
		finished = 1;
	}
}


int main(int argc, char** argv)
{
	
	MQTTAsync client;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	int rc = 0;
	char url[100];

	//if (argc < 2)
		//usage();

	topic = "shangweiji";

	if (strchr(topic, '#') || strchr(topic, '+'))
		opts.showtopics = 1;
	if (opts.showtopics)
		printf("topic is %s\n", topic);

	//getopts(argc, argv);
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
		printf("Failed to start connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	while (!subscribed)
		#if defined(WIN64)
			Sleep(100);
		#else
			usleep(10000L);
		#endif

	if (finished)
		goto exit;

	while (!finished)
		#if defined(WIN64)
			Sleep(100);
		#else
			usleep(10000L);
		#endif

	disc_opts.onSuccess = onDisconnect;
	if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start disconnect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	while	(!disconnected)
		#if defined(WIN64)
			Sleep(100);
		#else
			usleep(10000L);
		#endif

exit:
	MQTTAsync_destroy(&client);

	return EXIT_SUCCESS;
}

