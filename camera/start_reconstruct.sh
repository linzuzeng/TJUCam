#!/bin/bash
echo starting reconstruct...
if [ -n "$(/home/alarm/findmqtt.sh)" ]; then 
  cd /home/alarm/rpifiles/calib
  /home/alarm/rpifiles/calib/calib3 --host "$(/home/alarm/findmqtt.sh)"
else
	echo [ERROR] Center not running, quitting...
fi