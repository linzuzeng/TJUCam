#define LED_PIN 2
#define LED_ERROR_LIMIT 2000

void INITled(){
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  flipper.once_ms(5,flip);
}
void flip()
{
  if (LED_period==0){
    digitalWrite(LED_PIN, LOW);
    return;
  }
  long settimer=0;
  phase=(micros()+time_bias-LED_on)%LED_period;
  long thre=LED_off-LED_on;
  if (thre<0) thre+=LED_period;
  digitalWrite(LED_PIN, phase<thre); //de facto

  //minor adjust
  if(phase<thre){
       if ((thre-phase>0)&&(thre-phase<=LED_ERROR_LIMIT)){
            delayMicroseconds(thre-phase);
            digitalWrite(LED_PIN, LOW);
       }
  }else{
       if ((LED_period-phase>0)&&(LED_period-phase<=LED_ERROR_LIMIT)){
           delayMicroseconds(LED_period-phase);
           digitalWrite(LED_PIN, HIGH);
        }
  }
  //major
  phase=(micros()+time_bias-LED_on)%LED_period;
  if(phase<thre-LED_ERROR_LIMIT)
      settimer=(thre-phase)/1000; 
  else
      settimer=(LED_period-phase)/1000;
  flipper.once_ms(settimer,flip);
}

