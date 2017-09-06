#!/bin/bash
mqtt="$( /usr/bin/avahi-browse -rptf _mqtt._tcp  | /usr/bin/tail -1 | /usr/bin/awk -F';' '{ print $8 }' )"
echo $mqtt