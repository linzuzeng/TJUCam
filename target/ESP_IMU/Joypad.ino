#define joypad_max 6
#define joypad_thr 3
//**joypad control Parameters set**//
#define scanPin1 5     // the number of the scan & joystick power pin
#define scanPin2  4     // the number of the scan & joystick power pin
#define buttonPin1 13     // the number of the butten pin
#define buttonPin2 16      // the number of the butten pin
#define trigger  15      // the number of the joystickButten pin
#define joystickPin  A0      // the number of the analog pin

#define JXH 5
#define JXL 13
#define JYH 4
#define JYL 16

////////////////////////////define change

int key1count=0;
int key2count=0;
int key3count=0;
int key4count=0;
int triggercount=0;


int scanNumb=0;
float y_range_up,y_range_down;
float x_range_left,x_range_right;

float y_up_inner=0.0f,y_down_inner=1000.0f;
float x_left_inner=1000.0f,x_right_inner=0.0f;
float y_up_outer=0.0f,y_down_outer=1000.0f;
float x_left_outer=1000.0f,x_right_outer=0.0f;

void joypad_init()
{
  if(SPIFFS.exists("/joystick_mode.txt")){
      File logF = SPIFFS.open("/joystick_mode.txt", "r");
      joypad_mode = logF.readStringUntil(' ').toInt();
      }
  else{ 
    joypad_mode=0;  
    Serial.println("Please set the joypad mode!");
    FORCE_FactoryMode();
    }
  if(joypad_mode==1)
  {
      pinMode(scanPin1, OUTPUT);   //digital
      pinMode(scanPin2, OUTPUT);   //digital
    
      pinMode(buttonPin1, INPUT);  //digital
      pinMode(buttonPin2, INPUT);  //digital
      switch_next();
  }
  if(joypad_mode==2)
  {
        switch_next();
        joystick_cal();
        y_range_up = y_up_outer - y_up_inner ;
        y_range_down = y_down_inner - y_down_outer;
        x_range_left = x_left_inner - x_left_outer;
        x_range_right = x_right_outer - x_right_inner;
  }
}
float max(float a,float b){
  if (a>b) return a;else return b;
}
float min(float a,float b){
  if (a<b) return a;else return b;
}

void switch_next()
{
   if(joypad_mode==2)
  {
     if(scanNumb==0){
        pinMode(JYH, OUTPUT);
        pinMode(JYL, OUTPUT); 
        pinMode(JXH, INPUT) ;
        pinMode(JXL, INPUT);
        digitalWrite(JYH,HIGH);
        digitalWrite(JYL,LOW);
        scanNumb=1;
      }else{
        pinMode(JXH, OUTPUT);
        pinMode(JXL, OUTPUT); 
        pinMode(JYH, INPUT) ;
        pinMode(JYL, INPUT);
        digitalWrite(JXH,HIGH);
        digitalWrite(JXL,LOW);
        scanNumb=0;
      }
  }
  if (joypad_mode==1)
  {
      if(scanNumb==0)
      {
        //digitalWrite(scanPin2,LOW); 
        pinMode(scanPin2, INPUT) ;
        pinMode(scanPin1, OUTPUT);
        digitalWrite(scanPin1,HIGH); 
        scanNumb=1;
      }else{
        //digitalWrite(scanPin1,LOW); 
        pinMode(scanPin1, INPUT) ;
        pinMode(scanPin2, OUTPUT);
        digitalWrite(scanPin2,HIGH);      
        scanNumb=0;
      }
  }
}
void joypad_max_min_cal(float &y_up_inner,float &y_down_inner,float &x_right_inner,float &x_left_inner,int delaytimes){
    
  float joystick_analog  =analogRead(joystickPin);
  for(int i=0;i<=2000;i++)
  {
        if(scanNumb==0){
          y_up_inner=max(joystick_analog,y_up_inner);
          y_down_inner=min(joystick_analog,y_down_inner);
        }else{
          x_right_inner=max(joystick_analog,x_right_inner);
          x_left_inner=min(joystick_analog,x_left_inner);
        } 
        switch_next();
        delay(delaytimes);
  }
}


