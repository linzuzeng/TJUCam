#!/bin/bash
echo starting camera...
if [ -n "$(/home/alarm/findmqtt.sh)" ]; then 
  sleep 1
  /home/alarm/initcam.sh
  /usr/bin/v4l2-ctl -c auto_exposure=1
  /usr/bin/v4l2-ctl -c exposure_time_absolute=20
  sleep 1
  /home/alarm/rpifiles/CamPaho --host  "$(/home/alarm/findmqtt.sh)"
else
	echo [ERROR] Center not running, quitting...
fi
