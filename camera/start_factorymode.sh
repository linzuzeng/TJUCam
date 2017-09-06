#!/bin/bash
echo starting factorymode...
/usr/bin/avahi-publish -s cam _http._tcp 80 &
/home/alarm/rpifiles/factorymode/factorymode