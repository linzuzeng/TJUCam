void INITfilesystem(){
  // initialize the SPIFFS
  if (!SPIFFS.begin()) {
     ERROR("Fs init faild, will reboot and try again.");
     abort();
  }
  INFO("Fs init.");
}
void magcalMPU9250(float * dest1, float * dest2) 
{
  if(SPIFFS.exists("/magcalMPU9250.txt")){
     File logF = SPIFFS.open("/magcalMPU9250.txt", "r");
          dest1[0] = logF.readStringUntil(' ').toFloat();
          dest1[1] = logF.readStringUntil(' ').toFloat();
          dest1[2] = logF.readStringUntil(' ').toFloat();
          dest2[0] = logF.readStringUntil(' ').toFloat();
          dest2[1] = logF.readStringUntil(' ').toFloat();
          dest2[2] = logF.readStringUntil(' ').toFloat();
    }else{
      uint16_t ii = 0, sample_count = 0;
      int32_t mag_bias[3] = {0, 0, 0}, mag_scale[3] = {0, 0, 0};
      int16_t mag_max[3] = {-32767, -32767, -32767}, mag_min[3] = {32767, 32767, 32767}, mag_temp[3] = {0, 0, 0};
    
      INFO("Mag Calibration: Wave device in a figure eight until done!");
      delay(4000);
    
      // shoot for ~fifteen seconds of mag data
      if(Mmode == 0x02) sample_count = 128;  // at 8 Hz ODR, new mag data is available every 125 ms
      if(Mmode == 0x06) sample_count = 1500;  // at 100 Hz ODR, new mag data is available every 10 ms
      for(ii = 0; ii < sample_count; ii++) {
          readMagData(mag_temp);  // Read the mag data   
          for (int jj = 0; jj < 3; jj++) {
            if(mag_temp[jj] > mag_max[jj]) mag_max[jj] = mag_temp[jj];
            if(mag_temp[jj] < mag_min[jj]) mag_min[jj] = mag_temp[jj];
          }
          if(Mmode == 0x02) delay(135);  // at 8 Hz ODR, new mag data is available every 125 ms
          if(Mmode == 0x06) delay(12);  // at 100 Hz ODR, new mag data is available every 10 ms
      }
      if(SerialDebug) {
          Serial.println("mag x min/max:"); Serial.println(mag_max[0]); Serial.println(mag_min[0]);
          Serial.println("mag y min/max:"); Serial.println(mag_max[1]); Serial.println(mag_min[1]);
          Serial.println("mag z min/max:"); Serial.println(mag_max[2]); Serial.println(mag_min[2]);
      }
      // Get hard iron correction
      mag_bias[0]  = (mag_max[0] + mag_min[0])/2;  // get average x mag bias in counts
      mag_bias[1]  = (mag_max[1] + mag_min[1])/2;  // get average y mag bias in counts
      mag_bias[2]  = (mag_max[2] + mag_min[2])/2;  // get average z mag bias in counts//锛侊紒锛�
      
      dest1[0] = (float) mag_bias[0]*mRes*magCalibration[0];  // save mag biases in G for main program
      dest1[1] = (float) mag_bias[1]*mRes*magCalibration[1];   
      dest1[2] = (float) mag_bias[2]*mRes*magCalibration[2];  
         
      // Get soft iron correction estimate
      mag_scale[0]  = (mag_max[0] - mag_min[0])/2;  // get average x axis max chord length in counts
      mag_scale[1]  = (mag_max[1] - mag_min[1])/2;  // get average y axis max chord length in counts
      mag_scale[2]  = (mag_max[2] - mag_min[2])/2;  // get average z axis max chord length in counts//锛侊紒锛�
  
      float avg_rad = mag_scale[0] + mag_scale[1] + mag_scale[2];
      avg_rad /= 3.0;
  
      dest2[0] = avg_rad/((float)mag_scale[0]);
      dest2[1] = avg_rad/((float)mag_scale[1]);
      dest2[2] = avg_rad/((float)mag_scale[2]);
      File logF = SPIFFS.open("/magcalMPU9250.txt", "w");
        if (logF) { 
          logF.print(dest1[0]);logF.print(" ");
          logF.print(dest1[1]);logF.print(" ");
          logF.print(dest1[2]);logF.print(" ");
          logF.print(dest2[0]);logF.print(" ");
          logF.print(dest2[1]);logF.print(" ");
          logF.print(dest2[2]);logF.print(" ");      
          logF.close();
        }else{
          ERROR("file open failed.");
        }
    }
   INFO("Mag Calibration done!");
}



