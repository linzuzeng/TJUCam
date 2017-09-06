#include "FSEditor.h"

#define BUILTIN_LED 2
#define BUILTIN_BUTTON_FLASH 0

int FactoryModeButtonCounter=0;
AsyncWebServer server(80);
DNSServer dnsServer;
String InitHostName()
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String result="";
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  clientName = "target-"+result;
  return result;
}
void FORCE_FactoryMode(){
        File factorymodeF = SPIFFS.open("/factorymode.txt", "w");
      if (factorymodeF) factorymodeF.print(1);
      factorymodeF.close();
      abort();      
}
void checkFLASHbutton(){
  if(!FactoryMode){
    if(!digitalRead(BUILTIN_BUTTON_FLASH)) FactoryModeButtonCounter++; else FactoryModeButtonCounter--;
    if (FactoryModeButtonCounter<0) FactoryModeButtonCounter=0;
    //if (FactoryModeButtonCounter>0) digitalWrite(BUILTIN_LED,!(FactoryModeButtonCounter%10));
    if (FactoryModeButtonCounter>5){
      FORCE_FactoryMode();
    }
  }else{
    dnsServer.processNextRequest();
    if (micros()%100000==1) Serial.print('.');
  }
}
void CaptiveHandler (AsyncWebServerRequest *request){
    request->send(200, "text/html","<!DOCTYPE html><html><head><title>Redirecting...</title><meta http-equiv='refresh' content='1;url=/'></head><body></body></html>");
}
void INITwifi(){
  //pinMode(BUILTIN_LED, OUTPUT);
  //digitalWrite(BUILTIN_LED,HIGH);
  if(SPIFFS.exists("/wifi.txt")&&(!SPIFFS.exists("/factorymode.txt"))){
      File wifiF = SPIFFS.open("/wifi.txt", "r");
      strcpy(ssid,(wifiF.readStringUntil(' ')).c_str());
      strcpy(password,(wifiF.readString()).c_str());
      wifiF.close(); 
      WiFi.softAPdisconnect(true);
      Serial.print("Connecting to " );Serial.println(ssid);
      Serial.print("Password " );Serial.println(password);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      WiFi.setAutoReconnect(true);  
      //digitalWrite(BUILTIN_LED,LOW);
      while (WiFi.status() != WL_CONNECTED) {
        for(int i=1;i<25;i++){
          delay(20);
          checkFLASHbutton();
        }
        Serial.print(".");
      }
      Serial.println("");
      Serial.print("Connected! IP address: ");
      Serial.println(WiFi.localIP());
  }else{
    if (SPIFFS.exists("/factorymode.txt")) SPIFFS.remove("/factorymode.txt");
    WiFi.mode(WIFI_AP);
    IPAddress apIP(192, 168, 1, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(clientName.c_str());
    server.addHandler(new FSEditor());
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");
    server.on("/generate_204", HTTP_GET,CaptiveHandler);
    server.on("/fwlink", HTTP_GET,CaptiveHandler);
    server.onNotFound([](AsyncWebServerRequest *request){
      request->send(404, "", "404 - file not found. ");
    });
    server.begin();
    Serial.println("HTTP server started");
    dnsServer.start(53, "*", apIP);
    Serial.println("DNS started");
    FactoryMode=true;
  }
}

