
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
	"CenterNode", 1, '\n', 2, NULL, NULL, "localhost", "1883", 1, 10
};


void usage(void)
{
	printf("MQTT stdout subscriber\n");
	exit(EXIT_FAILURE);
}


void getopts(int argc, char** argv)
{
	int count = 1;

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
MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;


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

void onPublishFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Publish failed, rc %d\n", response ? -1 : response->code);
	//published = -1;
}


void onPublish(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;

	//published = 1;
}
