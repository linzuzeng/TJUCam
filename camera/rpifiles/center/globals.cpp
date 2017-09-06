
using namespace std;
enum CENTER_MODE {
    MEASURE = 0,
    SYNCHRONOUS = 1,
    BLANK = 2,
    WAITFORHELLO = 3
};

enum TOPIC {
    CONTROL = 0,
    SYNC = 1,
    HELLO = 2,
    LOG = 3,
    MEASUREMENT = 4,
    ACK = 5,
    TAR = 6,
    CAM =7,
    NAME = 8
};
enum CONTROL_VERB {
    SYNC_MODE = 0,
    SET_TIME_C = 1,
    SET_TIME_T = 2,
    FLASH_ON_MODE = 3,
    FLASH_OFF_MODE = 4,
    FLASH_PRID = 5,
    FLASH_MODE = 6,
    MAGIC = 7,
    ACCCAL = 8,
    LEAVE = 9,
    LIGHT = 10,
    FORDEBUG = 11
};
const map<string, TOPIC> topicByString {
    map<string, TOPIC> :: value_type("control", CONTROL),
    map<string, TOPIC> :: value_type("sync", SYNC),
    map<string, TOPIC> :: value_type("hello", HELLO),
    map<string, TOPIC> :: value_type("log", LOG),
    map<string, TOPIC> :: value_type("measurement", MEASUREMENT),
    map<string, TOPIC> :: value_type("ack", ACK),
    map<string, TOPIC> :: value_type("target", TAR),
    map<string, TOPIC> :: value_type("cam", CAM),
    map<string, TOPIC> :: value_type("name", NAME)
};
const map<TOPIC, string> topicByTOPIC {
    map<TOPIC, string> :: value_type(CONTROL, "control"),
    map<TOPIC, string> :: value_type(SYNC, "sync"),
    map<TOPIC, string> :: value_type(HELLO, "hello"),
    map<TOPIC, string> :: value_type(LOG, "log"),
    map<TOPIC, string> :: value_type(MEASUREMENT, "measurement"),
    map<TOPIC, string> :: value_type(ACK, "ack"),
    map<TOPIC, string> :: value_type(TAR, "target"),
    map<TOPIC, string> :: value_type(CAM, "cam"),
    map<TOPIC, string> :: value_type(NAME, "name")
};
const map<string, CONTROL_VERB> controlVerbByString {
    map<string, CONTROL_VERB> :: value_type("settime_t", SET_TIME_T),
    map<string, CONTROL_VERB> :: value_type("settime_c", SET_TIME_C),
    map<string, CONTROL_VERB> :: value_type("syncmode", SYNC_MODE),
    map<string, CONTROL_VERB> :: value_type("flashonmode", FLASH_ON_MODE),
    map<string, CONTROL_VERB> :: value_type("flashoffmode", FLASH_OFF_MODE),
    map<string, CONTROL_VERB> :: value_type("flashprid", FLASH_PRID),
    map<string, CONTROL_VERB> :: value_type("flashmode", FLASH_MODE),
    map<string, CONTROL_VERB> :: value_type("magic", MAGIC),
    map<string, CONTROL_VERB> :: value_type("acccal", ACCCAL),
    map<string, CONTROL_VERB> :: value_type("leave", LEAVE),
    map<string, CONTROL_VERB> :: value_type("light", LIGHT),
    map<string, CONTROL_VERB> :: value_type("forDebug", FORDEBUG)
};
const map<CONTROL_VERB, string> controlVerbByControlVerb {
    map<CONTROL_VERB, string> :: value_type(SET_TIME_T, "settime_t"),
    map<CONTROL_VERB, string> :: value_type(SET_TIME_C, "settime_c"),
    map<CONTROL_VERB, string> :: value_type(SYNC_MODE, "syncmode"),
    map<CONTROL_VERB, string> :: value_type(FLASH_ON_MODE, "flashonmode"),
    map<CONTROL_VERB, string> :: value_type(FLASH_OFF_MODE, "flashoffmode"),
    map<CONTROL_VERB, string> :: value_type(FLASH_PRID,"flashprid"),
    map<CONTROL_VERB, string> :: value_type(FLASH_MODE, "flashmode"),
    map<CONTROL_VERB, string> :: value_type(MAGIC, "magic"),
    map<CONTROL_VERB, string> :: value_type(ACCCAL, "acccal"),
    map<CONTROL_VERB, string> :: value_type(LEAVE, "leave"),
    map<CONTROL_VERB, string> :: value_type(LIGHT, "light"),
    map<CONTROL_VERB, string> :: value_type(FORDEBUG, "forDebug")

};
enum TYPE {
    TARGET = 0,
    CAMERA = 1
};

int CamNumber = 3;
int TargetNumber = 1;
int periode = 80;   //Unit: ms
CENTER_MODE centerMode = WAITFORHELLO;
int HelloCount = 0;
vector<string> CamName;
vector<string> TargetName;
const map<TYPE, string> Type {
	map<TYPE, string> :: value_type(TARGET, "TARGET"),
	map<TYPE, string> :: value_type(CAMERA, "CAMERA")
};
map<string, bool> ifGetCamTime;
map<string, bool> ifGetTargetTime;
//map<string, bool> ifCamSettingComplete;
//map<string, bool> ifTargetSettingComplete;
map<string, bool> ifSendMessage2Cam;
map<string, bool> ifSendMessage2Target;
map<string, bool> ifSendControl2Target;
map<string, long long> CamTime;
map<string, long long> TargetTime;
map<string, pair<bool, string>> CamValue[60];
map<string, pair<bool,string>> TargetValue[60];
map<string, unsigned long long> CamTimeList[60];
map<string, unsigned long long> TargetTimeList[60];
//bool camComplete = true;
bool getCamTime = false;
bool sendMessage2CamComplete = false;
bool sendMessage2TargetComplete = false;
bool getTargetTime = false;
bool ifFirstTargetTime = true;
unsigned long long firstTargetTimeForSync;
const string syncmode = "syncmode";
const string light = "light";
const string areyouok = "areyouok ";
const string name = "name";
const string data = "data";
const string leave = "leave";
string controlTarget;
bool ifHelloComplete = false;
int offlight[2] = {0,0};
const string zero = "0";

volatile int finished = 0;
char* topic1 = NULL;
char* topic2 = NULL;
char* topic3 = NULL;
char* topic4 = NULL;
char* topic5 = NULL;
char* topic6 = NULL;
char* topic7 = NULL;
char* topic8 = NULL;
char* topic9 = NULL;
char* topic10 = NULL;
int subscribed = 0;
int disconnected = 0;
