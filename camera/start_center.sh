#!/bin/bash
echo starting center...
sleep 5
if [ -z "$(/home/alarm/findmqtt.sh)" ]; then 
   /usr/bin/avahi-publish -s center _mqtt._tcp 1883 &
   /usr/bin/killall CenterPaho
   /home/alarm/rpifiles/CenterPaho -host "$(/home/alarm/findmqtt.sh)"
else
   echo [ERROR] Center already running at "$(/home/alarm/findmqtt.sh)" , quitting...
fi 
