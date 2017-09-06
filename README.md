# TJUCam
SteamVR Compatible Motion Controller with ESP8266 and Raspberry Pi

This project allows you to quciky setup a VIVE-like motion controller with opensource hardware and software. 

Inspired by [[1]](https://github.com/cbuchner1/driver_leap) and [[2]](http://www.cs.bu.edu/fac/betke/jeb/).

## Demonstration
[![Demonstration video on youtube](https://img.youtube.com/vi/POsGebR3wQ0/0.jpg)](https://www.youtube.com/watch?v=POsGebR3wQ0)

# Features
- SteamVR-Compatible Windows Driver (with OpenVR SDK)
- MQTT protocol for underlying messaging system
- Avahi for automatic node discovery
- Automatic calibration for MPU9250 on Target Node
- Web-based control & configuration (with Golang on Raspbery Pi and HTTPServer on ESP8266)
- Kalman Filter fusing data form MVG and inertia sensor 

# Setup
## Hardware
-	 Camera Node
     + Raspberry Pi with built-in Wi-Fi or ethernet connected
     + PI NOIR CAMERA V2 (Sony IMX219 8-megapixel sensor)
     + IR-pass filter (optional)
-	Target Node
     + ESP8266 (NodeMCU v2 board)
     + IR-Leds or markers for tracking
     + MPU9250 (GY-9255 board)
     + Joystick for simulating touchpad behavior (3D Joystick Repair Parts for XBox 360)
     + Buttons for triggers
     + Battery
     + Custom-built PCB holding everything above
- Central Node
     + PC with good enough video card for VR rendering
     + VR headset or cardboard (optional)
## Software Dependency
-	 Camera Node
     +  Arch Linux for ARM
     +  OpenCV 3.1
     +  paho.mqtt.c
     +  Avahi
     +  base-devel, gcc (build)
-	Target Node
     +  Arduino for ESP8266
- Central Node (Windows-side)
     +  SteamVR for game support
     +  OpenVR SDK
     +  Visual Studio (build)
- Central Node (Linux-side)
     +  Arch Linux environment with VM or WSL 
     (or you can build on any one of the existing Camera Nodes if you hate VMs)
     +  OpenCV 3.1 with opencv_contrib (for MVG features)
     +  any kind of MQTT broker (mosquito, etc.)
     +  paho.mqtt.c
     +  Avahi
     +  base-devel, gcc and golang (build)
     
# Note
This project was built for a Hackathon-style event held in Jun, 2017 at TJU. 
Currently, it is able to demonstrate basic functionality, but the design and code need more polish due to limited time.

Contributions are welcome!
