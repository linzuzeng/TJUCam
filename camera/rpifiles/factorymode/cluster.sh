#!/bin/sh
for host in $(/usr/bin/avahi-browse -rptf _http._tcp | /home/alarm/rpifiles/factorymode/listnodes.py); do
	echo  alarm@"$host" "$1 $2 $3 $4 $5"
	ssh -o StrictHostKeyChecking=no alarm@"$host" "$1 $2 $3 $4 $5"
done
