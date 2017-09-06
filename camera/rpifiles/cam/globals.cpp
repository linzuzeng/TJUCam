
volatile int finished = 0;

int subscribed = 0;
int disconnected = 0;
enum TOPIC {
    CONTROL = 0,
    SYNC = 1,
    HELLO = 2,
    LOG = 3,
    MEASUREMENT = 4,
    ACK = 5,
    TAR = 6,
    CAM =7
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
    LIGHT = 10
};

enum TYPE {
    TARGET = 0,
    CAMERA = 1
};

enum CAM_NODE_MODE {
    NEEDHELLO = 0,
    SYNCHRONOUS = 1,
    MEASURE = 2
};
const map<string, TOPIC> topicByString {
    map<string, TOPIC> :: value_type("control", CONTROL),
    map<string, TOPIC> :: value_type("sync", SYNC),
    map<string, TOPIC> :: value_type("hello", HELLO),
    map<string, TOPIC> :: value_type("log", LOG),
    map<string, TOPIC> :: value_type("measurement", MEASUREMENT),
    map<string, TOPIC> :: value_type("ack", ACK)
};
const map<TOPIC, string> topicByTOPIC {
    map<TOPIC, string> :: value_type(CONTROL, "control"),
    map<TOPIC, string> :: value_type(SYNC, "sync"),
    map<TOPIC, string> :: value_type(HELLO, "hello"),
    map<TOPIC, string> :: value_type(LOG, "log"),
    map<TOPIC, string> :: value_type(MEASUREMENT, "measurement"),
    map<TOPIC, string> :: value_type(ACK, "ack")
};
const map<TYPE, string> Type {
    map<TYPE, string> :: value_type(TARGET, "TARGET"),
    map<TYPE, string> :: value_type(CAMERA, "CAMERA")
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
    map<string, CONTROL_VERB> :: value_type("light", LIGHT)
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
    map<CONTROL_VERB, string> :: value_type(LIGHT, "light")
};
string thisNodeName = "CAM_1";
const string thisNodeType = Type.at(CAMERA);
Mat oriImage;

vector<vector<pair<int, int>>> region;
vector<pair<double, double>> centroid;
int Number_of_regions = 1;
unsigned long long diff = 0;
unsigned long long timeNow = 0;
CAM_NODE_MODE camNodeMode = NEEDHELLO;
bool needFoundCentroid = false;
bool ifGetAreyouok = false;
const string areyouok = "areyouok";

unsigned long long time_YU(unsigned long long diff);
