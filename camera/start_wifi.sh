#!/bin/bash
while true; do
cp /home/alarm/wpa_supplicant-wlan0.conf /etc/wpa_supplicant/wpa_supplicant-wlan0.conf 
sleep 10
router=`/usr/bin/ifconfig wlan0 | grep 'inet ' | awk '{ print $2}'`
/usr/bin/ping -q -c1 $router > /dev/null
if [ $? -eq  0 ]

then
  echo $router
  #echo $router >> /tmp/ip.txt
else
  echo "Network down, fixing..."
  /usr/bin/systemctl restart wpa_supplicant@wlan0
  echo "wlan0 reconnected at `date`"
fi
done