void joystick_cal()
{
  if(SPIFFS.exists("/joystick_cal.txt")){
      File logF = SPIFFS.open("/joystick_cal.txt", "r");
      y_up_inner = logF.readStringUntil(' ').toFloat();
      y_down_inner = logF.readStringUntil(' ').toFloat();
      x_left_inner = logF.readStringUntil(' ').toFloat();
      x_right_inner = logF.readStringUntil(' ').toFloat();
      y_up_outer = logF.readStringUntil(' ').toFloat();
      y_down_outer = logF.readStringUntil(' ').toFloat();
      x_left_outer = logF.readStringUntil(' ').toFloat();
      x_right_outer = logF.readStringUntil(' ').toFloat();
      Serial.println("read the joypadstick cal data!");
      delay(300);
    }else{
        Serial.println("joystrik static calibration start!");
        delay(3000);
        joypad_max_min_cal(y_up_inner,y_down_inner,x_right_inner,x_left_inner,5);
        Serial.println("joystrik dynamic calibration start!");
      
        delay(1000);
        joypad_max_min_cal(y_up_outer,y_down_outer,x_right_outer,x_left_outer,5);

        File logF = SPIFFS.open("/joystick_cal.txt", "w");
        if (logF) { 
          logF.print(y_up_inner);logF.print(" ");
          logF.print(y_down_inner);logF.print(" ");
          logF.print(x_left_inner);logF.print(" ");
          logF.print(x_right_inner);logF.print(" ");
          logF.print(y_up_outer);logF.print(" ");
          logF.print(y_down_outer);logF.print(" ");     
          logF.print(x_left_outer);logF.print(" ");
          logF.print(x_right_outer);logF.print(" ");    
          logF.close();
        }else{
          ERROR("file open failed.");
        } 
   }
}

void joystick_trans()
{
  float joystick_analog  = analogRead(joystickPin);
  Serial.print("analog ");
  Serial.print(scanNumb);
  Serial.print(" : ");
  Serial.println(joystick_analog);
  if(scanNumb==0)
  {
    if(joystick_analog<=x_left_inner)
      x_coor=(joystick_analog-x_left_inner)/x_range_left;
      else
      {
        if(joystick_analog>=x_right_inner)
        x_coor=(joystick_analog-x_right_inner)/x_range_right;
        else
        x_coor=0;
        }
    }
    else
    {
      if(joystick_analog<=y_down_inner)
      y_coor=(joystick_analog-y_down_inner)/y_range_down;
      else
      {
        if(joystick_analog>=y_up_inner)
        y_coor=(joystick_analog-y_up_inner)/y_range_up;
        else
        y_coor=0;
        }
      }
   switch_next();
}

void key_control()
{
  if (scanNumb==0){
     if(digitalRead(buttonPin1)==1)key1count++;else key1count--; 
     if(digitalRead(buttonPin2)==1)key2count++;else key2count--; 
   }else{
     if(digitalRead(buttonPin1)==1) key3count++;else key3count--; 
     if(digitalRead(buttonPin2)==1) key4count++;else key4count--; 
     if(digitalRead(trigger)==1)triggercount++;else triggercount--; 
   }
   if(key1count>=joypad_max) key1count=joypad_max;
   if(key1count<=0) key1count=0;
   if(key2count>=joypad_max) key2count=joypad_max;
   if(key2count<=0) key2count=0;
   if(key3count>=joypad_max) key3count=joypad_max;
   if(key3count<=0) key3count=0;
   if(key4count>=joypad_max) key4count=joypad_max;
   if(key4count<=0) key4count=0;
   if(triggercount>=joypad_max) triggercount=joypad_max;
   if(triggercount<=0) triggercount=0;
    keyoutput=0;           //determine keyputput number
    keyoutput+=0b1*(key1count>=joypad_thr);            
    keyoutput+=0b10*(key2count>=joypad_thr);
    keyoutput+=0b100*(key3count>=joypad_thr);
    keyoutput+=0b1000*(key4count>=joypad_thr);
    keyoutput+=0b10000*(triggercount>=joypad_thr);

    switch_next();
  }

void JOYPAD_LOOP_READ()
{
   if(joypad_mode==1)
      key_control();
   else{
      if(joypad_mode==2) joystick_trans(); 
      else Serial.println("Please set the joypad mode!") ;
   }
}

