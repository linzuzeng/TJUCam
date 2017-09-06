#include <PubSubClient.h>
#include <Ticker.h>
#include <StreamString.h>
#include <Wire.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

bool FactoryMode = false;

// MPU9250 RELATED
#define I2C_NOSTOP false
#define SerialDebug false  // set to true to get Serial output for debugging
#define I2C_SCL 14
#define I2C_SDA 12

int16_t tempCount;            // temperature raw count output
float   temperature;          // Stores the MPU9250 gyro internal chip temperature in degrees Celsius
double Temperature, Pressure; // stores MS5637 pressures sensor pressure and temperature

uint32_t delt_t = 0, count = 0, sumCount = 0;  // used to control display output rate
float pitch, yaw, roll;
float a12, a22, a31, a32, a33;            // rotation matrix coefficients for Euler angles and gravity components
float deltat = 0.0f, sum = 0.0f;          // integration interval for both filter schemes
uint32_t lastUpdate = 0, firstUpdate = 0; // used to calculate integration interval

float ax, ay, az, gx, gy, gz, mx, my, mz; // variables to hold latest sensor data values
float lin_ax, lin_ay, lin_az;             // linear acceleration (acceleration with gravity component subtracted)
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};    // vector to hold quaternion
float eInt[3] = {0.0f, 0.0f, 0.0f};       // vector to hold integral error for Mahony method

// WIFI RELATED
char ssid[32];
char password[64];
WiFiClient wifiClient;

// MQTT RELATED
String clientName;
String Obj1;
String Verb1;
String Subj1;
char* topic = "target";

PubSubClient client(wifiClient);

//LED RELATED
Ticker flipper;
unsigned long LED_period = 0;
unsigned long LED_on = 0;
unsigned long LED_off = 0;
unsigned long LED_period_last;
unsigned long LED_on_last;
unsigned long LED_off_last;


//CLOCK RELATED
unsigned long phase = 0;
int MODE = 0;
uint32_t Now = 0;                        // used to calculate integration interval
uint32_t time_bias = 0;                  // time bias synced with the system

//JOYPAD RELATED
int joypad_mode = 0;
uint8_t keyoutput = 0;
float x_coor, y_coor;

void setup()
{
  Serial.begin(115200); Serial.print("\n");
  //Serial.setDebugOutput(true);
  INITfilesystem();
  InitHostName();
  INITwifi();
  if (!FactoryMode) {
    INITmpu9250();
    INITled();
    INITmqtt();
    joypad_init();
  }
}

