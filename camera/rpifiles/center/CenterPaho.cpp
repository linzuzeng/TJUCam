#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"

#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <time.h>
#include <map>
#include <cstring>
#include "globals.cpp"

#if defined(WIN32)
#define sleep Sleep
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "mqttutil.cpp"
void initialState() {
	CamName.clear();
	TargetName.clear();
	ifGetCamTime.clear();
	ifGetTargetTime.clear();
	ifSendMessage2Cam.clear();
	ifSendMessage2Target.clear();
	ifSendControl2Target.clear();
	CamTime.clear();
	TargetTime.clear();
	for (int i = 0; i < 60; i++) {
		CamValue[i].clear();
		TargetValue[i].clear();
		CamTimeList[i].clear();
		TargetTimeList[i].clear();
	}
	getCamTime = false;
	sendMessage2CamComplete = false;
	sendMessage2TargetComplete = false;
	getTargetTime = false;
	ifFirstTargetTime = true;
	ifHelloComplete = false;
	offlight[0] = 0;
	offlight[1] = 0;
}
int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
	string line;
	string word;
	string nodeName;
	string operationName;
	string type;
    //cout << "centerMode" << centerMode << endl;
    //cout << "opts.nodelimiter:" << opts.nodelimiter << endl;
	//if (message->)
	//cout << "payloda:" <<(char*)message->payload << endl;
	//if (opts.showtopics)
	//	printf("%s\t:\n", topicName);
	if (opts.nodelimiter){
        //string topicS(topicName);
		string payloadST((char*)message->payload);
        string payloadS = payloadST.substr(0, message->payloadlen);
		cout << "centerMode: " << centerMode << endl;
		cout << topicName << ": " <<  payloadS << endl;
		string line = payloadST.substr(0, message->payloadlen);
		stringstream messageStream(line);
		switch (centerMode) {
			case WAITFORHELLO: {
				switch (topicByString.at(topicName)) {
					case HELLO: {
						messageStream >> type;
						messageStream >> nodeName;
						if (type == Type.at(TARGET)
	                    && ifGetTargetTime.count(nodeName) == 0
	                    && ifSendMessage2Target.count(nodeName) == 0
	                    && ifSendControl2Target.count(nodeName) == 0) {
	                        TargetName.push_back(nodeName);
	                        ifGetTargetTime[nodeName] = false;
	                        ifSendMessage2Target[nodeName] = false;
	                        ifSendControl2Target[nodeName] = false;
	                        for (int i = 0; i < 60; i++) {
	                            TargetValue[i].insert(make_pair(nodeName, make_pair(false, "")));
								TargetTimeList[i].insert(make_pair(nodeName, 0));
	                        }
	                    }
	                    else if (type == Type.at(CAMERA)
	                        && ifGetCamTime.count(nodeName) == 0
	                        && ifSendMessage2Cam.count(nodeName) == 0) {
	                        CamName.push_back(nodeName);
	                        ifGetCamTime[nodeName] = false;
	                        ifSendMessage2Cam[nodeName] = false;
	                        for (int i = 0; i < 60; i++) {
	                            CamValue[i][nodeName] = make_pair(false, "");
								CamTimeList[i][nodeName] = 0;
	                        }
	                    }
						if ((TargetName.size() == TargetNumber) && (CamName.size() == CamNumber)) {
		                    //centerMode = SYNCHRONOUS;
		                    //cout << "CenterMode change into SYNCHRONOUS" << endl;
		                    ifHelloComplete = true;
		                }
						break;
					}
					default: break;
				}
				break;
			}
			case SYNCHRONOUS: {
				unsigned long long camTime;
                unsigned long long targetTime;
				switch (topicByString.at(topicName)) {
					case SYNC: {
						messageStream >> type;
						//cout << "Type:" << type << endl;
						messageStream >> nodeName;
						//cout << "nodeName:" << nodeName << endl;
						if (type == Type.at(TARGET)) {
                            messageStream >> targetTime;
                            //cout << "ifGetTargetTime.count(nodeName):" << ifGetTargetTime.count(nodeName) << endl;
                            //cout << "ifSendControl2Target[nodeName]:" << ifSendControl2Target[nodeName] << endl;
                            //cout << "ifGetTargetTime[nodeName]" << ifGetTargetTime.at(nodeName) << endl;
                            if (ifSendControl2Target[nodeName] && (ifGetTargetTime.count(nodeName) == 1) && !ifGetTargetTime[nodeName]) {
                                ifGetTargetTime.at(nodeName) = true;
                                TargetTime[nodeName] = targetTime;
                                //cout << "log:" << "Center get targetTime from " + nodeName << endl;
                                if (ifFirstTargetTime) {
    					            firstTargetTimeForSync = targetTime;
    						        ifFirstTargetTime = false;
    					        }
                            }
                        }
                        else if (type == Type.at(CAMERA)) {
                            messageStream >> camTime;
                            //cout << "log:" << "Center get camTime from " + nodeName << endl;
                            ifGetCamTime.at(nodeName) = true;
                            CamTime[nodeName] = camTime;
                        }
						break;
					}
					case ACK: {
						string tt;
                        messageStream >> tt;
                        messageStream >> tt;
                        if (tt == TargetName[0]) {
                            offlight[0]=0;
                        }
                        /*
                        if (tt == TargetName[1]) {
                            offlight[1]=0;
                        }
                        */
                        break;
					}
					case FORDEBUG:
                        break;
                    default:
                        break;
				}
				break;

			}
			case MEASURE: {
				switch(topicByString.at(topicName)) {
					case TAR: {
						unsigned long long timeT;
		                messageStream >> timeT;
		                messageStream >> nodeName;
		                cout << "in TAR" << endl;
		                TargetValue[timeT/(100*1000)%60].at(nodeName) = make_pair(true, payloadS);
						TargetTimeList[timeT/(100*1000)%60].at(nodeName) = timeT;
						cout << timeT/(100*1000)%60 << endl;
						break;
					}
					case CAM: {
						unsigned long long timeT;
						messageStream >> timeT;
						messageStream >> nodeName;
						cout << "in CAM" << endl;
						CamValue[timeT/(100*1000)%60].at(nodeName) = make_pair(true, payloadS);
						CamTimeList[timeT/(100*1000)%60].at(nodeName) = timeT;
						cout << timeT/(100*1000)%60 << endl;
						break;
					}
					default: break;
				}
				break;
			}
			default: break;

		}
	}
		//printf("%.*s", message->payloadlen, (char*)message->payload);
	else
		printf("%.*s%c", message->payloadlen, (char*)message->payload, opts.delimiter);
	fflush(stdout);
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return 1;
}



