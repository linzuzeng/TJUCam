String mqtt_server;
int mqtt_port;
String MQTTUSER;
String MQTTPWD ;
void callback(char* topic, byte* payload, unsigned int plength) {
 // Serial.print("Message arrived [");
 // Serial.print(topic);
  //Serial.print("] ");
  if (String(topic)=="control"){
      StreamString ss;
      ss.write((unsigned char*)payload,plength);
      Verb1=ss.readStringUntil(' ');
      Obj1=ss.readStringUntil(' ');
      Subj1=ss.readString();
      String combined = Verb1+" "+Obj1+" "+Subj1;
     // Serial.println(combined);
      //echo 
      if(Obj1==clientName){
         client.publish("ack", (char*)combined.c_str());
      }
      MQTT_FAST_COMMAND();
  }
  if (String(topic)=="hello"){
      StreamString ss;
      ss.write((unsigned char*)payload,plength);
      if (ss.readString()=="areyouok")
         MQTT_HEART_BEAT();
  }
}

void read_mqttconfig()
{
  if(SPIFFS.exists("/mqtt.txt")){
      File logF = SPIFFS.open("/mqtt.txt", "r");
      mqtt_server=logF.readStringUntil(' ');
      mqtt_port=logF.parseInt();
      logF.readStringUntil(' ');
      MQTTUSER=logF.readStringUntil(' ');
      MQTTPWD=logF.readString();
  }
  else{ 
    Serial.println("Please set mqtt config!") ;
    FORCE_FactoryMode();
  }
}


void INITmqtt(){
  read_mqttconfig();
  if (!MDNS.begin(clientName.c_str())) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("Sending mDNS query");
  int n = MDNS.queryService("mqtt", "tcp"); // Send out query for esp tcp services
  Serial.println("mDNS query done");
  if (n != 1) {
    Serial.println("No mqtt services found, using default in factory mode.");
  }
  else {
      Serial.print("overriding MQTT config as: ");
      Serial.print(MDNS.hostname(0));
      Serial.print(" (");
      Serial.print(MDNS.IP(0));
      mqtt_server=MDNS.IP(0).toString();
      Serial.print(":");
      Serial.print(MDNS.port(0));
      mqtt_port=MDNS.port(0);
      Serial.println(")");
    }
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.print(mqtt_port);
  Serial.print(" as ");
  Serial.println(clientName);
  Serial.print("MQTTUSER");
  Serial.println(MQTTUSER);
  Serial.print("MQTTPWD");
  Serial.println(MQTTPWD);
  client.setServer((char*) mqtt_server.c_str(), mqtt_port);
  client.setCallback(callback);
  if (client.connect((char*) clientName.c_str(),(char*) MQTTUSER.c_str(),(char*) MQTTPWD.c_str())) {
    Serial.println("Connected to MQTT broker");
    Serial.print("Measurement topic is: ");
    Serial.println(topic);
    client.subscribe("control");
    client.subscribe("hello");
    MQTT_HEART_BEAT();
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will fuck you and try again...");
    //abort();
  }
}
void MQTT_HEART_BEAT(){
    String greeting ="TARGET "+clientName;
    if (client.publish("hello", greeting.c_str())) {
      Serial.println("Publish hello ok.");
    }
    else {
      Serial.println("Publish hello failed");
      client.disconnect();
    }
    MQTT_LOOP_MEASURE();
}
void MQTT_LOOP_COMMAND(){
 if(Verb1=="syncmode")
    { 
     Serial.println(Verb1); 
     Serial.println(Obj1); 
     Serial.print ("I AM ");
     Serial.println(clientName); 
     unsigned long LED_last_period=LED_period;
     unsigned long LED_last_on = LED_on ;
     unsigned long LED_last_off=LED_off;
     LED_period=0;
     LED_on=0;
     LED_off=0;
     digitalWrite(LED_PIN, LOW);
     if(Obj1!=clientName)
     {
        Serial.println("I will do nothing"); 
        delay(3000);
     }else{
        delay(1000);
        unsigned int Led_flip_time;
        String Led_init_payload;
        digitalWrite(LED_PIN, HIGH);
        Led_flip_time=micros();
        Led_init_payload ="TARGET ";
        Led_init_payload +=clientName;
        Led_init_payload +=" ";
        Led_init_payload +=Led_flip_time;
        Serial.print(Led_init_payload); 
        client.publish("sync", Led_init_payload.c_str());
        MQTT_LOOP_MEASURE();
        delay(1000);
        digitalWrite(LED_PIN, LOW);   
        delay(1000);
      } 
     LED_period=LED_last_period;
     LED_on = LED_last_on ;
     LED_off=LED_last_off;
     Verb1="syncdone";
     flipper.once_ms(5,flip);
  }
}


void MQTT_FAST_COMMAND()
{
 if(Obj1==clientName){
    Serial.println(Verb1); 
    Serial.println(Obj1); 
    Serial.println(Subj1); 
    if(Verb1=="settime_t")
       time_bias+=Subj1.toInt();
    if(Verb1=="flashprid")
       LED_period=Subj1.toInt();
    if(Verb1=="flashonmod")
       LED_on=Subj1.toInt();
    if(Verb1=="flashoffmod")
       LED_off=Subj1.toInt();
   /* if(Verb1=="light"){ 
          if (Subj1.toInt()==0){
            LED_period_last=LED_period;
            LED_on_last=LED_on;
            LED_off_last=LED_off;
            LED_period=100;
            LED_on=0;
            LED_off=0;
          }else{
              LED_period=LED_period_last;
              LED_on=LED_on_last;
              LED_off=LED_off_last;
          }
    }*/
    flipper.once_ms(5,flip);
 }
}
void MQTT_LOOP_MEASURE(){
  if (client.connected()){
    client.loop();
  }else
  {
      Serial.println("MQTT not connected! ");
      INITmqtt();
  }
}
void MQTT_SEND_MEASURE(){
  String payload = (String)(micros()+time_bias);
  payload += " ";
  payload += clientName;
  payload += " ";
  payload += q[0];
  payload += " ";
  payload += q[1];
  payload += " ";
  payload += q[2];
  payload += " ";
  payload += q[3];
  payload += " ";
  payload += ax;
  payload += " ";
  payload += ay;
  payload += " ";
  payload += az;
  payload += " ";
  payload += keyoutput;
  payload += " ";
  payload += x_coor;
  payload += " ";
  payload += y_coor;
  if (client.publish(topic, (char*) payload.c_str())) {
        Serial.println(payload);
        //Serial.print("rate = "); Serial.print((float)sumCount / sum, 2); Serial.println(" Hz");
  }
  else {
    Serial.println("Publish failed");
    client.disconnect();
  }
}