void loop()
{
  checkFLASHbutton();
  if (!FactoryMode) {
    MPU9250Measure();
    MQTT_LOOP_MEASURE();
    if (delt_t > 50) {
      //  变大掉帧
      if (SerialDebug) {
        Serial.print("ax = "); Serial.print((int)1000 * ax);
        Serial.print(" ay = "); Serial.print((int)1000 * ay);
        Serial.print(" az = "); Serial.print((int)1000 * az); Serial.println(" mg");
        Serial.print("gx = "); Serial.print( gx, 2);
        Serial.print(" gy = "); Serial.print( gy, 2);
        Serial.print(" gz = "); Serial.print( gz, 2); Serial.println(" deg/s");
        Serial.print("mx = "); Serial.print( (int)mx );
        Serial.print(" my = "); Serial.print( (int)my );
        Serial.print(" mz = "); Serial.print( (int)mz ); Serial.println(" mG");

        Serial.print("q0 = "); Serial.print(q[0]);
        Serial.print(" qx = "); Serial.print(q[1]);
        Serial.print(" qy = "); Serial.print(q[2]);
        Serial.print(" qz = "); Serial.println(q[3]);
        tempCount = readTempData();  // Read the gyro adc values
        temperature = ((float) tempCount) / 333.87 + 21.0; // Gyro chip temperature in degrees Centigrade
        // Print temperature in degrees Centigrade
        Serial.print("Gyro temperature is ");  Serial.print(temperature, 1);  Serial.println(" degrees C"); // Print T values to tenths of s degree C
      }

      // Define output variables from updated quaternion---these are Tait-Bryan angles, commonly used in aircraft orientation.
      // In this coordinate system, the positive z-axis is down toward Earth.
      // Yaw is the angle between Sensor x-axis and Earth magnetic North (or true North if corrected for local declination, looking down on the sensor positive yaw is counterclockwise.
      // Pitch is angle between sensor x-axis and Earth ground plane, toward the Earth is positive, up toward the sky is negative.
      // Roll is angle between sensor y-axis and Earth ground plane, y-axis up is positive roll.
      // These arise from the definition of the homogeneous rotation matrix constructed from quaternions.
      // Tait-Bryan angles as well as Euler angles are non-commutative; that is, the get the correct orientation the rotations must be
      // applied in the correct order which for this configuration is yaw, pitch, and then roll.
      // For more see http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles which has additional links.
      //Software AHRS:
      //   yaw   = atan2f(2.0f * (q[1] * q[2] + q[0] * q[3]), q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3]);
      //   pitch = -asinf(2.0f * (q[1] * q[3] - q[0] * q[2]));
      //   roll  = atan2f(2.0f * (q[0] * q[1] + q[2] * q[3]), q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);
      //   pitch *= 180.0f / PI;
      //   yaw   *= 180.0f / PI;
      //   yaw   += 13.8f; // Declination at Danville, California is 13 degrees 48 minutes and 47 seconds on 2014-04-04
      //   if(yaw < 0) yaw   += 360.0f; // Ensure yaw stays between 0 and 360
      //   roll  *= 180.0f / PI;
      a12 =   2.0f * (q[1] * q[2] + q[0] * q[3]);
      a22 =   q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3];
      a31 =   2.0f * (q[0] * q[1] + q[2] * q[3]);
      a32 =   2.0f * (q[1] * q[3] - q[0] * q[2]);
      a33 =   q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3];
      pitch = -asinf(a32);
      roll  = atan2f(a31, a33);
      yaw   = atan2f(a12, a22);


      pitch *= 180.0f / PI;
      yaw   *= 180.0f / PI;
      yaw   += 13.8f; // Declination at Danville, California is 13 degrees 48 minutes and 47 seconds on 2014-04-04
      if (yaw < 0) yaw   += 360.0f; // Ensure yaw stays between 0 and 360
      roll  *= 180.0f / PI;
      lin_ax = ax + a31;
      lin_ay = ay + a32;
      lin_az = az - a33;
      if (SerialDebug) {
        Serial.print("Yaw, Pitch, Roll: ");
        Serial.print(yaw, 2);
        Serial.print(", ");
        Serial.print(pitch, 2);
        Serial.print(", ");
        Serial.println(roll, 2);

        Serial.print("Grav_x, Grav_y, Grav_z: ");
        Serial.print(-a31 * 1000, 2);
        Serial.print(", ");
        Serial.print(-a32 * 1000, 2);
        Serial.print(", ");
        Serial.print(a33 * 1000, 2);  Serial.println(" mg");
        Serial.print("Lin_ax, Lin_ay, Lin_az: ");
        Serial.print(lin_ax * 1000, 2);
        Serial.print(", ");
        Serial.print(lin_ay * 1000, 2);
        Serial.print(", ");
        Serial.print(lin_az * 1000, 2);  Serial.println(" mg");

        Serial.print("rate = "); Serial.print((float)sumCount / sum, 2); Serial.println(" Hz");
      }
      MQTT_SEND_MEASURE();
      count = millis();
      if (sumCount > 10000) {
        sumCount = 0;
        sum = 0;
      }
    }
    MQTT_LOOP_COMMAND();
    JOYPAD_LOOP_READ();
  }
}

