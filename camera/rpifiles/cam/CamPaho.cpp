 #include "MQTTAsync.h"
#include "MQTTClientPersistence.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <time.h>
#include <map>
#include <cstring>
#include <opencv2/opencv.hpp>
#include <stack>
#include <sys/time.h>

using namespace std;
using namespace cv;
#define topic1 "control"
#define topic2 "sync"
#define topic3 "hello"
#define topic4 "log"
#define topic5 "cam"
#if defined(WIN32)
#define sleep Sleep
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "globals.cpp"

#include "mqttutil.cpp"

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
	 if (opts.showtopics)
		  printf("messageArrive at %s:\n", topicName);
   string line((char*)message->payload);
   stringstream messageStream(line);
		switch (topicByString.at(topicName)) {
			case HELLO: {
				string helloVerb;
				messageStream >> helloVerb;
				//cout << "controlVerb:" << controlVerb << endl;
				if (helloVerb == areyouok) {
					cout << "ifGetAreyouok change to true" << endl;
					ifGetAreyouok = true;
				}
				break;
			}
			case CONTROL: {
				string controlVerb;
				messageStream >> controlVerb;
				cout << "controlVerb:" << controlVerb << endl;
				switch (controlVerbByString.at(controlVerb)) {
					case SYNC_MODE: {
						needFoundCentroid = true;
						camNodeMode = SYNCHRONOUS;
						cout << "in syncmode" << endl;
						break;
					}
					case SET_TIME_C: {
						string nodeName;
                        unsigned long long ttt;
						messageStream >> nodeName;
						if (nodeName == thisNodeName) {
                            unsigned long long diffinp;
							messageStream >> diffinp;
                            diff+=diffinp;
							needFoundCentroid = false;
							cout << "in settime_c" << endl;
						}
						break;
					}
					default:
						break;
				}
				break;
			}
			default:
				break;
		}
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
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic3, opts.clientid, opts.qos);
    }

	ropts.onSuccess = onSubscribe;
	ropts.onFailure = onSubscribeFailure;
	ropts.context = client;
	if ((rc = MQTTAsync_subscribe(client, topic1, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe %s, return code %d\n",topic1, rc);
		finished = 1;
	}
 if ((rc = MQTTAsync_subscribe(client, topic3, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe %s, return code %d\n",topic3, rc);
		finished = 1;
	}
}

#define HSVWEIGHT(x,y) (inputImage.at<Vec3b>(x, y)[0]+inputImage.at<Vec3b>(x, y)[1]+inputImage.at<Vec3b>(x, y)[2])
#define HSVCONDITION(x,y) ( HSVWEIGHT(x,y) >90 )
void floodFillStack(Mat& inputImage, int x, int y, vector<pair<int, int>>& domain) {
	std::stack<std::pair<int, int>> target;
	int rowNumber = inputImage.rows;
	int colNumber = inputImage.cols;
	int nx;
	int ny;
	static const int dx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	static const int dy[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };
	//uchar thresholdValue = 40;
	target.push(std::make_pair(x, y));
	inputImage.at<Vec3b>(x, y) = Vec3b(0,0,0);
	while (target.size()) {
		domain.push_back(target.top());
		int currentX = target.top().first;
		int currentY = target.top().second;
		target.pop();
		for (int i = 0; i < 8; i++) {
			nx = currentX + dx[i];
			ny = currentY + dy[i];
			if (nx >= 0 && nx < rowNumber && ny >= 0 && ny < colNumber && HSVCONDITION(nx,ny)) {
				target.push(std::make_pair(nx, ny));
				inputImage.at<Vec3b>(nx, ny)  = Vec3b(0,0,0);
			}
		}
	}
}

void findCentroid(Mat& inputImage, vector<vector<pair<int, int>>>& domain, const int& domainNumber, vector<pair<double, double>>&  centroid) {

	int colNumber = inputImage.cols;
	int rowNumber = inputImage.rows;
	Mat copyImage = inputImage.clone();
	vector<pair<int, int>> tempvector;
	int k = 0;
	for (int i = 0; i < rowNumber; i++) {
		for (int j = 0; j < colNumber; j++) {
			if ( HSVCONDITION(i,j) ) {
				//std::cout << "jesus \n";
				floodFillStack(copyImage, i, j, tempvector);
				//std::cout << "jesus boom boomboomboom\n";
				domain.push_back(tempvector);
				tempvector.clear();
				k++;
			}
			if (k == domainNumber)
				break;
		}
		if (k == domainNumber)
			break;
	}
	if (domain.size() == 0)
		return;
	for (int i = 0; i < domainNumber && domain[i].size() > 0 && k != i; i++) {
		double nx = 0;
		double ny = 0;
		double sumValue = 0;
		for (vector<pair<int, int>>::iterator vectPairIt = domain[i].begin(); vectPairIt != domain[i].end(); vectPairIt++) {
			//std::cout << "domain[i]:"<<domain[i].size() << std::endl;
			nx += vectPairIt->first * HSVWEIGHT(vectPairIt->first, vectPairIt->second);
			ny += vectPairIt->second * HSVWEIGHT(vectPairIt->first, vectPairIt->second);
			sumValue += HSVWEIGHT(vectPairIt->first, vectPairIt->second);
		}
		//if (sumValue>0)
		//std::cout << "aaa:" << sumValue << std::endl;
        if (sumValue != 0) {
            centroid.push_back(std::make_pair(nx / sumValue, ny / sumValue));
        }
		//centroid.push_back(std::make_pair(nx / sumValue, ny / sumValue));
	}
}


int main(int argc , char **argv)
{
 get_name();
 getopts(argc, argv);

	MQTTAsync client;
	char* buffer = NULL;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
	int rc = 0;
	char url[100];

	sprintf(url, "%s:%s", opts.host, opts.port);
	printf("%s\n", url);
	rc = MQTTAsync_create(&client, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTAsync_setCallbacks(client, client, connectionLost, messageArrived, NULL);

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

    VideoCapture capture1(0);
    cout << "Camera initialized." <<endl;
    buffer = (char*)malloc(4096);

    while (!finished)
	{
		if (ifGetAreyouok) {
            cout << "in if GetAreyouok" << endl;
								string message;
            message = thisNodeType + " " + thisNodeName;
            strcpy(buffer, message.c_str());
			int data_len = message.length();
            do
			{
				rc = MQTTAsync_send(client, topic3, data_len, buffer, opts.qos, 0, &pub_opts);
	        }
			while (rc != MQTTASYNC_SUCCESS);
            cout << "log:" << message << endl;
            camNodeMode = SYNCHRONOUS;
			ifGetAreyouok = false;
		}
		else if (needFoundCentroid) {
            stringstream timeNowStream;
            cout << "needFoundCentroid == true" << endl;
            centroid.clear();
			region.clear();
            while (1) {
                capture1.grab();
                timeNow = time_YU(diff);
                //cout << "timeNow:" << timeNow << endl;
                capture1.retrieve(oriImage);
                if(!oriImage.empty()) {
                    //resize(oriImage, oriImage, Size(640, 480), 0, 0, CV_INTER_LINEAR);
                    findCentroid(oriImage, region, Number_of_regions, centroid);
                    if (centroid.size() > 0) {
                        timeNowStream << timeNow;
                        waitKey(30);
                        break;
                    }
                }
            }
            if (centroid.size() > 0) {
                string message = thisNodeType + " " + thisNodeName + " " + timeNowStream.str();
                strcpy(buffer, message.c_str());
				int data_len = message.length();
      do
				{
			        rc = MQTTAsync_send(client, topic2, data_len, buffer, opts.qos, 0, &pub_opts);
				}
				while (rc != MQTTASYNC_SUCCESS);
                cout << "timeNowStream.str():" << timeNowStream.str() << endl;
                cout << "log:" << message << endl;
                needFoundCentroid = false;
            }
        }
        else {
            //cout << "in measutr" << endl;
            capture1.grab();
            timeNow = time_YU(diff);
            capture1.retrieve(oriImage);
            //cout << oriImage.empty() << endl;
            centroid.clear();
			region.clear();
            findCentroid(oriImage, region, Number_of_regions, centroid);
            //if (!oriImage.empty())
            //    imshow("oriImage", oriImage);
            //waitKey(100);
            stringstream timeString;
            timeString << timeNow;
            stringstream xString,yString;
            if (centroid.size() > 0) {
                xString << centroid[0].first;
                yString << centroid[0].second;
                //cout << thisNodeName<< " "<< "x: " << centroid[0].first << " " << "y: " << centroid[0].second << endl;
            }else{
                xString << -1.0;
                yString << -1.0;
            }
            string message = timeString.str() + " " + thisNodeName + " " + xString.str() + " " + yString.str();

            strcpy(buffer, message.c_str());
            int data_len = message.length();
            do
            {
                rc = MQTTAsync_send(client, topic5, data_len, buffer, opts.qos, 0, &pub_opts);
            }while (rc != MQTTASYNC_SUCCESS);
        }
    }

	printf("Stopping\n");


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


unsigned long long  time_YU(unsigned long long diff) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    unsigned long long timeNow ;
        timeNow = (tv.tv_sec%3600)*1000000 + tv.tv_usec + diff;

    return timeNow;
}
