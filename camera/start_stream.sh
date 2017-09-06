#!/bin/bash
sleep 1
/home/alarm/initcam.sh
/usr/bin/v4l2-ctl -c auto_exposure=0
/usr/bin/v4l2-ctl -c exposure_time_absolute=20
sleep 1
/usr/bin/gst-launch-1.0  -e v4l2src device=/dev/video0 ! video/x-h264, width=820, height=616, framerate=20/1  ! h264parse ! rtph264pay config-interval=1 pt=96 ! gdppay ! tcpserversink host="::0" port=5000
#/opt/vc/bin/raspivid -t 0 -ss 20 -hf -fps 40 -w 1640 -h 1232 -o - | gst-launch-1.0 fdsrc ! h264parse ! rtph264pay config-interval=1 pt=96 ! gdppay ! tcpserversink host=0.0.0.0 port=5000
