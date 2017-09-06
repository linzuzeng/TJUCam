#!/bin/bash
echo starting camera...
if [ -n "$(/home/alarm/findmqtt.sh)" ]; then 
    cd /home/alarm/rpifiles/calib
  /home/alarm/rpifiles/calib/calib1 --host "$(/home/alarm/findmqtt.sh)"
else
	echo [ERROR] Center not running, quitting...
fi