void onConnect(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	if (opts.showtopics){
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic1, opts.clientid, opts.qos);
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic2, opts.clientid, opts.qos);
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic3, opts.clientid, opts.qos);
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic4, opts.clientid, opts.qos);
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic5, opts.clientid, opts.qos);
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic5, opts.clientid, opts.qos);
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic6, opts.clientid, opts.qos);
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic7, opts.clientid, opts.qos);
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic8, opts.clientid, opts.qos);
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic9, opts.clientid, opts.qos);
    }


	ropts.onSuccess = onSubscribe;
	ropts.onFailure = onSubscribeFailure;
	ropts.context = client;
	if ((rc = MQTTAsync_subscribe(client, topic1, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe topic1, return code %d\n", rc);
		finished = 1;
	}
    if ((rc = MQTTAsync_subscribe(client, topic2, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe topic2, return code %d\n", rc);
		finished = 1;
	}
    if ((rc = MQTTAsync_subscribe(client, topic3, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe topic3, return code %d\n", rc);
		finished = 1;
	}
    if ((rc = MQTTAsync_subscribe(client, topic4, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe topic4, return code %d\n", rc);
		finished = 1;
	}
    if ((rc = MQTTAsync_subscribe(client, topic5, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe topic5, return code %d\n", rc);
		finished = 1;
	}
    if ((rc = MQTTAsync_subscribe(client, topic6, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe topic6, return code %d\n", rc);
		finished = 1;
	}
    if ((rc = MQTTAsync_subscribe(client, topic7, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe topic7, return code %d\n", rc);
		finished = 1;
	}
    if ((rc = MQTTAsync_subscribe(client, topic8, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe topic8, return code %d\n", rc);
		finished = 1;
	}
    if ((rc = MQTTAsync_subscribe(client, topic9, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe topic9, return code %d\n", rc);
		finished = 1;
	}
}



int main(int argc , char **argv)
{
    ofstream fout;
	fout.open("data.txt");

	MQTTAsync client;
	char* buffer = NULL;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
	int rc = 0;
	char url[100];

	//if (argc < 2)
		//usage();

	topic1 = "control";
    topic2 = "sync";
    topic3 = "hello";
    topic4 = "log";
    topic5 = "ack";
    topic6 = "target";
    topic7 = "cam";
    topic8 = "measurement";
    topic9 = "name";



	if (strchr(topic1, '#') || strchr(topic1, '+'))
		opts.showtopics = 1;
	if (opts.showtopics)
		printf("topic1 is %s\n", topic1);

    if (strchr(topic2, '#') || strchr(topic2, '+'))
    	opts.showtopics = 1;
    if (opts.showtopics)
    	printf("topic2 is %s\n", topic2);

    if (strchr(topic3, '#') || strchr(topic3, '+'))
    	opts.showtopics = 1;
    if (opts.showtopics)
    	printf("topic3 is %s\n", topic3);

    if (strchr(topic4, '#') || strchr(topic4, '+'))
        opts.showtopics = 1;
    if (opts.showtopics)
    	printf("topic4 is %s\n", topic4);

	getopts(argc, argv);
	sprintf(url, "%s:%s", opts.host, opts.port);
	printf("%s\n", url);
	rc = MQTTAsync_create(&client, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTAsync_setCallbacks(client, client, connectionLost, messageArrived, NULL);

	//signal(SIGINT, cfinish);
	//signal(SIGTERM, cfinish);

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
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif


/*
if (finished)
	goto exit;
	while (!finished)
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif
*/
buffer = (char*)malloc(4096);
char delimiter[2]="\n";

pub_opts.onSuccess = onPublish;
pub_opts.onFailure = onPublishFailure;
do
{
	rc = MQTTAsync_send(client, topic3, sizeof("areyouok"), (char*)"areyouok", opts.qos, 0, &pub_opts);
}
while (rc != MQTTASYNC_SUCCESS);
while (!finished)	{		string sendMessage;        //cout << "centerMode: " << centerMode << endl;		if (centerMode == SYNCHRONOUS) {            sendMessage2CamComplete = (ifSendMessage2Cam.size() == CamNumber);            map<string, bool>::iterator iter;            for (iter = ifSendMessage2Cam.begin(); iter != ifSendMessage2Cam.end(); iter++) {                sendMessage2CamComplete = (sendMessage2CamComplete && iter->second);            }            getCamTime = (ifGetCamTime.size() == CamNumber);            for (iter = ifGetCamTime.begin(); iter != ifGetCamTime.end(); iter++) {                getCamTime = (getCamTime && iter->second);            }            sendMessage2TargetComplete = (ifSendMessage2Target.size() == TargetNumber);            for (iter = ifSendMessage2Target.begin(); iter != ifSendMessage2Target.end(); iter++) {                sendMessage2TargetComplete = (sendMessage2TargetComplete && iter->second);            }            int numOfMessage2Target = 0;            for (iter = ifSendMessage2Target.begin();iter != ifSendMessage2Target.end();iter++) {                if (iter->second == true)                    numOfMessage2Target++;            }            getTargetTime = (ifGetTargetTime.size() == TargetNumber);            for (iter = ifGetTargetTime.begin(); iter != ifGetTargetTime.end(); iter++) {                getTargetTime = (getTargetTime && iter->second);            }            int numOfControl2Target = 0;            for (iter = ifSendControl2Target.begin(); iter != ifSendControl2Target.end(); iter++) {                if (iter->second == true) {                    numOfControl2Target++;                }            }            if (numOfControl2Target == 0) {                vector<string> :: iterator iterV;                int jubu=0;                for (iterV = TargetName.begin(); iterV != TargetName.end(); iterV++ ) {                    pub_opts.onSuccess = onPublish;				    pub_opts.onFailure = onPublishFailure;                    string lightOffMessage = light + " " + *iterV + " " + zero + "\n";                    strcpy(buffer, lightOffMessage.c_str());				    int data_len = lightOffMessage.length();				    do				    {					 rc = MQTTAsync_send(client, topic1, data_len, buffer, opts.qos, 0, &pub_opts);                    }                    while (rc != MQTTASYNC_SUCCESS);                    offlight[jubu++] = 1;                    cout << "in jubu = 1" << endl;                }                while (offlight[0] == 1 /*|| offlight[1] == 1*/) {                    //cout << "offlight not zero!" << endl;                    sleep(1);                }                map<string, bool>::iterator iterB;                iterB = ifSendControl2Target.begin();				sendMessage =syncmode + " " + iterB->first  + " 1";				strcpy(buffer, sendMessage.c_str());				int data_len = sendMessage.length();				//printf("Publishing data of length %d\n", data_len);				pub_opts.onSuccess = onPublish;				pub_opts.onFailure = onPublishFailure;				do				{					rc = MQTTAsync_send(client, topic1, data_len, buffer, opts.qos, 0, &pub_opts);				}				while (rc != MQTTASYNC_SUCCESS);                do				{					rc = MQTTAsync_send(client, topic4, data_len, buffer, opts.qos, 0, &pub_opts);				}				while (rc != MQTTASYNC_SUCCESS);                controlTarget = iterB->first;                cout << "log:" << "Send Control to " << iterB->first << endl;                ifSendControl2Target[iterB->first] = true;                ifSendMessage2Target[iterB->first] = true;            }            else if ((numOfMessage2Target == numOfControl2Target) && !sendMessage2TargetComplete && sendMessage2CamComplete) {                map<string, bool>::iterator iterB;                for (iterB = ifSendControl2Target.begin(); iterB != ifSendControl2Target.end(); iterB++) {                    if (iterB->second == false) {                        vector<string> :: iterator iterV;                        int jubu=0;                        for (iterV = TargetName.begin(); iterV != TargetName.end(); iterV++ ) {                            pub_opts.onSuccess = onPublish;				            pub_opts.onFailure = onPublishFailure;                            string lightOffMessage = light + " " + *iterV + " " + zero + "\n";                            strcpy(buffer, lightOffMessage.c_str());				            int data_len = lightOffMessage.length();				            do				            {					            rc = MQTTAsync_send(client, topic1, data_len, buffer, opts.qos, 0, &pub_opts);                            }                            while (rc != MQTTASYNC_SUCCESS);                                offlight[jubu++] = 1;                        }                        while (offlight[0]==1/* || offlight[1]==1*/) {                            //cout << "offlight not zero!" << endl;                            sleep(1);                        }						sendMessage = syncmode + " " + iterB->first + " 1";                        map<string, bool> :: iterator iter;                        for (iter = ifGetCamTime.begin(); iter != ifGetCamTime.end(); iter++) {                            iter->second = false;                        }						strcpy(buffer, sendMessage.c_str());						int data_len = sendMessage.length();						//printf("Publishing data of length %d\n", data_len);						pub_opts.onSuccess = onPublish;						pub_opts.onFailure = onPublishFailure;						do						{							rc = MQTTAsync_send(client, topic1, data_len, buffer, opts.qos, 0, &pub_opts);						}						while (rc != MQTTASYNC_SUCCESS);                		//cout << "log:" << "Send Control to " << iterB->first << endl;                		ifSendControl2Target[iterB->first] = true;                		//ifSendMessage2Target[iterB->first] = true;                        do				        {                            rc = MQTTAsync_send(client, topic4, data_len, buffer, opts.qos, 0, &pub_opts);				        }				        while (rc != MQTTASYNC_SUCCESS);                        controlTarget = iterB->first;                        cout << "log:" << "Send Control to " << iterB->first << endl;                        break;                    }                }            }            if (getCamTime) {                //cout << "numOfControl2Target:" << numOfControl2Target << endl;                //cout << "sendMessage2TargetComplete" << sendMessage2TargetComplete << endl                //cout << "controlTarget:" << controlTarget << endl;                if (!ifFirstTargetTime && numOfControl2Target == 1 && !sendMessage2CamComplete && ifGetTargetTime.count(controlTarget) == 1 && ifGetTargetTime[controlTarget] == true) {                    unsigned long long diff;                    map<string, long long>::iterator iterC;                    stringstream diffStringStream;                    for (iterC = CamTime.begin(); iterC != CamTime.end(); iterC++) {                        diff = firstTargetTimeForSync - iterC->second;						stringstream fT;						fT << firstTargetTimeForSync;						strcpy(buffer, fT.str().c_str());						int data_len = fT.str().length();                        diffStringStream << diff;						do				        {                            rc = MQTTAsync_send(client, topic4, data_len, buffer, opts.qos, 0, &pub_opts);                        }				        while (rc != MQTTASYNC_SUCCESS);                        //cout << "firstTargetTimeForSync:" << firstTargetTimeForSync << endl;                        //cout << iterC->first << ":" << iterC->second << endl;						sendMessage = controlVerbByControlVerb.at(SET_TIME_C) + " " + iterC->first + " " + diffStringStream.str()  + "\n";                        cout << "sendMessage:" << sendMessage << endl;						strcpy(buffer, sendMessage.c_str());						data_len = sendMessage.length();						//printf("Publishing data of length %d\n", data_len);						pub_opts.onSuccess = onPublish;						pub_opts.onFailure = onPublishFailure;						do						{							rc = MQTTAsync_send(client, topic1, data_len, buffer, opts.qos, 0, &pub_opts);						}						while (rc != MQTTASYNC_SUCCESS);                        ifSendMessage2Cam[iterC->first] = true;                        ifGetCamTime[iterC->first] = false;                        do				        {                            rc = MQTTAsync_send(client, topic4, data_len, buffer, opts.qos, 0, &pub_opts);                        }				        while (rc != MQTTASYNC_SUCCESS);                        cout << "log:" << "Seng diff to " << iterC->first << diff << endl;                        diffStringStream.str("");                    }                }                else if (numOfControl2Target >1 && !sendMessage2TargetComplete){                    unsigned long long meanDiff;                    unsigned long long sumCamTime = 0;                    //long long time2Target;                    unsigned long long meanTime;                    stringstream meanDiffStringStream;                    map<string, long long>::iterator iterC;                    for (iterC = CamTime.begin(); iterC != CamTime.end(); iterC++) {                        sumCamTime += iterC->second;                    }                    meanTime = sumCamTime / CamNumber;                    meanDiffStringStream << meanDiff;                    map<string, bool>::iterator iterB;                    for (iterB = ifSendMessage2Target.begin(); iterB != ifSendMessage2Target.end(); iterB++) {                        if ((iterB->second == false) && (ifGetTargetTime[iterB->first] == true)) {                            meanDiffStringStream.str("");                            meanDiff = meanTime - TargetTime[iterB->first];                            meanDiffStringStream << meanDiff;							sendMessage = controlVerbByControlVerb.at(SET_TIME_T) + " " + iterB->first + " " + meanDiffStringStream.str() + "\n";							strcpy(buffer, sendMessage.c_str());							int data_len = sendMessage.length();							//printf("Publishing data of length %d\n", data_len);							pub_opts.onSuccess = onPublish;							pub_opts.onFailure = onPublishFailure;							do							{								rc = MQTTAsync_send(client, topic1, data_len, buffer, opts.qos, 0, &pub_opts);							}							while (rc != MQTTASYNC_SUCCESS);                            ifSendMessage2Target[iterB->first] = true;                            do				            {                                rc = MQTTAsync_send(client, topic4, data_len, buffer, opts.qos, 0, &pub_opts);            	            }				            while (rc != MQTTASYNC_SUCCESS);                            cout << "log:" << "Seng diff to " << iterB->first << meanDiff << endl;                        }                    }                }            }            if (sendMessage2TargetComplete && sendMessage2CamComplete) {                centerMode = MEASURE;                cout << "log:" << "centerMode change into MEASURE." << endl;            }        }        else if (centerMode == WAITFORHELLO) {            if (ifHelloComplete) {                stringstream TargetNumberSS;                TargetNumberSS << TargetNumber;                stringstream CamNumberSS;                CamNumberSS << CamNumber;                string message = TargetNumberSS.str();                vector<string> :: iterator iterS;                for (iterS = TargetName.begin(); iterS != TargetName.end(); iterS++) {                    message = message + " " + (*iterS);                }                message = message + " " + CamNumberSS.str();                for (iterS = CamName.begin(); iterS != CamName.end(); iterS++) {                    message = message + " " + (*iterS);                }                int data_len = message.length();                strcpy(buffer, message.c_str());                do				{				    rc = MQTTAsync_send(client, topic9, data_len, buffer, opts.qos, 0, &pub_opts);				}                while (rc != MQTTASYNC_SUCCESS);                centerMode = SYNCHRONOUS;                cout << "centerMode:" << centerMode << endl;            }        }        else if(centerMode == MEASURE) {            bool ifDataReady = true;            //cout <<"===================================== "<<endl;            map<string, pair<bool, string>> :: iterator iterbS;            for (int i = 0; i < 60; i++) {                for (iterbS = CamValue[i].begin(); iterbS != CamValue[i].end(); iterbS++) {                    ifDataReady = ifDataReady && iterbS->second.first;                    //cout <<iterbS->second.first<<" ";                }                //cout << endl;                for (iterbS = TargetValue[i].begin(); iterbS != TargetValue[i].end(); iterbS++) {                    ifDataReady = ifDataReady && iterbS->second.first;                }                if (ifDataReady) {                					map<string, unsigned long long> :: iterator iterST;					unsigned long long lastTime;					unsigned long long currentTime;					currentTime = CamTimeList[i].begin()->second;					for (iterST = CamTimeList[i].begin(); ifDataReady == true && iterST != CamTimeList[i].end(); iterST++) {						lastTime = currentTime;						currentTime = iterST->second;						if (currentTime/(100*1000) != lastTime/(100*1000)) {							ifDataReady = false;							break;						}					}					for (iterST = TargetTimeList[i].begin(); ifDataReady == true && iterST != TargetTimeList[i].end(); iterST++) {						lastTime = currentTime;						currentTime = iterST->second;						if (currentTime/(100*1000) != lastTime/(100*1000)) {							ifDataReady = false;							break;						}					}					if (!ifDataReady) {						break;					}					if (ifDataReady) {                    	stringstream number;                    	number << TargetNumber;                    	string needSendData = data + "\n" + number.str() + "\n";                    	for (iterbS = TargetValue[i].begin(); iterbS != TargetValue[i].end(); iterbS++) {                        	iterbS->second.first = false;                        	needSendData = needSendData + iterbS->second.second + "\n";                    	}                    	number.str("");                    	number << CamNumber;                    	needSendData +=number.str() + "\n";                    	for (iterbS = CamValue[i].begin(); iterbS != CamValue[i].end(); iterbS++) {                        	needSendData = needSendData + iterbS->second.second + "\n";                        	iterbS->second.first = false;                    	}                    	fout << needSendData << endl;                    	int data_len = needSendData.length();                    	strcpy(buffer, needSendData.c_str());                    	do				    	{				        	rc = MQTTAsync_send(client, topic8, data_len, buffer, opts.qos, 0, &pub_opts);				    	}                    	while (rc != MQTTASYNC_SUCCESS);					}                }                ifDataReady = true;            }        }	}	printf("Stopping\n");

	disc_opts.onSuccess = onDisconnect;
	if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start disconnect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	while	(!disconnected)
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif

exit:
	MQTTAsync_destroy(&client);

	return EXIT_SUCCESS;
}
