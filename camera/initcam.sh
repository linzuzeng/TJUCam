/usr/bin/rmmod bcm2835-v4l2
/usr/bin/modprobe bcm2835-v4l2
/usr/bin/chmod 777 /dev/video0
/usr/bin/killall gst-launch-1.0
/usr/bin/killall CamPaho
